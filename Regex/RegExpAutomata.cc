#include "RegExpAutomata.h"

#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include "Parsing/LexException.h"
#include "RegExpTokenizer.h"
#include "RegExpSyntaxTree.h"
#include "RegExpSynTreeNode.h"

RegExpNFA::RegExpNFA(bool partial)
    :AutomatonBase(AutomatonType_NFA), stateIndex_(0)
    ,headState_(-1), tailState_(-1), support_partial_match_(partial)
    ,states_(), NFAStatTran_()
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    ,unitMatchPair_()
#endif
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
    recycleStates_.clear();

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    groupCapture_.clear();
    groupWatcher_.clear();
    unitMatchPair_.clear();

    hasReferNode_ = tree->HasRefNode();
#endif

    int leaf_node_num = tree->GetNodeNumber() * 2;
    states_.reserve(leaf_node_num);
    recycleStates_.reserve(leaf_node_num/2);
    NFAStatTran_.reserve(leaf_node_num);

    int num = BuildNFAImp(dynamic_cast<RegExpSynTreeNode*>(tree->GetSynTree()), start_, accept_, false, -1);

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

    return num;
}

// merge s2 into s1
void RegExpNFA::MergeState(int, int)
{
}

int RegExpNFA::CreateState(StateType type)
{
    int new_st;
    if (recycleStates_.empty())
    {
        new_st = stateIndex_++;
        MachineState state(new_st, type);
        std::vector<std::vector<int> > tmps(STATE_TRAN_MAX + 1);
        states_.push_back(state);
        NFAStatTran_.push_back(tmps);
    }
    else
    {
        new_st = recycleStates_[recycleStates_.size() - 1];
        recycleStates_.pop_back();
        assert(new_st < NFAStatTran_.size());
        assert(states_[new_st].GetType() & State_None);
    }

    return new_st;
}

void RegExpNFA::ReleaseState(int st)
{
    std::vector<std::vector<int> > tmps(STATE_TRAN_MAX + 1);

    NFAStatTran_[st].swap(tmps);
    states_[st].SetType(State_None);
}

