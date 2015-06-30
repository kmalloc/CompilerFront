#include "Lexer.h"

Lexer::Lexer(const CharType* buf)
    : token_(TOK_UNKNOWN), curChar_(*buf), skipper_(' ')
    , intVal_(0), floatVal_(0), strVal_("")
    , text_(buf), curPos_(buf),
{
    strVal_.reserve(64);
}

int Lexer::ExtractToken()
{
    if (token_ == TOK_QUO)
    {
        strVal_ = curChar_;
        while ((curChar_ = GetNextChar()) != '"') strVal_ += curChar_;

        return TOK_STR;
    }

    while (IsSkipChar(curChar_)) curChar_ = GetNextChar();

    if (IsAlpha(curChar_))
    {
        // identifier or function name
        strVal_ = curChar_;
        while (IsAlNum(curChar_ = GetNextChar())) strVal_ += curChar_;

        if (strVal_ == "func") return TOK_FUN;

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
        bool is_float = false;
        string num;
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
        do {
            curChar_ = GetNextChar();
        } while (curChar_ != 0 && curChar_ != '\n' && curChar_ != '\r');

        if (curChar_) return ExtractToken();
    }

    if (!curChar_) return TOK_EOF;

    // operators
    CharType cur = curChar_;
    curChar_ = GetNextChar();

    switch (cur)
    {
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
                  // negate oerator
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

// this precedence mapping of the operators must
// follows the exact order defined in the TokenType
static const int gs_token_prec_map[] =
{
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

#define ArrSize(arr) (sizeof(arr)/sizeof(arr[0]))

int Lexer::GetCurTokenPrec() const
{
    BOOST_STATIC_ASSERT(ArrSize(gs_token_prec_map) == TOK_OP_END - TOK_OP_END);

    if (token_ <= TOK_OP_START) return -1;

    return gs_token_prec_map[token_ - TOK_OP_START];
}

