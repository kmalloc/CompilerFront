#include "RegExpAutomata.h"

#include "LexException.h"
#include "RegExpTokenizer.h"
#include "RegExpSyntaxTree.h"

RegExpAutomata::RegExpAutomata(AutomatonType type)
    :AutomatonBase(type), stateIndex_(0)
{
}

RegExpAutomata::~RegExpAutomata()
{
}

int RegExpAutomata::BuildNFA(RegExpSyntaxTree* tree)
{
    stateIndex_ = 0;
    states_.clear();
    NFAStatTran_.clear();

    return BuildNFAImp(dynamic_cast<RegExpSynTreeNode*>(tree->GetSynTree()), start_, accept_);
}

// merge s2 into s1
void RegExpAutomata::MergeState(int s1, int s2)
{
    states_[s1].SetType(State_Norm);
    for (int i = 0; i < STATE_TRAN_MAX; ++i)
    {
        NFAStatTran_[s1][i].insert(NFAStatTran_[s1][i].end(),
                NFAStatTran_[s2][i].begin(), NFAStatTran_[s2][i].end());

        std::sort(NFAStatTran_[s1][i].begin(), NFAStatTran_[s1][i].end());
        std::unique(NFAStatTran_[s1][i].begin(), NFAStatTran_[s1][i].end());
    }

    NFAStatTran_[s2].clear();
    states_[s2].SetType(State_None);
}

int RegExpAutomata::CreateState(StateType type)
{
    MachineState state(stateIndex_, type);
    std::vector<std::vector<int> > tmps(STATE_TRAN_MAX);

    stateIndex_++;
    states_.push_back(state);
    NFAStatTran_.push_back(tmps);
    return stateIndex_ - 1;
}

int RegExpAutomata::BuildNFAImp(RegExpSynTreeNode* root, int& start, int& accept)
{
    if (!root) return 0;

    if (root->IsLeafNode())
    {
        RegExpSynTreeLeafNode* ln = dynamic_cast<RegExpSynTreeLeafNode*>(root);
        assert(ln);

        return BuildNFAStateForLeafNode(ln, start, accept);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Star)
    {
        RegExpSynTreeStarNode* sn = dynamic_cast<RegExpSynTreeStarNode*>(root);
        assert(sn);

        return BuildNFAStateForStarNode(sn, start, accept);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Or)
    {
        return BuildNFAStateForOrNode(root, start, accept);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Concat)
    {
        return BuildNFAStateForCatNode(root, start, accept);
    }

    assert(0);

    return 0;
}

int RegExpAutomata::BuildNFAStateForLeafNode(RegExpSynTreeLeafNode* ln, int& start, int& accept)
{
    start = CreateState(State_Start);
    accept = CreateState(State_Accept);

    std::string txt = ln->GetNodeText();
    RegExpSynTreeNodeLeafNodeType lt = ln->GetLeafNodeType();
    if (lt == RegExpSynTreeNodeLeafNodeType_Dot)
    {
        std::vector<int> to(1, accept);
        std::vector<std::vector<int> > char_to_state(STATE_TRAN_MAX, to);
        char_to_state[STATE_EPSILON].clear();
        NFAStatTran_[start].swap(char_to_state);
    }
    else if (lt == RegExpSynTreeNodeLeafNodeType_Head
            || lt == RegExpSynTreeNodeLeafNodeType_Tail)
    {
        std::vector<int> acc(1, accept);
        NFAStatTran_[start][STATE_EPSILON] = acc; // epsilon transition
    }
    else if (lt == RegExpSynTreeNodeLeafNodeType_Alt)
    {
        std::vector<int> acc(1, accept);
        for (int i = 0; i < txt.size(); ++i)
        {
            NFAStatTran_[start][txt[i]] = acc;
        }
    }
    else if (lt == RegExpSynTreeNodeLeafNodeType_Esc && (txt[0] == 's' || txt[0] == 'w' || txt[0] == 'd'))
    {
        std::vector<int> acc(1, accept);
        if (txt[0] == 's')
        {
            NFAStatTran_[start][' '] = acc;
        }
        else if (txt[0] == 'w')
        {
            for(int i = 'a'; i <= 'z'; ++i)
            {
                NFAStatTran_[start][i] = acc;
                NFAStatTran_[start][i + 'a' - 'A']  = acc;
            }
        }
        else
        {
            for(int i = '0'; i <= '9'; ++i)
            {
                NFAStatTran_[start][i] = acc;
            }
        }
    }
    else
    {
        std::vector<int> acc(1, accept);
        NFAStatTran_[start][txt[0]] = acc;
    }

    return 2;
}

int RegExpAutomata::BuildNFAStateForOrNode(RegExpSynTreeNode* node, int& start, int& accept)
{
    int left_child_start, left_child_accept;
    int right_child_start, right_child_accept;

    RegExpSynTreeNode* lc = dynamic_cast<RegExpSynTreeNode*>(node->GetLeftChild());
    RegExpSynTreeNode* rc = dynamic_cast<RegExpSynTreeNode*>(node->GetRightChild());

    assert(lc);
    assert(rc);

    start = CreateState(State_Start);
    int left_child_state_num = BuildNFAImp(lc, left_child_start, left_child_accept);
    int right_child_state_num = BuildNFAImp(rc, right_child_start, right_child_accept);
    accept = CreateState(State_Accept);

    states_[left_child_start].SetType(State_Norm);
    states_[right_child_start].SetType(State_Norm);
    NFAStatTran_[start][STATE_EPSILON].push_back(left_child_start); // epsilon transition
    NFAStatTran_[start][STATE_EPSILON].push_back(right_child_start);

    states_[left_child_accept].SetType(State_Norm);
    states_[right_child_accept].SetType(State_Norm);
    NFAStatTran_[left_child_accept][STATE_EPSILON].push_back(accept);
    NFAStatTran_[right_child_accept][STATE_EPSILON].push_back(accept);

    return left_child_state_num + right_child_state_num + 2;
}

