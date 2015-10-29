//
// Created by miliao on 10/22/15.
//

#include "gtest/gtest.h"
#include "Variant.h"

#include <functional>


struct ForDestroy
{
    ForDestroy() = default;

    explicit ForDestroy(std::function<void(void)> fun)
            :fun_(std::move(fun)) {}

    ForDestroy(const ForDestroy& other) = default;

    ForDestroy(ForDestroy&& other)
    {
        fun_ = std::move(other.fun_);
        other.fun_ = std::function<void()>();
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

    v1 = 2.333;
    ASSERT_EQ(2, v1.GetType());
    ASSERT_DOUBLE_EQ(2.333, v1.GetRef<double>());

    Variant<int, double> v2(2.2);
    ASSERT_EQ(2, v2.GetType());
    ASSERT_DOUBLE_EQ(2.2, v2.GetRef<double>());

    v2 = 3;
    ASSERT_EQ(1, v2.GetType());
    ASSERT_EQ(3, v2.GetRef<int>());

    Variant<int, std::string> v3(std::string("ww"));
    ASSERT_STREQ("ww", v3.GetRef<std::string>().c_str());

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

        v4.Get<ForDestroy>()->fun_();
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
}


struct ForMove
{
    explicit ForMove(int res)
        : res_(res)
    {

    }

    ForMove(const ForMove& v) = default;

    ForMove(ForMove&& v)
    {
        res_ = v.res_;
        v.res_ = 0;
    }

    ForMove& operator=(const ForMove& v)
    {
        if (this == &v) return *this;

        res_ = v.res_;
        return *this;
    }

    ForMove& operator=(ForMove&& v)
    {
        if (this == &v) return *this;

        res_ = v.res_;
        v.res_ = 0;

        return *this;
    }

    int GetRes() const { return res_; }

private:

    int res_;
};

TEST(ink_test_suit, test_variant_internal)
{
    // 1. test alignment.
    // 2. test move semantic.
    // 3. test copy construct.

    Variant<int, double> v(232.3);
    ASSERT_EQ(alignof(double), v.GetSize());

    Variant<int, double, std::string, ForDestroy> v2(2);
    ASSERT_EQ(alignof(ForDestroy), v2.GetSize());

    // test copy

    Variant<int, std::string> v3;
    v3.EmplaceSet<std::string>("wwww");

    ASSERT_STREQ("wwww", v3.Get<std::string>()->c_str());

    v3.EmplaceSet<std::string>(4, 'c');
    ASSERT_STREQ("cccc", v3.Get<std::string>()->c_str());

    Variant<int, std::string> v4(v3);
    ASSERT_EQ(2, v4.GetType());
    ASSERT_STREQ("cccc", v4.Get<std::string>()->c_str());

    Variant<int, std::string> v5 = v3;
    ASSERT_EQ(2, v5.GetType());
    ASSERT_STREQ("cccc", v5.Get<std::string>()->c_str());

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

    Variant<int, ForMove> v6(ForMove(23));
    ASSERT_EQ(23, v6.GetRef<ForMove>().GetRes());

    ForMove fm(42);
    v6 = std::move(fm);

    ASSERT_EQ(0, fm.GetRes());
    ASSERT_EQ(42, v6.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v7(v6);
    ASSERT_EQ(42, v6.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42, v7.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v8 = v6;
    ASSERT_EQ(42, v6.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42, v8.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v9(std::move(v6));
    ASSERT_EQ(0, v6.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42, v9.GetRef<ForMove>().GetRes());

    Variant<int, ForMove> v10 = std::move(v7);
    ASSERT_EQ(0, v7.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42, v10.GetRef<ForMove>().GetRes());

    v7 = v8;
    ASSERT_EQ(42, v7.GetRef<ForMove>().GetRes());
    ASSERT_EQ(42, v8.GetRef<ForMove>().GetRes());

    v8 = std::move(v10);
    ASSERT_EQ(42, v8.GetRef<ForMove>().GetRes());
    ASSERT_EQ(0, v10.GetRef<ForMove>().GetRes());
}


struct NonCopyableType
{
    explicit NonCopyableType(int i)
            : res_(i) {}

    NonCopyableType(const NonCopyableType&) = delete;

    NonCopyableType(NonCopyableType&& other)
    {
        res_ = other.res_;
        other.res_ = 0;
    }

    int res_ = 0;
};

struct NonMovableType
{
    explicit NonMovableType(int i)
            : res_(i) {}

    NonMovableType(const NonMovableType& other)
    {
        res_ = other.res_;
    }

    NonMovableType(NonMovableType&&) = delete;

    int res_ = 0;
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

