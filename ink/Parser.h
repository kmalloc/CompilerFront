#ifndef __PARSER_H__
#define __PARSER_H__

#include "Lexer.h"
#include <boost/noncopyable.h>

namespace ink {

class AstBase;
class AstIntExp;

class Parser: public boost::noncopyable
{
    public:
        AstBase* ParsePrimary();
        AstBase* ParseExpression();

        // followings are primary expression.
        AstBase* ParseIntExp();
        AstBase* ParseFloatExp();
        AstBase* ParseParenExp();
        AstBase* ParseStringExp();
        AstBase* ParseIdentifierExp();

        AstBase* ParseUaryExp(TokenType op);
        AstBase* ParseBinaryExp(int prec, TokenType op);

        AstBase* ReportError(const char* msg);

    private:
        Lexer lex_;
};

} // end ink

#endif

