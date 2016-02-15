//
// Created by miliao on 10/22/15.
//

#include "gtest/gtest.h"
#include "Variant.h"

TEST(ink_test_suit, test_variant_inline_func)
{
    Variant<std::string> v("aaa");
    ASSERT_STREQ("aaa", v.GetConstRef<std::string>().c_str());
}
