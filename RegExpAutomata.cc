#include "RegExpAutomata.h"

#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include "LexException.h"
#include "RegExpTokenizer.h"
#include "RegExpSyntaxTree.h"
#include "RegExpSynTreeNode.h"

RegExpNFA::RegExpNFA(bool partial)
    :AutomatonBase(AutomatonType_NFA), stateIndex_(0)
    ,headState_(-1), tailState_(-1), support_partial_match_(partial)
{
}

RegExpNFA::~RegExpNFA()
{
}

int RegExpNFA::BuildNFA(RegExpSyntaxTree* tree)
{
    stateIndex_ = 0;
    headState_ = tailState_ = -1;
    states_.clear();
    NFAStatTran_.clear();

    states_.reserve(tree->GetNodeNumber() * 2);
    int num = BuildNFAImp(dynamic_cast<RegExpSynTreeNode*>(tree->GetSynTree()), start_, accept_);

    if (support_partial_match_ && headState_ == -1)
    {
        for (int i = 0; i < STATE_TRAN_MAX; ++i)
        {
            if (NFAStatTran_[start_][i].empty()) NFAStatTran_[start_][i].push_back(start_);
        }
    }

    if (support_partial_match_ && tailState_ == -1)
    {
        for (int i = 0; i < STATE_TRAN_MAX; ++i)
        {
            if (NFAStatTran_[accept_][i].empty()) NFAStatTran_[accept_][i].push_back(accept_);
        }
    }
}

// merge s2 into s1
void RegExpNFA::MergeState(int s1, int s2)
{
}

int RegExpNFA::CreateState(StateType type)
{
    MachineState state(stateIndex_, type);
    std::vector<std::vector<int> > tmps(STATE_TRAN_MAX + 1);

    stateIndex_++;
    states_.push_back(state);
    NFAStatTran_.push_back(tmps);
    return stateIndex_ - 1;
}

int RegExpNFA::BuildNFAImp(RegExpSynTreeNode* root, int& start, int& accept)
{
    if (!root) return 0;

    int num = 0;
    if (root->IsLeafNode())
    {
        RegExpSynTreeLeafNode* ln = dynamic_cast<RegExpSynTreeLeafNode*>(root);
        assert(ln);

        num = BuildStateForLeafNode(ln, start, accept);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Star)
    {
        RegExpSynTreeStarNode* sn = dynamic_cast<RegExpSynTreeStarNode*>(root);
        assert(sn);

        num = BuildStateForStarNode(sn, start, accept);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Or)
    {
        num = BuildStateForOrNode(root, start, accept);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Concat)
    {
        num = BuildStateForCatNode(root, start, accept);
    }
    else
    {
        assert(0);
    }

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
    if (root->IsUnit())
    {
        states_[start].SetStartUnit();
        states_[accept].SetEndUnit();
    }
#endif

    return num;
}

int RegExpNFA::BuildStateForLeafNode(RegExpSynTreeLeafNode* ln, int& start, int& accept)
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
    else if (lt == RegExpSynTreeNodeLeafNodeType_Head)
    {
        std::vector<int> acc(1, accept);
        NFAStatTran_[start][STATE_EPSILON] = acc; // epsilon transition
        states_[start].AppendType(State_Head);
        headState_ = start;
    }
    else if(lt == RegExpSynTreeNodeLeafNodeType_Tail)
    {
        std::vector<int> acc(1, accept);
        NFAStatTran_[start][STATE_EPSILON] = acc; // epsilon transition
        states_[accept].AppendType(State_Tail);
        tailState_ = accept;
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
                NFAStatTran_[start][i + 'A' - 'a']  = acc;
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
    else if (lt == RegExpSynTreeNodeLeafNodeType_Ref)
    {
        RegExpSynTreeRefNode* rt = dynamic_cast<RegExpSynTreeRefNode*>(ln);
        NFAStatTran_[start][STATE_TRAN_MAX].push_back(accept);
        NFAStatTran_[start][STATE_TRAN_MAX].push_back(rt->GetRef());
        states_[start].AppendType(State_Ref);
        // states_[accept].AppendType(State_Ref);
    }
    else
    {
        std::vector<int> acc(1, accept);
        NFAStatTran_[start][txt[0]] = acc;
    }

    return 2;
}

