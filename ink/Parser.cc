#include "Parser.h"

namespace ink {

AstBasePtr Parser::ParseIntExp()
{
    AstBasePtr ret(new AstIntExp(lex_.GetIntVal()));
    lex_.ConsumeCurToken();
    return ret;
}

AstBasePtr Parser::ParseFloatExp()
{
    AstBasePtr ret(new AstFloatExp(lex_.GetFloatVal()));
    lex_.ConsumeCurToken();
    return ret;
}

AstBasePtr Parser::ParseParenExp()
{
    // consume '('
    lex_.ConsumeCurToken();

    // parse main expression
    AstBasePtr ret = ParseExpression();
    if (!ret) return AstBasePtr();

    if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
    {
        return ReportError("expected ')'");
    }

    // consume ')'
    lex_.ConsumeCurToken();
    return ret;
}

AstBasePtr Parser::ParseStringExp()
{
    // consume left "
    lex_.ConsumeCurToken();

    AstBasePtr ret(new AstStringExp(lex_.GetStringVal()));
    if (!ret) return ReportError("invalid string literal");

    lex_.ConsumeCurToken();
    if (lex_.GetCurToken() != TOK_QUO) return ReportError("expected '\"'");

    // consume '"'
    lex_.ConsumeCurToken();
    return ret;
}

AstBasePtr Parser::ParseFuncProtoExp()
{
    if (lex_.GetCurToken() != TOK_ID)
    {
        return ReportError("expected identifier after 'func'");
    }

    std::string name = lex_.GetStringVal();
    lex_.ConsumeCurToken();

    if (lex_.GetCurToken() != TOK_PAREN_LEFT)
    {
        return ReportError("expected '(' in function definition");
    }

    std::vector<AstFuncProtoExp::ArgType> args;
    // TODO, support static checking for parameter type

    return AstBasePtr(new AstFuncProtoExp(name, args));
}

AstBasePtr Parser::ParseFuncDefExp()
{
    // consume "func" keyword
    lex_.ConsumeCurToken();
    AstFuncProtoExpPtr proto =
        boost::static_pointer_cast<AstFuncProtoExp>(ParseFuncProtoExp());

    if (!proto) return AstBasePtr();

    if (lex_.GetCurToken() != TOK_BRACE_LEFT)
    {
        return ReportError("expected '{' for function definition");
    }

    lex_.ConsumeCurToken();
    std::vector<AstBasePtr> body;

    body.reserve(64);
    while (lex_.GetCurToken() != TOK_BRACE_RIGHT)
    {
        AstBasePtr exp = ParseExpression();
        if (!exp)
        {
            return ReportError("expected '}' in function definition");
        }

        body.push_back(exp);
    }

    AstBasePtr ret(new AstFuncDefExp(proto, body));
    return ret;
}

AstBasePtr Parser::ParseFuncCallExp(const std::string& name)
{
    std::vector<AstBasePtr> args;
    if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
    {
        while (true)
        {
            AstBasePtr arg = ParseExpression();
            if (!arg) return AstBasePtr();

            args.push_back(arg);
            if (lex_.GetCurToken() == TOK_PAREN_RIGHT) break;

            if (lex_.GetCurToken() != TOK_COMA)
            {
                return ReportError("expected ')' or ',' in function argument list");
            }

            // consume ','
            lex_.ConsumeCurToken();
        }
    }

    // consume ')'
    lex_.ConsumeCurToken();
    return AstBasePtr(new AstFuncCallExp(name, args));
}

AstBasePtr Parser::ParseArrIndexExp(const std::string& name)
{
    AstBasePtr index = ParseExpression();
    if (!index) return AstBasePtr();

    if (lex_.GetCurToken() != TOK_IND_RIGHT)
    {
        return ReportError("expected ']' for array indexing expression");
    }

    lex_.ConsumeCurToken();
    return AstBasePtr(new AstArrayIndexExp(name, index));
}

AstBasePtr Parser::ParseIdentifierExp()
{
    std::string name = lex_.GetStringVal();

    // consume name
    lex_.ConsumeCurToken();
    TokenType tok = lex_.GetCurToken();
    if (tok != TOK_PAREN_LEFT && tok != TOK_IND_LEFT)
    {
        return AstBasePtr(new AstVarExp(name));
    }

    // consume '(' or '['
    lex_.ConsumeCurToken();

    if (tok == TOK_PAREN_LEFT) return ParseFuncCallExp(name);

    return ParseArrIndexExp(name);
}

AstBasePtr Parser::ParseUaryExp(TokenType op)
{
    // consume unary operator('!' or '~')
    lex_.ConsumeCurToken();
    AstBasePtr arg = ParseExpression();
    if (!arg) return AstBasePtr();

    return AstBasePtr(new AstUnaryExp(op, arg));
}

AstBasePtr Parser::ParseBinaryExp(int prev_prec, const AstBasePtr& arg)
{
    // precedence climbing algo
    AstBasePtr lhs = arg;
    while (true)
    {
        int cur_prec = lex_.GetCurTokenPrec();
        if (cur_prec < prev_prec) return lhs;

        TokenType bin_op = lex_.GetCurToken();
        lex_.ConsumeCurToken();
        AstBasePtr rhs = ParsePrimary();

        if (!rhs) return AstBasePtr();

        int next_prec = lex_.GetCurTokenPrec();
        if (cur_prec < next_prec)
        {
            rhs = ParseBinaryExp(cur_prec + 1, rhs);
            if (!rhs) return AstBasePtr();
        }

        lhs = AstBasePtr(new AstBinaryExp(bin_op, lhs, rhs));
    }

    return AstBasePtr();
}

AstBasePtr Parser::ParsePrimary()
{
    switch (lex_.GetCurToken())
    {
        case TOK_ID: return ParseIdentifierExp();
        case TOK_INT: return ParseIntExp();
        case TOK_FLOAT: return ParseFloatExp();
        case TOK_QUO: return ParseStringExp();
        case TOK_PAREN_LEFT: return ParseParenExp();
        case TOK_FUN: return ParseFuncDefExp();
        case TOK_EXT: return ParseExternExp();
        case TOK_RET: return ParseFuncRetExp();
        case TOK_CLASS: return ParseClassDefExp();
        case TOK_IF: return ParseIfExp();
        case TOK_WHILE: return ParseWhileExp();
        case TOK_FOR: return ParseForExp();

        default: return ReportError("unknown token when expecting an expression");
    }
}

AstBasePtr Parser::ParseExpression()
{
    TokenType type = lex_.GetCurToken();
    if (type == TOK_NEG || type == TOK_INV)
    {
        return ParseUaryExp(type);
    }

    AstBasePtr ret = ParsePrimary();
    if (!ret) return AstBasePtr();

    return ParseBinaryExp(0, ret);
}

}  // end namespace

