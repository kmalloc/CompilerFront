#include "OpCode.h"

#include <iostream>

#include <assert.h>

namespace ink {

void AstWalker::ReportError(const AstBase* t, const std::string& msg)
{
    std::cerr << msg << std::endl;
    std::cerr << "from file:" << t->GetLocFile()
    << ", line:" << t->GetLocLine() << std::endl;
}

size_t AstWalker::AddTable(InkTable* t)
{
    // TODO
    (void)t;
    return 0xffff;
}

VarInfo AstWalker::AddVar(const std::string& name, bool is_local, bool write)
{
    // all global variables stored in main's stack.

    size_t sp;
    VarInfo ret;

    if (is_local)
    {
        sp = scope_.size() - 1;

        do
        {
            auto& scope = scope_[sp];
            auto& idx = scope.var_pool_index_;

            auto it = idx.find(name);
            if (it == idx.end()) continue;

            ret.addr_idx_ = it->second;
            ret.scope_ = sp;
            return ret;

        } while (!write && sp--);

        ret.scope_ = sp;
    }
    else
    {
        sp = 0;
        auto& scope = scope_[sp];
        auto& idx = scope.var_pool_index_;

        auto it = idx.find(name);
        if (it != idx.end())
        {
            ret.scope_ = 0;
            ret.addr_idx_ = it->second;
            return ret;
        }
    }

    auto& scope = scope_[sp];
    auto& var = scope.var_pool_;
    auto& idx = scope.var_pool_index_;

    auto i = var.size();
    if (i >= MaxOpAddr())
    {
        assert(0 && "number of local variable exceed.");

        ret.addr_idx_ = ~0u;
        ret.addr_idx_ = ~0u;
        return ret;
    }

    ret.addr_idx_ = idx[name] = i;
    var.emplace_back(std::string(name));

    return ret;
}

uint32_t AstWalker::Visit(AstIntExp* node)
{
    auto v = node->GetValue();
    auto i = AddLiteralInt(v);
    auto r = s_func_.back()->FetchAndIncIdx();

    // load pointer of the literal value(which stored in slot i) to register r.
    ins_t in = OP_LDK | (i << InsAPos()) | (r << InsBPos());

    s_func_.back()->AddInstruction(in);

    return r;
}

uint32_t AstWalker::Visit(AstBoolExp* node)
{
    uint32_t v = node->GetValue();
    auto i = AddLiteralInt(v);
    auto r = s_func_.back()->FetchAndIncIdx();
    ins_t in = OP_LDK | (i << InsAPos()) | (r << InsBPos());

    s_func_.back()->AddInstruction(in);

    return r;
}

uint32_t AstWalker::Visit(AstFloatExp* node)
{
    auto v = node->GetValue();
    auto i = AddLiteralFloat(v);
    auto r = s_func_.back()->FetchAndIncIdx();
    ins_t in = OP_LDF | (i << InsAPos()) | (r << InsBPos());

    s_func_.back()->AddInstruction(in);

    return r;
}

uint32_t AstWalker::Visit(AstStringExp* node)
{
    const auto& v = node->GetValue();
    auto i = AddLiteralString(v);
    auto r = s_func_.back()->FetchAndIncIdx();
    ins_t in = OP_LDS | (i << InsAPos()) | (r << InsBPos());

    s_func_.back()->AddInstruction(in);

    return r;
}

uint32_t AstWalker::Visit(AstVarExp* v)
{
    auto is_local = v->IsLocal();
    const auto& name = v->GetName();

    auto c = AddVar(name, is_local, v->IsWriteMode());
    auto r = s_func_.back()->FetchAndIncIdx();

    auto s = c.scope_;
    auto i = c.addr_idx_;

    is_local = (s == scope_id_);
    ins_t in = is_local? OP_LDL:OP_LDU;

    // load pointer of the variable i to register r.
    in = in | (i << InsAPos()) | (r << InsBPos());

    if (!is_local)
    {
        in = in | (s << InsCPos());
    }

    s_func_.back()->AddInstruction(in);
    return r;
}

// op: 6 bits, out: 8 bits, l: 9 bits, r: 9 bits
// highest bit of l and r indicates whether it is const
void AstWalker::CreateBinInstruction(OpCode op,
                          uint32_t out, uint32_t l, uint32_t r)
{
    ins_t in = op << InsOpPos();

    in |= out << InsAPos();
    in |= l << InsBPos();
    in |= r << InsCPos();

    s_func_.back()->AddInstruction(in);
}

uint32_t AstWalker::Visit(AstBinaryExp* exp)
{
    OpCode op;
    auto ret = 0u;
    auto func = s_func_.back();

    switch (exp->GetOpType())
    {
        case TOK_ADD:
            op  = OP_ADD;
            ret = func->FetchAndIncIdx();
            break;
        case TOK_SUB:
            op  = OP_SUB;
            ret = func->FetchAndIncIdx();
            break;
        case TOK_MUL:
            op  = OP_MUL;
            ret = func->FetchAndIncIdx();
            break;
        case TOK_DIV:
            op  = OP_DIV;
            ret = func->FetchAndIncIdx();
            break;
        case TOK_POW:
            op  = OP_POW;
            ret = func->FetchAndIncIdx();
            break;
        case TOK_AS:
            op = OP_MOV;
            exp->GetLeftOperand()->SetWriteMode(true);
            break;
        default:
            assert(0 && "unregconize binary operator.");
            exit(0);
    }

    auto l_rdx = exp->GetLeftOperand()->Accept(*this);
    auto r_rdx = exp->GetRightOperand()->Accept(*this);

    CreateBinInstruction(op, ret, l_rdx, r_rdx);
    return ret;
}

std::unique_ptr<InkTable> AstWalker::CreateTable(const std::vector<Value>& vs)
{
    //TODO
    (void)vs;
    return std::unique_ptr<InkTable>();
}

uint32_t AstWalker::Visit(AstArrayExp* exp)
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

