//
// Created by miliao on 10/22/15.
//

#include "gtest/gtest.h"
#include "Variant.h"

#include <functional>

struct ForDestroy
{
    ForDestroy() = default;

    explicit ForDestroy(std::function<void(void)> fun): fun_(std::move(fun)) {}

    ForDestroy(const ForDestroy& other) = default;

    ForDestroy(ForDestroy&& other)
    {
        fun_ = std::move(other.fun_);
        other.fun_ = std::function<void()>();
    }

    ForDestroy& operator=(ForDestroy&& other)
    {
        if (this == &other) return *this;

        fun_ = std::move(other.fun_);
        other.fun_ = std::function<void()>();
        return *this;
    }

    ~ForDestroy()
    {
        if (fun_) fun_();
    }

    char data[513];
    std::function<void(void)> fun_;
};

TEST(ink_test_suit, test_variant_basic)
{
    Variant<int, double> v1(32);
    ASSERT_EQ(1, v1.GetType());
    ASSERT_EQ(32, v1.GetRef<int>());

    int& ri = v1.GetRef<int>();
    ri = 33;
    ASSERT_EQ(33, v1.GetRef<int>());
    ASSERT_THROW(v1.GetRef<double>(), const char*);

    v1 = 2.333;
    ASSERT_EQ(2, v1.GetType());
    ASSERT_DOUBLE_EQ(2.333, v1.GetRef<double>());
    ASSERT_THROW(v1.GetRef<int>(), const char*);

    double& rd = v1.GetRef<double>();
    rd = 23.33;
    ASSERT_DOUBLE_EQ(23.33, v1.GetRef<double>());

    Variant<int, double> v2(2.2);
    ASSERT_EQ(2, v2.GetType());
    ASSERT_DOUBLE_EQ(2.2, v2.GetRef<double>());

    v2 = 3;
    ASSERT_EQ(1, v2.GetType());
    ASSERT_EQ(3, v2.GetRef<int>());

    Variant<int, std::string> v3(std::string("ww"));
    ASSERT_STREQ("ww", v3.GetRef<std::string>().c_str());

    std::string& rs = v3.GetRef<std::string>();
    rs = "hello";
    ASSERT_STREQ("hello", v3.GetRef<std::string>().c_str());

    v3 = 23;
    ASSERT_EQ(1, v3.GetType());
    ASSERT_EQ(23, v3.GetRef<int>());

    {
        ForDestroy fm2;
        Variant<int, ForDestroy> vfm = fm2;
        ASSERT_EQ(2, vfm.GetType());
    }

    {
        Variant<int, std::string, ForDestroy> v4(2);
        ASSERT_EQ(1, v4.GetType());

        int tag = 23;
        v4 = ForDestroy([&tag]() { tag--; });
        ASSERT_EQ(3, v4.GetType());

        v4.GetPtr<ForDestroy>()->fun_();
        ASSERT_EQ(22, tag);

        v4 = 42;
        ASSERT_EQ(1, v4.GetType());
        ASSERT_EQ(21, tag);

        Variant<int, std::string, ForDestroy> v5(v4);
        ASSERT_EQ(42, v5.GetRef<int>());
    }

    // test implicit convertion.
    Variant<int, std::string, double> v6("abc");
    ASSERT_EQ(2, v6.GetType());
    ASSERT_STREQ("abc", v6.GetRef<std::string>().c_str());

    char c = 'a';
    v6 = c;
    ASSERT_EQ(1, v6.GetType());
    ASSERT_EQ(static_cast<int>(c), v6.GetRef<int>());

    v6 = "hello";
    ASSERT_EQ(2, v6.GetType());
    ASSERT_STREQ("hello", v6.GetRef<std::string>().c_str());

    Variant<double, std::string> v7(23);
    ASSERT_EQ(1, v7.GetType());
    ASSERT_DOUBLE_EQ(23, v7.GetRef<double>());

    char str[] = "hello2";
    v7 = str;
    ASSERT_EQ(2, v7.GetType());
    ASSERT_STREQ("hello2", v7.GetRef<std::string>().c_str());

    // variant of variant.
    Variant<std::string, int, Variant<std::string, int>> vv("wwww");
    ASSERT_EQ(1, vv.GetType());
    ASSERT_STREQ("wwww", vv.GetRef<std::string>().c_str());

    vv = Variant<std::string, int>(23);
    ASSERT_EQ(3, vv.GetType());
    Variant<std::string, int>& nv = vv.GetRef<Variant<std::string, int>>();
    ASSERT_EQ(2, nv.GetType());
    ASSERT_EQ(23, nv.GetRef<int>());

    Variant<int, std::string> cv(23);

    int& vi1 = cv;
    ASSERT_EQ(23, vi1);
    cv = 3232;
    ASSERT_EQ(3232, vi1);

    vi1 = 244;
    int vi2 = cv;
    ASSERT_EQ(244, vi2);
    ASSERT_EQ(244, cv.GetRef<int>());

    ASSERT_THROW([&](){ std::string s = cv; }(), const char*);

    cv = "22223333sdkfjlsdkfjsldjfklsjflkjsdklfjs";
    std::string& rs2 = cv;
    ASSERT_STREQ("22223333sdkfjlsdkfjsldjfklsjflkjsdklfjs", rs2.c_str());

    rs2 = "vvvvvvvvvvvvvvvsjdlfksjkldfj232r2r323rv";
    ASSERT_STREQ("vvvvvvvvvvvvvvvsjdlfksjkldfj232r2r323rv", cv.GetRef<std::string>().c_str());

    cv = "wewewewewerw";
    ASSERT_STREQ("wewewewewerw", rs2.c_str());
}

