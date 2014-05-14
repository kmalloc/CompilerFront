#ifndef LEX_TOKENIZER_BASE_H_
#define LEX_TOKENIZER_BASE_H_

#include <string>

class LexTokenizerBase
{
    public:

        LexTokenizerBase() {}
        virtual ~LexTokenizerBase() {}

        virtual bool IsMetaChar(char c) const = 0;
        virtual const char* IsToken(const std::string& str) const = 0;
        virtual const char* IsToken(const char* s, const char* e) const = 0;
};

#endif