    std::unique_ptr<InkTable> t = CreateTable(vs);
    AddTable(t.get());

    // TODO, return value
    return 0;
}

uint32_t AstWalker::Visit(AstArrayIndexExp*)
{
    // TODO
    return 0;
}

uint32_t AstWalker::Visit(AstUnaryExp*)
{
    // TODO
    return 0;
}

uint32_t AstWalker::Visit(AstFuncProtoExp* f)
{
    const auto& name = f->GetName();
    const auto& params = f->GetParams();

    auto cur_func = s_func_.back();
    auto& func_pool = cur_func->sub_func_;
    auto& func_pool_index = cur_func->sub_func_index_;

    auto it = func_pool_index.find(name);
    if (it != func_pool_index.end())
    {
        auto ind = it->second;
        const auto& func = func_pool[ind];

        // TODO
        if (func.params_ == params) return 0x0000;

        ReportError(f, std::string("redefinition of function:") + name);

        return 0xffffff;
    }

    // a proto type contains just name & signature, nothing more, not a complete function yet.
    // so set the index of this prototype to 0xffffffff
    func_pool.emplace_back(std::string(name), params, ~0x0);

    return 0;
}

uint32_t AstWalker::Visit(AstFuncDefExp* f)
{
    auto proto = f->GetProto();
    proto->Accept(*this);

    auto name = proto->GetName();
    const auto& params = proto->GetParams();

    auto cur_func = s_func_.back();
    auto pos = cur_func->sub_func_.size();

    assert(cur_func->sub_func_index_.find(name) != cur_func->sub_func_index_.end());

    cur_func->sub_func_index_[name] = pos;
    cur_func->sub_func_.emplace_back(std::move(name), params, pos);

    s_func_.push_back(&cur_func->sub_func_[pos]);

    // TODO, store/serialize the function to somewhere appropriately.

    s_func_.pop_back();

    // TODO, gen instruction.
    return 0;
}

uint32_t AstWalker::Visit(AstScopeStatementExp* s)
{
    auto& body = s->GetBody();

    auto idx = scope_id_++;
    scope_.push_back(ScopeInfo());

    for(auto& ast: body)
    {
        ast->Accept(*this);
    }

    scope_id_--;
    scope_.pop_back();

    assert(idx == scope_id_);

    return 0;
}

uint32_t AstWalker::Visit(AstFuncCallExp*) { return 0; }
uint32_t AstWalker::Visit(AstRetExp*) { return 0; }
uint32_t AstWalker::Visit(AstIfExp*) { return 0; }
uint32_t AstWalker::Visit(AstTrueExp*) { return 0; }
uint32_t AstWalker::Visit(AstWhileExp*) { return 0; }
uint32_t AstWalker::Visit(AstForExp*) { return 0; }
uint32_t AstWalker::Visit(AstErrInfo*) { return 0; }

// code gen impl
std::string CodeGen::StartGenCode(const std::string& buff)
{
    if (buff.size() < 256 || !parser_) return "";

    parser_->SetBuffer(buff);
    parser_->StartParsing();

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

