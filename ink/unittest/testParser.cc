#include "gtest/gtest.h"

#include "Parser.h"
#include "AstVisitor.h"

using namespace ink;

class BinVisitor: public VisitorBase
{
    public:
        typedef void (*VisitProc)(AstBinaryExp*);

        virtual void Visit(AstBinaryExp* t)
        {
            proc_(t);
        }

        void SetProc(VisitProc proc) { proc_ = proc; }

    private:
        VisitProc proc_;
};

TEST(ink_test_suit, test_var_definition)
{
    const char* txt = "a = 23 b = a + 2.2";
    Parser p(txt, "dummy.cc");

    p.StartParsing();
    std::vector<AstBasePtr>& res = p.GetResult();

    ASSERT_EQ(2, res.size());

    AstBasePtr pt = res[0];
    ASSERT_EQ(AST_OP_BINARY, pt->GetType());

    AstBinaryExpPtr bpt = boost::dynamic_pointer_cast<AstBinaryExp>(pt);

    ASSERT_EQ(TOK_AS, bpt->GetOpType());

    AstBasePtr lhs = bpt->GetLeftOperand();
    ASSERT_EQ(AST_VAR, lhs->GetType());
    AstVarExpPtr spl = boost::dynamic_pointer_cast<AstVarExp>(lhs);
    ASSERT_STREQ("a", spl->GetName().c_str());

    AstBasePtr rhs = bpt->GetRightOperand();
    ASSERT_EQ(AST_INT, rhs->GetType());
    AstIntExpPtr spr = boost::dynamic_pointer_cast<AstIntExp>(rhs);
    ASSERT_EQ(23, spr->GetValue());

    // 2th expression
    AstBasePtr pt2 = res[1];
    ASSERT_EQ(AST_OP_BINARY, pt2->GetType());

    AstBinaryExpPtr bpt2 = boost::dynamic_pointer_cast<AstBinaryExp>(pt2);

    ASSERT_EQ(TOK_AS, bpt2->GetOpType());

    AstBasePtr lhs2 = bpt2->GetLeftOperand();
    ASSERT_EQ(AST_VAR, lhs2->GetType());
    AstVarExpPtr spl2 = boost::dynamic_pointer_cast<AstVarExp>(lhs2);
    ASSERT_STREQ("b", spl2->GetName().c_str());

    AstBasePtr rhs2 = bpt2->GetRightOperand();
    ASSERT_EQ(AST_OP_BINARY, rhs2->GetType());
    AstBinaryExpPtr spr2 = boost::dynamic_pointer_cast<AstBinaryExp>(rhs2);

    ASSERT_EQ(AST_OP_BINARY, spr2->GetType());
    ASSERT_EQ(TOK_ADD, spr2->GetOpType());

    AstBasePtr op1 = spr2->GetLeftOperand();
    AstBasePtr op2 = spr2->GetRightOperand();

    ASSERT_EQ(AST_VAR, op1->GetType());
    AstVarExpPtr var_op1 = boost::dynamic_pointer_cast<AstVarExp>(op1);
    ASSERT_STREQ("a", var_op1->GetName().c_str());

    ASSERT_EQ(AST_FLOAT, op2->GetType());
    AstFloatExpPtr float_op2 = boost::dynamic_pointer_cast<AstFloatExp>(op2);
    ASSERT_DOUBLE_EQ(2.2, float_op2->GetValue());
}

TEST(ink_test_suit, test_var_calc)
{
    // binary & unary operator

    const char* txt = "a = 23 - a * (2 + c) + 2.2";
    Parser p(txt, "dummy.cc");

    p.StartParsing();
    std::vector<AstBasePtr>& res = p.GetResult();

    ASSERT_EQ(1, res.size());
}

TEST(ink_test_suit, test_array_definition)
{
    const char* txt = "a = [2, 3, \"abc\"] b = [] c = a[0] + b[1]";
    Parser p(txt, "dummy.cc");

    p.StartParsing();
    std::vector<AstBasePtr>& res = p.GetResult();

    ASSERT_EQ(3, res.size());

    ASSERT_EQ(AST_OP_BINARY, res[0]->GetType());
    ASSERT_EQ(AST_OP_BINARY, res[1]->GetType());
    ASSERT_EQ(AST_OP_BINARY, res[2]->GetType());

    AstBinaryExpPtr bp1 = boost::dynamic_pointer_cast<AstBinaryExp>(res[0]);
    AstBinaryExpPtr bp2 = boost::dynamic_pointer_cast<AstBinaryExp>(res[1]);
    AstBinaryExpPtr bp3 = boost::dynamic_pointer_cast<AstBinaryExp>(res[2]);

    ASSERT_EQ(TOK_AS, bp1->GetOpType());
    ASSERT_EQ(TOK_AS, bp2->GetOpType());
    ASSERT_EQ(TOK_AS, bp3->GetOpType());

    AstBasePtr vpl = bp1->GetLeftOperand();
    AstBasePtr vpr = bp1->GetRightOperand();

    ASSERT_EQ(AST_VAR, vpl->GetType());
    ASSERT_EQ(AST_ARR, vpr->GetType());

    AstVarExpPtr var_sp = boost::dynamic_pointer_cast<AstVarExp>(vpl);
    ASSERT_STREQ("a", var_sp->GetName().c_str());

    AstArrayExpPtr arr_sp = boost::dynamic_pointer_cast<AstArrayExp>(vpr);
    std::vector<AstBasePtr> arr = arr_sp->GetArray();

    ASSERT_EQ(3, arr.size());
    ASSERT_EQ(AST_INT, arr[0]->GetType());
    ASSERT_EQ(AST_INT, arr[1]->GetType());
    ASSERT_EQ(AST_STRING, arr[2]->GetType());

    AstIntExpPtr int_sp = boost::dynamic_pointer_cast<AstIntExp>(arr[0]);
    ASSERT_EQ(2, int_sp->GetValue());

    int_sp = boost::dynamic_pointer_cast<AstIntExp>(arr[1]);
    ASSERT_EQ(3, int_sp->GetValue());

    AstStringExpPtr str_sp = boost::dynamic_pointer_cast<AstStringExp>(arr[2]);
    ASSERT_STREQ("abc", str_sp->GetValue().c_str());


    vpl = bp2->GetLeftOperand();
    vpr = bp2->GetRightOperand();

    ASSERT_EQ(AST_VAR, vpl->GetType());
    ASSERT_EQ(AST_ARR, vpr->GetType());

    var_sp = boost::dynamic_pointer_cast<AstVarExp>(vpl);
    ASSERT_STREQ("b", var_sp->GetName().c_str());

    arr_sp = boost::dynamic_pointer_cast<AstArrayExp>(vpr);
    arr = arr_sp->GetArray();

    ASSERT_EQ(0, arr.size());
}

TEST(ink_test_suit, test_array_indexing)
{
}

TEST(ink_test_suit, test_if_statement)
{
}

TEST(ink_test_suit, test_while_statement)
{
}

TEST(ink_test_suit, test_func_declaration)
{
}

TEST(ink_test_suit, test_func_definition)
{
}

TEST(ink_test_suit, test_func_call)
{
}

