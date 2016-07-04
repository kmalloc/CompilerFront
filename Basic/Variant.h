//
// Created by miliao on 10/22/15.
//

#ifndef COMPILERFRONT_INVARIANT_H
#define COMPILERFRONT_INVARIANT_H

#include <algorithm>
#include <type_traits>

namespace VariantHelper {

    // check if a type exists in the variadic type list
    template <typename T, typename ...TS>
    struct TypeExist
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
    template <typename ...TS>
    struct TypeMaxSize
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
    inline void CopyConstruct<void>(const unsigned char*, unsigned char*)
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
    inline void MoveConstruct<void>(unsigned char*, unsigned char*)
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
    inline void MoveAssignConstruct<void>(unsigned char*, unsigned char*)
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
    inline void CopyAssignConstruct<void>(const unsigned char*, unsigned char*)
    {
        throw "try to copy assign Variant object containing non-assignable type.";
    }

    template<class T>
    struct SelectCopy
    {
        using type = typename std::conditional<std::is_copy_constructible<T>::value, T, void>::type;
        constexpr static copy_func_t fun = CopyConstruct<type>;
    };

    template<class T>
    struct SelectMove
    {
        using type = typename std::conditional<std::is_move_constructible<T>::value, T, void>::type;
        constexpr static move_func_t fun = MoveConstruct<type>;
    };

    template<class T>
    struct SelectMoveAssign
    {
        using type = typename std::conditional<std::is_move_assignable<T>::value, T, void>::type;
        constexpr static move_func_t fun = MoveAssignConstruct<type>;
    };

    template<class T>
    struct SelectCopyAssign
    {
        using type = typename std::conditional<std::is_copy_assignable<T>::value, T, void>::type;
        constexpr static copy_func_t fun = CopyAssignConstruct<type>;
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

    template <typename T, typename R = void>
    struct GetVisitorResultType
    {
        template <typename T2> static R foo(...);
        template <typename T2> static typename T2::result_type foo(typename T2::result_type* v);

        using type = decltype(foo<T>(NULL));
    };

    template<class V, class ...TS>
    struct check_visitor_func
    {
        enum { value = 1 };
    };

    template<class V, class T, class ...TS>
    struct check_visitor_func<V, T, TS...>
    {
        using ret_t = typename VariantHelper::GetVisitorResultType<V>::type;

        static T& getFakeValRef();
        using func_t = decltype(std::declval<V>()(getFakeValRef()));

        enum { value = std::is_convertible<ret_t, func_t>::value && check_visitor_func<V, TS...>::value };

        // static_assert(value, "signature of visitor is not correct.");
    };

    template<class T, class V>
    struct variant_cast
    {
        operator T& () { return static_cast<V&>(*this).template GetRef<T>(); }
    };

} // end namespace VariantHelper


// implementation of variant.

template <typename ...TS>
class Variant: public VariantHelper::variant_cast<TS, Variant<TS...>>...
{
public:
    Variant(): type_(0)
    {
    }

    template <typename T, typename D = typename std::enable_if<
            !std::is_same<typename std::remove_reference<T>::type, Variant<TS...>>::value>::type,
            typename CT = typename VariantHelper::SelectType<
                    typename std::remove_reference<T>::type, TS...>::type>
    Variant(T&& v): type_(VariantHelper::TypeExist<CT, TS...>::id)
    {
        static_assert(VariantHelper::TypeExist<CT, TS...>::exist,
                     "invalid type for the variant.");

        static_assert(VariantHelper::CheckConstructible<std::is_lvalue_reference<T>::value, CT>::value,
                     "try to copy or move an object that is not copyable or movable.");

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
                      "try to copy or move an object that is not copyable or movable.");

        if (type_ != VariantHelper::TypeExist<CT,TS...>::id)
        {
            Release();
            new(data_) CT(std::forward<T>(v));
            type_ = VariantHelper::TypeExist<CT, TS...>::id;
        }
        else if (type_)
        {
            // if exception occurs, then variant remains invalid
            const auto type = type_;
            type_ = 0;
            *reinterpret_cast<CT*>(data_) = std::forward<T>(v);
            type_ = type;
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
        else if (type_)
        {
            const auto type = type_;
            type_ = 0;
            copy_assign_[type - 1](other.data_, data_);
            type_ = type;
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
        else if (type_)
        {
            const auto type = type_;
            type_ = 0;
            move_assign_[type - 1](other.data_, data_);
            type_ = type;
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
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for the variant.");

        Release();

        new(data_) T(std::forward<TS2>(arg)...);
        type_ = VariantHelper::TypeExist<T, TS...>::id;
    }

    template <typename T>
    T* GetPtr() noexcept
    {
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for the variant.");

        if (type_ != VariantHelper::TypeExist<T, TS...>::id) return NULL;

        return reinterpret_cast<T*>(data_);
    }

    template <typename T>
    const T* GetConstPtr() const noexcept
    {
        static_assert(VariantHelper::TypeExist<T, TS...>::exist, "invalid type for the variant.");

        if (type_ != VariantHelper::TypeExist<T, TS...>::id) return NULL;

        return reinterpret_cast<const T*>(data_);
    }

    template <typename T>
    T Get() const noexcept
    {
        return *GetConstPtr();
    }

    template <typename T>
    T& GetRef()
    {
        T* p = GetPtr<T>();
        if (!p) throw "invalid type for variant::Get<>()";

        return *p;
    }

    template <typename T>
    const T& GetConstRef() const
    {
        const T* p = GetConstPtr<T>();
        if (!p) throw "invalid type for variant::Get<>()";

        return *p;
    }

    std::size_t GetType() const { return type_; }

    constexpr static size_t Alignment() { return VariantHelper::TypeMaxSize<TS...>::align; }
    constexpr static std::size_t GetSize() { return VariantHelper::TypeMaxSize<TS...>::value; }

    template<class V>
    struct VisitorImpl
    {
        using R = typename VariantHelper::GetVisitorResultType<V>::type;
        using visitor_func_t = R (*)(Variant& v, V& visitor, unsigned char*);

        static R Visit(Variant& var, V& vi)
        {
            const int which = var.GetType() - 1;
            return visitors_[which](var, vi, var.data_);
        }

        template<class T>
        static R do_visit(Variant& v, V& vi, unsigned char* s)
        {
            T& val = *reinterpret_cast<T*>(s);
            return vi(val);
        }

        constexpr static visitor_func_t visitors_[] = {do_visit<TS>...};
    };

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

template<class ...TS>
template<class V>
constexpr typename Variant<TS...>::template VisitorImpl<V>::visitor_func_t Variant<TS...>::VisitorImpl<V>::visitors_[];

template<class Visitor, class ...TS>
typename VariantHelper::GetVisitorResultType<Visitor>::type
VisitVariant(Variant<TS...>& variant, Visitor& visitor)
{
    static_assert(VariantHelper::check_visitor_func<Visitor, TS...>::value,
                  "signature of the visitor is not correct, please check the return type of your visitor.");

    return Variant<TS...>::template VisitorImpl<Visitor>::Visit(variant, visitor);
}

#endif // COMPILERFRONT_INVARIANT_H
