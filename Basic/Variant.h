//
// Created by miliao on 10/22/15.
//

#ifndef COMPILERFRONT_INVARIANT_H
#define COMPILERFRONT_INVARIANT_H

#include <algorithm>
#include <type_traits>


namespace VariantHelper {

    // check if a type exists in the variadic type list
    template <typename T, typename ...TS> struct TypeExist;

    template <typename T>
    struct TypeExist<T>
    {
        enum { exist = 0 };
        static constexpr std::size_t id = 0;
    };

    template <typename T, typename T2, typename ...TS>
    struct TypeExist<T, T2, TS...>
    {
        enum { exist = std::is_same<T, T2>::value || TypeExist<T, TS...>::exist };
        static constexpr std::size_t id = std::is_same<T, T2>::value? 1 : 1 + TypeExist<T, TS...>::id;
    };

    // extract type from the variadic type list at position k
    template <std::size_t k, typename ...TS> struct ExtractType;

    template <typename T, typename ...TS>
    struct ExtractType<0, T, TS...>
    {
    };

    template <typename T, typename ...TS>
    struct ExtractType<1, T, TS...>
    {
        using type = T;
    };

    template <std::size_t k, typename T, typename ...TS>
    struct ExtractType<k, T, TS...>
    {
        using type = typename ExtractType<k - 1, TS...>::type;
    };

    // get the max size of type in the type list
    template <typename ...TS> struct TypeMaxSize;

    template <>
    struct TypeMaxSize<>
    {
        static constexpr std::size_t value = 0;
    };

    template <typename T, typename ...TS>
    struct TypeMaxSize<T, TS...>
    {
        static constexpr std::size_t cur = alignof(T);
        static constexpr std::size_t next = TypeMaxSize<TS...>::value;
        static constexpr std::size_t value = cur > next? cur : next;
    };

    template <typename ...TS> struct TryRelease
    {
        static void Destroy(unsigned char* p, std::size_t id) {}
    };

    template <typename T, typename ...TS>
    struct TryRelease<T, TS...>
    {
        static void Destroy(unsigned char* p, std::size_t id)
        {
            if (id == 0) return;

            if (id > 1)
            {
                TryRelease<TS...>::Destroy(p, id - 1);
            }
            else
            {
                reinterpret_cast<T*>(p)->~T();
            }
        }
    };

    template <typename ...TS>
    struct CopyConstruct
    {
        static void Copy(const unsigned char*, unsigned char*, std::size_t) {}
    };

    template <typename T, typename ...TS>
    struct CopyConstruct<T, TS...>
    {
        static void Copy(const unsigned char* f, unsigned char* p, std::size_t id)
        {
            if (id == 0) return;

            if (id > 1)
            {
                CopyConstruct<TS...>::Copy(f, p, id - 1);
            }
            else
            {
                new(p) T(*reinterpret_cast<const T*>(f));
            }
        }
    };

    template <typename ...TS>
    struct MoveConstruct
    {
        static void Move(unsigned char* f, unsigned char* p, std::size_t id) {}
    };

    template <typename T, typename ...TS>
    struct MoveConstruct<T, TS...>
    {
        static void Move(unsigned char* f, unsigned char* p, std::size_t id)
        {
            if (id == 0) return;

            if (id > 1)
            {
                MoveConstruct<TS...>::Move(f, p, id - 1);
            }
            else
            {
                T* fp = reinterpret_cast<T*>(f);
                new(p) T(std::move(*fp));
            }
        }
    };

} // end namespace VariantHelper


// implementation of variant.

template <typename ...TS>
class Variant
{
public:
    Variant()
        : type_(0)
    {
    }

    template <typename T, typename D = typename std::enable_if<
            !std::is_same<typename std::remove_reference<T>::type, Variant<TS...>>::value>::type>
    Variant(T&& v)
        : type_(VariantHelper::TypeExist<T, TS...>::id)
    {
        // following is a little overkill maybe.
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for invariant.");

        new(data_) T(std::forward<T>(v));
    }

    Variant(const Variant<TS...>& other)
    {
        type_ = other.type_;
        if (other.type_ == 0) return;

        // TODO, check if other is copyable.
        VariantHelper::CopyConstruct<TS...>::Copy(other.data_, data_, type_);
    }

    Variant(Variant<TS...>&& other)
    {
        if (this == &other) return;

        // TODO, check if other is movable.

        Release();
        if (other.type_ == 0) return;

        type_ = other.type_;
        VariantHelper::MoveConstruct<TS...>::Move(other.data_, data_, type_);
    }

    template <typename T, typename D = typename std::enable_if<
            !std::is_same<typename std::remove_reference<T>::type, Variant<TS...>>::value>::type>
    Variant& operator=(T&& v)
    {
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for invariant.");

        Release();
        new(data_) T(std::forward<T>(v));
        type_ = static_cast<std::size_t>(VariantHelper::TypeExist<T, TS...>::id);

        return *this;
    }

    Variant& operator=(const Variant<TS...>& other)
    {
        if (this == &other) return *this;

        Release();
        type_ = other.type_;
        VariantHelper::CopyConstruct<TS...>::Copy(other.data_, data_, type_);

        return *this;
    }

    Variant& operator=(Variant<TS...>&& other)
    {
        if (this == &other) return *this;

        Release();
        type_ = other.type_;
        VariantHelper::MoveConstruct<TS...>::Move(other.data_, data_, type_);

        return *this;
    }

    ~Variant()
    {
        Release();
    }

    template <typename T, typename ...TS2>
    void EmplaceSet(TS2&& ...arg)
    {
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for invariant.");

        Release();

        type_ = VariantHelper::TypeExist<T, TS...>::id;
        new(data_) T(std::forward<TS2>(arg)...);
    }

    template <typename T>
    T* Get()
    {
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for invariant.");

        if (type_ != VariantHelper::TypeExist<T, TS...>::id) return NULL;

        return reinterpret_cast<T*>(data_);
    }

    template <typename T>
    T& GetRef()
    {
        T* p = Get<T>();
        if (!p) throw "invalid type for Invariant::Get<>()";

        return *p;
    }

    std::size_t GetType() const { return type_; }
    std::size_t GetSize() const { return VariantHelper::TypeMaxSize<TS...>::value; }

private:

    void Release()
    {
        VariantHelper::TryRelease<TS...>::Destroy(data_, type_);
        type_ = 0;
    }

private:
    std::size_t type_;
    unsigned char data_[VariantHelper::TypeMaxSize<TS...>::value];
};

#endif //COMPILERFRONT_INVARIANT_H
