#include "Lexer.h"

#include <unordered_map>
#include <type_traits>

namespace ink {

Lexer::Lexer(const CharType* buf)
    : token_(TOK_UNKNOWN), curChar_(' '), skipper_(' ')
    , intVal_(0), floatVal_(0), strVal_("")
    , text_(buf), curPos_(buf)
{
    strVal_.reserve(64);
}

void Lexer::Reset(const CharType* buf)
{
    token_ = TOK_UNKNOWN;
    curChar_ = skipper_;
    intVal_ = 0;
    floatVal_ = 0;
    strVal_ = "";
    text_ = buf;
    curPos_ = buf;
}

static const std::unordered_map<std::string, TokenType> g_keyword_m =
{
    {"func", TOK_FUN}, {"return", TOK_RET}, {"class", TOK_CLASS},
    {"self", TOK_SELF}, {"extern", TOK_EXT}, {"if", TOK_IF},
    {"while", TOK_WHILE}, {"for", TOK_FOR}, {"in", TOK_IN},
    {"else", TOK_ELSE}, {"elif", TOK_ELIF}
};

TokenType Lexer::ExtractToken()
{
    if (token_ == TOK_QUO)
    {
        strVal_ = curChar_;
        while ((curChar_ = GetNextChar()) != '"' && curChar_) strVal_ += curChar_;

        if (!curChar_) return TOK_EOF;

        curChar_ = GetNextChar();
        return TOK_STR;
    }

    while (IsSkipChar(curChar_)) curChar_ = GetNextChar();

    if (IsAlpha(curChar_))
    {
        // identifier or function name
        strVal_ = curChar_;
        while (IsAlNum(curChar_ = GetNextChar())) strVal_ += curChar_;

        auto it = g_keyword_m.find(strVal_);
        if (it != g_keyword_m.end()) return it->second;

        return TOK_ID;
    }

    if (curChar_ == '"')
    {
        // string literal
        curChar_ = GetNextChar();
        return TOK_QUO;
    }

    if (IsDigit(curChar_) || curChar_ == '.')
    {
        std::string num;
        bool is_float = false;

        do {
            num += curChar_;
            is_float = (is_float || curChar_ == '.');

            curChar_ = GetNextChar();
        } while (IsDigit(curChar_) || (!is_float && curChar_ == '.'));

        if (is_float)
        {
            floatVal_ = atof(num.c_str());
            return TOK_FLOAT;
        }

        intVal_ = atoi(num.c_str());
        return TOK_INT;
    }

    if (curChar_ == '#')
    {
        // comment line
        strVal_ = "";
        curChar_ = GetNextChar();

        while (curChar_ != 0 && curChar_ != '\n' && curChar_ != '\r')
        {
            strVal_ += curChar_;
            curChar_ = GetNextChar();
        }

        return TOK_COMMENT;
    }

    if (!curChar_) return TOK_EOF;

    // operators
    CharType cur = curChar_;
    curChar_ = GetNextChar();

    switch (cur)
    {
        case ',': return TOK_COMA;
        case '"': return TOK_QUO;
        case '[': return TOK_BRACKET_LEFT;
        case ']': return TOK_BRACKET_RIGHT;
        case '(': return TOK_PAREN_LEFT;
        case ')': return TOK_PAREN_RIGHT;
        case '{': return TOK_BRACE_LEFT;
        case '}': return TOK_BRACE_RIGHT;

        case '+': return TOK_ADD;
        case '-': return TOK_SUB;
        case '*': return TOK_MUL;
        case '/': return TOK_DIV;
        case '%': return TOK_MOD;
        case '^': return TOK_XOR;
        case '~': return TOK_INV;
        case '=':
                  if (curChar_ == '=')
                  {
                      curChar_ = GetNextChar();
                      return TOK_EQ;
                  }
                  return TOK_AS;
        case '>':
                  if (curChar_ == '=')
                  {
                      curChar_ = GetNextChar();
                      return TOK_GE;
                  }
                  else if (curChar_ == '>')
                  {
                      curChar_ = GetNextChar();
                      return TOK_RSH;
                  }
                  return TOK_GT;
        case '<':
                  if (curChar_ == '=')
                  {
                      curChar_ = GetNextChar();
                      return TOK_LE;
                  }
                  else if (curChar_ == '<')
                  {
                      curChar_ = GetNextChar();
                      return TOK_LSH;
                  }
                  return TOK_LT;
        case '!':
                  if (curChar_ == '=')
                  {
                      curChar_ = GetNextChar();
                      return TOK_NE;
                  }
                  // negate operator
                  return TOK_NEG;
        case '&':
                  if (curChar_ == '&')
                  {
                      curChar_ = GetNextChar();
                      return TOK_LAND;
                  }
                  return TOK_AND;
        case '|':
                  if (curChar_ == '|')
                  {
                      curChar_ = GetNextChar();
                      return TOK_LOR;
                  }
                  return TOK_OR;
        default:  return TOK_UNKNOWN;
    }
}

// this precedence mapping for the operators must
// follows the exact order defined in the TokenType

// 35 -> 36 -> 37 -> 38 -> 39 -> 40 -> 41 41 -> 42 42 42 42
// =  -> || -> && -> |  -> ^  -> &  -> == != -> < <= > >=
// 43 43 -> 44 44 -> 45 45 -> 46 -> 47 -> 48 48
// >> << -> + -   -> * / -> % -> power -> ~ !
static const int g_op_prec_map[] =
{
    35, // =
    42, 42, 42, 42, // <, <=, >, >=
    41, 41, // ==, !=
    36, 37, // ||, &&
    44, 44, // +, -,
    45, 45, 46, // * / %
    47, // power
    48, // !
    40, 39, 38, // &, ^, |
    48, // ~
    43, 43, // >>, <<
};

#define ArrSz(arr) (sizeof(arr)/sizeof(arr[0]))

int Lexer::GetTokenPrec(TokenType tok) const
{
    static_assert(ArrSz(g_op_prec_map) == TOK_OP_END - TOK_OP_START - 1,
            "precedence settings for operators do not match the defined order.");

    if (tok <= TOK_OP_START || tok >= TOK_OP_END) return -1;

    return g_op_prec_map[tok - TOK_OP_START - 1];
}

}  // end namespace

