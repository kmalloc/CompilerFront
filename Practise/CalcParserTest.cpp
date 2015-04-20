#include <precompile.h>
#include <boost/assign.hpp>

#include "CalcParser.h"

using namespace std;
using namespace CalcParser;

static std::map<OpType, std::string> gs_op_map = boost::assign::map_list_of(OT_2_Eq, "==")
    (OT_2_Add, "+")(OT_2_Sub, "-")(OT_2_Mul, "*")(OT_2_Div, "/")
    (OT_2_Mod, "%")(OT_2_Xor, "^")(OT_2_And, "&")(OT_2_Or, "|")
    (OT_2_Left, "left")(OT_2_Right, "right")(OT_2_Concat, "concat")
    (OT_3_If, "if")(OT_NOP, "non");

struct ast_print: public FuncHandlerBase
{
    ostream& os;
    explicit ast_print(ostream& s = cout): os(s) {}

    OperandType Func0(OpType op) const
    {
        assert(0);
    }

    OperandType Func1(OpType op, OperandType a1) const
    {
        OperandType res = FuncHandlerBase::Func1(op, a1);
        os << gs_op_map[op] << "(" << a1 << "):" << res << endl;
        return res;
    }

    OperandType Func2(OpType op, OperandType a1, OperandType a2) const
    {
        OperandType res = FuncHandlerBase::Func2(op, a1, a2);
        os << gs_op_map[op] << "(" << a1 << ", " << a2 << "):" << res << endl;
        return res;
    }

    OperandType Func3(OpType op, OperandType a1,
            OperandType a2, OperandType a3) const
    {
        OperandType res = FuncHandlerBase::Func3(op, a1, a2, a3);
        os << gs_op_map[op] << "(" << a1 << ", " << a2 << ", " << a3 << "):" << res << endl;
        return res;
    }

    OperandType Func4(OpType op, OperandType a1,
            OperandType a2, OperandType a3, OperandType a4) const
    {
        OperandType res = FuncHandlerBase::Func4(op, a1, a2, a3, a4);
        os << gs_op_map[op] << "(" << a1 << ", " << a2 << ", " <<
            a3 << ", " << a4 << "):" << res << endl;
        return res;
    }
};

TEST(CalcParserTestSuit, TestCalcParserAst)
{
    map<string, OperandType> vars;
    string s = "if(xyz==xyz, 11, 13) + abc + if(23, 32 % 5, 12 ^ 3)"
        " - 23 | 4 & 2 + 13335 * x - 3 / (x + y)";

    vars["x"] = 213;
    vars["y"] = 313;
    vars["abc"] = 2.3;

    string res_of_s = "==(xyz, xyz):1\n" "if(1, 11, 13):11\n" "+(11, 2.3):13.3\n"
        "%(32, 5):2\n" "^(12, 3):15\n" "if(23, 2, 15):2\n" "+(13.3, 2):15.3\n"
        "|(23, 4):23\n" "&(23, 2):2\n" "-(15.3, 2):13.3\n" "*(13335, 213):2840355\n"
        "+(13.3, 2840355):2840368.3\n" "+(213, 313):526\n" "/(3, 526):0.0057034221\n"
        "-(2840368.3, 0.0057034221):2840368.3\n";

    string err;
    OperandType res;
    ostringstream oss;
    oss.precision(8);
    cout.precision(8);

    ast_print print(oss);
    CalculatorParser calc(&print);

    res = calc.GenValue(s, vars, err);

    cout << "expression to parse:" << s << endl << endl;
    cout << "byte code:\n" << oss.str() << endl << "final result:" << res << endl << endl;
    ASSERT_TRUE(err.empty()) << err;
    ASSERT_STREQ(oss.str().c_str(), res_of_s.c_str());
    ASSERT_DOUBLE_EQ(2840368.2942965776, boost::get<double>(res));

    vars["xx"] = "xyz";
    s = "if(xx==xyz, \"ab\", oo)";
    res_of_s = "==(xyz, xyz):1\n" "if(1, ab, oo):ab\n";

    err.clear();
    oss.str("");
    res = calc.GenValue(s, vars, err);
    cout << "expression to parse:" << s << endl << endl;
    cout << "byte code:\n" << oss.str() << endl << "final result:" << res << endl << endl;
    ASSERT_TRUE(err.empty()) << err;
    ASSERT_STREQ(oss.str().c_str(), res_of_s.c_str());
    ASSERT_STREQ("ab", boost::get<string>(res).c_str());

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
    s = "if(seven66==(if(s1==\"six\", seven, five) + nn + \"233\"), 2, 4) + "
        "if(x2 + 3 * x1 - if(x4, 23, x3), y2 + y1 * x3, b2) + if(x5, x3, x4) * 2 + 23";

    res_of_s = "==(six, six):1\n" "if(1, seven, five):seven\n" "+(seven, nn):sevennn\n"
        "+(sevennn, 233):sevennn233\n" "==(seven66, sevennn233):0\n"
        "if(0, 2, 4):4\n" "*(3, 1):3\n" "+(2, 3):5\n" "if(4, 23, 3):23\n"
        "-(5, 23):-18\n" "*(11, 3):33\n" "+(12, 33):45\n" "if(-18, 45, b2):45\n"
        "+(4, 45):49\n" "if(5, 3, 4):3\n" "*(3, 2):6\n" "+(49, 6):55\n" "+(55, 23):78\n";

    err.clear();
    oss.str("");
    res = calc.GenValue(s, vars, err);
    cout << "expression to parse:" << s << endl << endl;
    cout << "byte code:\n" << oss.str() << endl << "final result:" << res << endl << endl;
    ASSERT_TRUE(err.empty()) << err;
    ASSERT_STREQ(oss.str().c_str(), res_of_s.c_str());
    ASSERT_DOUBLE_EQ(78, boost::get<double>(res));

    s = "if(\"\", 23, if(\"a\", 32, 45))";
    res_of_s = "if(a, 32, 45):32\n" "if(, 23, 32):32\n";
    vars.clear();
    oss.str("");
    res = calc.GenValue(s, vars, err);
    cout << "expression to parse:" << s << endl << endl;
    cout << "byte code:\n" << oss.str() << endl << "final result:" << res << endl << endl;
    ASSERT_TRUE(err.empty()) << err;
    ASSERT_STREQ(oss.str().c_str(), res_of_s.c_str());
    ASSERT_DOUBLE_EQ(32, boost::get<double>(res));

    s = "right(left(some_string, 4), 3) + concat(left(x, 2), right(y, 3))";
    res_of_s = "left(abcdefg, 4):abcd\n" "right(abcd, 3):bcd\n" "left(x1234y, 2):x1\n"
        "right(y4567x, 3):67x\n" "concat(x1, 67x):x167x\n" "+(bcd, x167x):bcdx167x\n";

    oss.str("");
    vars.clear();
    vars["bcd"] = 23;
    vars["abcd"] = 233;
    vars["x167x"] = 433;
    vars["x"] = "x1234y";
    vars["y"] = "y4567x";
    vars["some_string"] = "abcdefg";
    res = calc.GenValue(s, vars, err);
    cout << "expression to parse:" << s << endl << endl;
    cout << "byte code:\n" << oss.str() << endl << "final result:" << res << endl << endl;
    ASSERT_TRUE(err.empty()) << err;
    ASSERT_STREQ(oss.str().c_str(), res_of_s.c_str());
    ASSERT_STREQ("bcdx167x", boost::get<string>(res).c_str());
}

TEST(CalcParserTestSuit, TestCalcHandlerBase)
{
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

