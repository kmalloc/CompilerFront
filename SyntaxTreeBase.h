#ifndef SYNTAX_TREE_BASE_H_
#define SYNTAX_TREE_BASE_H_

#include "NonCopyable.h"
#include "SyntaxTreeNodeBase.h"

class SyntaxTreeBase: public NonCopyable
{
    public:

        SyntaxTreeBase() {}
        virtual ~SyntaxTreeBase() {}

        virtual SynTreeNodeBase* GetSynTree() const { return NULL; }

    protected:

        virtual int GetNodeNumber() const = 0;
        virtual SynTreeNodeBase* ConstructSyntaxTree(const char* ps, const char* pe) = 0;
        virtual SynTreeNodeBase* ConstructSyntaxTreeImp(const char* ps, const char* pe) = 0;
};

#endif