int RegExpNFA::BuildStateForOrNode(RegExpSynTreeNode* node, int& start, int& accept)
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

    states_[left_child_start].SetNormType();
    states_[right_child_start].SetNormType();
    NFAStatTran_[start][STATE_EPSILON].push_back(left_child_start); // epsilon transition
    NFAStatTran_[start][STATE_EPSILON].push_back(right_child_start);

    states_[left_child_accept].SetNormType();
    states_[right_child_accept].SetNormType();
    NFAStatTran_[left_child_accept][STATE_EPSILON].push_back(accept);
    NFAStatTran_[right_child_accept][STATE_EPSILON].push_back(accept);

    return left_child_state_num + right_child_state_num + 2;
}

int RegExpNFA::BuildStateForCatNode(RegExpSynTreeNode* node, int& start, int& accept)
{
    int left_child_start, left_child_accept;
    int right_child_start, right_child_accept;

    RegExpSynTreeNode* lc = dynamic_cast<RegExpSynTreeNode*>(node->GetLeftChild());
    RegExpSynTreeNode* rc = dynamic_cast<RegExpSynTreeNode*>(node->GetRightChild());

    assert(lc);
    assert(rc);

    int left_child_state_num = BuildNFAImp(lc, left_child_start, left_child_accept);
    int right_child_state_num = BuildNFAImp(rc, right_child_start, right_child_accept);

    states_[left_child_accept].SetNormType();
    states_[right_child_start].SetNormType();
    NFAStatTran_[left_child_accept][STATE_EPSILON].push_back(right_child_start);

    start = left_child_start;
    accept = right_child_accept;

    return left_child_state_num + right_child_state_num - 1;
}

int RegExpNFA::BuildStateForStarNode(RegExpSynTreeStarNode* sn, int& start, int& accept)
{
    int child_start, child_accept;
    RegExpSynTreeNode* child = dynamic_cast<RegExpSynTreeNode*>(sn->GetLeftChild());
    if (!child) throw LexErrException("\'*\' should come after specific character", NULL);

    bool is_unit = child->IsUnit();
    if (sn->GetMinRepeat() == 0 && sn->GetMaxRepeat() == INT_MAX)
    {
        // (ab)*
        start = CreateState(State_Start);
        int child_states_num = BuildNFAImp(child, child_start, child_accept);
        accept = CreateState(State_Accept);

        NFAStatTran_[start][STATE_EPSILON].push_back(child_start);
        NFAStatTran_[start][STATE_EPSILON].push_back(accept);

        states_[child_start].SetNormType();
        states_[child_accept].SetNormType();

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

        states_[child_start].SetNormType();
        states_[child_accept].SetNormType();

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

        child->SetUnit(false);
        for (int i = 0; i < min - 1; ++i)
        {
            BuildNFAImp(child, cs, ca);

            states_[ta].SetNormType();
            states_[cs].SetNormType();
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);

            ts = cs;
            ta = ca;
        }

        states_[ta].SetNormType();

        start = child_start;
        accept = CreateState(State_Accept);

        NFAStatTran_[ta][STATE_EPSILON].push_back(ts);
        NFAStatTran_[ta][STATE_EPSILON].push_back(accept);

        child->SetUnit(is_unit);

        return child_states_num * min + 1;
    }
    else
    {
        //(ab){2, 4}
        int min = sn->GetMinRepeat();
        int max = sn->GetMaxRepeat();

        accept = CreateState(State_Accept);

        int child_states_num = BuildNFAImp(child, child_start, child_accept);
        // copy child min times

        int cs, ca;
        int ts = child_start;
        int ta = child_accept;

        start = child_start;

        child->SetUnit(false);
        for (int i = 0; i < min - 1; ++i)
        {
            BuildNFAImp(child, cs, ca);

            states_[ta].SetNormType();
            states_[cs].SetNormType();
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);

            ts = cs;
            ta = ca;
        }

        for (int i = min - 1; i < max - 1; ++i)
        {
            BuildNFAImp(child, cs, ca);
            states_[ta].SetNormType();
            states_[cs].SetNormType();
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);

            NFAStatTran_[ta][STATE_EPSILON].push_back(accept);

            ts = cs;
            ta = ca;
        }

        NFAStatTran_[ta][STATE_EPSILON].push_back(accept);

        child->SetUnit(is_unit);
        return child_states_num * max + 1;
    }
}

int RegExpNFA::BuildMachine(SyntaxTreeBase* tree)
{
    RegExpSyntaxTree* reg_tree = dynamic_cast<RegExpSyntaxTree*>(tree);
    if (!reg_tree) return 0;

    return BuildNFA(reg_tree);
}