struct exception_on_copy_move
{
    unsigned int type_;

    exception_on_copy_move(unsigned int type = 7): type_(type) {}

    exception_on_copy_move(const exception_on_copy_move& o)
    {
        if (o.type_ & 1) throw("throw from copy");

        type_ = o.type_;
    }

    exception_on_copy_move(exception_on_copy_move&& o)
    {
        if (o.type_ & 2) throw("throw from copy");

        type_ = o.type_;
    }

    exception_on_copy_move& operator=(const exception_on_copy_move& o)
    {
        if (type_ & 1) throw("throw from copy");

        type_ = o.type_;
        return *this;
    }

    exception_on_copy_move& operator=(exception_on_copy_move&& o)
    {
        if (type_ & 2) throw("throw from move");

        type_ = o.type_;
        return *this;
    }
};

TEST(ink_test_suit, test_variant_raise_exception)
{
    exception_on_copy_move ecm;
    Variant<int, exception_on_copy_move> v(233);

    ASSERT_EQ(1, v.GetType());
    ASSERT_THROW(v=ecm, const char*);

    ASSERT_EQ(0, v.GetType());

    ecm.type_ = 0;
    v = ecm;
    ecm.type_ = 3;

    ASSERT_EQ(2, v.GetType());

    exception_on_copy_move& recm = v;
    recm.type_ = 3;

    ASSERT_THROW(v=ecm, const char*);
    ASSERT_EQ(0, v.GetType());

    v = 244;
    ASSERT_THROW(v=std::move(ecm), const char*);
    ASSERT_EQ(0, v.GetType());

    v = 33;
    ecm.type_ = 0;

    v = ecm;
    ASSERT_EQ(2, v.GetType());

    ecm.type_ = 7;
    ASSERT_EQ(0, v.GetRef<exception_on_copy_move>().type_);

    v = ecm;
    ASSERT_EQ(7, v.GetRef<exception_on_copy_move>().type_);

    ASSERT_THROW(v=std::move(ecm), const char*);
    ASSERT_EQ(0, v.GetType());
}

struct ForMove
{
    ForMove(int res): res_(res), notify_([](){})
    {

    }

    ForMove(int res, std::function<void()> f): res_(res), notify_(std::move(f))
    {

    }

    ~ForMove()
    {
        notify_();
    }

    ForMove(const ForMove& v) = default;

    ForMove(ForMove&& v) noexcept(true)
    {
        res_ = v.res_ + 23;

        // copy notifier, not move
        notify_ = v.notify_;
        v.res_ = 0;
    }

