#ifndef _AUTOMATON_BASE_H_
#define _AUTOMATON_BASE_H_

#include <vector>
#include <string>

#include "Basic/NonCopyable.h"
#include "Parsing/SyntaxTreeBase.h"

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

        virtual void SerializeState() const = 0;
        virtual void DeserializeState() = 0;

        virtual int  BuildMachine(SyntaxTreeBase* tree) = 0;
        virtual bool RunMachine(const char* ps, const char* pe) = 0;

    protected:

        int start_, accept_; // start state and the accepting state
        AutomatonType automataType_;
};

#endif

