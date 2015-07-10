#include "Parser.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <assert.h>

namespace ink {

Parser::Parser(const std::string& file)
    : file_(file)
{
    std::ifstream fin(file.c_str(), std::fstream::in);
    assert(fin);

    buff_ = std::string((std::istreambuf_iterator<Lexer::CharType>(fin)),
            std::istreambuf_iterator<Lexer::CharType>());

}

Parser::Parser(const std::string& buff, const std::string& file)
    : file_(file), buff_(buff)
{
}

AstBasePtr Parser::ReportError(const char* msg)
{
    // should provide more information about location.

    std::ostringstream oss;

    oss << msg << ", from " << file_ << ", line:"
        << lex_.GetCurLineNum() << std::endl;
    oss << "current token:" << lex_.GetCurToken() << " ";
    oss << "val(int):" << lex_.GetIntVal()
        << ", val(float):" << lex_.GetFloatVal()
        << ", val(string):" << lex_.GetStringVal() << std::endl;
    oss << "parsing stop at:" << lex_.GetCurCharPos() << std::endl;

    return AstErrInfoPtr(new AstErrInfo(oss.str()));
}

AstBasePtr Parser::ParseIntExp()
{
    AstBasePtr ret(new AstIntExp(lex_.GetIntVal()));
    lex_.ConsumeCurToken();
    return ret;
}

AstBasePtr Parser::ParseBoolExp()
{
    bool val = ("true" == lex_.GetStringVal());
    AstBasePtr ret(new AstBoolExp(val));
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
    if (IsError(ret)) return ret;

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

    if (lex_.GetCurToken() != TOK_STR)
    {
        return ReportError("invalid string literal");
    }

    AstBasePtr ret(new AstStringExp(lex_.GetStringVal()));
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
    int line = lex_.GetCurLineNum();

    lex_.ConsumeCurToken();

    if (lex_.GetCurToken() != TOK_PAREN_LEFT)
    {
        return ReportError("expected '(' in function definition");
    }

    // consume '('
    lex_.ConsumeCurToken();
    std::vector<std::string> args;

    std::string arg;
    if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
    {
        while (true)
        {
            if (lex_.GetCurToken() != TOK_ID)
            {
                return ReportError("expected parameter name");
            }

            arg = lex_.GetStringVal();
            if (arg.empty()) return ReportError("expected non-empty parameter name");

            if (std::find(args.begin(), args.end(), arg) != args.end())
            {
                return ReportError("identical parameter name in function definition");
            }

            args.push_back(std::move(arg));
            lex_.ConsumeCurToken();

            if (lex_.GetCurToken() == TOK_PAREN_RIGHT) break;

            if (lex_.GetCurToken() != TOK_COMA)
            {
                return ReportError("expected ',' between parameters");
            }

            // consume ','
            lex_.ConsumeCurToken();
        }
    }

    // consume ')'
    lex_.ConsumeCurToken();
    AstBasePtr ret(new AstFuncProtoExp(std::move(name), std::move(args)));

    ret->SetLocation(file_, line);
    return ret;
}

AstBasePtr Parser::ParseFuncDefExp()
{
    // consume "func" keyword
    lex_.ConsumeCurToken();
    auto proto = std::dynamic_pointer_cast<AstFuncProtoExp>(ParseFuncProtoExp());

    if (IsError(proto)) return proto;

    auto body = ParseScopeStatement();
    if (IsError(body)) return body;

    return AstBasePtr(new AstFuncDefExp(proto, body));
}

AstBasePtr Parser::ParseFuncCallExp(std::string name)
{
    int line = lex_.GetCurLineNum();

    std::vector<AstBasePtr> args;
    if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
    {
        while (true)
        {
            AstBasePtr arg = ParseExpression();
            if (IsError(arg)) return arg;

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
    AstBasePtr ret(new AstFuncCallExp(std::move(name), std::move(args)));

    ret->SetLocation(file_, line);
    return ret;
}

AstBasePtr Parser::ParseArrayExp()
{
    lex_.ConsumeCurToken(); // '['

    std::vector<AstBasePtr> elem;
    if (lex_.GetCurToken() != TOK_BRACKET_RIGHT)
    {
        while (true)
        {
            AstBasePtr v = ParseExpression();
            if (IsError(v)) return ReportError("expected array element");

            elem.push_back(v);

            TokenType tok = lex_.GetCurToken();
            if (tok == TOK_EOF || tok == TOK_BRACKET_RIGHT) break;

            if (lex_.GetCurToken() != TOK_COMA)
            {
                return ReportError("expected ',' between array element");
            }

            // consume ','
            lex_.ConsumeCurToken();
        }
    }

    if (lex_.GetCurToken() != TOK_BRACKET_RIGHT)
    {
        return ReportError("expected ']' for array expression");
    }

    lex_.ConsumeCurToken();
    return AstBasePtr(new AstArrayExp(elem));
}

AstBasePtr Parser::ParseArrIndexExp(std::string name)
{
    AstBasePtr index = ParseExpression();
    if (IsError(index)) return index;

    if (lex_.GetCurToken() != TOK_BRACKET_RIGHT)
    {
        return ReportError("expected ']' for array indexing expression");
    }

    lex_.ConsumeCurToken();
    return AstBasePtr(new AstArrayIndexExp(std::move(name), index));
}

AstBasePtr Parser::ParseIdentifierExp()
{
    std::string name = lex_.GetStringVal();

    // consume name
    lex_.ConsumeCurToken();
    TokenType tok = lex_.GetCurToken();
    if (tok != TOK_PAREN_LEFT && tok != TOK_BRACKET_LEFT)
    {
        return AstBasePtr(new AstVarExp(name));
    }

    // consume '(' or '['
    lex_.ConsumeCurToken();

    if (tok == TOK_PAREN_LEFT) return ParseFuncCallExp(std::move(name));

    return ParseArrIndexExp(std::move(name));
}

AstBasePtr Parser::ParseUaryExp(TokenType op)
{
    // unary operator has the highest priority.

    // consume unary operator('!' or '~')
    lex_.ConsumeCurToken();
    AstBasePtr arg = ParsePrimary();
    if (IsError(arg)) return arg;

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

        if (IsError(rhs)) return rhs;

        int next_prec = lex_.GetCurTokenPrec();
        if (cur_prec < next_prec)
        {
            rhs = ParseBinaryExp(cur_prec + 1, rhs);
            if (IsError(rhs)) return rhs;
        }

        lhs = AstBasePtr(new AstBinaryExp(bin_op, lhs, rhs));
    }

    return AstBasePtr();
}

AstBasePtr Parser::ParseExternExp()
{
    // consume extern
    lex_.ConsumeCurToken();

    if (lex_.GetCurToken() != TOK_FUN)
    {
        return ReportError("missing 'func' in function declaration");
    }

    // consume 'func'
    lex_.ConsumeCurToken();
    return ParseFuncProtoExp();
}

AstBasePtr Parser::ParseFuncRetExp()
{
    int line = lex_.GetCurLineNum();

    lex_.ConsumeCurToken();
    AstBasePtr val = ParseExpression();

    AstBasePtr ret(new AstRetExp(val));

    ret->SetLocation(file_, line);
    return ret;
}

AstBasePtr Parser::ParseClassDefExp()
{
    // TODO
    return AstBasePtr();
}

AstBasePtr Parser::ParseIfExp()
{
    std::vector<AstIfExp::IfEntity> exe;
    exe.reserve(16);

    do {
        // consume if elif else
        AstIfExp::IfEntity entity;
        bool is_if = lex_.GetCurToken() == TOK_IF || lex_.GetCurToken() == TOK_ELIF;

        lex_.ConsumeCurToken();

        if (is_if)
        {
            if (lex_.GetCurToken() != TOK_PAREN_LEFT)
            {
                return ReportError("expected '(' in if statement");
            }

            lex_.ConsumeCurToken();

            AstBasePtr cond = ParseExpression();
            if (IsError(cond)) return cond;

            entity.cond = cond;
            if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
            {
                return ReportError("expected ')' in if statement");
            }

            lex_.ConsumeCurToken();
        }
        else
        {
            entity.cond = AstBasePtr();
        }

        auto body = ParseScopeStatement();
        if (IsError(body)) return body;

        entity.exp = body;
        exe.push_back(entity);

    } while (lex_.GetCurToken() == TOK_ELIF || lex_.GetCurToken() == TOK_ELSE);

    return AstBasePtr(new AstIfExp(exe));
}

AstScopeStatementExpPtr Parser::ParseScopeStatement()
{
    if (lex_.GetCurToken() != TOK_BRACE_LEFT)
    {
        return std::static_pointer_cast<AstScopeStatementExp>(
                ReportError("expected '{' in scope statement"));
    }

    lex_.ConsumeCurToken();

    std::vector<AstBasePtr> exps;
    exps.reserve(64);

    while (lex_.GetCurToken() != TOK_EOF && lex_.GetCurToken() != TOK_BRACE_RIGHT)
    {
        AstBasePtr exp = ParseExpression();
        if (IsError(exp)) return AstScopeStatementExpPtr();

        exps.push_back(exp);
    }

    if (lex_.GetCurToken() != TOK_BRACE_RIGHT)
    {
        return std::static_pointer_cast<AstScopeStatementExp>(
                ReportError("expected '}' for scope statement"));
    }

    lex_.ConsumeCurToken();
    return AstScopeStatementExpPtr(new AstScopeStatementExp(std::move(exps)));
}

AstBasePtr Parser::ParseWhileExp()
{
    lex_.ConsumeCurToken();
    if (lex_.GetCurToken() != TOK_PAREN_LEFT)
    {
        return ReportError("expected '(' in while statement");
    }

    lex_.ConsumeCurToken();
    AstBasePtr cond = ParseExpression();
    if (IsError(cond)) return cond;

    if (lex_.GetCurToken() != TOK_PAREN_RIGHT)
    {
        return ReportError("expected ')' in while statement");
    }

    lex_.ConsumeCurToken();
    auto body = ParseScopeStatement();
    if (IsError(body)) return body;

    return AstBasePtr(new AstWhileExp(cond, body));
}

AstBasePtr Parser::ParseForExp()
{
    lex_.ConsumeCurToken();
    AstBasePtr var = ParseExpression();
    if (IsError(var)) return var;

    if (lex_.GetCurToken() != TOK_IN)
    {
        return ReportError("expected 'in' in a for loop");
    }

    lex_.ConsumeCurToken();

    AstBasePtr arr = ParseExpression();
    if (IsError(arr)) return arr;

    auto body = ParseScopeStatement();
    if (IsError(body)) return body;

    return AstForExpPtr(new AstForExp(var, arr, body));
}

AstBasePtr Parser::ParsePrimary()
{
    switch (lex_.GetCurToken())
    {
        case TOK_ID: return ParseIdentifierExp();
        case TOK_BOOL: return ParseBoolExp();
        case TOK_INT: return ParseIntExp();
        case TOK_FLOAT: return ParseFloatExp();
        case TOK_QUO: return ParseStringExp();
        case TOK_PAREN_LEFT: return ParseParenExp();
        case TOK_BRACKET_LEFT: return ParseArrayExp();
        case TOK_FUN: return ParseFuncDefExp();
        case TOK_EXT: return ParseExternExp();
        case TOK_RET: return ParseFuncRetExp();
        case TOK_CLASS: return ParseClassDefExp();
        case TOK_IF: return ParseIfExp();
        case TOK_WHILE: return ParseWhileExp();
        case TOK_FOR: return ParseForExp();
        case TOK_EOF: return AstBasePtr();
        case TOK_NEG:
        case TOK_INV: return ParseUaryExp(lex_.GetCurToken());

        default: return ReportError("unknown token when expecting an expression");
    }
}

AstBasePtr Parser::ParseExpression()
{
    AstBasePtr ret = ParsePrimary();
    if (IsError(ret)) return ret;

    return ParseBinaryExp(0, ret);
}

std::string Parser::StartParsing()
{
    lex_.Reset(buff_.c_str());
    res_.clear();
    lex_.Reset(buff_.c_str());
    lex_.Start();

    AstBasePtr v = ParseExpression();
    while (!IsError(v))
    {
        res_.push_back(v);
        v = ParseExpression();
    }

    if (!v) return "";

    auto err = std::dynamic_pointer_cast<AstErrInfo>(v);
    return err->GetErrorInfo();
}

}  // end namespace

