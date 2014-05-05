#ifndef REGEXP_AUTOMA_H_
#define REGEXP_AUTOMA_H_

#include <map>
#include <vector>
#include "AutomatonBase.h"
#include "MachineComponent.h"

class RegExpDFA;
class SyntaxTreeBase;
class RegExpSyntaxTree;
class RegExpSynTreeNode;
class RegExpSynTreeLeafNode;
class RegExpSynTreeStarNode;

class RegExpNFA: public AutomatonBase
{
    public:

        typedef std::vector<std::vector<std::vector<int> > > NFA_TRAN_T;

        /*
            a) if partial match mode is enabled:
                1) abc[123]ef, 23ef will match.
                2) ^abc[123]ef$, 23ef will fail.

            b) if partial match mode is disabled, matching will always from start from beginning to the end.
               just as if every pattern is prefixed by ^ and has a trailing $.
        */
        explicit RegExpNFA(bool enable_partial_match = true);
        ~RegExpNFA();

        virtual void SerializeState() const;
        virtual void DeserializeState();

        virtual int  BuildMachine(SyntaxTreeBase* tree);
        virtual bool RunMachine(const char* ps, const char* pe);

        RegExpDFA ConvertToDFA() const;

        const NFA_TRAN_T& GetNFATran() const { return NFAStatTran_; }
        const std::vector<MachineState>& GetAllStates() const { return states_; }

    protected:

        int  BuildNFA(RegExpSyntaxTree* tree);
        void RunNFA(const char* ps, const char* pe);

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
        void ConstructReferenceState(int st, int to, const char* ps, const char* pe);
#endif

    private:

        void MergeState(int s1, int s2);
        int  AddStateWithEpsilon(int st, std::vector<char>& ison, std::vector<int>& to) const;

        int CreateState(StateType type);
        int BuildNFAImp(RegExpSynTreeNode* node, int& start, int& accept);

        int BuildStateForLeafNode(RegExpSynTreeLeafNode* node, int& start, int& accept);
        int BuildStateForStarNode(RegExpSynTreeStarNode* node, int& start, int& accept);
        int BuildStateForOrNode(RegExpSynTreeNode* node, int& start, int& accept);
        int BuildStateForCatNode(RegExpSynTreeNode* node, int& start, int& accept);

    private:

        int stateIndex_;
        int headState_, tailState_;
        bool support_partial_match_;
        std::vector<MachineState> states_;
        std::vector<std::vector<std::vector<int> > > NFAStatTran_; // state to char to state
};

class RegExpDFA: public AutomatonBase
{
    public:

        RegExpDFA();
        ~RegExpDFA();

        virtual int  BuildMachine(SyntaxTreeBase* tree);
        virtual bool RunMachine(const char* ps, const char* pe);

    private:

        int BuildDFA(RegExpSyntaxTree* tree);
        int RunDFA(const char* ps, const char* pe) const;

    private:

        int stateIndex_;
        std::vector<MachineState> states_;
};

#endif

