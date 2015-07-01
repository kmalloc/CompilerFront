#include "gtest/gtest.h"

#include "Lexer.h"

using namespace ink;

TEST(ink_test_suit, test_lexer)
{
    Lexer lex("");
    const char* txt = "a = c\n a * c + 25 - 23.3 / \"abc\" % ww";

    lex.Reset(txt);

    ASSERT_TRUE(lex.GetCurToken() == TOK_UNKNOWN);
    ASSERT_TRUE(lex.GetCurTokenPrec() == -1);

    lex.Start();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "c");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_MUL);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "c");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ADD);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(25, lex.GetIntVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_SUB);
    lex.ConsumeCurToken();
    ASSERT_DOUBLE_EQ(23.3, lex.GetFloatVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_DIV);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_QUO);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_STR);
    ASSERT_STREQ("abc", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_MOD) << lex.GetCurToken();
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("ww", lex.GetStringVal().c_str());


    txt = "extern foo(a, b, c)\nfunc foo(a, b, c)\n { a = b + c\n return a }";
    lex.Reset(txt);

    ASSERT_TRUE(lex.GetCurToken() == TOK_UNKNOWN);
    ASSERT_TRUE(lex.GetCurTokenPrec() == -1);

    lex.Start();

    ASSERT_TRUE(lex.GetCurToken() == TOK_EXT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("foo", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("a", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("b", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("c", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_FUN);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("foo", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_LEFT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("a", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("b", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("c", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_LEFT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "b");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ADD);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "c");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_RET);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");

    // TODO: class, array index, bitwise operator(&^|~), and negate operation.
    // and logical operation, &&, ||, >, >=, <, <=, !=, ==
    // and comment, operator precedences
}


