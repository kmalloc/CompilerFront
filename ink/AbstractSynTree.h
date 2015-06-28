#ifndef __ABSTRACT_SYN_TREE_H__
#define __ABSTRACT_SYN_TREE_H__

#include <string>

namespace ink {

class AstBase
{
    public:
        virtual ~AstBase() = 0;
        int GetType() const { return type_; }

    public:
        int type_;
};

class AstIntExp: public AstBase
{
    public:
        explicit AstIntExp(int64_t val);
        ~AstIntExp();

    private:
        int64_t val_;
};

class AstDoubleExp: public AstBase
{
    public:
        explicit AstDoubleExp(double v);
        ~AstDoubleExp();

    private:
        double val_;
};

class AstVariableExp: public AstBase
{
    public:
        explicit AstVariableExp(const std::string& name);

    private:
        std::string name_;
};

class AstBinaryExp: public AstBase
{
    public:
        AstBinaryExp(const std::string& op, AstBase* lhs, AstBase* rhs);

    private:
        AstBase* lhs_;
        AstBase* rhs_;
        std::string op_;
};

class AstFuncExp: public AstBase
{
    public:
        struct ArgType
        {
            int type;
            std::string name;
        };

    public:
        AstFuncExp(const std::string& fun, const std::vector<ArgType>& args);

    private:
        std::string fun_;
        std::vector<ArgType> args_;
};


} // end ink

#endif

