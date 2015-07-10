#include "OpCode.h"
#include "AstVisitor.h"

#include <stack>
#include <vector>
#include <iostream>
#include <unordered_map>

namespace ink {

struct CodeVar
{
    explicit CodeVar(std::string name)
        : name_(std::move(name))
    {
    }

    std::string name_;
    std::vector<int> ins_;
};

struct CodeFunc
{
    // sink parameter
    CodeFunc(std::string name, std::vector<std::string> param)
        : name_(std::move(name)), params_(param)
    {
        ins_.reserve(64);
    }

    std::string name_;
    std::vector<std::string> params_;
    std::vector<int> ins_;
    std::vector<std::string> var_pool_;
};

struct CodeClass
{
    std::string name_;
    std::vector<CodeFunc> func_; // member function
    std::vector<std::string> mem_; // member variables
};

class AstWalker: public VisitorBase
{
    public:
        AstWalker()
        {
            CodeFunc main("main", std::vector<std::string>());
            func_.push(main);
        }

        void ReportError(const AstBase* t, const std::string& msg)
        {
            std::cerr << msg << std::endl;
            std::cerr << "from file:" << t->GetLocFile()
                << ", line:" << t->GetLocLine() << std::endl;
        }

        void AddLiteralInt(int64_t v)
        {
            auto it = int_pool_index_.find(v);
            if (it != int_pool_index_.end()) return;

            auto i = int_pool_.size();
            int_pool_.push_back(v);
            int_pool_index_[v] = i;
        }

        virtual void Visit(AstIntExp* node)
        {
            auto v = node->GetValue();
            AddLiteralInt(v);
        }

        virtual void Visit(AstBoolExp* node)
        {
            int64_t v = node->GetValue();
            AddLiteralInt(v);
        }

        virtual void Visit(AstFloatExp* node)
        {
            auto v = node->GetValue();
            auto it = float_pool_index_.find(v);
            if (it != float_pool_index_.end()) return;

            auto i = float_pool_.size();
            float_pool_.push_back(v);
            float_pool_index_[v] = i;
        }

        virtual void Visit(AstStringExp* node)
        {
            auto v = node->GetValue();
            auto it = str_pool_index_.find(v);
            if (it != str_pool_index_.end()) return;

            auto i = str_pool_.size();
            str_pool_.push_back(v);
            str_pool_index_[v] = i;
        }

        virtual void Visit(AstFuncProtoExp* f)
        {
            auto name = f->GetName();
            auto params = f->GetParams();

            auto it = func_pool_index_.find(name);
            if (it != func_pool_index_.end())
            {
                auto ind = it->second;
                auto func = func_pool_[ind];

                if (func.params_ == params) return;

                ReportError(f, std::string("redefinition of function:") + name);
                return;
            }

            auto ind = func_pool_.size();
            func_pool_index_[name] = ind;
            func_pool_.emplace_back(name, params);
        }

        virtual void Visit(AstScopeStatementExp*) {}
        virtual void Visit(AstFuncDefExp*) {}
        virtual void Visit(AstFuncCallExp*) {}
        virtual void Visit(AstArrayExp*) {}
        virtual void Visit(AstArrayIndexExp*) {}
        virtual void Visit(AstVarExp*) {}
        virtual void Visit(AstUnaryExp*) {}
        virtual void Visit(AstBinaryExp*) {}
        virtual void Visit(AstRetExp*) {}
        virtual void Visit(AstIfExp*) {}
        virtual void Visit(AstTrueExp*) {}
        virtual void Visit(AstWhileExp*) {}
        virtual void Visit(AstForExp*) {}
        virtual void Visit(AstErrInfo*) {}

    private:
        std::stack<CodeFunc> func_;
        std::stack<CodeClass> class_;

        // const pool

        std::vector<int64_t> int_pool_;
        // value to index
        std::unordered_map<int64_t, size_t> int_pool_index_;

        std::vector<double> float_pool_;
        std::unordered_map<double, size_t> float_pool_index_;

        std::vector<std::string> str_pool_;
        std::unordered_map<std::string, size_t> str_pool_index_;

        std::vector<CodeFunc> func_pool_;
        std::unordered_map<std::string, size_t> func_pool_index_;
};

CodeGen::CodeGen()
{
}

CodeGen::~CodeGen()
{
}

std::string CodeGen::StartGenCode(unsigned char* buff, size_t sz)
{
    if (sz < 256 || !parser_) return "";

    (void)buff; (void)sz;

    const std::vector<AstBasePtr>& ast = parser_->GetResult();
    if (ast.empty()) return "no ast input";

    AstWalker walker;
    for (auto t: ast)
    {
        t->Accept(walker);
    }

    return "";
}

}

