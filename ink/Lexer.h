#ifndef __INK_LEXER_H__
#define __INK_LEXER_H__

#include "Noncopyable.h"

#include <cctype>
#include <string>

namespace ink {

struct nil {};

enum TokenType
{
    TOK_EOF,
    TOK_UNKNOWN,

    // builtin keyword, 2 ~ 9
    TOK_FUN, // function definition
    TOK_EXT,  // extern, for function declaration
    TOK_CLASS, // class
    TOK_SELF, // "this" of class
    TOK_ID,  // variable
    TOK_STR, // literal string
    TOK_INT, // literal int
    TOK_FLOAT, // literal floatting point
    TOK_COMMENT,

    // control flow
    TOK_RET, // return
    TOK_IF, // if statement
    TOK_ELSE, // else
    TOK_ELIF, // else if
    TOK_FOR, // for loop
    TOK_IN, // for a in [a,b,c]
    TOK_WHILE, // while loop
    TOK_LOCAL, // local
    TOK_GLOBAL, // global
    TOK_BOOL,
    ToK_NIL,

    // primary operators
    TOK_COMA, //,
    TOK_PAREN_LEFT, // (
    TOK_PAREN_RIGHT, // )
    TOK_BRACE_LEFT,
    TOK_BRACE_RIGHT,
    TOK_QUO, // "
    TOK_BRACKET_LEFT, // [
    TOK_BRACKET_RIGHT, // ]

    // operators, the order matter
    // precedences from lower to higher

    TOK_OP_START,

    TOK_AS, // = assign

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

class Lexer: public noncopyable
{
    public:
        typedef char CharType;

    public:
        explicit Lexer(const CharType* buf = "");

        void Reset(const CharType* buf);
        void Start() { if (GetCurToken() == TOK_UNKNOWN) ConsumeCurToken(); }

        TokenType GetCurToken() const { return token_; }

        // get precedence of token
        int GetTokenPrec(TokenType tok) const;
        int GetCurTokenPrec() const { return GetTokenPrec(GetCurToken()); }
        void ConsumeCurToken() { token_ = ExtractToken(); }

        int GetCurLineNum() const { return curLine_; }
        const std::string& GetStringVal() const { return strVal_; }
        int64_t GetIntVal() const { return intVal_; }
        double GetFloatVal() const { return floatVal_; }
        const CharType* GetCurCharPos() const { return curPos_; }

    private:
        TokenType ExtractToken();
        CharType GetNextChar() { curLine_ += *curPos_ == '\n'; return *curPos_++; }
        bool IsAlpha(CharType c) const { return std::isalpha(c); }
        bool IsAlNum(CharType c) const { return std::isalnum(c); }
        bool IsDigit(CharType c) const { return std::isdigit(c); }
        bool IsSkipChar(CharType c) const { return c == ';' || std::isspace(c); }

    private:
        TokenType token_;
        CharType curChar_;
        CharType skipper_; // char that is skipped, default is space

        int curLine_;
        int64_t intVal_;
        double floatVal_;
        std::string strVal_;

        const CharType* text_;
        const CharType* curPos_;
};


} // end ink

#endif

