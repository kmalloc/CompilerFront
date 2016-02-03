#include "gtest/gtest.h"

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <boost/assign.hpp>

#include "CalcParser.h"

using namespace std;
using namespace CalcParser;

static std::map<OpType, std::string> gs_op_map = boost::assign::map_list_of(OT_2_Eq, "==")(
    OT_2_Add, "+")(OT_2_Sub, "-")(OT_2_Mul, "*")(OT_2_Div, "/")(OT_2_Mod, "%")(OT_2_Xor, "^")(
    OT_2_And, "&")(OT_2_Or, "|")(OT_2_Left, "left")(OT_2_Right, "right")(OT_2_Concat, "concat")(
    OT_3_If, "if")(OT_NOP, "non")(OT_1_Abs, "abs")(OT_2_GT, ">")(OT_2_GET, ">=")(OT_2_LT, "<")(
    OT_2_LET, "<=")(OT_2_Neq, "!=")(OT_2_LAND, "&&")(OT_2_LOR, "||");

struct ast_print : public FuncHandlerBase {
    ostream& os;
    explicit ast_print(ostream& s = cout) : os(s) {}

    ostream& GetStream() { return os; }

    OperandType Func0(OpType op) const { assert(0); }

    OperandType Func1(OpType op, OperandType a1) const {
        OperandType res = FuncHandlerBase::Func1(op, a1);
        os << gs_op_map[op] << "(" << a1 << "):" << res << endl;
        return res;
    }

    OperandType Func2(OpType op, OperandType a1, OperandType a2) const {
        OperandType res = FuncHandlerBase::Func2(op, a1, a2);
        os << gs_op_map[op] << "(" << a1 << ", " << a2 << "):" << res << endl;
        return res;
    }

    OperandType Func3(OpType op, OperandType a1, OperandType a2, OperandType a3) const {
        OperandType res = FuncHandlerBase::Func3(op, a1, a2, a3);
        os << gs_op_map[op] << "(" << a1 << ", " << a2 << ", " << a3 << "):" << res << endl;
        return res;
    }

    OperandType Func4(
        OpType op, OperandType a1, OperandType a2, OperandType a3, OperandType a4) const {
        OperandType res = FuncHandlerBase::Func4(op, a1, a2, a3, a4);
        os << gs_op_map[op] << "(" << a1 << ", " << a2 << ", " << a3 << ", " << a4 << "):" << res
           << endl;
        return res;
    }
};

TEST(CalcParserTestSuit, TestCalcParserParsing) {
    std::string err;
    std::string exp;
    CalculatorParser calc;

    exp = "a + b * c - x / y * x + a % c & c ^ w | z";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    // white space testing, tab, space
    exp = "  a + b * c - x   / y * x + a % c & c ^ w | z      ";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a + * b * c";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp = "a * * b / c";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp = "a * * b / ";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp = "*a + b * c";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp = "a + (-b) * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a + (+b) * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a + -b * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a +- b * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a + +b * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a ++ b * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "(a) + (b) * (c)";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "(a + b) * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "if(a, b, c) * c";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "if(c == 'a', 'b', c) * 23";
    ASSERT_TRUE(calc.ParseExpression(exp, err)) << err;
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "if(c == \"a\", 'b', \"c\") * 23";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "c == a";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "c != a";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a=b";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp = "a>b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a>=b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a<b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "a<=b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "left(b, 23)";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "right(b, a)";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "concat(b, a)";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "left(b, )";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp = "left()";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_TRUE(err.empty());

    err = "";
    exp = "not_exist_func(b)";
    ASSERT_FALSE(calc.ParseExpression(exp, err));
    ASSERT_FALSE(err.empty());

    err = "";
    exp =
        "left(11==b, if(2, a | b, if(ab, if(left(w, (2)), a & 2, b ^ c))))"
        " + right(concat(a, b), a == b)";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_STREQ("", err.c_str());

    err = "";
    exp = "2.1 + abs(b)";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_TRUE(err.empty());

    err = "";
    exp = "a && b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_TRUE(err.empty());

    err = "";
    exp = "a || b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_TRUE(err.empty());

    err = "";
    exp = "a and b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_TRUE(err.empty());

    err = "";
    exp = "a or b";
    ASSERT_TRUE(calc.ParseExpression(exp, err));
    ASSERT_TRUE(err.empty());
}

