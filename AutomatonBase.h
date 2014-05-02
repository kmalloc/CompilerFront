#ifndef _AUTOMATON_BASE_H_
#define _AUTOMATON_BASE_H_

#include <vector>
#include <string>
#include "NonCopyable.h"
#include "SyntaxTreeBase.h"

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

        int  GetStartState() const { return start_; }
        int  GetAcceptState() const { return accept_; }

        virtual int  BuildMachine(SyntaxTreeBase* tree) = 0;
        virtual bool RunMachine(const char* ps, const char* pe) const = 0;

    protected:

        int start_, accept_; // start state and the accepting state
        AutomatonType automataType_;
};

#endif

