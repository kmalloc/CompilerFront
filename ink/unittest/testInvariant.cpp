//
// Created by miliao on 10/22/15.
//

#include "gtest/gtest.h"
#include "Invariant.h"

#include <functional>


struct ForDestroy
{
    ForDestroy(std::function<void(void)> fun)
            :fun_(std::move(fun))
    {}

    ForDestroy(ForDestroy&& other)
    {
        fun_ = std::move(other.fun_);
    }

    ~ForDestroy()
    {
        fun_();
    }

    std::function<void(void)> fun_;
};


TEST(ink_test_suit, test_invariant)
{
    Invariant<int, double> v1(32);
    ASSERT_EQ(1, v1.GetType());
    ASSERT_EQ(32, v1.GetRef<int>());

    v1 = 2.333;
    ASSERT_EQ(2, v1.GetType());
    ASSERT_DOUBLE_EQ(2.333, v1.GetRef<double>());

    Invariant<int, double> v2(2.2);
    ASSERT_EQ(2, v2.GetType());
    ASSERT_DOUBLE_EQ(2.2, v2.GetRef<double>());

    v2 = 3;
    ASSERT_EQ(1, v2.GetType());
    ASSERT_EQ(3, v2.GetRef<int>());

    Invariant<int, std::string> v3(std::string("ww"));
    ASSERT_STREQ("ww", v3.GetRef<std::string>().c_str());

    v3 = 23;
    ASSERT_EQ(1, v3.GetType());
    ASSERT_EQ(23, v3.GetRef<int>());

    Invariant<int, std::string, ForDestroy> v4(2);

    int tag = 23;
    v4 = ForDestroy([&tag]() { tag--; });

    v4.Get<ForDestroy>()->fun_();
    ASSERT_EQ(22, tag);

    v4 = 42;
    ASSERT_EQ(21, tag);
}
