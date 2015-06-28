#ifndef __LEXER_H__
#define __LEXER_H__

#include <boost/variant.hpp>
#include <boost/noncopyable.h>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

namespace ink {

struct nil {};

enum TokenType
{
    TOK_EOF,
    TOK_FUN, // function definition
    TOK_ID,  // variable
    TOK_STR, // literal string
    TOK_INT, // literal int
    TOK_DOUBLE, // literal string

    TOK_IF,
    TOK_FOR,
    TOK_WHILE,
};

class Lexer: public boost::noncopyable
{
    public:
        explicit Lexer(const char* buf);

        int ExtractToken();
        int GetCurToken();

        std::string GetStringVal() const;
        int64_t GetIntVal() const;
        double GetDoubleVal() const;

    private:
        typedef boost::variant<nil, int64_t v, double, std::string> TokenType;

        TokenType token_;
        const char* text_;
};


} // end ink

#endif

