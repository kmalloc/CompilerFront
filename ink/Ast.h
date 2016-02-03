#ifndef __INK_AST_H__
#define __INK_AST_H__

#include "Lexer.h"
#include "AstVisitor.h"
#include "Noncopyable.h"

#include <string>
#include <vector>
#include <memory>

#define DefSharedPtr(T) typedef std::shared_ptr<T> T##Ptr

namespace ink {

enum AstType
{
    AST_NONE = -1,
    AST_INT,
    AST_BOOL,
    AST_FLOAT,
    AST_STRING,
    AST_VAR,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_OP_UNARY,
    AST_OP_BINARY,
    AST_RET,
    AST_CLASS,
    AST_FUNC_PROTO,
    AST_FUNC_DEF,
    AST_FUNC_CALL,
    AST_ARR,
    AST_ARR_INDEX,
    AST_SCOPE,
    AST_ERR_INFO,

    AST_BUILTIN_ALL,
};

class ValueNode
{
    public:
        virtual int GetValue() = 0;
};
DefSharedPtr(ValueNode);

class AstBase: noncopyable
{
    public:
        AstBase(AstType t): type_(t), line_(-1) {}

        virtual ~AstBase() {}
        virtual ValueNodePtr Evaluate() = 0;
        virtual int64_t Accept(VisitorBase& v) = 0;

        inline bool IsError() const;
        int GetType() const { return type_; }

        void SetLocation(std::string file, int line)
        {
            line_ = line;
            file_ = std::move(file);
        }

        int GetLocLine() const { return line_; }
        std::string GetLocFile() const { return file_; }

    public:
        int type_;

        int line_;
        std::string file_;
};
DefSharedPtr(AstBase);

// TODO, sink parameter for c++11
class AstErrInfo: public AstBase
{
    public:
        explicit AstErrInfo(const std::string& info)
            : AstBase(AST_ERR_INFO), err_(info) {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::string& GetErrorInfo() const { return err_; }

    private:
        std::string err_;
};
DefSharedPtr(AstErrInfo);

bool AstBase::IsError() const { return dynamic_cast<const AstErrInfo*>(this); }

class AstIntExp: public AstBase
{
    public:
        explicit AstIntExp(int64_t val): AstBase(AST_INT), val_(val) {}
        ~AstIntExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        int64_t GetValue() const { return val_; }

    private:
        int64_t val_;
};
typedef std::shared_ptr<AstIntExp> AstIntExpPtr;

class AstBoolExp: public AstBase
{
    public:
        explicit AstBoolExp(bool b): AstBase(AST_BOOL), val_(b) {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        bool GetValue() const { return val_; }

    private:
        bool val_;
};
typedef std::shared_ptr<AstBoolExp> AstBoolExpPtr;

class AstFloatExp: public AstBase
{
    public:
        explicit AstFloatExp(double v): AstBase(AST_FLOAT), val_(v) {}
        ~AstFloatExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        double GetValue() const { return val_; }

    private:
        double val_;
};
typedef std::shared_ptr<AstFloatExp> AstFloatExpPtr;

// literal string
class AstStringExp: public AstBase
{
    public:
        explicit AstStringExp(std::string val)
            : AstBase(AST_STRING), val_(std::move(val)) {}

        ~AstStringExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::string& GetValue() const { return val_; }

    private:
        std::string val_;
};
typedef std::shared_ptr<AstStringExp> AstStringExpPtr;

class AstVarExp: public AstBase
{
    public:
        AstVarExp(std::string name, bool is_local)
            : AstBase(AST_VAR), is_local_(is_local), name_(std::move(name)) {}

        ~AstVarExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        bool IsLocal() const { return is_local_; }
        const std::string& GetName() const { return name_; }

    private:
        bool is_local_;
        std::string name_;
};
typedef std::shared_ptr<AstVarExp> AstVarExpPtr;

class AstUnaryExp: public AstBase
{
    public:
        AstUnaryExp(TokenType op, const AstBasePtr& arg)
            : AstBase(AST_OP_UNARY), op_(op), arg_(arg) {}

        ~AstUnaryExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        TokenType GetOpType() const { return op_; }
        AstBasePtr GetOperand() const { return arg_; }

    private:
        TokenType op_;
        AstBasePtr arg_;
};
typedef std::shared_ptr<AstUnaryExp> AstUnaryExpPtr;

class AstBinaryExp: public AstBase
{
    public:
        AstBinaryExp(TokenType op, const AstBasePtr& lhs, const AstBasePtr& rhs)
            : AstBase(AST_OP_BINARY), op_(op), lhs_(lhs), rhs_(rhs) {}

        ~AstBinaryExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        TokenType GetOpType() const { return op_; }
        const AstBasePtr& GetLeftOperand() const { return lhs_; }
        const AstBasePtr& GetRightOperand() const { return rhs_; }

    private:
        TokenType op_;
        AstBasePtr lhs_;
        AstBasePtr rhs_;
};
typedef std::shared_ptr<AstBinaryExp> AstBinaryExpPtr;

class AstFuncProtoExp: public AstBase
{
    public:
        AstFuncProtoExp(std::string fun, std::vector<std::string> args)
            : AstBase(AST_FUNC_PROTO), func_(std::move(fun))
            , params_(std::move(args))
        {
        }

