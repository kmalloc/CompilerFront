#ifndef __ABSTRACT_SYN_TREE_H__
#define __ABSTRACT_SYN_TREE_H__

#include <string>

namespace ink {

enum AstType
{
    AST_NONE,
    AST_INT,
    AST_FLOAT,
    AST_STRING,
    AST_VAR,
    AST_OP_UNARY,
    AST_OP_BINARY,
    AST_FUNC_PROTO,
    AST_FUNC_DEF,
    AST_FUNC_CALL,
    AST_ARR_INDEX,
};

class AstBase
{
    public:
        AstBase(AstType t): type_(t) {}

        virtual ~AstBase() = 0;
        int GetType() const { return type_; }

    public:
        int type_;
};

class AstIntExp: public AstBase
{
    public:
        explicit AstIntExp(int64_t val): AstBase(AST_INT), val_(val) {}
        ~AstIntExp();

    private:
        int64_t val_;
};

class AstFloatExp: public AstBase
{
    public:
        explicit AstFloatExp(double v): AstBase(AST_FLOAT), val_(v) {}
        ~AstFloatExp();

    private:
        double val_;
};

// literal string
class AstStringExp: public AstBase
{
    public:
        explicit AstStringExp(const std::string& val)
            : AstBase(AST_STRING), val_(val) {}

    private:
        std::string val_;
};

class AstVariableExp: public AstBase
{
    public:
        explicit AstVariableExp(const std::string& name)
            : AstBase(AST_VAR), name_(name) {}

    private:
        std::string name_;
};

class AstUnaryExp: public AstBase
{
    public:
        AstUnaryExp(TokenType op, AstBase* arg)
            : AstBase(AST_OP_UNARY), op_(op), arg_(arg) {}
    private:
        TokenType op_;
        AstBase* arg_;
};

class AstBinaryExp: public AstBase
{
    public:
        AstBinaryExp(TokenType op, AstBase* lhs, AstBase* rhs)
            : AstBase(AST_OP_BINARY), lhs_(lhs), rhs_(rhs), op_(op) {}

    private:
        AstBase* lhs_;
        AstBase* rhs_;
        TokenType op_;
};

class AstFuncProtoExp: public AstBase
{
    public:
        struct ArgType
        {
            int type;
            std::string name;
        };

    public:
        AstFuncProtoExp(const std::string& fun, const std::vector<ArgType>& args)
            : AstBase(AST_FUNC_PROTO), func_(fun), args_(args) {}

    private:
        std::string fun_;
        std::vector<ArgType> args_;
};

class AstFuncDefExp: public AstBase
{
    public:
        AstFuncDefExp(AstFuncProtoExp* proto, AstBase* body)
            : AstBase(AST_FUNC_DEF), body_(body), proto_(proto) {}

    private:
        AstBase* body_;
        AstFuncProtoExp* proto_;
};

class AstFuncCallExp: public AstBase
{
    public:
        AstFuncCallExp(const std::string& fun, std::vector<AstBase*>& args)
            : AstBase(AST_FUNC_CALL), func_(fun), args_(args) {}

    private:
        std::string func_;
        std::vector<AstBase*> args_;
};

class AstArrayIndexExp: public AstBase
{
    public:
        AstArrayIndexExp(const std::string& arr, AstBase* index)
            : AstBase(AST_ARR_INDEX), arr_(arr), index_(index) {}

    private:
        std::string arr_;
        AstBase* index_;
};

} // end ink

#endif