static void run_test_case(CalculatorParser& calc, const string& s,
    const map<string, OperandType>& vars, const string& res_of_s, const OperandType& expect_res) {
    string err;
    FuncHandlerBase* handler = calc.GetHandler();
    ast_print* printer = dynamic_cast<ast_print*>(handler);
    ostringstream& oss = dynamic_cast<ostringstream&>(printer->GetStream());

    oss.str("");
    OperandType res = calc.GenValue(s, vars, err);

    cout << "expression to parse:" << s << endl
         << endl;
    cout << "produced byte-code:\n" << oss.str() << endl
         << "expected result:" << res_of_s << endl
         << endl;

    ASSERT_TRUE(err.empty()) << err;
    ASSERT_STREQ(res_of_s.c_str(), oss.str().c_str());
    ASSERT_TRUE(expect_res == res) << "actual value:" << res << ", expected:" << expect_res;
}

TEST(CalcParserTestSuit, TestCalcParserAst) {
    ostringstream oss;
    oss.precision(8);
    cout.precision(8);

    OperandType res;
    string res_of_s;

    map<string, OperandType> vars;

    ast_print print(oss);
    CalculatorParser calc(&print);

    // test case 1
    string s =
        "if(xyz==xyz, 11, 13) + abc + if(23, 32 % 5, 12 ^ 3)"
        " - 23 | 4 & 2 + 13335 * x - 3 / (x + y)";

    vars["x"] = 213;
    vars["y"] = 313;
    vars["abc"] = 2.3;

    res_of_s =
        "==(xyz, xyz):1\n"
        "if(1, 11, 13):11\n"
        "+(11, 2.3):13.3\n"
        "%(32, 5):2\n^(12, 3):15\nif(23, 2, 15):2\n"
        "+(13.3, 2):15.3\n-(15.3, 23):-7.7\n|(-7.7, 4):-3\n"
        "*(13335, 213):2840355\n+(2, 2840355):2840357\n+(213, 313):526\n"
        "/(3, 526):0.0057034221\n-(2840357, 0.0057034221):2840357\n&(-3, 2840357):2840356\n";

    run_test_case(calc, s, vars, res_of_s, 2840356);

    // test case 2
    vars["xx"] = "xyz";
    s = "if(xx==xyz, \"ab\", 23)";
    res_of_s =
        "==(xyz, xyz):1\n"
        "if(1, ab, 23):ab\n";

    run_test_case(calc, s, vars, res_of_s, "ab");

    // test case 3
    vars["x1"] = 1;
    vars["x2"] = 2;
    vars["x3"] = 3;
    vars["x4"] = 4;
    vars["x5"] = 5;
    vars["y1"] = 11;
    vars["y2"] = 12;
    vars["y3"] = 13;
    vars["s1"] = "six";
    vars["six"] = "s2ix";
    s =
        "if(seven66==(if(s1==\"six\", seven, five) + nn + \"233\"), 2, 4) + "
        "if(x2 + 3 * x1 - if(x4, 23, x3), y2 + y1 * x3, b2) + if(x5, x3, x4) * 2 + 23";

    res_of_s =
        "==(six, six):1\n"
        "if(1, seven, five):seven\n"
        "+(seven, nn):sevennn\n"
        "+(sevennn, 233):sevennn233\n"
        "==(seven66, sevennn233):0\n"
        "if(0, 2, 4):4\n"
        "*(3, 1):3\n"
        "+(2, 3):5\n"
        "if(4, 23, 3):23\n"
        "-(5, 23):-18\n"
        "*(11, 3):33\n"
        "+(12, 33):45\n"
        "if(-18, 45, b2):45\n"
        "+(4, 45):49\n"
        "if(5, 3, 4):3\n"
        "*(3, 2):6\n"
        "+(49, 6):55\n"
        "+(55, 23):78\n";

    run_test_case(calc, s, vars, res_of_s, 78);

    // test case 4
    s = "if(\"\", 23, if(\"a\", 32, 45))";
    res_of_s =
        "if(a, 32, 45):32\n"
        "if(, 23, 32):32\n";
    vars.clear();

    run_test_case(calc, s, vars, res_of_s, 32);

    // test case 5
    s =
        "if(a > b, 23, if(c != d, 32, 45)) + if (e >= f, 1, 3) + if (g <= n, 12, 5) + "
        "if(if(sx != sy, \"s12\", \"we\") >= if(sx2 > sy2, \"s14\", \"sw1\"),"
        " \"abc\" < \"efg\", \"aa\" <= \"aa\")";

    res_of_s =
        ">(23, 23):0\n"
        "!=(2, 3):1\n"
        "if(1, 32, 45):32\n"
        "if(0, 23, 32):32\n"
        ">=(12, 13):0\n"
        "if(0, 1, 3):3\n"
        "+(32, 3):35\n"
        "<=(5, 6):1\n"
        "if(1, 12, 5):12\n"
        "+(35, 12):47\n"
        "!=(ssx, ssy):1\n"
        "if(1, s12, we):s12\n"
        ">(sx2, sy2):0\n"
        "if(0, s14, sw1):sw1\n"
        ">=(s12, sw1):0\n"
        "<(abc, efg):1\n"
        "<=(aa, aa):1\n"
        "if(0, 1, 1):1\n"
        "+(47, 1):48\n";

    vars.clear();
    vars["a"] = 23;
    vars["b"] = 23;
    vars["c"] = 2;
    vars["d"] = 3;
    vars["e"] = 12;
    vars["f"] = 13;
    vars["g"] = 5;
    vars["n"] = 6;
    vars["sx"] = "ssx";
    vars["sy"] = "ssy";
    vars["sx2"] = "sx2";
    vars["sy2"] = "sy2";

    run_test_case(calc, s, vars, res_of_s, 48);

    // test case 6
    s = "right(left(some_string, 4), 3) + concat(left(x, 2), right(y, 3))";
    res_of_s =
        "left(abcdefg, 4):abcd\n"
        "right(abcd, 3):bcd\n"
        "left(x1234y, 2):x1\n"
        "right(y4567x, 3):67x\n"
        "concat(x1, 67x):x167x\n"
        "+(bcd, x167x):bcdx167x\n";

    oss.str("");
    vars.clear();
    vars["bcd"] = 23;
    vars["abcd"] = 233;
    vars["x167x"] = 433;
    vars["x"] = "x1234y";
    vars["y"] = "y4567x";
    vars["some_string"] = "abcdefg";

    run_test_case(calc, s, vars, res_of_s, "bcdx167x");

    // test case 7
    s = "if($xy=='abc', 23, \"xy'\")";
    res_of_s =
        "==(abcd, abc):0\n"
        "if(0, 23, xy'):xy'\n";

    vars.clear();
    vars["$xy"] = "abcd";

    run_test_case(calc, s, vars, res_of_s, "xy'");

    // test case 8
    s = "2 + abs(a - 23) + a";
    res_of_s = "-(-3, 23):-26\nabs(-26):26\n+(2, 26):28\n+(28, -3):25\n";
    vars.clear();
    vars["a"] = -3;

    run_test_case(calc, s, vars, res_of_s, 25);
}