int RegExpNFA::AddStateWithEpsilon(int st, std::vector<char>& isOn, std::vector<int>& to) const
{
    isOn[st] = 1;
    to.push_back(st);
    for (int i = 0; i < NFAStatTran_[st][STATE_EPSILON].size(); ++i)
    {
        int epsilon = NFAStatTran_[st][STATE_EPSILON][i];
        if (isOn[epsilon]) continue;

        AddStateWithEpsilon(epsilon, isOn, to);
    }

    return  to.size();
}

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
void RegExpNFA::ConstructReferenceState(int st, int to, const char* ps, const char* pe)
{
    int new_st;

    states_[st].ClearType(State_Ref);
    NFAStatTran_[st][STATE_TRAN_MAX].clear();
    while (ps <= pe)
    {
        new_st = CreateState(State_Norm);
        NFAStatTran_[st][*ps].push_back(new_st);
        st = new_st;
    }

    NFAStatTran_[st][STATE_EPSILON].push_back(to);
}
#endif

/*
  unit matching: (e((a)|(b)ef), ((a|b)|(a|c)), (a(b))
*/
bool RegExpNFA::RunMachine(const char* ps, const char* pe)
{
    char ch;
    const char* in = ps;

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
    int curUnit = -1;
    short unit_state_end   = 0;
    short unit_state_start = 0;

    typedef std::pair<const char*, const char*> UnitTxtAddr;
    std::vector<short> curUnitStack;
    std::vector<UnitTxtAddr> unitTxt;

    unitTxt.reserve(states_.size());
    curUnitStack.reserve(states_.size());
#endif

    std::vector<int> curStat;
    std::vector<int> toStat;
    std::vector<char> alreadyOn(states_.size(), 0);

    curStat.reserve(states_.size());
    toStat.reserve(states_.size());

    AddStateWithEpsilon(start_, alreadyOn, curStat);

    for (int i = 0; i < curStat.size(); ++i)
    {
        alreadyOn[curStat[i]] = false;
#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
        unit_state_end = std::max(unit_state_end, states_[curStat[i]].UnitEnd());
        unit_state_start = std::max(unit_state_start, states_[curStat[i]].UnitStart());
#endif
    }

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
    for (int i = 0; i < unit_state_start; ++i)
    {
        unitTxt.push_back(UnitTxtAddr(in, NULL));
        curUnit = unitTxt.size() - 1;
        curUnitStack.push_back(curUnit);
    }
#endif

    while (in <= pe && !curStat.empty())
    {
        ch = *in++;
        for (int i = 0; i < curStat.size(); ++i)
        {
            int st = curStat[i];
            const std::vector<int>& vc = NFAStatTran_[st][ch];

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
            if (states_[st].GetType() & State_Ref)
            {
                // reference state
                assert(NFAStatTran_[st][STATE_TRAN_MAX].size() == 2);

                int unit = NFAStatTran_[st][STATE_TRAN_MAX][1];
                int to   = NFAStatTran_[st][STATE_TRAN_MAX][0];
                ConstructReferenceState(st, to, unitTxt[unit].first, unitTxt[unit].second);
            }
#endif

            if (vc.empty()) continue;

            for (int j = 0; j < vc.size(); ++j)
            {
                if (alreadyOn[vc[j]]) continue;

                AddStateWithEpsilon(vc[j], alreadyOn, toStat);
            }
        }

        curStat.swap(toStat);
        toStat.clear();

#ifdef SUPPORT_REG_EXP_BACK_REFEREENCE
        unit_state_end = 0;
        unit_state_start = 0;

        for (int i = 0; i < curStat.size(); ++i)
        {
            alreadyOn[curStat[i]] = false;
            unit_state_end = std::max(unit_state_end, states_[curStat[i]].UnitEnd());
            unit_state_start = std::max(unit_state_start, states_[curStat[i]].UnitStart());
        }

        for (int i = 0; i < unit_state_start; ++i)
        {
            unitTxt.push_back(UnitTxtAddr(in, NULL));
            curUnit = unitTxt.size() - 1;
            curUnitStack.push_back(curUnit);
        }

        for (int i = 0; i < unit_state_end; ++i)
        {
            unitTxt[curUnit].second = in;
            curUnitStack.pop_back();
            curUnit = curUnitStack.empty()? -1 : curUnitStack[curUnitStack.size() - 1];
        }
#else
        for (int i = 0; i < curStat.size(); ++i)
        {
            alreadyOn[curStat[i]] = false;
        }
#endif
    }

    return !curStat.empty() && std::find(curStat.begin(), curStat.end(), accept_) != curStat.end();
}

void RegExpNFA::SerializeState() const
{
}

void RegExpNFA::DeserializeState()
{
}

void RegExpNFA::ConvertToDFA(RegExpDFA& dfa) const
{

}

