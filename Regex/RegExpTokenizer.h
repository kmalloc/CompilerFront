#ifndef REGEXP_TOKENIZER_H_
#define REGEXP_TOKENIZER_H_

#include "Parsing/LexTokenizerBase.h"

#define REG_EXP_CHAR_MAX (128)
#define REG_EXP_CHAR_EPSILON (REG_EXP_CHAR_MAX - 1)

class RegExpTokenizer: public LexTokenizerBase
{
    public:

        virtual bool IsMetaChar(char c) const;
        virtual const char* IsToken(const char* s, const char* e) const;
        virtual const char* IsToken(const std::string& str) const;

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        static bool IsRefToken(const char* s);
#endif
        static bool CanCharEscape(char c);
        static bool IsCharEscape(const char* ps, const char* pc);

        void ExtractRegUnit(const char* s, const char* e,
                const char*& us, const char*& ue,
                const char*& beforeUnit, const char*& afterUnit) const;

        bool ExtractRepeatCount(const char* s, const char* e, int& min, int& max) const;

        // TODO, support 2-bytes char.
        static std::string ConstructEscapeString(const char* s, const char* e);
        static std::string ConstructOptionString(const char* s, const char* e);
};

#endif

