#ifndef __INK_PARSER_H__
#define __INK_PARSER_H__

#include "Ast.h"
#include "Lexer.h"

#include <boost/noncopyable.hpp>

namespace ink {

class AstBase;

class Parser: public boost::noncopyable
{
    public:
        Parser(const std::string& file, const std::string& buff);
        explicit Parser(const std::string& file);

        AstBasePtr ParsePrimary();
        AstBasePtr ParseExpression();

        // followings are primary expression.
        AstBasePtr ParseIntExp();
        AstBasePtr ParseFloatExp();
        AstBasePtr ParseParenExp();
        AstBasePtr ParseStringExp();
        AstBasePtr ParseIdentifierExp();

        AstBasePtr ParseUaryExp(TokenType op);
        AstBasePtr ParseBinaryExp(int prec, const AstBasePtr& lhs);

        AstBasePtr ParseFuncProtoExp();
        AstBasePtr ParseFuncDefExp();
        AstBasePtr ParseFuncCallExp(const std::string& fun);
        AstBasePtr ParseArrayExp();
        AstBasePtr ParseArrIndexExp(const std::string& arr);

        AstBasePtr ParseExternExp();
        AstBasePtr ParseFuncRetExp();
        AstBasePtr ParseClassDefExp();
        AstBasePtr ParseIfExp();
        AstBasePtr ParseWhileExp();
        AstBasePtr ParseForExp();

        AstBasePtr ReportError(const char* msg);
        AstScopeStatementExpPtr ParseScopeStatement();

    private:
        Lexer lex_;
        std::string file_;
        std::string buff_;
};

} // end ink

#endif

