#ifndef REGEXP_AUTOMA_H_
#define REGEXP_AUTOMA_H_

#include <vector>
#include "AutomatonBase.h"
#include "MachineComponent.h"

class RegExpSyntaxTree;

class RegExpAutomata: public AutomatonBase
{
    public:

        typedef std::vector<std::vector<std::vector<int> > > NFATRAN_T;

        explicit RegExpAutomata(AutomatonType type = AutomatonType_NFA);
        ~RegExpAutomata();

        virtual int BuildNFA(RegExpSyntaxTree* tree);
        virtual int BuildDFA(RegExpSyntaxTree* tree);

        int   GetStartState() const { return start_; }
        int   GetAcceptState() const { return accept_; }
        const std::vector<MachineState>& GetAllStates() const { return states_; }
        const NFATRAN_T& GetNFATran() const { return NFAStatTran_; }

    protected:

        virtual int ConvertSynTreeToNFA();
        virtual int ConvertSynTreeToDFA();

    private:

        void MergeState(int s1, int s2);

        int CreateState(StateType type);
        int BuildNFAImp(RegExpSynTreeNode* node, int& start, int& accept);

        int BuildNFAStateForLeafNode(RegExpSynTreeLeafNode* node, int& start, int& accept);
        int BuildNFAStateForStarNode(RegExpSynTreeStarNode* node, int& start, int& accept);
        int BuildNFAStateForOrNode(RegExpSynTreeNode* node, int& start, int& accept);
        int BuildNFAStateForCatNode(RegExpSynTreeNode* node, int& start, int& accept);

    private:

        int stateIndex_;
        int start_, accept_;
        std::vector<MachineState> states_;
        std::vector<std::vector<std::vector<int> > > NFAStatTran_; // state char state
};

#endif

