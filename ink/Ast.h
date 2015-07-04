#ifndef __INK_AST_H__
#define __INK_AST_H__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "Lexer.h"
#include "AstVisitor.h"

namespace ink {

enum AstType
{
    AST_NONE = -1,
    AST_INT,
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

    AST_BUILTIN_ALL,
};

class ValueNode
{
    public:
        virtual int GetValue() = 0;
};
typedef boost::shared_ptr<ValueNode> ValueNodePtr;

class AstBase: boost::noncopyable
{
    public:
        AstBase(AstType t): type_(t) {}

        virtual ~AstBase() {}
        virtual ValueNodePtr Evaluate() = 0;
        virtual void Accept(VisitorBase& v) = 0;

        int GetType() const { return type_; }

    public:
        int type_;
};
typedef boost::shared_ptr<AstBase> AstBasePtr;

class AstIntExp: public AstBase
{
    public:
        explicit AstIntExp(int64_t val): AstBase(AST_INT), val_(val) {}
        ~AstIntExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstIntExp> AstIntExpPtr;

class AstFloatExp: public AstBase
{
    public:
        explicit AstFloatExp(double v): AstBase(AST_FLOAT), val_(v) {}
        ~AstFloatExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstFloatExp> AstFloatExpPtr;

// literal string
class AstStringExp: public AstBase
{
    public:
        explicit AstStringExp(const std::string& val)
            : AstBase(AST_STRING), val_(val) {}

        ~AstStringExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstStringExp> AstStringExpPtr;

class AstVarExp: public AstBase
{
    public:
        explicit AstVarExp(const std::string& name)
            : AstBase(AST_VAR), name_(name) {}

        ~AstVarExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        const std::string& GetName() const { return name_; }

    private:
        std::string name_;
};
typedef boost::shared_ptr<AstVarExp> AstVarExpPtr;

class AstUnaryExp: public AstBase
{
    public:
        AstUnaryExp(TokenType op, const AstBasePtr& arg)
            : AstBase(AST_OP_UNARY), op_(op), arg_(arg) {}

        ~AstUnaryExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstUnaryExp> AstUnaryExpPtr;

class AstBinaryExp: public AstBase
{
    public:
        AstBinaryExp(TokenType op, const AstBasePtr& lhs, const AstBasePtr& rhs)
            : AstBase(AST_OP_BINARY), op_(op), lhs_(lhs), rhs_(rhs) {}

        ~AstBinaryExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstBinaryExp> AstBinaryExpPtr;

class AstFuncProtoExp: public AstBase
{
    public:
        AstFuncProtoExp(const std::string& fun, const std::vector<std::string>& args)
            : AstBase(AST_FUNC_PROTO), func_(fun), args_(args) {}

        ~AstFuncProtoExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

    private:
        std::string func_;
        std::vector<std::string> args_;
};
typedef boost::shared_ptr<AstFuncProtoExp> AstFuncProtoExpPtr;

// represents a collections of expression in a pair of brace
class AstScopeStatementExp: public AstBase
{
    public:
        // attention: parameter will be cleared, this is to improve efficiency
        // should use rvalue reference if not need to support old compiler
        explicit AstScopeStatementExp(std::vector<AstBasePtr>& exp)
            : AstBase(AST_SCOPE)
        {
            exp_.swap(exp);
        }

        ~AstScopeStatementExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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

typedef boost::shared_ptr<AstScopeStatementExp> AstScopeStatementExpPtr;

class AstFuncDefExp: public AstBase
{
    public:
        AstFuncDefExp(const AstFuncProtoExpPtr& proto, const AstScopeStatementExpPtr& body)
            : AstBase(AST_FUNC_DEF), proto_(proto), body_(body) {}

        ~AstFuncDefExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

        AstScopeStatementExpPtr GetBody() const { return body_; }
        AstFuncProtoExpPtr GetFuncProto() const { return proto_; }

    private:
        AstFuncProtoExpPtr proto_;
        AstScopeStatementExpPtr body_;
};
typedef boost::shared_ptr<AstFuncDefExp> AstFuncDefExpPtr;

class AstFuncCallExp: public AstBase
{
    public:
        AstFuncCallExp(const std::string& fun, std::vector<AstBasePtr>& args)
            : AstBase(AST_FUNC_CALL), func_(fun), args_(args) {}

        ~AstFuncCallExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

    private:
        std::string func_;
        std::vector<AstBasePtr> args_;
};
typedef boost::shared_ptr<AstFuncCallExp> AstFuncCallExpPtr;

class AstArrayExp: public AstBase
{
    public:
        explicit AstArrayExp(const std::vector<AstBasePtr>& arr)
            : AstBase(AST_ARR), arr_(arr) {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstArrayExp> AstArrayExpPtr;

class AstArrayIndexExp: public AstBase
{
    public:
        AstArrayIndexExp(const std::string& arr, const AstBasePtr& index)
            : AstBase(AST_ARR_INDEX), arr_(arr), index_(index) {}

        ~AstArrayIndexExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstArrayIndexExp> AstArrayIndexExpPtr;

class AstRetExp: public AstBase
{
    public:
        explicit AstRetExp(const AstBasePtr& ret): AstBase(AST_RET), val_(ret) {}

        ~AstRetExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
        }

        virtual ValueNodePtr Evaluate()
        {
            // TODO
            return ValueNodePtr();
        }

    private:
        AstBasePtr val_;
};
typedef boost::shared_ptr<AstRetExp> AstRetExpPtr;

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

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstIfExp> AstIfExpPtr;

class AstWhileExp: public AstBase
{
    public:
        AstWhileExp(const AstBasePtr& cond, const AstScopeStatementExpPtr& body)
            : AstBase(AST_WHILE), cond_(cond), body_(body) {}

        ~AstWhileExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstWhileExp> AstWhileExpPtr;

class AstForExp: public AstBase
{
    // python style for
    public:
        AstForExp(AstBasePtr var, AstBasePtr arr, const AstScopeStatementExpPtr& body)
            : AstBase(AST_FOR), var_(var), range_(arr), body_(body) {}

        ~AstForExp() {}

        virtual void Accept(VisitorBase& v)
        {
            v.Visit(this);
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
typedef boost::shared_ptr<AstForExp> AstForExpPtr;

} // end ink

#endif

