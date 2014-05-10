#ifndef REG_EX_SYNTAX_TREE_NODE_H_
#define REG_EX_SYNTAX_TREE_NODE_H_

#include "SyntaxTreeNodeBase.h"

enum RegExpSynTreeNodeType
{
    RegExpSynTreeNodeType_None,
    RegExpSynTreeNodeType_Leaf,
    RegExpSynTreeNodeType_Concat,
    RegExpSynTreeNodeType_Or,
    RegExpSynTreeNodeType_Star,
};

enum RegExpSynTreeNodeLeafNodeType
{
    RegExpSynTreeNodeLeafNodeType_None,
    RegExpSynTreeNodeLeafNodeType_Norm,
    RegExpSynTreeNodeLeafNodeType_Esc,
    RegExpSynTreeNodeLeafNodeType_Alt, //[]
    RegExpSynTreeNodeLeafNodeType_Dot, //.
    RegExpSynTreeNodeLeafNodeType_Head, //^
    RegExpSynTreeNodeLeafNodeType_Tail, //$
    // TODO
    RegExpSynTreeNodeLeafNodeType_Ref, // for back-reference, \1, \2, etc
};

class RegExpSynTreeNode: public SynTreeNodeBase
{
    public:

        explicit RegExpSynTreeNode(RegExpSynTreeNodeType type, int pos = -1);
        ~RegExpSynTreeNode() {}

        int IsUnit() const { return isUnit_; }
        void SetUnit(bool isUnit) { isUnit_ += isUnit; }
        virtual bool IsLeafNode() const;
        virtual const std::string& GetNodeText() const;
        virtual int  GetNodePosition() const { return position_; }

        RegExpSynTreeNodeType GetNodeType() const { return type_; }

    protected:

        RegExpSynTreeNode(const char* s, const char* e,
                RegExpSynTreeNodeType type, int pos = -1);

    private:

        int isUnit_;
        int position_;
        RegExpSynTreeNodeType type_;
};

class RegExpSynTreeStarNode: public RegExpSynTreeNode
{
    public:

        RegExpSynTreeStarNode(int min, int max);

        int GetMinRepeat() const { return min_; }
        int GetMaxRepeat() const { return max_; }

    private:

        int min_;
        int max_;
};

class RegExpSynTreeLeafNode: public RegExpSynTreeNode
{
    public:

        RegExpSynTreeLeafNode(const char* s, const char* e, int pos);
        RegExpSynTreeNodeLeafNodeType GetLeafNodeType() const { return leafType_; }

    protected:

        RegExpSynTreeLeafNode(int pos);

    protected:

        RegExpSynTreeNodeLeafNodeType leafType_;
};

class RegExpSynTreeRefNode: public RegExpSynTreeLeafNode
{
    public:

        RegExpSynTreeRefNode(const char*, const char* e, int pos);

        short GetRef() const { return ref_; }

    private:

        int ref_;
};

#endif