void test_relation_and_logical() {
    // test operator precedence.
    // 1. *, /, %
    // 2. +, -,
    // 3. >=, >, <=, <, ==, !=,
    // 4. &, |, ^,
    // 5. &&, ||

    ostringstream oss;
    oss.precision(8);
    cout.precision(8);

    OperandType res;
    string s, res_of_s;

    map<string, OperandType> vars;

    ast_print print(oss);
    CalculatorParser calc(&print);

    vars["a"] = -3;

    s = "2 + 3 * a - 12/2%4 > 2 + 3";
    res_of_s =
        "*(3, -3):-9\n+(2, -9):-7\n/(12, 2):6\n"
        "%(6, 4):2\n-(-7, 2):-9\n+(2, 3):5\n>(-9, 5):0\n";
    run_test_case(calc, s, vars, res_of_s, 0);

    s = "2 + 3 * a - 12/2%4 >= 2 + 3";
    res_of_s =
        "*(3, -3):-9\n+(2, -9):-7\n/(12, 2):6\n"
        "%(6, 4):2\n-(-7, 2):-9\n+(2, 3):5\n>=(-9, 5):0\n";
    run_test_case(calc, s, vars, res_of_s, 0);

    s = "2 + 3 * a - 12/2%4 < 2 + 3";
    res_of_s =
        "*(3, -3):-9\n+(2, -9):-7\n/(12, 2):6\n"
        "%(6, 4):2\n-(-7, 2):-9\n+(2, 3):5\n<(-9, 5):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "2 + 3 * a - 12/2%4 <= 2 + 3";
    res_of_s =
        "*(3, -3):-9\n+(2, -9):-7\n/(12, 2):6\n"
        "%(6, 4):2\n-(-7, 2):-9\n+(2, 3):5\n<=(-9, 5):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "2 + 3 * a - 12/2%4 == 2 + 3";
    res_of_s =
        "*(3, -3):-9\n+(2, -9):-7\n/(12, 2):6\n"
        "%(6, 4):2\n-(-7, 2):-9\n+(2, 3):5\n==(-9, 5):0\n";
    run_test_case(calc, s, vars, res_of_s, 0);

    s = "2 + 3 * a - 12/2%4 != 2 + 3";
    res_of_s =
        "*(3, -3):-9\n+(2, -9):-7\n/(12, 2):6\n"
        "%(6, 4):2\n-(-7, 2):-9\n+(2, 3):5\n!=(-9, 5):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "2 >= 3 & 4";
    res_of_s = ">=(2, 3):0\n&(0, 4):0\n";
    run_test_case(calc, s, vars, res_of_s, 0);

    s = "2 >= 3 | 4";
    res_of_s = ">=(2, 3):0\n|(0, 4):4\n";
    run_test_case(calc, s, vars, res_of_s, 4);

    s = "3 >= 3";
    res_of_s = ">=(3, 3):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "3 > 3";
    res_of_s = ">(3, 3):0\n";
    run_test_case(calc, s, vars, res_of_s, 0);

    s = "3 && 2 ";
    res_of_s = "&&(3, 2):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "3 | 4 and 2 ^ 1";
    res_of_s = "|(3, 4):7\n^(2, 1):3\n&&(7, 3):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "3 & 4 && 2 ^ 1";
    res_of_s = "&(3, 4):0\n^(2, 1):3\n&&(0, 3):0\n";
    run_test_case(calc, s, vars, res_of_s, 0);

    s = "3 | 4 || 2 ^ 1";
    res_of_s = "|(3, 4):7\n^(2, 1):3\n||(7, 3):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "3 | 4 or 2 ^ 1";
    res_of_s = "|(3, 4):7\n^(2, 1):3\n||(7, 3):1\n";
    run_test_case(calc, s, vars, res_of_s, 1);

    s = "if(2 > 1 and \"abc\" == \"abc\", 23, \"ab\")";
    res_of_s = ">(2, 1):1\n==(abc, abc):1\n&&(1, 1):1\nif(1, 23, ab):23\n";
    run_test_case(calc, s, vars, res_of_s, 23);

    s = "if(2 > 1 and a == \"abc\" or b == 3, \"ab\", 23)";
    res_of_s =
        ">(2, 1):1\n==(ef, abc):0\n&&(1, 0):0\n"
        "==(3, 3):1\n||(0, 1):1\nif(1, ab, 23):ab\n";
    vars["a"] = "ef";
    vars["b"] = 3;
    run_test_case(calc, s, vars, res_of_s, "ab");

    s = "if(2 > 1 and (a == \"abc\" or b == 3), \"ab\", 23)";
    res_of_s =
        ">(2, 1):1\n==(ef, abc):0\n==(4, 3):0\n||(0, 0):0\n"
        "&&(1, 0):0\nif(0, ab, 23):23\n";
    vars["a"] = "ef";
    vars["b"] = 4;
    run_test_case(calc, s, vars, res_of_s, 23);

    s = "if(2 > 3 and (a == \"abc\" or b == 3), \"ab\", 23)";
    res_of_s =
        ">(2, 3):0\n==(ef, abc):0\n==(3, 3):1\n||(0, 1):1\n"
        "&&(0, 1):0\nif(0, ab, 23):23\n";
    vars["a"] = "ef";
    vars["b"] = 3;
    run_test_case(calc, s, vars, res_of_s, 23);

    s = "if(2 > 3 and a == \"abc\" or b == 3, \"ab\", 23)";
    res_of_s =
        ">(2, 3):0\n==(ef, abc):0\n&&(0, 0):0\n==(3, 3):1\n"
        "||(0, 1):1\nif(1, ab, 23):ab\n";
    run_test_case(calc, s, vars, res_of_s, "ab");
}

