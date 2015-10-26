#include "OpCode.h"
#include "Types.h"

#include <stack>
#include <iostream>

#include <assert.h>

namespace ink {

constexpr uint32_t GetReg64Num() { return 32; }
constexpr uint32_t GetReg32Num() { return 128; }

constexpr uint32_t InsSize() { return 32; }
constexpr uint32_t InsOpSize() { return 6; }
constexpr uint32_t InsOpASize() { return 8; }
constexpr uint32_t InsOpBSize() { return 9; }
constexpr uint32_t InsOpCSize() { return 9; }

constexpr uint32_t InsAPos() { return 18; }
constexpr uint32_t InsBPos() { return 9; }
constexpr uint32_t InsCPos() { return 0; }
constexpr uint32_t InsOpPos() { return 26; }
constexpr uint32_t MaxConstPoolNum() { return (1 << InsOpSize()) - 1; }

struct CodeVar
{
    explicit CodeVar(std::string name)
        : val_(NULL), name_(std::move(name))
    {
    }

    CodeVar(CodeVar&& v)
    {
        if (this == &v) return;

        name_ = std::move(v.name_);
    }

    void* val_; // TODO, should handle simple type & array & table.
    std::string name_;
};

struct CodeFunc
{
    // sink parameter
    CodeFunc(std::string name, std::vector<std::string> param)
        : name_(std::move(name)), params_(std::move(param))
        , rdx_(0)
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

    void AddInstruction(uint32_t in)
    {
        ins_.push_back(in);
    }

    void IncRegIdx() { ++rdx_; }
    uint32_t GetRegIdx() const { return rdx_; }

    std::string name_;
    std::vector<std::string> params_; // TODO, not used?

    std::vector<CodeVar> var_pool_;
    std::unordered_map<std::string, size_t> var_pool_index_; // value to index

    uint32_t rdx_;
    std::vector<uint32_t> ins_;

    uint32_t reg32_[GetReg32Num()];
    uint64_t reg64_[GetReg64Num()];

    std::vector<CodeFunc> sub_func_;
    // function name to index of sub_func_
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
            : debug_(false)
            , main_func_("main", std::vector<std::string>())
        {
            s_func_.push(&main_func_);
        }

        void EnableDebugInfo(bool enable)
        {
            debug_ = enable;
        }

        void ReportError(const AstBase* t, const std::string& msg)
        {
            std::cerr << msg << std::endl;
            std::cerr << "from file:" << t->GetLocFile()
                << ", line:" << t->GetLocLine() << std::endl;
        }

        size_t AddLiteralInt(int64_t v)
        {
            auto i  = 0xffffu;
            auto it = int_pool_index_.find(v);
            if (it == int_pool_index_.end())
            {
                i = int_pool_.size();
                assert(i < MaxConstPoolNum());

                int_pool_.push_back(v);
                int_pool_index_[v] = i;
            }
            else
            {
                i = it->second;
            }

            return i | (1 << InsOpASize());
        }

        size_t AddLiteralFloat(double v)
        {
            auto i  = 0xffffu;
            auto it = float_pool_index_.find(v);

            if (it == float_pool_index_.end())
            {
                i = float_pool_.size();
                assert(i < MaxConstPoolNum());

                float_pool_.push_back(v);
                float_pool_index_[v] = i;
            }
            else
            {
                i = it->second;
            }

            return i | (1 << InsOpASize());
        }

        size_t AddLiteralString(const std::string& v)
        {
            auto i  = 0xffffu;
            auto it = str_pool_index_.find(v);

            if (it == str_pool_index_.end())
            {
                i = str_pool_.size();
                assert(i < MaxConstPoolNum());

                str_pool_.push_back(v);
                str_pool_index_[v] = i;
            }
            else
            {
                i = it->second;
            }

            return i | (1 << InsOpASize());
        }

        size_t AddTable(Table* t)
        {
            // TODO
            (void)t;
            return 0xffff;
        }

        size_t AddVar(const std::string& name)
        {
            auto i = 0xffffu;
            auto& var = s_func_.top()->var_pool_;
            auto& idx = s_func_.top()->var_pool_index_;

            auto it = idx.find(name);
            if (it == idx.end())
            {
                i = var.size();
                assert(i < MaxConstPoolNum());

                idx[name] = i;
                var.emplace_back(name);
            }
            else
            {
                i = it->second;
            }

            return i | (1 << InsOpASize());
        }

