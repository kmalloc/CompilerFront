#include "OpCode.h"
#include "Lexer.h"
#include "AstVisitor.h"

#include <stack>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include <assert.h>

namespace ink {

struct CodeVar
{
    explicit CodeVar(std::string name)
        : name_(std::move(name))
    {
    }

    CodeVar(CodeVar&& v)
    {
        if (this == &v) return;

        name_ = std::move(v.name_);
        ins_ = std::move(v.ins_);
    }

    std::string name_;
    std::vector<int> ins_;
};

struct CodeFunc
{
    // sink parameter
    CodeFunc(std::string name, std::vector<std::string> param)
        : name_(std::move(name)), params_(std::move(param))
    {
        ins_.reserve(64);
    }

    CodeFunc(CodeFunc&& fun)
    {
        if (this == &fun) return;

        name_ = std::move(fun.name_);
        params_ = std::move(fun.params_);
        ins_ = std::move(fun.ins_);
        var_pool_ = std::move(fun.var_pool_);
    }

    std::string name_;
    std::vector<int> ins_;
    std::vector<std::string> params_;
    std::vector<std::string> var_pool_;

    std::vector<CodeFunc> sub_func_;
    std::unordered_map<std::string, size_t> sub_func_index_;
};

struct CodeClass
{
    CodeClass() {}
    CodeClass(CodeClass&&) {}

    std::string name_;
    std::vector<CodeFunc> func_; // member function
    std::vector<std::string> mem_; // member variables
};

class AstWalker: public VisitorBase
{
    public:
        AstWalker()
            : main_func_("main", std::vector<std::string>())
        {
            s_func_.push(&main_func_);
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
            auto& v = node->GetValue();
            auto it = str_pool_index_.find(v);
            if (it != str_pool_index_.end()) return;

            auto i = str_pool_.size();
            str_pool_.push_back(v);
            str_pool_index_[v] = i;
        }

        virtual void Visit(AstVarExp* v)
        {
            auto& name = v->GetName();
            auto& var = s_func_.top()->var_pool_;

            auto it = std::find(var.begin(), var.end(), name);
            if (it != var.end()) return;

            var.emplace_back(name);
        }

        virtual void Visit(AstBinaryExp* exp)
        {
            OpCode op = OP_NOP;
            switch (exp->GetOpType())
            {
                case TOK_ADD:
                    op = OP_ADD;
                    break;
                case TOK_SUB:
                    op = OP_SUB;
                    break;
                case TOK_MUL:
                    op = OP_MUL;
                    break;
                case TOK_DIV:
                    op = OP_DIV;
                    break;
                case TOK_POW:
                    op = OP_POW;
                    break;
                default:
                    break;
            }
        }

        virtual void Visit(AstArrayExp*) {}
        virtual void Visit(AstArrayIndexExp*) {}

        virtual void Visit(AstUnaryExp*) {}

        virtual void Visit(AstFuncProtoExp* f)
        {
            auto& name = f->GetName();
            auto& params = f->GetParams();

            auto cur_func = s_func_.top();
            auto& func_pool = cur_func->sub_func_;
            auto& func_pool_index = cur_func->sub_func_index_;

            auto it = func_pool_index.find(name);
            if (it != func_pool_index.end())
            {
                auto ind = it->second;
                auto& func = func_pool[ind];

                if (func.params_ == params) return;

                ReportError(f, std::string("redefinition of function:") + name);
                return;
            }

            auto ind = func_pool.size();
            func_pool_index[name] = ind;
            func_pool.emplace_back(name, params);
        }

        virtual void Visit(AstFuncDefExp* f)
        {
            auto proto = f->GetProto();
            proto->Accept(*this);

            auto& name = proto->GetName();
            auto& params = proto->GetParams();

            auto cur_func = s_func_.top();
            auto pos = cur_func->sub_func_.size();

            cur_func->sub_func_index_[name] = pos;
            cur_func->sub_func_.emplace_back(name, params);

            s_func_.push(&cur_func->sub_func_[pos]);
            // TODO

            s_func_.pop();
        }

        virtual void Visit(AstScopeStatementExp* s)
        {
            auto& body = s->GetBody();
            // TODO
            (void)body;
        }

        virtual void Visit(AstFuncCallExp*) {}
        virtual void Visit(AstRetExp*) {}
        virtual void Visit(AstIfExp*) {}
        virtual void Visit(AstTrueExp*) {}
        virtual void Visit(AstWhileExp*) {}
        virtual void Visit(AstForExp*) {}
        virtual void Visit(AstErrInfo*) {}

    private:
        CodeFunc main_func_;
        std::stack<CodeFunc*> s_func_;

        // const pool

        std::vector<int64_t> int_pool_;
        std::unordered_map<int64_t, size_t> int_pool_index_; // value to index

        std::vector<double> float_pool_;
        std::unordered_map<double, size_t> float_pool_index_;

        std::vector<std::string> str_pool_;
        std::unordered_map<std::string, size_t> str_pool_index_;

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