TEST(CalcParserTestSuit, TestCalcExpLogical) { test_relation_and_logical(); }

TEST(CalcParserTestSuit, TestCalcHandlerBase) {
    OperandType res;
    FuncHandlerBase fb;

    res = fb.Func1(OT_1_Neg, 2);
    ASSERT_DOUBLE_EQ(-2, boost::get<double>(res));

    res = fb.Func1(OT_1_Neg, -2);
    ASSERT_DOUBLE_EQ(2, boost::get<double>(res));

    res = fb.Func1(OT_1_Pos, 2);
    ASSERT_DOUBLE_EQ(2, boost::get<double>(res));

    res = fb.Func1(OT_1_Pos, -2);
    ASSERT_DOUBLE_EQ(-2, boost::get<double>(res));

    res = fb.Func1(OT_1_Abs, -2);
    ASSERT_DOUBLE_EQ(2, boost::get<double>(res));

    res = fb.Func1(OT_1_Abs, 2);
    ASSERT_DOUBLE_EQ(2, boost::get<double>(res));

    res = fb.Func1(OT_1_Abs, 2.3);
    ASSERT_DOUBLE_EQ(2.3, boost::get<double>(res));

    res = fb.Func1(OT_1_Abs, -2.3);
    ASSERT_DOUBLE_EQ(2.3, boost::get<double>(res));

    res = fb.Func3(OT_3_If, 0, 23, "23");
    ASSERT_STREQ("23", boost::get<string>(res).c_str());

    res = fb.Func3(OT_3_If, 2, 23, "23");
    ASSERT_DOUBLE_EQ(23, boost::get<double>(res));

    res = fb.Func3(OT_3_If, 2, 23, 232);
    ASSERT_DOUBLE_EQ(23, boost::get<double>(res));

    res = fb.Func3(OT_3_If, 0, 23, 232);
    ASSERT_DOUBLE_EQ(232, boost::get<double>(res));

    res = fb.Func3(OT_3_If, "", 23, 232);
    ASSERT_DOUBLE_EQ(232, boost::get<double>(res));

    res = fb.Func3(OT_3_If, "aa", 23, 232);
    ASSERT_DOUBLE_EQ(23, boost::get<double>(res));
}