        ~AstFuncProtoExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::string& GetName() const { return func_; }
        const std::vector<std::string>& GetParams() const { return params_; }

    private:
        std::string func_;
        std::vector<std::string> params_;
};
typedef std::shared_ptr<AstFuncProtoExp> AstFuncProtoExpPtr;

// represents a collections of expression in a pair of brace
class AstScopeStatementExp: public AstBase
{
    public:
        explicit AstScopeStatementExp(std::vector<AstBasePtr> exp)
            : AstBase(AST_SCOPE), exp_(std::move(exp))
        {
        }

        ~AstScopeStatementExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::vector<AstBasePtr>& GetBody() const { return exp_; }

    private:
        std::vector<AstBasePtr> exp_;
};

typedef std::shared_ptr<AstScopeStatementExp> AstScopeStatementExpPtr;

class AstFuncDefExp: public AstBase
{
    public:
        AstFuncDefExp(const AstFuncProtoExpPtr& proto, const AstScopeStatementExpPtr& body)
            : AstBase(AST_FUNC_DEF), proto_(proto), body_(body) {}

        ~AstFuncDefExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        AstScopeStatementExpPtr GetBody() const { return body_; }
        AstFuncProtoExpPtr GetProto() const { return proto_; }

    private:
        AstFuncProtoExpPtr proto_;
        AstScopeStatementExpPtr body_;
};
typedef std::shared_ptr<AstFuncDefExp> AstFuncDefExpPtr;

class AstFuncCallExp: public AstBase
{
    public:
        AstFuncCallExp(std::string fun, std::vector<AstBasePtr> args)
            : AstBase(AST_FUNC_CALL), func_(fun), args_(args) {}

        ~AstFuncCallExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::string& GetName() const { return func_; }
        const std::vector<AstBasePtr>& GetArgument() const { return args_; }

    private:
        std::string func_;
        std::vector<AstBasePtr> args_;
};
typedef std::shared_ptr<AstFuncCallExp> AstFuncCallExpPtr;

class AstArrayExp: public AstBase
{
    public:
        explicit AstArrayExp(const std::vector<AstBasePtr>& arr)
            : AstBase(AST_ARR), arr_(arr) {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::vector<AstBasePtr>& GetArray() const { return arr_; }

    private:
        std::vector<AstBasePtr> arr_;
};
typedef std::shared_ptr<AstArrayExp> AstArrayExpPtr;

class AstArrayIndexExp: public AstBase
{
    public:
        AstArrayIndexExp(std::string arr, const AstBasePtr& index)
            : AstBase(AST_ARR_INDEX), arr_(std::move(arr)), index_(index) {}

        ~AstArrayIndexExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        AstBasePtr GetIndexAst() const { return index_; }
        const std::string& GetArrayName() const { return arr_; }

    private:
        std::string arr_;
        AstBasePtr index_;
};
typedef std::shared_ptr<AstArrayIndexExp> AstArrayIndexExpPtr;

class AstRetExp: public AstBase
{
    public:
        explicit AstRetExp(const AstBasePtr& ret): AstBase(AST_RET), val_(ret) {}

        ~AstRetExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        AstBasePtr GetValue() const { return val_; }

    private:
        AstBasePtr val_;
};
typedef std::shared_ptr<AstRetExp> AstRetExpPtr;

class AstIfExp: public AstBase
{
    public:
        struct IfEntity
        {
            // in case of an else construct, cond always == null
            AstBasePtr cond;
            AstScopeStatementExpPtr exp;
        };

    public:
        explicit AstIfExp(const std::vector<IfEntity>& exe)
            : AstBase(AST_IF), exe_(exe) {}

        ~AstIfExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        std::vector<IfEntity>& GetBody() { return exe_; }

    private:
        std::vector<IfEntity> exe_;
};
typedef std::shared_ptr<AstIfExp> AstIfExpPtr;

class AstWhileExp: public AstBase
{
    public:
        AstWhileExp(const AstBasePtr& cond, const AstScopeStatementExpPtr& body)
            : AstBase(AST_WHILE), cond_(cond), body_(body) {}

        ~AstWhileExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        AstBasePtr GetCondition() const { return cond_; }
        AstScopeStatementExpPtr GetBody() const { return body_; }

    private:
        AstBasePtr cond_;
        AstScopeStatementExpPtr body_;
};
typedef std::shared_ptr<AstWhileExp> AstWhileExpPtr;

class AstForExp: public AstBase
{
    // python style for
    public:
        AstForExp(AstBasePtr var, AstBasePtr arr, const AstScopeStatementExpPtr& body)
            : AstBase(AST_FOR), var_(var), range_(arr), body_(body) {}

        ~AstForExp() {}

        virtual int64_t Accept(VisitorBase& v)
        {
            return v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

    private:
        AstBasePtr var_;
        AstBasePtr range_; // an array actually
        AstScopeStatementExpPtr body_;
};
typedef std::shared_ptr<AstForExp> AstForExpPtr;

inline bool IsError(AstBasePtr p) { return !p || p->IsError(); }

} // end ink

#endif

