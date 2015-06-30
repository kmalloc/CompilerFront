#include "Parser.h"
#include "AbstractSynTree.h"

namespace ink {

AstBase* Parser::ParseIntExp()
{
    AstBase* ret = new AstIntExp(lex_.GetIntVal());
    lex_.ConsumeCurToken();
    return ret;
}

AstBase* Parser::ParseFloatExp()
{
    AstBase* ret = new AstFloatExp(lex_.GetFloatVal());
    lex_.ConsumeCurToken();
    return ret;
}

AstBase* Parser::ParseParenExp()
{
    // consume '('
    lex_.ConsumeCurToken();

    // parse main expression
    AstBase* ret = ParseExpression();
    if (!ret) return NULL;

    if (lex_.GetCurToken() != TOK_PAREN_RIGHT) return ReportError("expected ')'");

    // consume ')'
    lex_.ConsumeCurToken();
    return ret;
}

AstBase* Parser::ParseStringExp()
{
    // consume left "
    lex_.ConsumeCurToken();

    AstBase* ret = new AstStringExp(lex_.GetCurToken());
    if (!ret) return ReportError("invalid string literal");

    lex_.ConsumeCurToken();
    if (lex_.GetCurToken() != TOK_QUO) return ReportError("expected '\"'");

    // consume '"'
    lex_.ConsumeCurToken();
    return ret;
}

AstBase* Parser::ParseFuncCallExp(const std::string& name)
{
    std::vector<AstBase*> args;
    if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
    {
        while (true)
        {
            AstBase* arg = ParseExpression();
            if (!arg) return 0;

            args.push_back(arg);
            if (lex_.GetCurToken() == TOK_PAREN_RIGHT) break;

            if (lex.GetCurToken() != TOK_COMA)
            {
                return ReportError("expected ')' or ',' in function argument list");
            }

            // consume ','
            lex_.ConsumeCurToken();
        }
    }

    // consume ')'
    lex_.ConsumeCurToken();
    return new AstFuncCallExp(name, args);
}

AstBase* Parser::ParseArrIndexExp(const std::string& name)
{
    AstBase* index = ParseExpression();
    if (!index) return 0;

    if (lex_.GetCurToken() != TOK_IND_RIGHT)
    {
        return ReportError("expected ']' for array indexing expression");
    }

    lex_.ConsumeCurToken();
    return new AstArrayIndexExp(name, index);
}

AstBase* Parser::ParseIdentifierExp()
{
    std::string name = lex_.GetStringVal();

    // consume name
    lex_.ConsumeCurToken();
    TokenType tok = lex_.GetCurToken();
    if (tok != TOK_PAREN_LEFT && tok != TOK_IND_LEFT) return new AstVariableExp(name);

    // consume '(' or '['
    lex_.ConsumeCurToken();

    if (tok == TOK_PAREN_LEFT) return ParseFuncCallExp(name);

    return ParseArrIndexExp(name);
}

AstBase* Parse::ParseUaryExp(TokenType op)
{
    // consume unary operator('!' or '~')
    lex_.ConsumeCurToken();
    AstBase* arg = ParseExpression();
    if (!arg) return NULL;

    return new AstUnaryExp(op, arg);
}

AstBase* Parser::ParseBinaryExp(int prev_prec, AstBase* lhs)
{
    // precedence climbing algo
    while (true)
    {
        int cur_prec = lex_.GetCurTokenPrec();
        if (cur_prec < prev_prec) return lhs;

        TokenType bin_op = lex_.GetCurToken();
        lex_.ConsumeCurToken();
        AstBase* rhs = ParsePrimary();

        if (!rhs) return 0;

        int next_prec = lex_.GetCurTokenPrec();
        if (cur_prec < next_prec)
        {
            rhs = ParseBinaryExp(cur_prec + 1, rhs);
            if (!rhs) return 0;
        }

        lhs = new AstBinaryExp(bin_op, lhs, rhs);
    }

    return 0;
}

AstBase* Parser::ParsePrimary()
{
    switch (lex_.GetCurToken())
    {
        case TOK_ID: return ParseIdentifierExp();
        case TOK_INT: return ParseIntExp();
        case TOK_DOUBLE: return ParseFloatExp();
        case TOK_QUO: return ParseStringExp();
        case TOK_PAREN_LEFT: return ParseParenExp();

        default: return ReportError("unknown token when expecting an expression");
    }
}

AstBase* Parser::ParseExpression()
{
    TokenType type = lex_.GetCurToken();
    if (type == TOK_NEG || type == TOK_INV)
    {
        return ParseUaryExp(type);
    }

    AstBase* ret = ParsePrimary();
    if (!ret) return NULL;

    return ParseBinaryExp(0, ret);
}

}  // end namespace

