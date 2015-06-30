#ifndef __LEXER_H__
#define __LEXER_H__

#include <cctype>
#include <boost/variant.hpp>
#include <boost/noncopyable.h>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

namespace ink {

struct nil {};

enum TokenType
{
    TOK_EOF,
    TOK_UNKNOWN,

    // primary identifier
    TOK_FUN, // function definition
    TOK_ID,  // variable
    TOK_STR, // literal string
    TOK_INT, // literal int
    TOK_FLOAT, // literal floatting point

    // control flow
    TOK_IF,
    TOK_FOR,
    TOK_WHILE,

    // primary operators
    TOK_COMA, //,
    TOK_PAREN_LEFT, // (
    TOK_PAREN_RIGHT, // )
    TOK_QUO, // "
    TOK_AS, // = assign
    TOK_IND_LEFT, // [
    TOK_IND_RIGHT, // ]

    // operators, the order matter
    // precedences from lower to higher

    TOK_OP_START,

    // logical operator
    TOK_LT, // <
    TOK_LE, // <=
    TOK_GT, // >
    TOK_GE, // >=
    TOK_NE, // !=
    TOK_EQ, // ==
    TOK_LOR, // logical or
    TOK_LAND, // logical and

    // arithmetic operator
    TOK_ADD, // +
    TOK_SUB, // -
    TOK_MUL, // *
    TOK_DIV, // /
    TOK_MOD, // %
    TOK_POW, // not use

    // bitwise operator
    TOK_NEG, // !, negate, unary operator
    TOK_AND, // &, bitwise and
    TOK_XOR, // ^, bitwise xor
    TOK_OR, // |, bitwise or
    TOK_INV, // ~, bitwise inverse, unary operator
    TOK_LSH, // <<, left shift
    TOK_RSH, // >>, right shift

    TOK_OP_END,
};

class Lexer: public boost::noncopyable
{
    public:
        typedef char CharType;
        typedef boost::variant<nil, int64_t v, double, std::string> TokenType;

    public:
        explicit Lexer(const CharType* buf);

        TokenType GetCurToken() { return token_; }
        int ConsumeCurToken() { token_ = ExtractToken(); }
        int GetCurTokenPrec() const; // get precedence of current token

        std::string GetStringVal() const { return strVal_; }
        int64_t GetIntVal() const { return intVal_; }
        double GetFloatVal() const { return floatVal_; }

    private:
        int ExtractToken();

        CharType GetNextChar() { return *curChar_++; }
        bool IsAlpha(CharType c) const { return std::isalpha(c); }
        bool IsAlNum(CharType c) const { return std::isalnum(c); }
        bool IsDigit(CharType c) const { return std::isdigit(c); }
        bool IsSkipChar(CharType c) const { return  c== ' '; }

    private:
        TokenType token_;
        CharType curChar_;
        CharType skipper_; // char that is skipped, default is space

        int64_t intVal_;
        double floatVal_;
        std::string strVal_;

        const CharType* text_;
        const CharType* curPos_;
};


} // end ink

#endif

