#include "gtest/gtest.h"

#include "Lexer.h"

using namespace ink;

TEST(ink_test_suit, test_lexer_literal)
{
    Lexer lex("");

    const char *txt = "12 2.2 \"abc\" true false";
    lex.Reset(txt);
    lex.Start();

    ASSERT_EQ(TOK_INT, lex.GetCurToken());
    ASSERT_EQ(12, lex.GetIntVal());
    lex.ConsumeCurToken();

    ASSERT_EQ(TOK_FLOAT, lex.GetCurToken());
    ASSERT_EQ(2.2, lex.GetFloatVal());
    lex.ConsumeCurToken();

    ASSERT_EQ(TOK_QUO, lex.GetCurToken());
    lex.ConsumeCurToken();

    ASSERT_EQ(TOK_STR, lex.GetCurToken());
    ASSERT_STREQ("abc", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();

    ASSERT_EQ(TOK_BOOL, lex.GetCurToken());
    ASSERT_STREQ("true", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();

    ASSERT_EQ(TOK_BOOL, lex.GetCurToken());
    ASSERT_STREQ("false", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
}

TEST(ink_test_suit, test_lexer_variable_def)
{
    Lexer lex("");
    auto txt = "a = 3; local b = 2.3; global c = \"ss\"";

    lex.Reset(txt);
    lex.Start();

    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("a", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_LOCAL);
    lex.ConsumeCurToken();

    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("b", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_FLOAT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_GLOBAL);
    lex.ConsumeCurToken();
}

TEST(ink_test_suit, test_lexer_arithmtic)
{
    Lexer lex("");
    auto txt = "a = c\n a * c + 25 - 23.3 / \"abc\" % ww";
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

}

TEST(ink_test_suit, test_lexer_function)
{
    Lexer lex("");
    auto txt = "extern foo(a, b, c)\nfunc foo(a, b, c)\n { a = b + c\n return a }";
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

}

TEST(ink_test_suit, test_lexer_class_and_calc)
{
    // comment, class, array index, bitwise operator(&^|~), and negate operation.
    Lexer lex("");
    auto txt = "class foo {} \n a = [23, 23.2, \"abc\"]\n# dummy comment\n b = a[2] + a[c]\n a = b & c | e + w ^ !w";
    lex.Reset(txt);

    ASSERT_TRUE(lex.GetCurToken() == TOK_UNKNOWN);
    ASSERT_TRUE(lex.GetCurTokenPrec() == -1);

    lex.Start();
    ASSERT_TRUE(lex.GetCurToken() == TOK_CLASS);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "foo");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_LEFT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(23, lex.GetIntVal());

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_FLOAT);
    ASSERT_DOUBLE_EQ(23.2, lex.GetFloatVal());

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_QUO);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_STR);
    ASSERT_STREQ("abc", lex.GetStringVal().c_str());

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMMENT);
    ASSERT_STREQ(" dummy comment", lex.GetStringVal().c_str());

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "b");

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(2, lex.GetIntVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ADD);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ("c", lex.GetStringVal().c_str());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_RIGHT);

    // a = b & c | e + w ^ !w
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "b");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AND);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "c");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_OR);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "e");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ADD);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "w");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_XOR);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_NEG);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "w");
}

TEST(ink_test_suit, test_lexer_stm_construct)
{
    // operator precedences
    // if elif else while for
    // logical operation, &&, ||, >, >=, <, <=, !=, ==

    Lexer lex("");
    auto txt = "if (a) { a = b + c && 23 } elif (c || a) {}\n elif (23 && w) {} else {} \n"
            "while (a) { a = 12 } \nfor a in [a,b,c] { a = a + 1 }";
    lex.Reset(txt);

    ASSERT_TRUE(lex.GetCurToken() == TOK_UNKNOWN);
    ASSERT_TRUE(lex.GetCurTokenPrec() == -1);

    lex.Start();
    ASSERT_TRUE(lex.GetCurToken() == TOK_IF);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
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
    ASSERT_TRUE(lex.GetCurToken() == TOK_LAND);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(23, lex.GetIntVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ELIF);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "c");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_LOR);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_RIGHT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ELIF);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(23, lex.GetIntVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_LAND);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "w");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_RIGHT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);

    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ELSE);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);

    // while
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_WHILE);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_PAREN_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
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
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(12, lex.GetIntVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);

    // for
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_FOR);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_IN);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "b");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_COMA);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "c");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACKET_RIGHT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_LEFT);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_AS);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ID);
    ASSERT_STREQ(lex.GetStringVal().c_str(), "a");
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_ADD);
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_INT);
    ASSERT_EQ(1, lex.GetIntVal());
    lex.ConsumeCurToken();
    ASSERT_TRUE(lex.GetCurToken() == TOK_BRACE_RIGHT);

}

TEST(ink_test_suit,  test_lexer_precedence)
{
    // ascending order for operator precedences
    // 35 -> 36 -> 37 -> 38 -> 39 -> 40 -> 41 41 -> 42 42 42 42
    // =  -> || -> && -> |  -> ^  -> &  -> == != -> < <= > >=
    // 43 43 -> 44 44 -> 45 45 -> 46 -> 47 -> 48 48
    // >> << -> + - -> * / -> % -> power -> ~ !

    Lexer lex("");

    ASSERT_TRUE(lex.GetTokenPrec(TOK_AS) < lex.GetTokenPrec(TOK_LOR));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_LOR) < lex.GetTokenPrec(TOK_LAND));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_LAND) < lex.GetTokenPrec(TOK_OR));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_OR) < lex.GetTokenPrec(TOK_XOR));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_XOR) < lex.GetTokenPrec(TOK_AND));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_AND) < lex.GetTokenPrec(TOK_EQ));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_EQ) == lex.GetTokenPrec(TOK_NE));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_NE) < lex.GetTokenPrec(TOK_LT));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_LT) == lex.GetTokenPrec(TOK_LE));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_LT) == lex.GetTokenPrec(TOK_GT));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_GE) == lex.GetTokenPrec(TOK_GT));

    ASSERT_TRUE(lex.GetTokenPrec(TOK_GE) < lex.GetTokenPrec(TOK_RSH));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_RSH) == lex.GetTokenPrec(TOK_LSH));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_RSH) < lex.GetTokenPrec(TOK_ADD));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_ADD) == lex.GetTokenPrec(TOK_SUB));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_ADD) < lex.GetTokenPrec(TOK_MUL));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_MUL) == lex.GetTokenPrec(TOK_DIV));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_MUL) < lex.GetTokenPrec(TOK_MOD));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_MOD) < lex.GetTokenPrec(TOK_POW));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_POW) < lex.GetTokenPrec(TOK_NEG));
    ASSERT_TRUE(lex.GetTokenPrec(TOK_INV) == lex.GetTokenPrec(TOK_NEG));
}


