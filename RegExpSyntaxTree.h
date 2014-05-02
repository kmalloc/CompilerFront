#ifndef REG_EXP_SYNTAX_TREE_H_
#define REG_EXP_SYNTAX_TREE_H_

#include <vector>
#include "SyntaxTreeBase.h"
#include "RegExpSynTreeNode.h"

class RegExpTokenizer;

class RegExpSyntaxTree: public SyntaxTreeBase
{
    public:

        RegExpSyntaxTree();
        ~RegExpSyntaxTree();

        bool BuildSyntaxTree(const char* ps, const char* pe);
        virtual SynTreeNodeBase* GetSynTree() const { return synTreeRoot_; }

        virtual int GetNodeNumber() const { return leafIndex_ + 1; }

    private:

        virtual SynTreeNodeBase* ConstructSyntaxTree(const char* ps, const char* pe);
        virtual SynTreeNodeBase* ConstructSyntaxTreeImp(const char* ps, const char* pe);

    private:

        int leafIndex_;
        const char* txtEnd_;
        const char* txtStart_;

        RegExpTokenizer* tokenizer_;
        RegExpSynTreeNode* synTreeRoot_;
        std::vector<RegExpSynTreeNode*> unitMap_;
};

#endif

