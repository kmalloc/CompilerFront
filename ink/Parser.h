#ifndef __PARSER_H__
#define __PARSER_H__

#include "Lexer.h"
#include "AbstractSynTree.h"

#include <boost/noncopyable.hpp>

namespace ink {

class AstBase;

class Parser: public boost::noncopyable
{
    public:
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
        AstBasePtr ParseArrIndexExp(const std::string& arr);

        AstBasePtr ParseExternExp();
        AstBasePtr ParseFuncRetExp();
        AstBasePtr ParseClassDefExp();
        AstBasePtr ParseIfExp();
        AstBasePtr ParseWhileExp();
        AstBasePtr ParseForExp();

        AstBasePtr ReportError(const char* msg);

    private:
        Lexer lex_;
};

} // end ink

#endif

