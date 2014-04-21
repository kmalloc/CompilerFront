#ifndef _AUTOMATON_BASE_H_
#define _AUTOMATON_BASE_H_

#include <vector>
#include <string>
#include "NonCopyable.h"
#include "RegExpSynTreeNode.h"

enum AutomatonType
{
    AutomatonType_NFA,
    AutomatonType_DFA
};

class AutomatonBase: public NonCopyable
{
    public:

        explicit AutomatonBase(AutomatonType type);
        virtual ~AutomatonBase();

        bool RunMachine(const char* ps, const char* pe) const;

    protected:

        virtual int ConvertSynTreeToNFA() = 0;
        virtual int ConvertSynTreeToDFA() = 0;

    protected:

        AutomatonType automataType_;
        int (AutomatonBase::* automatonConverter_)();
};

#endif

