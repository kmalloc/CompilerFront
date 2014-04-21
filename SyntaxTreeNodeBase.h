#ifndef SYNTAX_TREE_NODE_BASE_H_
#define SYNTAX_TREE_NODE_BASE_H_

#include <string>
#include "NonCopyable.h"

class SynTreeNodeBase: public NonCopyable
{
    public:

        SynTreeNodeBase(): left_(NULL), right_(NULL) {}
        virtual ~SynTreeNodeBase()
        {
            if (left_) delete left_;
            if (right_) delete right_;
        }

        void SetLeftChild(SynTreeNodeBase* left) { left_ = left; }
        void SetRightChild(SynTreeNodeBase* right) { right_ = right; }

        virtual const std::string& GetNodeText() const = 0;
        virtual bool IsLeafNode() const = 0;

        SynTreeNodeBase* GetLeftChild() const { return left_; }
        SynTreeNodeBase* GetRightChild() const { return right_; }

    protected:

        std::string text_;

        SynTreeNodeBase* left_;
        SynTreeNodeBase* right_;
};

#endif

