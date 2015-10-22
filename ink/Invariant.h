//
// Created by miliao on 10/22/15.
//

#ifndef COMPILERFRONT_INVARIANT_H
#define COMPILERFRONT_INVARIANT_H

#include <algorithm>


// check if a type exists the variadic types
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


// extract type from the variadic types at position k
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


// get max size of type
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
class Invariant
{
public:
    Invariant()
        : type_(0)
    {
    }

    template <typename T>
    Invariant(T&& v)
        : type_(TypeExist<T, TS...>::id)
    {
        static_assert(TypeExist<T, TS...>::exist, "invalid type for invariant.");

        new(data_) T(std::forward<T>(v));
    }

    template <typename T>
    Invariant& operator=(T&& v)
    {
        static_assert(TypeExist<T, TS...>::exist, "invalid type for invariant.");

        Release();
        new(data_) T(std::forward<T>(v));
        type_ = static_cast<std::size_t>(TypeExist<T, TS...>::id);
    }

    ~Invariant()
    {
        Release();
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

            if (id >= 1)
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
    }

private:
    std::size_t type_;
    unsigned char data_[TypeMaxSize<TS...>::value];
};

#endif //COMPILERFRONT_INVARIANT_H
