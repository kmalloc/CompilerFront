#include "RegExpSyntaxTree.h"

#include "LexException.h"
#include "RegExpTokenizer.h"
#include "RegExpSynTreeNode.h"

RegExpSyntaxTree::RegExpSyntaxTree()
    :synTreeRoot_(NULL), leafIndex_(0)
    ,tokenizer_(new RegExpTokenizer())
{
}

RegExpSyntaxTree::~RegExpSyntaxTree()
{
    delete tokenizer_;
    if (synTreeRoot_) delete synTreeRoot_;
}

bool RegExpSyntaxTree::BuildSyntaxTree(const char* ps, const char* pe)
{
    if (!ps || !pe || ps > pe) return false;

    if (synTreeRoot_) delete synTreeRoot_;

    leafIndex_ = 0;
    txtStart_ = ps;
    txtEnd_ = pe;

    synTreeRoot_ = dynamic_cast<RegExpSynTreeNode*>(ConstructSyntaxTree(ps, pe));
}

SynTreeNodeBase* RegExpSyntaxTree::ConstructSyntaxTree(const char* ps, const char* pe)
{
    const char* p = ps;
    int paren_count = 0;

    while (p < pe)
    {
        if (!tokenizer_->IsCharEscape(ps, p))
        {
            if (*p == '|' && paren_count == 0) break;

            paren_count += (*p == '(');
            paren_count -= (*p == ')');

            if (paren_count < 0) throw LexErrException(p, "parenthesis not matched!");
        }

        ++p;
    }

    if (p == pe) return ConstructSyntaxTreeImp(ps, pe);

    SynTreeNodeBase* left = ConstructSyntaxTree(ps, p - 1);
    SynTreeNodeBase* right = ConstructSyntaxTree(p + 1, pe);

    RegExpSynTreeNode* cat = new RegExpSynTreeNode(RegExpSynTreeNodeType_Or);
    cat->SetLeftChild(left);
    cat->SetRightChild(right);

    return cat;
}

SynTreeNodeBase* RegExpSyntaxTree::ConstructSyntaxTreeImp(const char* ps, const char* pe)
{
    if (!ps || !pe || ps > pe) return NULL;

    const char* te = tokenizer_->IsToken(ps, pe);
    if (te) return new RegExpSynTreeLeafNode(ps, te, leafIndex_++);

    bool is_parenthesis_unit = false;
    const char* us = NULL, *ue = NULL; // unit start, unit end
    const char* before_unit = NULL, *after_unit = NULL;

    tokenizer_->ExtractRegUnit(ps, pe, us, ue, before_unit, after_unit);
    is_parenthesis_unit = (*(before_unit + 1) == '(');

    SynTreeNodeBase* left = NULL, *right = NULL;
    if (before_unit >= ps)
    {
        left = ConstructSyntaxTreeImp(ps, before_unit);
    }

    if (is_parenthesis_unit)
    {
        right = ConstructSyntaxTree(us, ue);

        if (right)
        {
            RegExpSynTreeNode* rn = dynamic_cast<RegExpSynTreeNode*>(right);
            rn->SetUnit(true);
            unitMap_.push_back(rn);
        }
    }
    else
    {
        right = ConstructSyntaxTreeImp(us, ue);
    }

    if (after_unit <= pe && right)
    {
        int min = -1, max = -1;

        if (!tokenizer_->IsCharEscape(ps, after_unit)) // *(after_unit - 1) != '\\')
        {
            if (*after_unit == '*')
            {
                min = 0;
                max = INT_MAX;
            }
            else if (*after_unit == '+')
            {
                min = 1;
                max = INT_MAX;
            }
            else if (*after_unit == '?')
            {
                min = 0;
                max = 1;
            }
            else if (*after_unit == '{')
            {
                tokenizer_->ExtractRepeatCount(after_unit, pe, min, max);
            }
        }

        if (min >= 0)
        {
            RegExpSynTreeNode* sn = new RegExpSynTreeStarNode(min, max);
            sn->SetLeftChild(right);
            right = sn;
        }
    }

    if (!left) return right;
    if (!right) return left;

    RegExpSynTreeNodeType type = RegExpSynTreeNodeType_Concat;
    RegExpSynTreeNode* cat_node = new RegExpSynTreeNode(type);

    cat_node->SetLeftChild(left);
    cat_node->SetRightChild(right);
    return cat_node;
}