    ForMove& operator=(const ForMove& v)
    {
        if (this == &v) return *this;

        res_ = v.res_ + 3;
        notify_ = v.notify_;
        return *this;
    }

    ForMove& operator=(ForMove&& v) noexcept(false)
    {
        if (this == &v) return *this;

        res_ = v.res_ + 13;
        notify_ = v.notify_;
        v.res_ = 0;

        return *this;
    }

    int GetRes() const { return res_; }

private:
    int res_;
    std::function<void(void)> notify_;
};

struct ForNoexcept
{
    ForNoexcept(ForNoexcept&& v) noexcept(false)
    {

    }

    ForNoexcept& operator=(ForNoexcept&& v) noexcept(true)
    {
        return *this;
    }
};

struct ForNoexcept2
{
    ForNoexcept2(ForNoexcept2&& v) noexcept(true)
    {
    }

    ForNoexcept2& operator=(ForNoexcept2&& v) noexcept(true)
    {
        return *this;
    }
};

TEST(ink_test_suit, test_variant_internal)
{
    // 1. test alignment.
    // 2. test move semantic.
    // 3. test copy construct.

    Variant<int, double> v(232.3);
    ASSERT_EQ(sizeof(double), v.Alignment());
    ASSERT_EQ(alignof(double), v.GetSize());

    Variant<int, double, std::string, ForDestroy> v2(2);
    ASSERT_EQ(alignof(ForDestroy), v2.Alignment());
    ASSERT_EQ(sizeof(ForDestroy), v2.GetSize());

    // test copy

    Variant<int, std::string> v3;
    v3.EmplaceSet<std::string>("wwww");

    ASSERT_STREQ("wwww", v3.GetPtr<std::string>()->c_str());

    v3.EmplaceSet<std::string>(4, 'c');
    ASSERT_STREQ("cccc", v3.GetPtr<std::string>()->c_str());

    Variant<int, std::string> v4(v3);
    ASSERT_EQ(2, v4.GetType());
    ASSERT_STREQ("cccc", v4.GetPtr<std::string>()->c_str());

    Variant<int, std::string> v5 = v3;
    ASSERT_EQ(2, v5.GetType());
    ASSERT_STREQ("cccc", v5.GetPtr<std::string>()->c_str());

    ForMove fm1(33);
    Variant<int, ForMove> vfm = fm1;
    ASSERT_EQ(2, vfm.GetType());
    vfm = 23;
    ASSERT_EQ(1, vfm.GetType());

    v3 = 32;
    v5 = v3;
    ASSERT_EQ(1, v5.GetType());
    ASSERT_EQ(32, v5.GetRef<int>());

    // test move
    int num_destruct = 0;
    Variant<int, ForMove> v6(ForMove(23, [&](){ ++num_destruct; }));
    ASSERT_EQ(23 + 23, v6.GetRef<ForMove>().GetRes());

    ASSERT_EQ(1, num_destruct);

    // test assign
    ForMove fm(42, [&](){ ++num_destruct; });
    v6 = std::move(fm);
    ASSERT_EQ(1, num_destruct);

    ASSERT_EQ(0, fm.GetRes());
    ASSERT_EQ(42 + 13, v6.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v7(v6);
    ASSERT_EQ(42 + 13, v6.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42 + 13, v7.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v8 = v6;
    ASSERT_EQ(42 + 13, v6.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42 + 13, v8.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v9(std::move(v6));
    ASSERT_EQ(0, v6.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42 + 13 + 23, v9.GetRef<ForMove>().GetRes());

    v6 = 23;
    ASSERT_EQ(2, num_destruct);

    Variant<int, ForMove> v10 = std::move(v7);
    ASSERT_EQ(0, v7.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42 + 13 + 23, v10.GetRef<ForMove>().GetRes());

    v7 = v8;
    ASSERT_EQ(42 + 13, v8.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42 + 13 + 3, v7.GetRef<ForMove>().GetRes());

    v8 = std::move(v10);
    ASSERT_EQ(2, v10.GetType());
    ASSERT_EQ(42 + 13 + 23 + 13, v8.GetRef<ForMove>().GetRes());
    ASSERT_EQ(0, v10.GetRef<ForMove>().GetRes());

    num_destruct = 0;
    Variant<int, ForMove> va(ForMove(23, [&](){ ++num_destruct; }));

    ASSERT_EQ(1, num_destruct);
    ForMove df(23, [&](){ ++num_destruct; });
    va = df;
    ASSERT_EQ(1, num_destruct);
    va = 23;
    ASSERT_EQ(2, num_destruct);

    Variant<int, std::string> vn, vn2;
    Variant<int, std::string> vs("sss");

    vn = vs;
    vn2 = std::move(vs);

    ASSERT_EQ(2, vn.GetType());
    ASSERT_EQ(2, vn2.GetType());
    ASSERT_STREQ("sss", vn.GetRef<std::string>().c_str());
    ASSERT_STREQ("sss", vn2.GetRef<std::string>().c_str());

    vn = Variant<int, std::string>();
    vn2 = Variant<int, std::string>();
    ASSERT_EQ(0, vn2.GetType());

    vn = vn2;
    ASSERT_EQ(0, vn.GetType());

    vn = std::move(vn2);
    ASSERT_EQ(0, vn.GetType());

    vn2 = vn2;
    ASSERT_EQ(0, vn2.GetType());

    using variant_t1 = Variant<int, ForMove>;
    ASSERT_FALSE(std::is_nothrow_move_assignable<variant_t1>::value);
    ASSERT_TRUE(std::is_nothrow_move_constructible<variant_t1>::value);

    using variant_t2 = Variant<int, ForNoexcept>;
    ASSERT_FALSE(std::is_nothrow_move_assignable<variant_t2>::value);
    ASSERT_FALSE(std::is_nothrow_move_constructible<variant_t2>::value);

    using variant_t3 = Variant<int, ForNoexcept2>;
    ASSERT_TRUE(std::is_nothrow_move_assignable<variant_t3>::value);
    ASSERT_TRUE(std::is_nothrow_move_constructible<variant_t3>::value);

    using variant_t4 = Variant<int, ForMove, ForNoexcept, ForNoexcept2>;
    ASSERT_FALSE(std::is_nothrow_move_assignable<variant_t4>::value);
    ASSERT_FALSE(std::is_nothrow_move_constructible<variant_t4>::value);

    using variant_t5 = Variant<int, ForMove, ForNoexcept2>;
    ASSERT_FALSE(std::is_nothrow_move_assignable<variant_t5>::value);
    ASSERT_TRUE(std::is_nothrow_move_constructible<variant_t5>::value);
}

struct NonCopyableType
{
    int res_ = 0;

    explicit NonCopyableType(int i): res_(i) {}

    NonCopyableType(const NonCopyableType&) = delete;

    NonCopyableType(NonCopyableType&& other)
    {
        res_ = other.res_;
        other.res_ = 0;
    }

    NonCopyableType& operator=(NonCopyableType&& other)
    {
        if (this == &other) return *this;

        res_ = other.res_;
        other.res_ = 0;
        return *this;
    }
};

struct NonMovableType
{
    int res_ = 0;

    explicit NonMovableType(int i) : res_(i) {}

    NonMovableType(const NonMovableType& other)
    {
        res_ = other.res_;
    }

    NonMovableType& operator=(const NonMovableType& other)
    {
        if (this == &other) return *this;

        res_ = other.res_;
        return *this;
    }

    NonMovableType(NonMovableType&&) = delete;
};

TEST(ink_test_suit, test_variant_non_copy_move)
{
    Variant<int, std::string, NonCopyableType> v1("abc");
    ASSERT_EQ(2, v1.GetType());
    ASSERT_STREQ("abc", v1.GetRef<std::string>().c_str());

    Variant<int, std::string, NonCopyableType> v2(NonCopyableType(23));
    ASSERT_EQ(3, v2.GetType());
    ASSERT_EQ(23, v2.GetRef<NonCopyableType>().res_);

    v2 = v1;
    ASSERT_EQ(2, v2.GetType());
    ASSERT_EQ(2, v1.GetType());

    v1 = NonCopyableType(24);

    bool flag = false;
    try {
        v2 = v1;
    } catch (...) {
        flag = true;
        ASSERT_EQ(0, v2.GetType());
    }

    ASSERT_TRUE(flag);

    Variant<int, std::string, NonMovableType> v3("abc");
    ASSERT_EQ(2, v3.GetType());
    ASSERT_STREQ("abc", v3.GetRef<std::string>().c_str());

    NonMovableType nm(23);
    Variant<int, std::string, NonMovableType> v4(nm);

    ASSERT_EQ(3, v4.GetType());
    ASSERT_EQ(23, v4.GetRef<NonMovableType>().res_);

    v4 = std::move(v3);
    ASSERT_EQ(2, v4.GetType());
    ASSERT_EQ(2, v3.GetType());
    ASSERT_STREQ("", v3.GetRef<std::string>().c_str());

    v3 = nm;
    flag = false;

    try {
        v4 = std::move(v3);
    } catch (...) {
        flag = true;
        ASSERT_EQ(0, v4.GetType());
    }

    ASSERT_TRUE(flag);
}

struct complex
{
    std::string s1;
    int a, b;
    double d;
    std::string s2;
};

struct visitor
{
    using result_type = int;

    int type_;

    int ival_;
    double dval_;
    std::string sval_;
    complex cval_;

    visitor(): type_(0) {}

    int operator()(std::string& s)
    {
        type_ = 1;
        sval_ = s;
        s += "abc";
        return type_;
    }

    int operator()(int& f)
    {
        type_ = 2;
        ival_ = f;
        f += 100;
        return type_;
    }

    char operator()(double f)
    {
        type_ = 3;
        dval_ = f;
        return type_;
    }

    int operator()(visitor& v)
    {
        type_ = 4;
        return type_;
    }

    int operator()(complex& c)
    {
        type_ = 5;
        cval_ = c;
        c.d = 23.2323;
        c.s1 = "modified by visitor";
        c.b  = 23333;
        return type_;
    }
};


TEST(ink_test_suite, test_variant_visitor)
{
    visitor v;
    Variant<std::string, int, double, visitor, complex> var("abc");

    int ret = VisitVariant(var, v);

    ASSERT_EQ(1, ret);
    ASSERT_EQ(1, v.type_);
    ASSERT_STREQ("abc", v.sval_.c_str());
    ASSERT_STREQ("abcabc", var.GetRef<std::string>().c_str());

    var = 233;
    ret = VisitVariant(var, v);

    ASSERT_EQ(2, ret);
    ASSERT_EQ(2, v.type_);
    ASSERT_EQ(233, v.ival_);
    ASSERT_EQ(333, var.GetRef<int>());

    var = 233.23;
    ret = VisitVariant(var, v);

    ASSERT_EQ(3, ret);
    ASSERT_EQ(3, v.type_);
    ASSERT_DOUBLE_EQ(233.23, v.dval_);

    var = v;
    ret = VisitVariant(var, v);

    ASSERT_EQ(4, ret);
    ASSERT_EQ(4, v.type_);

    var = complex{"henhenhaha", 233, 533, 2213.233, "bbbbwwwwfff"};
    ret = VisitVariant(var, v);

    ASSERT_EQ(5, ret);
    ASSERT_EQ(5, v.type_);

    ASSERT_EQ(v.cval_.a, var.GetRef<complex>().a);
    ASSERT_EQ(v.cval_.b, 533);
    ASSERT_DOUBLE_EQ(v.cval_.d, 2213.233);
    ASSERT_STREQ(v.cval_.s1.c_str(), "henhenhaha");
    ASSERT_STREQ(v.cval_.s2.c_str(), var.GetRef<complex>().s2.c_str());

    ASSERT_EQ(23333, var.GetRef<complex>().b);
    ASSERT_DOUBLE_EQ(23.2323, var.GetRef<complex>().d);
    ASSERT_STREQ("modified by visitor", var.GetRef<complex>().s1.c_str());

    var = "done testing visitor";
    ret = VisitVariant(var, v);

    ASSERT_EQ(1, ret);
    ASSERT_EQ(1, v.type_);
    ASSERT_STREQ("done testing visitor", v.sval_.c_str());
    ASSERT_STREQ("done testing visitorabc", var.GetRef<std::string>().c_str());
}