        virtual void Visit(AstIntExp* node)
        {
            auto v = node->GetValue();
            auto i = AddLiteralInt(v);
            SetTmpRes(i);
        }

        virtual void Visit(AstBoolExp* node)
        {
            int64_t v = node->GetValue();
            auto i = AddLiteralInt(v);
            SetTmpRes(i);
        }

        virtual void Visit(AstFloatExp* node)
        {
            auto v = node->GetValue();
            auto i = AddLiteralFloat(v);
            SetTmpRes(i);
        }

        virtual void Visit(AstStringExp* node)
        {
            const auto& v = node->GetValue();
            auto i = AddLiteralString(v);
            SetTmpRes(i);
        }

        virtual void Visit(AstVarExp* v)
        {
            const auto& name = v->GetName();
            auto i = AddVar(name);
            SetTmpRes(i);
        }

        void SetTmpRes(uint32_t res)
        {
            tmpRes_ = res;
        }

        uint32_t GetTmpRes() const
        {
            return tmpRes_;
        }

        // op: 6 bits, out: 8 bits, l: 9 bits, r: 9 bits
        // highest bit of l and r indicates whether it is const
        void CreateBinInstruction(OpCode op,
                uint32_t out, uint32_t l, uint32_t r)
        {
            uint32_t in = 0xFC000000 & (op << InsOpPos());

            in |= 0x03FC0000 & (out << InsAPos());
            in |= 0x0003FE00 & (l << InsBPos());
            in |= 0x000001FF & (r << InsCPos());

            s_func_.top()->AddInstruction(in);
        }

        virtual void Visit(AstBinaryExp* exp)
        {
            OpCode op = OP_NOP;
            auto func = s_func_.top();
            auto ret = 0;

            switch (exp->GetOpType())
            {
                case TOK_ADD:
                    op  = OP_ADD;
                    ret = func->GetRegIdx();
                    func->IncRegIdx();
                    break;
                case TOK_SUB:
                    op  = OP_SUB;
                    ret = func->GetRegIdx();
                    func->IncRegIdx();
                    break;
                case TOK_MUL:
                    op  = OP_MUL;
                    ret = func->GetRegIdx();
                    func->IncRegIdx();
                    break;
                case TOK_DIV:
                    op  = OP_DIV;
                    ret = func->GetRegIdx();
                    func->IncRegIdx();
                    break;
                case TOK_POW:
                    op  = OP_POW;
                    ret = func->GetRegIdx();
                    func->IncRegIdx();
                    break;
                case TOK_AS:
                    op = OP_MOV;
                    break;
                default:
                    break;
            }

            exp->GetLeftOperand()->Accept(*this);
            auto l_rdx = GetTmpRes();

            exp->GetRightOperand()->Accept(*this);
            auto r_rdx = GetTmpRes();

            SetTmpRes(ret);
            CreateBinInstruction(op, ret, l_rdx, r_rdx);
        }

        std::unique_ptr<Table> CreateTable(const std::vector<Value>& vs)
        {
            //TODO
            (void)vs;
            return std::unique_ptr<Table>();
        }

        virtual void Visit(AstArrayExp* exp)
        {
            std::vector<Value> vs;
            const auto& arr = exp->GetArray();

            vs.reserve(arr.size());
            for (auto& p: arr)
            {
                Value v;
                p->Accept(*this);
                vs.push_back(v);
            }

            std::unique_ptr<Table> t = CreateTable(vs);
            AddTable(t.get());
        }

        virtual void Visit(AstArrayIndexExp*) {}

        virtual void Visit(AstUnaryExp*) {}

        virtual void Visit(AstFuncProtoExp* f)
        {
            const auto& name = f->GetName();
            const auto& params = f->GetParams();

            auto cur_func = s_func_.top();
            auto& func_pool = cur_func->sub_func_;
            auto& func_pool_index = cur_func->sub_func_index_;

            auto it = func_pool_index.find(name);
            if (it != func_pool_index.end())
            {
                auto ind = it->second;
                const auto& func = func_pool[ind];
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

            const auto& name = proto->GetName();
            const auto& params = proto->GetParams();

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
        bool debug_;
        uint32_t tmpRes_;
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