int RegExpAutomata::BuildNFAStateForCatNode(RegExpSynTreeNode* node, int& start, int& accept)
{
    int left_child_start, left_child_accept;
    int right_child_start, right_child_accept;

    RegExpSynTreeNode* lc = dynamic_cast<RegExpSynTreeNode*>(node->GetLeftChild());
    RegExpSynTreeNode* rc = dynamic_cast<RegExpSynTreeNode*>(node->GetRightChild());

    assert(lc);
    assert(rc);

    int left_child_state_num = BuildNFAImp(lc, left_child_start, left_child_accept);
    int right_child_state_num = BuildNFAImp(rc, right_child_start, right_child_accept);

    MergeState(left_child_accept, right_child_start);

    start = left_child_start;
    accept = right_child_accept;

    return left_child_state_num + right_child_state_num - 1;
}

int RegExpAutomata::BuildNFAStateForStarNode(RegExpSynTreeStarNode* sn, int& start, int& accept)
{
    int child_start, child_accept;
    RegExpSynTreeNode* child = dynamic_cast<RegExpSynTreeNode*>(sn->GetLeftChild());
    if (!child) throw LexErrException("\'*\' should come after specific character", NULL);

    if (sn->GetMinRepeat() == 0 && sn->GetMaxRepeat() == INT_MAX)
    {
        // (ab)*
        start = CreateState(State_Start);
        int child_states_num = BuildNFAImp(child, child_start, child_accept);
        accept = CreateState(State_Accept);

        NFAStatTran_[start][STATE_EPSILON].push_back(child_start);
        NFAStatTran_[start][STATE_EPSILON].push_back(accept);

        states_[child_start].SetType(State_Norm);
        states_[child_accept].SetType(State_Norm);

        NFAStatTran_[child_accept][STATE_EPSILON].push_back(child_start);
        NFAStatTran_[child_accept][STATE_EPSILON].push_back(accept);

        return child_states_num + 2;
    }
    else if (sn->GetMinRepeat() == 0 && sn->GetMaxRepeat() == 1)
    {
        // (ab)?
        start = CreateState(State_Start);
        int child_states_num = BuildNFAImp(child, child_start, child_accept);
        accept = CreateState(State_Accept);

        states_[child_start].SetType(State_Norm);
        states_[child_accept].SetType(State_Norm);

        NFAStatTran_[start][STATE_EPSILON].push_back(child_start);
        NFAStatTran_[start][STATE_EPSILON].push_back(accept);
        NFAStatTran_[child_accept][STATE_EPSILON].push_back(accept);

        return child_states_num + 2;
    }
    else if (sn->GetMaxRepeat() == INT_MAX)
    {
        // (ab){3, INT_MAX}
        // (ab)+
        int min = sn->GetMinRepeat();
        int child_states_num = BuildNFAImp(child, child_start, child_accept);

        int cs, ca;
        int ts = child_start;
        int ta = child_accept;

        for (int i = 0; i < min - 1; ++i)
        {
            BuildNFAImp(child, cs, ca);

            states_[ta].SetType(State_Norm);
            states_[cs].SetType(State_Norm);
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);

            ts = cs;
            ta = ca;
        }

        states_[ta].SetType(State_Norm);

        start = child_start;
        accept = CreateState(State_Accept);

        NFAStatTran_[ta][STATE_EPSILON].push_back(ts);
        NFAStatTran_[ta][STATE_EPSILON].push_back(accept);

        return child_states_num * min + 1;
    }
    else
    {
        //(ab){2, 4}
        int min = sn->GetMinRepeat();
        int max = sn->GetMaxRepeat();
        int child_states_num = BuildNFAImp(child, child_start, child_accept);
        // copy child min times

        int cs, ca;
        int ts = child_start;
        int ta = child_accept;

        for (int i = 0; i < min - 1; ++i)
        {
            BuildNFAImp(child, cs, ca);

            states_[ta].SetType(State_Norm);
            states_[cs].SetType(State_Norm);
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);

            ts = cs;
            ta = ca;
        }

        for (int i = min - 1; i < max - 1; ++i)
        {
            BuildNFAImp(child, cs, ca);
            states_[ta].SetType(State_Norm);
            states_[cs].SetType(State_Norm);
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);
            NFAStatTran_[ta][STATE_EPSILON].push_back(accept);

            ts = cs;
            ta = ca;
        }

        start = child_start;
        accept = CreateState(State_Accept);

        NFAStatTran_[ta][STATE_EPSILON].push_back(accept);

        return child_states_num * max + 1;
    }
}

int RegExpAutomata::ConvertSynTreeToNFA()
{
    // TODO
    return 0;
}

int RegExpAutomata::ConvertSynTreeToDFA()
{
    // TODO
    return 0;
}

int RegExpAutomata::BuildDFA(RegExpSyntaxTree* tree)
{
    // TODO
    return 0;
}

