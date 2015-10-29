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
        static constexpr std::size_t id =
                std::is_same<T, T2>::value? 1 : 1 + TypeExist<T, TS...>::id;
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


    template<bool f, class T1, class T2>
    struct SelectTypeIf
    {
        using type = T1;
    };

    template<class T1, class T2>
    struct SelectTypeIf<false, T1, T2>
    {
        using type = T2;
    };

    template<class T, class ...TS>
    struct SelectConvertible
    {
        enum { exist = false };
        using type = void;
    };

    template<class T, class T1, class ...TS>
    struct SelectConvertible<T, T1, TS...>
    {
        enum { exist = std::is_convertible<T, T1>::value || SelectConvertible<T, TS...>::exist };

        using type = typename SelectTypeIf<std::is_convertible<T, T1>::value,
                T1, typename SelectConvertible<T, TS...>::type>::type ;
    };

    template<class T, class ...TS>
    struct SelectType
    {
       using type = typename SelectTypeIf<TypeExist<T, TS...>::exist, T,
               typename SelectConvertible<T, TS...>::type>::type;
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
            !std::is_same<typename std::remove_reference<T>::type, Variant<TS...>>::value>::type,
            typename CT = typename VariantHelper::SelectType<T, TS...>::type>
    Variant(T&& v)
        : type_(VariantHelper::TypeExist<CT, TS...>::id)
    {
        // following is a little overkill maybe.
        static_assert(VariantHelper::SelectConvertible<T, TS...>::exist,
                      "invalid type for invariant.");

        new(data_) CT(std::forward<T>(v));
    }

    Variant(const Variant<TS...>& other)
    {
        // TODO, check if other is copyable.
        if (other.type_ == 0) return;

        type_ = other.type_;
        copy_[type_ - 1](other.data_, data_);
    }

    Variant(Variant<TS...>&& other)
    {
        // TODO, check if other is movable.
        if (other.type_ == 0) return;

        type_ = other.type_;
        move_[type_ - 1](other.data_, data_);
    }

    template <typename T, typename D = typename std::enable_if<
            !std::is_same<typename std::remove_reference<T>::type, Variant<TS...>>::value>::type,
            typename CT = typename VariantHelper::SelectType<T, TS...>::type>
    Variant& operator=(T&& v)
    {
        static_assert(VariantHelper::SelectConvertible<T, TS...>::exist,
                      "invalid type for invariant.");

        Release();
        new(data_) CT(std::forward<T>(v));
        type_ = static_cast<std::size_t>(VariantHelper::TypeExist<CT, TS...>::id);

        return *this;
    }

    Variant& operator=(const Variant<TS...>& other)
    {
        if (this == &other) return *this;

        Release();
        type_ = other.type_;
        if (!type_) return *this;

        copy_[type_ - 1](other.data_, data_);
        return *this;
    }

    Variant& operator=(Variant<TS...>&& other)
    {
        if (this == &other) return *this;

        Release();
        type_ = other.type_;
        if (!type_) return *this;

        move_[type_ - 1](other.data_, data_);
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
    T* Get() noexcept
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
        if (!type_) return;

        destroy_[type_ - 1](data_);
        type_ = 0;
    }

    template<class T>
    static void Destroy(unsigned char* data)
    {
        reinterpret_cast<T*>(data)->~T();
    }

    template<class T>
    static void CopyConstruct(const unsigned char* f, unsigned char* t)
    {
        new(t) T(*reinterpret_cast<const T*>(f));
    }

    template<class T>
    static void MoveConstruct(unsigned char* f, unsigned char* t)
    {
        T* fp = reinterpret_cast<T*>(f);
        new(t) T(std::move(*fp));
    }

    constexpr static size_t Alignment() { return VariantHelper::TypeMaxSize<TS...>::value; }

private:
    std::size_t type_ = 0;
    alignas(Alignment()) unsigned char data_[Alignment()];

    using destroy_func_t = void(*)(unsigned char*);
    constexpr static destroy_func_t destroy_[] = {Destroy<TS>...};

    using copy_func_t = void(*)(const unsigned char*, unsigned char*);
    constexpr static copy_func_t copy_[] = {CopyConstruct<TS>...};

    using move_func_t = void(*)(unsigned char*, unsigned char*);
    constexpr static move_func_t move_[] = {MoveConstruct<TS>...};
};

template<class ...TS>
constexpr typename Variant<TS...>::destroy_func_t Variant<TS...>::destroy_[];

template<class ...TS>
constexpr typename Variant<TS...>::copy_func_t Variant<TS...>::copy_[];

template<class ...TS>
constexpr typename Variant<TS...>::move_func_t Variant<TS...>::move_[];

#endif //COMPILERFRONT_INVARIANT_H
