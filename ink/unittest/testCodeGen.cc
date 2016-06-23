#include "gtest/gtest.h"

#include "OpCode.h"

#include "Ast.h"
#include "Parser.h"

using namespace ink;

TEST(ink_test_suit, test_code_gen_binary_op)
{
    const char* txt = "a = 23";
}

TEST(ink_test_suit, test_code_gen_scope)
{
   /*
    * {
    *    // scope
    *    {
    *       // scope 1
    *    }
    *
    *    {
    *       // scope 2
    *       {
    *          // scope_2_1
    *       }
    *    }
    * }
    *
    */
   AstScopeStatementExpPtr scp_1(std::make_shared<AstScopeStatementExp>(std::vector<AstBasePtr>()));
   AstScopeStatementExpPtr scp_2_1(std::make_shared<AstScopeStatementExp>(std::vector<AstBasePtr>()));
   AstScopeStatementExpPtr scp_2(std::make_shared<AstScopeStatementExp>(std::vector<AstBasePtr>({scp_2_1})));
   AstScopeStatementExpPtr scp(std::make_shared<AstScopeStatementExp>(std::vector<AstBasePtr>({scp_1, scp_2})));

   AstWalker walker;
   scp->Accept(walker);

   // TODO: verify result
}

TEST(ink_test_suit, test_code_gen_if_else)
{
}

TEST(ink_test_suit, test_code_gen_while)
{
}