TEST(CalcParserTestSuit, TestCalcArithmetic) {
    OperandType res;
    FuncHandlerBase fb;

    res = fb.Func2(OT_2_Add, 2, 3);
    ASSERT_DOUBLE_EQ(5, boost::get<double>(res));

    res = fb.Func2(OT_2_Sub, -2, 3);
    ASSERT_DOUBLE_EQ(-5, boost::get<double>(res));

    res = fb.Func2(OT_2_Add, -2, 3);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_Add, "abc", "xy3");
    ASSERT_STREQ("abcxy3", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Add, "", "xy3");
    ASSERT_STREQ("xy3", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Add, "abc", "");
    ASSERT_STREQ("abc", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Mul, 2, 3);
    ASSERT_DOUBLE_EQ(6, boost::get<double>(res));

    res = fb.Func2(OT_2_Div, 6, 3);
    ASSERT_DOUBLE_EQ(2, boost::get<double>(res));

    res = fb.Func2(OT_2_Mod, 6, 3);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_Mod, 6, 4);
    ASSERT_DOUBLE_EQ(2, boost::get<double>(res));
}

TEST(CalcParserTestSuit, TestCalcLogicalHandler) {
    OperandType res;
    FuncHandlerBase fb;

    res = fb.Func2(OT_2_And, 6, 4);
    ASSERT_DOUBLE_EQ(6 & 4, boost::get<double>(res));
    res = fb.Func2(OT_2_Xor, 6, 4);
    ASSERT_DOUBLE_EQ(6 ^ 4, boost::get<double>(res));

    res = fb.Func2(OT_2_Or, 6, 4);
    ASSERT_DOUBLE_EQ(6 | 4, boost::get<double>(res));

    res = fb.Func2(OT_2_Eq, 23, 23);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_Eq, 123, 23);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_Eq, "123", "23");
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_Eq, 23, "23");
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_Neq, 23, 23);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_Neq, 22, 23);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_Neq, "abc", "xy");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_Neq, "abc", "abc");
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_GT, "abc", "ab");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_GT, "bbc", "ab");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_GT, "bbc", "rb");
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_GT, 2, 1);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_GT, 2, 2);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_GT, 2, 121);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_GET, 2, 121);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_GET, 121, 121);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, 1, 2);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, 0, 0);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, 2, 0);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, 0, 2);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, "a", 2);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, "ab", "s2");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LAND, "", "s2");
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, 0, 0);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, 2, 0);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, 0, 2);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, "a", 2);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, "ab", "s2");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, "", "s2");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, 0, "s2");
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, "", 2);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LOR, "", "");
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LT, 2, 1);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LT, 2, 2);
    ASSERT_DOUBLE_EQ(0, boost::get<double>(res));

    res = fb.Func2(OT_2_LT, 2, 121);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LET, 2, 121);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));

    res = fb.Func2(OT_2_LET, 121, 121);
    ASSERT_DOUBLE_EQ(1, boost::get<double>(res));
}

TEST(CalcParserTestSuit, TestCalcStringOp) {
    OperandType res;
    FuncHandlerBase fb;

    res = fb.Func2(OT_2_Left, "abcd", 2);
    ASSERT_STREQ("ab", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Left, "abcd", 6);
    ASSERT_STREQ("abcd", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Right, "abcd", 2);
    ASSERT_STREQ("cd", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Right, "abcd", 6);
    ASSERT_STREQ("abcd", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Concat, "abcd", "ef");
    ASSERT_STREQ("abcdef", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Concat, "", "ef");
    ASSERT_STREQ("ef", boost::get<string>(res).c_str());

    res = fb.Func2(OT_2_Concat, "v112", "");
    ASSERT_STREQ("v112", boost::get<string>(res).c_str());
}

