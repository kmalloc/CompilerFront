//
// Created by miliao on 10/22/15.
//

#ifndef COMPILERFRONT_INVARIANT_H
#define COMPILERFRONT_INVARIANT_H

#include <algorithm>


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

template <typename ...TS>
class Variant
{
public:
    Variant()
        : type_(0)
    {
    }

    Variant(const Variant<TS...>& other)
    {
        type_ = other.type_;
        if (other.type_ == 0) return;

        // TODO, check if other is copyable.
        ConstructType<TS...>::Construct(data_, type_);
    }

    Variant(Variant<TS...>&& other)
    {
        if (this == &other) return;

        // TODO, check if other is movable.

        Release();
        if (other.type_ == 0) return;

        type_ = other.type_;
        MoveTypeObj<TS...>::Move(other.data_, data_, type_);
    }

    template <typename T>
    Variant(T&& v)
        : type_(TypeExist<T, TS...>::id)
    {
        static_assert(TypeExist<T, TS...>::exist, "invalid type for invariant.");

        new(data_) T(std::forward<T>(v));
    }

    template <typename T>
    Variant& operator=(T&& v)
    {
        static_assert(TypeExist<T, TS...>::exist, "invalid type for invariant.");

        Release();
        new(data_) T(std::forward<T>(v));
        type_ = static_cast<std::size_t>(TypeExist<T, TS...>::id);
    }

    Variant& operator=(const Variant<TS...>& other)
    {
        // TODO
    }

    Variant& operator=(Variant<TS...>&& other)
    {
        // TODO
    }

    ~Variant()
    {
        Release();
    }

    template <typename T, typename ...TS2>
    void EmplaceSet(TS2... arg)
    {
        // TODO
    }

    template <typename T>
    T* Get()
    {
        static_assert(TypeExist<T, TS...>::exist, "invalid type for invariant.");

        if (type_ != TypeExist<T, TS...>::id) return NULL;

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

private:

    template <typename ...TS2> struct TryRelease
    {
        static void Destroy(unsigned char* p, std::size_t id) {}
    };

    template <typename T2, typename ...TS2>
    struct TryRelease<T2, TS2...>
    {
        static void Destroy(unsigned char* p, std::size_t id)
        {
            if (id == 0) return;

            if (id > 1)
            {
                TryRelease<TS2...>::Destroy(p, id - 1);
            }
            else
            {
                reinterpret_cast<T2*>(p)->~T2();
            }
        }
    };

    void Release()
    {
        TryRelease<TS...>::Destroy(data_, type_);
        type_ = 0;
    }


    template <typename ...TS2>
    struct ConstructType
    {
       static void Construct(unsigned char* p, std::size_t id) {}
    };

    template <typename T, typename ...TS2>
    struct ConstructType<T, TS2...>
    {
        static void Construct(unsigned char* p, std::size_t id)
        {
            if (id == 0) return;

            if (id > 1)
            {
                ConstructType<TS2...>::Construct(p, id - 1);
            }
            else
            {
                new(p) T();
            }
        }
    };

    template <typename ...TS2>
    struct MoveTypeObj
    {
        static void Move(unsigned char* f, unsigned char* p, std::size_t id) {}
    };

    template <typename T, typename ...TS2>
    struct MoveTypeObj<T, TS2...>
    {
        static void Move(unsigned char* f, unsigned char* p, std::size_t id)
        {
            if (id == 0) return;

            if (id > 1)
            {
                MoveTypeObj<TS2...>::Move(f, p, id - 1);
            }
            else
            {
                T* fp = reinterpret_cast<T*>(f);
                new(p) T(std::move(*fp));
            }
        }
    };


private:
    std::size_t type_;
    unsigned char data_[TypeMaxSize<TS...>::value];
};

#endif //COMPILERFRONT_INVARIANT_H
