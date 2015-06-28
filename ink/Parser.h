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

        AstBase* ParseExpression();
        AstBase* ParseDispatcher();

        AstBase* ParseIntExp();
        AstBase* ParseParenExp();
        AstBase* ParseIdentifierExp();
};

} // end ink

#endif