int RegExpNFA::BuildNFAImp(RegExpSynTreeNode* root, int& start, int& accept, bool ignoreUnit, int parentUnit)
{
    if (!root) return 0;

    int unit_start = parentUnit;
    int unit_accept = -1;
    if (root->IsUnit() && !ignoreUnit)
    {
        unit_start = CreateState(State_Start);
    }

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

        num = BuildStateForStarNode(sn, start, accept, ignoreUnit, unit_start);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Or)
    {
        num = BuildStateForOrNode(root, start, accept, ignoreUnit, unit_start);
    }
    else if (root->GetNodeType() == RegExpSynTreeNodeType_Concat)
    {
        num = BuildStateForCatNode(root, start, accept, ignoreUnit, unit_start);
    }
    else
    {
        assert(0);
    }

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE

    if (states_[start].GetParentUnit() == -1) states_[start].SetParentUnit(unit_start);
    if (states_[accept].GetParentUnit() == -1) states_[accept].SetParentUnit(unit_start);

    if (root->IsUnit() && !ignoreUnit)
    {
        unit_accept = CreateState(State_Accept);

        states_[start].SetNormType();
        states_[accept].SetNormType();

        NFAStatTran_[unit_start][STATE_EPSILON].push_back(start);
        NFAStatTran_[accept][STATE_EPSILON].push_back(unit_accept);

        start = unit_start;
        accept = unit_accept;

        states_[start].SetStartUnit(root->IsUnit());
        states_[accept].SetEndUnit(root->IsUnit());

        states_[start].SetParentUnit(parentUnit);
        states_[accept].SetParentUnit(start);
        unitMatchPair_[start].insert(accept);
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
        NFAStatTran_[start][STATE_EPSILON].push_back(accept); // epsilon transition
        states_[start].AppendType(State_Head);
        headState_ = start;
    }
    else if(lt == RegExpSynTreeNodeLeafNodeType_Tail)
    {
        NFAStatTran_[start][STATE_EPSILON].push_back(accept); // epsilon transition
        states_[accept].AppendType(State_Tail);
        tailState_ = accept;
    }
    else if (lt == RegExpSynTreeNodeLeafNodeType_Alt)
    {
        bool is_negate = (txt[0] == '^');
        if (!is_negate)
        {
            for (size_t i = 0; i < txt.size(); ++i)
            {
                if (txt[i] == '-' && (i > 0 && i < txt.size() - 1))
                {
                    if (txt[i - 1] > txt[i + 1]) throw LexErrException("range values reversed in []:", ln->GetNodeText().c_str());

                    for (size_t j = txt[i - 1]; j <= txt[i + 1]; ++j)
                    {
                        NFAStatTran_[start][j].push_back(accept);
                    }
                    i += 1;
                }
                else
                {
                    if (i < txt.size() - 2 && txt[i] == '\\' && txt[i + 1] == '-')
                    {
                        ++i;
                    }
                    NFAStatTran_[start][txt[i]].push_back(accept);
                }
            }
        }
        else
        {
            unsigned short chosen[STATE_TRAN_MAX] = {0};
            for (size_t i = 1; i < txt.size(); ++i)
            {
                if (txt[i] == '-' && (i > 0 && i < txt.size() - 1))
                {
                    if (txt[i - 1] > txt[i + 1]) throw LexErrException("range values reversed in []:", ln->GetNodeText().c_str());

                    for (size_t j = txt[i - 1]; j <= txt[i + 1]; ++j)
                    {
                        chosen[j] = 1;
                    }

                    i += 1;
                }
                else
                {
                    if (i < txt.size() - 2 && txt[i] == '\\' && txt[i + 1] == '-')
                    {
                        ++i;
                    }

                    chosen[txt[i]] = 1;
                }
            }

            for (size_t i = 1; i < STATE_TRAN_MAX; ++i)
            {
                if (chosen[i]) continue;

                NFAStatTran_[start][i].push_back(accept);
            }
        }
    }
    else if (lt == RegExpSynTreeNodeLeafNodeType_Esc && (txt[0] == 's' || txt[0] == 'w' || txt[0] == 'd'))
    {
        if (txt[0] == 's')
        {
            NFAStatTran_[start][' '].push_back(accept);
        }
        else if (txt[0] == 'w')
        {
            for(int i = 'a'; i <= 'z'; ++i)
            {
                NFAStatTran_[start][i].push_back(accept);
                NFAStatTran_[start][i + 'A' - 'a'].push_back(accept);
            }
        }
        else
        {
            for(int i = '0'; i <= '9'; ++i)
            {
                NFAStatTran_[start][i].push_back(accept);
            }
        }
    }
    else if (lt == RegExpSynTreeNodeLeafNodeType_Ref)
    {
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        RegExpSynTreeRefNode* rt = dynamic_cast<RegExpSynTreeRefNode*>(ln);
        NFAStatTran_[start][STATE_TRAN_MAX].push_back(accept);
        NFAStatTran_[start][STATE_TRAN_MAX].push_back(rt->GetRef());
        states_[start].AppendType(State_Ref);
#else
        assert(0);
#endif
    }
    else
    {
        NFAStatTran_[start][txt[0]].push_back(accept);
    }

    return 2;
}

int RegExpNFA::BuildStateForOrNode(RegExpSynTreeNode* node, int& start, int& accept, bool ignoreUnit, int parentUnit)
{
    int left_child_start, left_child_accept;
    int right_child_start, right_child_accept;

    RegExpSynTreeNode* lc = dynamic_cast<RegExpSynTreeNode*>(node->GetLeftChild());
    RegExpSynTreeNode* rc = dynamic_cast<RegExpSynTreeNode*>(node->GetRightChild());

    assert(lc);
    assert(rc);

    int left_child_state_num = BuildNFAImp(lc, left_child_start, left_child_accept, ignoreUnit, parentUnit);
    int right_child_state_num = BuildNFAImp(rc, right_child_start, right_child_accept, ignoreUnit, parentUnit);

    states_[right_child_start].SetNormType();
    states_[right_child_accept].SetNormType();
    NFAStatTran_[left_child_start][STATE_EPSILON].push_back(right_child_start); // epsilon transition
    NFAStatTran_[right_child_accept][STATE_EPSILON].push_back(left_child_accept);

    start = left_child_start;
    accept = left_child_accept;

    return left_child_state_num + right_child_state_num;
}

