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

    const char* txt = "a = 23 - a * (2 + c) + 2.2\n a + (c) + ~2 * 3";
    /*
                |+|                   |  +  |
                /  \                 /      \
               /    \               /        \
             |-|   |2.2|          |+|        |*|
             / \                  / \        /  \
            /   \                /   \      /    \
          |23|  |*|            |a|  |c|    |~|   |3|
                / \                        /
               /   \                      /
             |a|   |+|                   |2|
                   / \
                  /   \
                |2|   |c|
   */


    Parser p(txt, "dummy.cc");

    p.StartParsing();
    std::vector<AstBasePtr>& res = p.GetResult();

    ASSERT_EQ(2, res.size());

    AstBasePtr ep1 = res[0];
    AstBasePtr ep2 = res[1];

    ASSERT_EQ(AST_OP_BINARY, ep1->GetType());
    ASSERT_EQ(AST_OP_BINARY, ep2->GetType());

    AstBinaryExpPtr bsp1 = boost::dynamic_pointer_cast<AstBinaryExp>(ep1);
    AstBinaryExpPtr bsp2 = boost::dynamic_pointer_cast<AstBinaryExp>(ep2);

    ASSERT_EQ(TOK_AS, bsp1->GetOpType());
    ASSERT_EQ(TOK_ADD, bsp2->GetOpType());

    AstBasePtr sp1 = bsp1->GetLeftOperand();
    AstBasePtr sp2 = bsp1->GetRightOperand();

    ASSERT_EQ(AST_VAR, sp1->GetType());
    ASSERT_EQ(AST_OP_BINARY, sp2->GetType());

    AstBinaryExpPtr bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(sp2);
    ASSERT_EQ(TOK_ADD, bin_sp->GetOpType());

    sp1 = bin_sp->GetLeftOperand();
    sp2 = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_OP_BINARY, sp1->GetType());
    ASSERT_EQ(AST_FLOAT, sp2->GetType());

    AstFloatExpPtr flt_sp = boost::dynamic_pointer_cast<AstFloatExp>(sp2);
    ASSERT_DOUBLE_EQ(2.2, flt_sp->GetValue());

    bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(sp1);
    ASSERT_EQ(TOK_SUB, bin_sp->GetOpType());

    sp1 = bin_sp->GetLeftOperand();
    sp2 = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_INT, sp1->GetType());
    ASSERT_EQ(AST_OP_BINARY, sp2->GetType());

    AstIntExpPtr int_sp = boost::dynamic_pointer_cast<AstIntExp>(sp1);
    ASSERT_EQ(23, int_sp->GetValue());

    bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(sp2);
    ASSERT_EQ(TOK_MUL, bin_sp->GetOpType());

    sp1 = bin_sp->GetLeftOperand();
    sp2 = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_VAR, sp1->GetType());
    ASSERT_EQ(AST_OP_BINARY, sp2->GetType());

    bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(sp2);
    ASSERT_EQ(TOK_ADD, bin_sp->GetOpType());

    sp1 = bin_sp->GetLeftOperand();
    sp2 = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_INT, sp1->GetType());
    ASSERT_EQ(AST_VAR, sp2->GetType());

    // 2th expression
    bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(bsp2);
    ASSERT_EQ(TOK_ADD, bin_sp->GetOpType());

    sp1 = bin_sp->GetLeftOperand();
    sp2 = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_OP_BINARY, sp1->GetType());
    ASSERT_EQ(AST_OP_BINARY, sp2->GetType());

    bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(sp1);
    ASSERT_EQ(TOK_ADD, bin_sp->GetOpType());

    AstBasePtr sp = bin_sp->GetLeftOperand();
    ASSERT_EQ(AST_VAR, sp->GetType());

    sp = bin_sp->GetRightOperand();
    ASSERT_EQ(AST_VAR, sp->GetType());

    bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(sp2);
    ASSERT_EQ(TOK_MUL, bin_sp->GetOpType());

    sp1 = bin_sp->GetLeftOperand();
    sp2 = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_OP_UNARY, sp1->GetType());
    ASSERT_EQ(AST_INT, sp2->GetType());

    int_sp = boost::dynamic_pointer_cast<AstIntExp>(sp2);
    ASSERT_EQ(3, int_sp->GetValue());

    AstUnaryExpPtr usp = boost::dynamic_pointer_cast<AstUnaryExp>(sp1);
    ASSERT_EQ(TOK_INV, usp->GetOpType());

    sp = usp->GetOperand();
    ASSERT_EQ(AST_INT, sp->GetType());

    int_sp = boost::dynamic_pointer_cast<AstIntExp>(sp);
    ASSERT_EQ(2, int_sp->GetValue());
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

    // 2th arr definition
    vpl = bp2->GetLeftOperand();
    vpr = bp2->GetRightOperand();

    ASSERT_EQ(AST_VAR, vpl->GetType());
    ASSERT_EQ(AST_ARR, vpr->GetType());

    var_sp = boost::dynamic_pointer_cast<AstVarExp>(vpl);
    ASSERT_STREQ("b", var_sp->GetName().c_str());

    arr_sp = boost::dynamic_pointer_cast<AstArrayExp>(vpr);
    arr = arr_sp->GetArray();

    ASSERT_EQ(0, arr.size());

    // 3th arr definition
    vpl = bp3->GetLeftOperand();
    vpr = bp3->GetRightOperand();

    ASSERT_EQ(AST_VAR, vpl->GetType());
    ASSERT_EQ(AST_OP_BINARY, vpr->GetType());

    var_sp = boost::dynamic_pointer_cast<AstVarExp>(vpl);
    ASSERT_STREQ("c", var_sp->GetName().c_str());

    AstBinaryExpPtr bin_sp = boost::dynamic_pointer_cast<AstBinaryExp>(vpr);
    ASSERT_EQ(TOK_ADD, bin_sp->GetOpType());

    AstBasePtr spl = bin_sp->GetLeftOperand();
    AstBasePtr spr = bin_sp->GetRightOperand();

    ASSERT_EQ(AST_ARR_INDEX, spl->GetType());
    ASSERT_EQ(AST_ARR_INDEX, spr->GetType());

    AstArrayIndexExpPtr ispl = boost::dynamic_pointer_cast<AstArrayIndexExp>(spl);
    AstArrayIndexExpPtr ispr = boost::dynamic_pointer_cast<AstArrayIndexExp>(spr);

    ASSERT_STREQ("a", ispl->GetArrayName().c_str());
    ASSERT_STREQ("b", ispr->GetArrayName().c_str());

    AstBasePtr inp1 = ispl->GetIndexAst();
    AstBasePtr inp2 = ispr->GetIndexAst();
    ASSERT_EQ(AST_INT, inp1->GetType());
    ASSERT_EQ(AST_INT, inp2->GetType());

    AstIntExpPtr int_sp1 = boost::dynamic_pointer_cast<AstIntExp>(inp1);
    AstIntExpPtr int_sp2 = boost::dynamic_pointer_cast<AstIntExp>(inp2);

    ASSERT_EQ(0, int_sp1->GetValue());
    ASSERT_EQ(1, int_sp2->GetValue());
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

