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
        static constexpr std::size_t align = 0;
    };

    template <typename T, typename ...TS>
    struct TypeMaxSize<T, TS...>
    {
        static constexpr std::size_t cur_align = alignof(T);
        static constexpr std::size_t next_align = TypeMaxSize<TS...>::align;
        static constexpr std::size_t align = cur_align > next_align? cur_align : next_align;

        static constexpr std::size_t cur_size = sizeof(T);
        static constexpr std::size_t next_size = TypeMaxSize<TS...>::value;
        static constexpr std::size_t value = cur_size > next_size? cur_size : next_size;
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

        using type = typename std::conditional<std::is_convertible<T, T1>::value,
                T1, typename SelectConvertible<T, TS...>::type>::type ;
    };

    template<class T, class ...TS>
    struct SelectType
    {
       using type = typename std::conditional<TypeExist<T, TS...>::exist, T,
               typename SelectConvertible<T, TS...>::type>::type;
    };

    using destroy_func_t = void(*)(unsigned char*);
    using move_func_t = void(*)(unsigned char*, unsigned char*);
    using copy_func_t = void(*)(const unsigned char*, unsigned char*);

    template<class T>
    void Destroy(unsigned char* data)
    {
        reinterpret_cast<T*>(data)->~T();
    }

    template<class T>
    void CopyConstruct(const unsigned char* f, unsigned char* t)
    {
        new(t) T(*reinterpret_cast<const T*>(f));
    }

    template<>
    void CopyConstruct<void>(const unsigned char*, unsigned char*)
    {
        throw "try to copy Variant object containing non-copyable type.";
    }

    template<class T>
    void MoveConstruct(unsigned char* f, unsigned char* t)
    {
        T* fp = reinterpret_cast<T*>(f);
        new(t) T(std::move(*fp));
    }

    template<>
    void MoveConstruct<void>(unsigned char*, unsigned char*)
    {
        throw "try to move Variant object containing non-movable type.";
    }

    template<class T>
    void MoveAssignConstruct(unsigned char* f, unsigned char* t)
    {
        T* fp = reinterpret_cast<T*>(f);
        T* tp = reinterpret_cast<T*>(t);

        *tp = std::move(*fp);
    }

    template<>
    void MoveAssignConstruct<void>(unsigned char*, unsigned char*)
    {
        throw "try to move assign Variant object containing non-assignable type.";
    }

    template<class T>
    void CopyAssignConstruct(const unsigned char* f, unsigned char* t)
    {
        T* tp = reinterpret_cast<T*>(t);
        const T* fp = reinterpret_cast<const T*>(f);

        *tp = *fp;
    }

    template<>
    void CopyAssignConstruct<void>(const unsigned char*, unsigned char*)
    {
        throw "try to copy assign Variant object containing non-assignable type.";
    }

    template<bool f, class T>
    struct SelectCopyIf
    {
        constexpr static copy_func_t fun = CopyConstruct<T>;
    };

    template<class T>
    struct SelectCopyIf<false, T>
    {
        constexpr static copy_func_t fun = CopyConstruct<void>;
    };

    template<class T>
    struct SelectCopy
    {
        constexpr static copy_func_t fun = SelectCopyIf<std::is_copy_constructible<T>::value, T>::fun;
    };

    template<bool f, class T>
    struct SelectMoveIf
    {
        constexpr static move_func_t fun = MoveConstruct<T>;
    };

    template<class T>
    struct SelectMoveIf<false, T>
    {
        constexpr static move_func_t fun = MoveConstruct<void>;
    };

    template<class T>
    struct SelectMove
    {
        constexpr static move_func_t fun = SelectMoveIf<std::is_move_constructible<T>::value, T>::fun;
    };

    template<bool f, class T>
    struct SelectMoveAssignIf
    {
        constexpr static move_func_t fun = MoveAssignConstruct<T>;
    };

    template<class T>
    struct SelectMoveAssignIf<false, T>
    {
        constexpr static move_func_t fun = MoveAssignConstruct<void>;
    };

    template<class T>
    struct SelectMoveAssign
    {
        constexpr static move_func_t fun = SelectMoveAssignIf<std::is_move_assignable<T>::value, T>::fun;
    };

    template<bool f, class T>
    struct SelectCopyAssignIf
    {
        constexpr static copy_func_t fun = CopyAssignConstruct<T>;
    };

    template<class T>
    struct SelectCopyAssignIf<false, T>
    {
        constexpr static copy_func_t fun = CopyAssignConstruct<void>;
    };

    template<class T>
    struct SelectCopyAssign
    {
        constexpr static copy_func_t fun = SelectCopyAssignIf<std::is_copy_assignable<T>::value, T>::fun;
    };

    template<bool lvalue, class T>
    struct CheckConstructible
    {
        enum { value = std::is_copy_constructible<T>::value };
    };

    template<class T>
    struct CheckConstructible<false, T>
    {
        enum { value = std::is_move_constructible<T>::value };
    };

    template<template<typename> class CHECK, typename ...TS>
    struct CheckTypeList
    {
        enum { value = 1 };
    };

    template<template<typename> class CHECK, typename T, typename ...TS>
    struct CheckTypeList<CHECK, T, TS...>
    {
        enum { value = CHECK<T>::value && CheckTypeList<CHECK, TS...>::value };
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
            typename CT = typename VariantHelper::SelectType<
                    typename std::remove_reference<T>::type, TS...>::type>
    Variant(T&& v)
        : type_(VariantHelper::TypeExist<CT, TS...>::id)
    {
        static_assert(VariantHelper::TypeExist<CT, TS...>::exist,
                     "invalid type for invariant.");

        static_assert(VariantHelper::CheckConstructible<std::is_lvalue_reference<T>::value, CT>::value,
                     "try to copy or move an object that is not copyable or moveable.");

        new(data_) CT(std::forward<T>(v));
    }

    Variant(const Variant<TS...>& other)
    {
        if (other.type_ == 0) return;

        copy_[other.type_ - 1](other.data_, data_);
        type_ = other.type_;
    }

    Variant(Variant<TS...>&& other) noexcept(
        VariantHelper::CheckTypeList<std::is_nothrow_move_constructible, TS...>::value)
    {
        if (other.type_ == 0) return;

        move_[other.type_ - 1](other.data_, data_);
        type_ = other.type_;
    }

    template <typename T, typename D = typename std::enable_if<
            !std::is_same<typename std::remove_reference<T>::type, Variant<TS...>>::value>::type,
            typename CT = typename VariantHelper::SelectType<
                    typename std::remove_reference<T>::type, TS...>::type>
    Variant& operator=(T&& v)
    {
        static_assert(VariantHelper::TypeExist<CT, TS...>::exist,
                      "invalid type for Variant.");

        static_assert(VariantHelper::CheckConstructible<std::is_lvalue_reference<T>::value, CT>::value,
                      "try to copy or move an object that is not copyable or moveable.");

        if (type_ != VariantHelper::TypeExist<CT,TS...>::id)
        {
            Release();
            new(data_) CT(std::forward<T>(v));
            type_ = VariantHelper::TypeExist<CT, TS...>::id;
        }
        else if (type_)
        {
            *reinterpret_cast<CT*>(data_) = std::forward<T>(v);
        }
        else
        {
            new(data_) CT(std::forward<T>(v));
            type_ = VariantHelper::TypeExist<CT, TS...>::id;
        }

        return *this;
    }

    Variant& operator=(const Variant<TS...>& other)
    {
        if (this == &other) return *this;

        if (type_ != other.type_)
        {
            Release();
            if (!other.type_) return *this;

            copy_[other.type_ - 1](other.data_, data_);
            type_ = other.type_;
        }
        else
        {
            copy_assign_[type_ - 1](other.data_, data_);
        }

        return *this;
    }

    Variant& operator=(Variant<TS...>&& other) noexcept(
        VariantHelper::CheckTypeList<std::is_nothrow_move_constructible, TS...>::value &&
        VariantHelper::CheckTypeList<std::is_nothrow_move_assignable, TS...>::value)
    {
        if (this == &other) return *this;

        if (type_ != other.type_)
        {
            Release();
            if (!other.type_) return *this;

            move_[other.type_ - 1](other.data_, data_);
            type_ = other.type_;
        }
        else
        {
            move_assign_[type_ - 1](other.data_, data_);
        }
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

    constexpr static size_t Alignment() { return VariantHelper::TypeMaxSize<TS...>::align; }
    constexpr static std::size_t GetSize() { return VariantHelper::TypeMaxSize<TS...>::value; }

private:
    void Release()
    {
        if (!type_) return;

        destroy_[type_ - 1](data_);
        type_ = 0;
    }

private:
    std::size_t type_ = 0;
    alignas(Alignment()) unsigned char data_[GetSize()];

    constexpr static VariantHelper::destroy_func_t destroy_[] = {VariantHelper::Destroy<TS>...};
    constexpr static VariantHelper::copy_func_t copy_[] = {VariantHelper::SelectCopy<TS>::fun...};
    constexpr static VariantHelper::move_func_t move_[] = {VariantHelper::SelectMove<TS>::fun...};
    constexpr static VariantHelper::copy_func_t copy_assign_[] = {VariantHelper::SelectCopyAssign<TS>::fun...};
    constexpr static VariantHelper::move_func_t move_assign_[] = {VariantHelper::SelectMoveAssign<TS>::fun...};
};

template<class ...TS>
constexpr typename VariantHelper::copy_func_t Variant<TS...>::copy_[];

template<class ...TS>
constexpr typename VariantHelper::move_func_t Variant<TS...>::move_[];

template<class ...TS>
constexpr typename VariantHelper::copy_func_t Variant<TS...>::copy_assign_[];

template<class ...TS>
constexpr typename VariantHelper::move_func_t Variant<TS...>::move_assign_[];

template<class ...TS>
constexpr typename VariantHelper::destroy_func_t Variant<TS...>::destroy_[];

#endif //COMPILERFRONT_INVARIANT_H
