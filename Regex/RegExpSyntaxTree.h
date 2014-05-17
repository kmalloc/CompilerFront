#ifndef REG_EXP_SYNTAX_TREE_H_
#define REG_EXP_SYNTAX_TREE_H_

#include <vector>
#include "Parsing/SyntaxTreeBase.h"
#include "RegExpSynTreeNode.h"

class RegExpTokenizer;

class RegExpSyntaxTree: public SyntaxTreeBase
{
    public:

        RegExpSyntaxTree();
        ~RegExpSyntaxTree();

        bool BuildSyntaxTree(const char* ps, const char* pe);
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        bool HasRefNode() const { return hasReferNode_; }
#endif
        virtual int GetNodeNumber() const { return leafIndex_ + 1; }
        virtual SynTreeNodeBase* GetSynTree() const { return synTreeRoot_; }
    private:

        virtual SynTreeNodeBase* ConstructSyntaxTree(const char* ps, const char* pe);
        virtual SynTreeNodeBase* ConstructSyntaxTreeImp(const char* ps, const char* pe);

    private:

        int leafIndex_;
        int unitCounter_;
#ifdef SUPPORT_REG_EXP_BACK_REFERENCE
        bool  hasReferNode_;
#endif
        const char* txtEnd_;
        const char* txtStart_;

        RegExpTokenizer* tokenizer_;
        RegExpSynTreeNode* synTreeRoot_;
};

#endif

