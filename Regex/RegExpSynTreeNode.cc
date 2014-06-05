#include "RegExpSynTreeNode.h"

#include <string>
#include <assert.h>

#include "RegExpTokenizer.h"
#include "Parsing/LexException.h"

RegExpSynTreeNode::RegExpSynTreeNode(const char*, const char*,
        RegExpSynTreeNodeType type, int pos)
    :isUnit_(0), position_(pos), type_(type)
{
}

RegExpSynTreeNode::RegExpSynTreeNode(RegExpSynTreeNodeType type, int pos)
    :isUnit_(0), position_(pos), type_(type)
{
}

bool RegExpSynTreeNode::IsLeafNode() const
{
    return type_ == RegExpSynTreeNodeType_Leaf;
}

const std::string& RegExpSynTreeNode::GetNodeText() const
{
    return text_;
}

// star node

RegExpSynTreeStarNode::RegExpSynTreeStarNode(int min, int max)
    :RegExpSynTreeNode(RegExpSynTreeNodeType_Star)
    ,min_(min), max_(max)
{
}

// leaf node

RegExpSynTreeLeafNode::RegExpSynTreeLeafNode(const char* s, const char* e, int pos)
    :RegExpSynTreeNode(s, e, RegExpSynTreeNodeType_Leaf, pos)
    ,textOrig_(s, e - s + 1)
    ,leafType_(RegExpSynTreeNodeLeafNodeType_None)
{
    if (*s == '\\')
    {
        leafType_ = RegExpSynTreeNodeLeafNodeType_Esc;
        text_ = RegExpTokenizer::ConstructEscapeString(s + 1, e);
    }
    else if (*s == '[')
    {
        if (*e != ']') throw LexErrException("[] does not match!", s);

        leafType_ = RegExpSynTreeNodeLeafNodeType_Alt;
        text_ = RegExpTokenizer::ConstructOptionString(s + 1, e - 1);
    }
    else if (*s == '^')
    {
        leafType_ = RegExpSynTreeNodeLeafNodeType_Head;
        text_ = *s;
    }
    else if (*s == '$')
    {
        leafType_ = RegExpSynTreeNodeLeafNodeType_Tail;
        text_ = *s;
    }
    else if (*s == '.')
    {
        leafType_ = RegExpSynTreeNodeLeafNodeType_Dot;
        text_ = *s;
    }
    else
    {
        leafType_ = RegExpSynTreeNodeLeafNodeType_Norm;
        text_ = *s;
    }
}

RegExpSynTreeLeafNode::RegExpSynTreeLeafNode(int pos)
    :RegExpSynTreeNode(NULL, NULL, RegExpSynTreeNodeType_Leaf, pos)
{
}

// reference node
RegExpSynTreeRefNode::RegExpSynTreeRefNode(const char* s, const char* e, int pos)
    :RegExpSynTreeLeafNode(pos), ref_(0)
{
    ++s; // skip '\'
    while (s <= e) ref_ = 10 * ref_ + *s++ - '0';

    assert(ref_ < 100 && ref_ >= 0);
    leafType_ = RegExpSynTreeNodeLeafNodeType_Ref;
}

//