int RegExpNFA::BuildStateForCatNode(RegExpSynTreeNode* node, int& start, int& accept, bool ignoreUnit, int parentUnit)
{
    int left_child_start, left_child_accept;
    int right_child_start, right_child_accept;

    RegExpSynTreeNode* lc = dynamic_cast<RegExpSynTreeNode*>(node->GetLeftChild());
    RegExpSynTreeNode* rc = dynamic_cast<RegExpSynTreeNode*>(node->GetRightChild());

    assert(lc);
    assert(rc);

    int left_child_state_num = BuildNFAImp(lc, left_child_start, left_child_accept, ignoreUnit, parentUnit);
    int right_child_state_num = BuildNFAImp(rc, right_child_start, right_child_accept, ignoreUnit, parentUnit);

    states_[left_child_accept].SetNormType();
    states_[right_child_start].SetNormType();

    NFAStatTran_[left_child_accept][STATE_EPSILON].push_back(right_child_start);

    start = left_child_start;
    accept = right_child_accept;

    return left_child_state_num + right_child_state_num - 1;
}

#define InsertIfNotExist(vec, val) \
    if (std::find((vec).begin(), (vec).end(), (val)) == (vec).end()) (vec).push_back((val));

int RegExpNFA::BuildStateForStarNode(RegExpSynTreeStarNode* sn, int& start, int& accept, bool ignoreUnit, int parentUnit)
{
    int child_start, child_accept;
    RegExpSynTreeNode* child = dynamic_cast<RegExpSynTreeNode*>(sn->GetLeftChild());
    if (!child) throw LexErrException("\'*\' should come after specific character", NULL);

    int min = sn->GetMinRepeat();
    int max = sn->GetMaxRepeat();
    bool is_unit = child->IsUnit();

    if (min == 0 && max == INT_MAX)
    {
        // (ab)*
        int child_states_num = BuildNFAImp(child, start, accept, ignoreUnit, parentUnit);

        InsertIfNotExist(NFAStatTran_[start][STATE_EPSILON], accept);
        InsertIfNotExist(NFAStatTran_[accept][STATE_EPSILON], start);

        return child_states_num;
    }
    else if (min == 0)
    {
        // (ab)?
        // (ab){0, 4}
        int cs, ca; // child start, child accept
        int cs2, ca2;
        int last_cs, last_ca;
        int child_states_num = BuildNFAImp(child, last_cs, last_ca, ignoreUnit, parentUnit);

        cs = last_cs;
        ca = last_ca;
        cs2 = last_cs;
        ca2 = last_ca;

        InsertIfNotExist(NFAStatTran_[last_cs][STATE_EPSILON], last_ca);

        // construct state from right to left
        for (int i = 0; i < max - 1; ++i)
        {
            child_states_num += BuildNFAImp(child, cs, ca, true, -1);
            states_[ca].SetNormType();
            states_[cs2].SetNormType();
            NFAStatTran_[ca][STATE_EPSILON].push_back(cs2);
            NFAStatTran_[cs][STATE_EPSILON].push_back(last_ca);

            cs2 = cs;
            ca2 = ca;
        }

        start = cs;
        accept = last_ca;
        return child_states_num;
    }
    else if (max == INT_MAX)
    {
        // (ab){3, INT_MAX}
        // (ab)+
        int child_states_num = BuildNFAImp(child, child_start, child_accept, ignoreUnit, parentUnit);

        int cs, ca;
        int ts = child_start;
        int ta = child_accept;

        child->SetUnit(false);
        for (int i = 0; i < min - 1; ++i)
        {
            BuildNFAImp(child, cs, ca, true, -1);

            states_[ta].SetNormType();
            states_[cs].SetNormType();
            NFAStatTran_[ta][STATE_EPSILON].push_back(cs);

            ts = cs;
            ta = ca;
        }

        start = child_start;
        accept = ta;

        InsertIfNotExist(NFAStatTran_[ta][STATE_EPSILON], ts);
        child->SetUnit(is_unit);

        return child_states_num * min;
    }
    else
    {
        //(ab){2, 4}
        int cs, ca;
        int cs2, ca2;
        child->SetUnit(false);

        int num = BuildNFAImp(child, cs2, ca2, ignoreUnit, parentUnit);
        start = cs2;

        // construct state from left to right
        for (int i = 0; i < min - 1; ++i)
        {
            num += BuildNFAImp(child, cs, ca, true, -1);

            states_[ca2].SetNormType();
            states_[cs].SetNormType();
            NFAStatTran_[ca2][STATE_EPSILON].push_back(cs);

            cs2 = cs;
            ca2 = ca;
        }

        int min_end = ca2;
        if (max > min)
        {
            num += BuildNFAImp(child, cs, ca, true, -1);

            states_[ca2].SetNormType();
            NFAStatTran_[ca2][STATE_EPSILON].push_back(ca);
            cs2 = cs;
            ca2 = ca;
        }

        accept = ca2;

        // construct state from right to left
        for (int i = min; i < max - 1; ++i)
        {
            num += BuildNFAImp(child, cs, ca, true, -1);
            states_[ca].SetNormType();
            states_[cs2].SetNormType();
            NFAStatTran_[ca][STATE_EPSILON].push_back(cs2);
            NFAStatTran_[ca][STATE_EPSILON].push_back(accept);

            cs2 = cs;
            ca2 = ca;
        }

        if (ca2 != min_end)
        {
            states_[cs2].SetNormType();
            states_[min_end].SetNormType();
            NFAStatTran_[min_end][STATE_EPSILON].push_back(cs2);
        }

        child->SetUnit(is_unit);
        return num;
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
    for (size_t i = 0; i < NFAStatTran_[st][STATE_EPSILON].size(); ++i)
    {
        int epsilon = NFAStatTran_[st][STATE_EPSILON][i];
        if (isOn[epsilon]) continue;

        AddStateWithEpsilon(epsilon, isOn, to);
    }

    return  to.size();
}

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
// if closure of state st has transition on input ch then return true;
// otherwise return false
bool RegExpNFA::IfStateClosureHasTrans(int st, int parentUnit, std::vector<char>& isCheck, char ch) const
{
    const std::vector<int>& vc = NFAStatTran_[st][STATE_EPSILON];
    if (!NFAStatTran_[st][ch].empty()) return true;

    isCheck[st] = 1;
    for (size_t i = 0; i < vc.size(); ++i)
    {
        st = vc[i];
        if (isCheck[st]) continue;

        int curParentUnit = states_[st].GetParentUnit();
        while (curParentUnit != -1 && curParentUnit != parentUnit)
        {
            curParentUnit = states_[curParentUnit].GetParentUnit();
        }

        if (curParentUnit == -1) continue;

        if (IfStateClosureHasTrans(st, parentUnit, isCheck, ch) ||
                states_[st].IsRefState())
        {
            return true;
        }
    }


    return false;
}

int RegExpNFA::DoSaveGroup(int st, int ac, const char* txtStart, const char* txtEnd)
{
    if (unitMatchPair_[st].find(ac) != unitMatchPair_[st].end())
    {
        int start_repeat = states_[st].UnitStart();
        int end_repeat = states_[ac].UnitEnd();
        int min = std::min(start_repeat, end_repeat);

        std::map<std::pair<int, int>, std::vector<int> >::iterator iter =
            groupWatcher_.find(std::pair<int, int>(st, ac));

        if (iter != groupWatcher_.end())
        {
            for (size_t i = 0; i < iter->second.size(); ++i)
            {
                groupCapture_[iter->second[i]] = UnitInfo(st, ac, txtStart, txtEnd);
            }
        }
        else
        {
            std::vector<int>& vc = groupWatcher_[std::pair<int, int>(st, ac)];
            for (int i = 0; i < min; ++i)
            {
                vc.push_back(groupCapture_.size());
                groupCapture_.push_back(UnitInfo(st, ac, txtStart, txtEnd));
            }
        }

        return min;
    }

    return 0;
}

int RegExpNFA::SaveCaptureGroup(const std::vector<int>& unitStart,
        const std::map<int, const char*>& unitSelect, int endState, const char* endTxt)
{
    int co = 0;
    std::map<int, const char*>::const_iterator it = unitSelect.begin();

    for (; it != unitSelect.end(); ++it)
    {
        int st = it->first;
        const char* txtStart = it->second;

        co += DoSaveGroup(st, endState, txtStart, endTxt);
    }

    for (size_t i = 0; i < unitStart.size(); ++i)
    {
        co += DoSaveGroup(unitStart[i], endState, "", "");
    }

    return co;
}

bool RegExpNFA::ConstructReferenceState(int st)
{
    // reference state
    assert(NFAStatTran_[st][STATE_TRAN_MAX].size() == 2);

    int to   = NFAStatTran_[st][STATE_TRAN_MAX][0];
    int unit = NFAStatTran_[st][STATE_TRAN_MAX][1];

    assert(unit < groupCapture_.size());
    // if (unit >= groupCapture_.size()) return false;

    int new_st;
    const char* ps = groupCapture_[unit].txtStart_;
    const char* pe = groupCapture_[unit].txtEnd_;

    states_[st].ClearType(State_Ref);
    while (ps <= pe && *ps)
    {
        new_st = CreateState(State_Norm);
        NFAStatTran_[st][*ps].push_back(new_st);
        st = new_st;
        ++ps;
    }

    NFAStatTran_[st][STATE_EPSILON].push_back(to);
    return true;
}

void RegExpNFA::RestoreRefStates(int st, int to, const char* ps, const char* pe)
{
    if (*ps == '\0' || ps > pe)
    {
        NFAStatTran_[st][STATE_EPSILON].clear();
        return;
    }

    assert(NFAStatTran_[st][*ps].size() == 1);
    int next_st = NFAStatTran_[st][*ps][0];
    NFAStatTran_[st][*ps].clear();
    st = next_st;

    ++ps;
    while (ps <= pe)
    {
        std::vector<int>& vc = NFAStatTran_[st][*ps];
        assert(vc.size() == 1);
        next_st = vc[0];
        ReleaseState(st);
        st = next_st;
        ++ps;
    }

    std::vector<int>& vc = NFAStatTran_[st][STATE_EPSILON];
    assert(vc.size() == 1);
    assert(vc[0] == to);
    ReleaseState(st);
}

#endif

void RegExpNFA::GenStatesClosure(char ch, const std::vector<int>& curStat,
        std::vector<int>& toStat, std::vector<char>& alreadyOn,
        std::vector<int>& refStates, bool ignoreRef)
{
    for (size_t i = 0; i < curStat.size(); ++i)
    {
        int st = curStat[i];
        const std::vector<int>* vc = &(NFAStatTran_[st][ch]);

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        if (!ignoreRef && states_[st].IsRefState() && ConstructReferenceState(st))
        {
            vc = &(NFAStatTran_[st][ch]);

            refStates.push_back(st);
            alreadyOn.resize(states_.size(), 0);
        }
#endif
        if (vc->empty()) continue;

        for (size_t j = 0; j < vc->size(); ++j)
        {
            if (alreadyOn[(*vc)[j]]) continue;

            AddStateWithEpsilon((*vc)[j], alreadyOn, toStat);
        }
    }
}

/*
  unit matching: (e((a)|(b)ef), ((a|b)|(a|c)), (a(b))
*/
bool RegExpNFA::RunMachine(const char* ps, const char* pe)
{
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    groupCapture_.clear();
    groupWatcher_.clear();
    groupCapture_.reserve(states_.size());
#endif
    return RunNFA(start_, accept_, ps, pe);
}

bool RegExpNFA::RunNFA(int start, int accept, const char* ps, const char* pe)
{
    char ch;
    const char* in = ps;

    std::vector<int> refStates;
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    std::vector<int> curUnitEndStack;
    std::vector<int> curUnitStartStack;
    std::map<int, const char*> curUnitSelectedStack;

    if (hasReferNode_)
    {
        refStates.reserve(states_.size());
        curUnitEndStack.reserve(states_.size());
        curUnitStartStack.reserve(states_.size());
    }
#endif

    std::vector<int> curStat;
    std::vector<int> toStat;
    std::vector<char> alreadyOn(states_.size(), 0);

    curStat.reserve(states_.size());
    toStat.reserve(states_.size());

    AddStateWithEpsilon(start, alreadyOn, curStat);

    while (in <= pe && !curStat.empty())
    {
        ch = *in++;

        for (size_t i = 0; i < curStat.size(); ++i)
        {
            alreadyOn[curStat[i]] = false;
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
            if (hasReferNode_ && states_[curStat[i]].UnitStart())
            {
                curUnitStartStack.push_back(curStat[i]);
            }

            if (hasReferNode_ && states_[curStat[i]].UnitEnd())
            {
                curUnitEndStack.push_back(curStat[i]);
            }
#endif
        }

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        for (size_t j = 0; j < curUnitStartStack.size(); ++j)
        {
            int st = curUnitStartStack[j];

            std::vector<char> isCheck(states_.size(), 0);
            if (IfStateClosureHasTrans(st, st, isCheck, ch))
            {
                curUnitStartStack[j] = -1;
                curUnitSelectedStack[st] = in - 1;
                continue;
            }
            else if (curUnitSelectedStack.find(st) != curUnitSelectedStack.end())
            {
                curUnitStartStack[j] = -1;
            }
        }

        if (!curUnitEndStack.empty())
        {
            GenStatesClosure(ch, curStat, toStat, alreadyOn, refStates, true);

            for (size_t j = 0; j < curUnitEndStack.size(); ++j)
            {
                int st = curUnitEndStack[j];

                if (std::find(toStat.begin(), toStat.end(), st) == toStat.end())
                {
                    size_t i = 0;
                    int parentUnit = states_[st].GetParentUnit();
                    for (; i < toStat.size(); ++i)
                    {
                        int curParentUnit = states_[toStat[i]].GetParentUnit();
                        while (parentUnit != curParentUnit && curParentUnit != -1)
                        {
                            curParentUnit = states_[curParentUnit].GetParentUnit();
                        }

                        if (curParentUnit != -1) break;
                    }

                    if (i == toStat.size())
                    {
                        SaveCaptureGroup(curUnitStartStack, curUnitSelectedStack, st, in - 2);
                    }
                }
            }
        }

        curUnitEndStack.clear();
        curUnitStartStack.clear();
#endif

        GenStatesClosure(ch, curStat, toStat, alreadyOn, refStates, false);

        curStat.swap(toStat);
        toStat.clear();
    }

#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
    // (a(b(cd)))\\0 , abcd
    // a(bc*)fe\\0 , afe
    if (hasReferNode_)
    {
        for (size_t i = 0; i < curStat.size(); ++i)
        {
            alreadyOn[curStat[i]] = false;
            if (states_[curStat[i]].UnitStart())
            {
                curUnitStartStack.push_back(curStat[i]);
            }

            if (states_[curStat[i]].UnitEnd())
            {
                curUnitEndStack.push_back(curStat[i]);
            }
        }

        if (!curUnitEndStack.empty())
        {
            for (size_t j = 0; j < curUnitEndStack.size(); ++j)
            {
                int st = curUnitEndStack[j];
                SaveCaptureGroup(curUnitStartStack, curUnitSelectedStack, st, pe);
            }
        }

        for (size_t j = 0; j < curStat.size(); ++j)
        {
            int st = curStat[j];
            if (states_[st].IsRefState() && ConstructReferenceState(st))
            {
                refStates.push_back(st);
                alreadyOn.resize(states_.size(), 0);
            }
            AddStateWithEpsilon(st, alreadyOn, toStat);
        }

        curStat.swap(toStat);

        // restore ref states
        for (size_t i = 0; i < refStates.size(); ++i)
        {
            int st   = refStates[i];
            int to   = NFAStatTran_[st][STATE_TRAN_MAX][0];
            int unit = NFAStatTran_[st][STATE_TRAN_MAX][1];

            RestoreRefStates(st, to, groupCapture_[unit].txtStart_, groupCapture_[unit].txtEnd_);
            states_[st].AppendType(State_Ref);
        }
    }
#else
    for (size_t j = 0; j < curStat.size(); ++j)
    {
        int st = curStat[j];
        AddStateWithEpsilon(curStat[j], alreadyOn, toStat);
    }
    curStat.swap(toStat);
#endif

    return !curStat.empty() && std::find(curStat.begin(), curStat.end(), accept) != curStat.end();
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

