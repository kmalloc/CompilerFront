#ifndef __LEXER_H__
#define __LEXER_H__

#include <cctype>
#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

namespace ink {

struct nil {};

enum TokenType
{
    TOK_EOF,
    TOK_UNKNOWN,

    // builtin keyword
    TOK_FUN, // function definition
    TOK_EXT,  // extern, for function declaration
    TOK_CLASS, // class
    TOK_ID,  // variable
    TOK_STR, // literal string
    TOK_INT, // literal int
    TOK_FLOAT, // literal floatting point

    // control flow
    TOK_RET, // return
    TOK_IF, // if statement
    TOK_FOR, // for loop
    TOK_WHILE, // while loop

    // primary operators
    TOK_COMA, //,
    TOK_PAREN_LEFT, // (
    TOK_PAREN_RIGHT, // )
    TOK_BRACE_LEFT,
    TOK_BRACE_RIGHT,
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

    public:
        explicit Lexer(const CharType* buf);

        TokenType GetCurToken() { return token_; }
        int GetCurTokenPrec() const; // get precedence of current token
        void ConsumeCurToken() { token_ = ExtractToken(); }

        std::string GetStringVal() const { return strVal_; }
        int64_t GetIntVal() const { return intVal_; }
        double GetFloatVal() const { return floatVal_; }

    private:
        TokenType ExtractToken();
        CharType GetNextChar() { return *curPos_++; }
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

