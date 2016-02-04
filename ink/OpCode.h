#ifndef __INK_OPCODE_H__
#define __INK_OPCODE_H__

#include "Parser.h"
#include "Types.h"

#include <string>
#include <vector>

namespace ink {

/*
register based instruction modeling lua byte code.
instructions are all 4 byte long, 3 address code

 |op(6 bits)|A(9 bits)|B(9 bits)|C(8 bits)|
*/

using ins_t = uint32_t;

constexpr uint32_t GetReg64Num() { return 32; }
constexpr uint32_t GetReg32Num() { return 128; }

constexpr uint32_t InsSize() { return 32; }
constexpr uint32_t InsOpSize() { return 6; }
constexpr uint32_t InsOpASize() { return 9; }
constexpr uint32_t InsOpBSize() { return 9; }
constexpr uint32_t InsOpCSize() { return 8; }
constexpr uint32_t InsOpBxSize() { return InsOpBSize() + InsOpCSize(); }

constexpr uint32_t InsOpPos() { return 0; }
constexpr uint32_t InsAPos() { return InsOpPos() + InsOpSize(); }
constexpr uint32_t InsBPos() { return InsAPos() + InsOpASize(); }
constexpr uint32_t InsCPos() { return InsBPos() + InsOpBSize(); }
constexpr uint32_t InsBxPos() { return InsAPos() + InsOpASize(); }

constexpr uint32_t MaxOpAddr() { return 1 << InsOpBSize(); }

enum OpCode
{
    OP_NOP,
    OP_MOV,
    OP_INI, // |OP|A|, set A to nil.
    /*
     * load instruction in form of: |op|A|B|
     * meaning: load pointer of value stored in address A(which may be a local addr or a global addr) to register B.
     */
    OP_LDL, // load a local variable
    OP_LDG, // load a global variable
    OP_LDK, // load literal int
    OP_LDS, // load literal string
    OP_LDF, // load literal float
    OP_LDU, // load an upvalue.
    OP_SET_UPVAL, // create an upvalue
    OP_CLOSE_UPVAL, // close an upvalue

    OP_ST, // store value to local
    OP_GST, // store value to global
    OP_JMP,
    OP_EQ,
    OP_LT,
    OP_LE, // less equal
    OP_TEST,
    OP_CALL,
    OP_RET,
    OP_LOOP,

    // arithmetic
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_FADD,
    OP_FSUB,
    OP_FMUL,
    OP_DIV,
    OP_POW,
    OP_NOT,
    OP_OR,
    OP_XOR,
    OP_AND,
    OP_SHL,
    OP_SHR,
    OP_INV, //~

    OP_NEW_TABLE, // create table
    OP_SET_TABLE, // set table

    // maximum instruction.
    OP_MAX = (1 << 6),
};

struct UpValue;

struct Variable
{
    explicit Variable(std::string name)
            : name_(std::move(name))
    {
    }

    Variable(const Variable &) =  default;

    Variable(Variable && v)
    {
        if (this == &v) return;

        name_ = std::move(v.name_);
    }

    Value val_;
    std::string name_;

    // close all upvalues when current variable is out of scope.
    std::vector<UpValue*> upvalue_;
};

struct UpValue
{
    bool own_; // is closed or not
    Variable* value_;
};

struct ScopeInfo
{
    std::vector<Variable> var_pool_;
    std::unordered_map<std::string, size_t> var_pool_index_; // value to index
};

struct CodeFunc
{
    // sink parameter
    CodeFunc(std::string name, std::vector<std::string> param)
            : name_(std::move(name)), params_(std::move(param)), rdx_(0)
    {
        ins_.reserve(64);
    }

    CodeFunc(const CodeFunc&) = delete;

    CodeFunc(CodeFunc&& fun)
    {
        if (this == &fun) return;

        name_ = std::move(fun.name_);
        params_ = std::move(fun.params_);
        ins_ = std::move(fun.ins_);
        upvalue_ = std::move(fun.upvalue_);
    }

    void AddInstruction(uint32_t in)
    {
        ins_.push_back(in);
    }

    uint32_t FetchAndIncIdx() { return rdx_++; }

    std::string name_; // function name.
    std::vector<std::string> params_;

    std::vector<std::unique_ptr<UpValue>> upvalue_;

    uint32_t rdx_;
    std::vector<ins_t> ins_;

    std::vector<CodeFunc> sub_func_;
    // function name to index of sub_func_
    std::unordered_map<std::string, size_t> sub_func_index_;

    // const values attached to the function.
    ConstPool const_val_pool_;
};

struct VarInfo
{
    size_t scope_;
    size_t addr_idx_;
};

struct CodeClass
{
    CodeClass() {}
    CodeClass(CodeClass&&) {}

private:
    std::string name_;
    std::vector<CodeFunc> func_; // member function
    std::vector<std::string> mem_; // member variables
};

class AstWalker: public VisitorBase
{
public:
    AstWalker()
            : debug_(false), scope_id_(0)
            , main_func_("main", std::vector<std::string>())
    {
        s_func_.push_back(&main_func_);
    }

    void EnableDebugInfo(bool enable)
    {
        debug_ = enable;
    }

    void ReportError(const AstBase* t, const std::string& msg);

    size_t AddLiteralInt(int64_t v)
    {
        return s_func_.back()->const_val_pool_.AddConst(v);
    }

    size_t AddLiteralFloat(double v)
    {
        return s_func_.back()->const_val_pool_.AddConst(v);
    }

    size_t AddLiteralString(const std::string& v)
    {
        return s_func_.back()->const_val_pool_.AddConst(v);
    }

    size_t AddTable(InkTable* t);
    VarInfo AddVar(const std::string& name, bool is_local, bool write);

    // op: 6 bits, out: 8 bits, l: 9 bits, r: 9 bits
    // highest bit of l and r indicates whether it is const
    void CreateBinInstruction(OpCode op, uint32_t out, uint32_t l, uint32_t r);

    std::unique_ptr<InkTable> CreateTable(const std::vector<Value>& vs);

    virtual int64_t Visit(AstIntExp* node);
    virtual int64_t Visit(AstBoolExp* node);
    virtual int64_t Visit(AstFloatExp* node);
    virtual int64_t Visit(AstStringExp* node);
    virtual int64_t Visit(AstVarExp* v);
    virtual int64_t Visit(AstBinaryExp* exp);
    virtual int64_t Visit(AstArrayExp* exp);

    virtual int64_t Visit(AstArrayIndexExp*);
    virtual int64_t Visit(AstUnaryExp*);
    virtual int64_t Visit(AstFuncProtoExp* f);
    virtual int64_t Visit(AstFuncDefExp* f);
    virtual int64_t Visit(AstScopeStatementExp* s);

    virtual int64_t Visit(AstFuncCallExp*);
    virtual int64_t Visit(AstRetExp*);
    virtual int64_t Visit(AstIfExp*);
    virtual int64_t Visit(AstTrueExp*);
    virtual int64_t Visit(AstWhileExp*);
    virtual int64_t Visit(AstForExp*);
    virtual int64_t Visit(AstErrInfo*);

private:
    bool debug_;

    // current scope.
    size_t scope_id_;
    std::vector<ScopeInfo> scope_;

    CodeFunc main_func_;
    std::vector<CodeFunc*> s_func_;
};

class CodeGen
{
    public:
        CodeGen();
        ~CodeGen();

        void SetParser(const ParserPtr& p) { parser_ = p; }
        std::string StartGenCode(unsigned char* buff, size_t sz);

    private:
        ParserPtr parser_;
};

} // end namespace

#endif

