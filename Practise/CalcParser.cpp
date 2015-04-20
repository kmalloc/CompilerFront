#include <precompile.h>

#include "CalcParser.h"

#include <vector>
#include <sstream>
#include <iostream>

#include <boost/assign.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

using namespace std;
using namespace boost::spirit;
using namespace phoenix;

namespace CalcParser {

struct ErrException: public exception
{
    string msg_;
    explicit ErrException(const string& s): msg_(s) {}
    ~ErrException() throw() {}

    const char* what() const throw() { return msg_.c_str(); }
};

struct binary_op;
struct unary_op;
struct func_op;
struct string_op;

struct ExpAst
{
    typedef boost::variant<nil
        , double
        , string
        , boost::recursive_wrapper<ExpAst>
        , boost::recursive_wrapper<binary_op>
        , boost::recursive_wrapper<unary_op>
        , boost::recursive_wrapper<func_op>
        , boost::recursive_wrapper<string_op>
        > Type;

    ExpAst(): expr_(nil()) {}

    template <typename EXP>
    ExpAst(const EXP& expr): expr_(expr) {} // NOLINT

    // add binary operator
    ExpAst& operator+=(const ExpAst& rhs);
    // add binary operand
    ExpAst& operator-=(const ExpAst& rhs);
    // add func call
    ExpAst& operator*=(const ExpAst& rhs);
    // add func call operand
    ExpAst& operator/=(const ExpAst& rhs);
    // add unary operator
    ExpAst& operator%=(const ExpAst& rhs);
    // add unary operand
    ExpAst& operator^=(const ExpAst& rhs);

    Type expr_;
};

static std::map<std::string, OpType> gs_op_map = boost::assign::map_list_of("==", OT_2_Eq)
    ("+", OT_2_Add)("-", OT_2_Sub)("*", OT_2_Mul)("/", OT_2_Div)
    ("%", OT_2_Mod)("^", OT_2_Xor)("&", OT_2_And)("|", OT_2_Or)
    ("left", OT_2_Left)("right", OT_2_Right)("concat", OT_2_Concat)
    ("if", OT_3_If);

static std::map<OpType, std::string> gs_op_map2 = boost::assign::map_list_of(OT_2_Eq, "==")
    (OT_2_Add, "+")(OT_2_Sub, "-")(OT_2_Mul, "*")(OT_2_Div, "/")
    (OT_2_Mod, "%")(OT_2_Xor, "^")(OT_2_And, "&")(OT_2_Or, "|")
    (OT_2_Left, "left")(OT_2_Right, "right")(OT_2_Concat, "concat")
    (OT_3_If, "if")(OT_NOP, "nop");

struct binary_op
{
    binary_op(const string& op, const ExpAst& left)
            : op_(gs_op_map[op]), left_(left), right_(nil())
    {
        if (op_ == OT_NOP) throw ErrException(string("unrecognized operator:") + op);
    }

    OpType op_;
    ExpAst left_;
    ExpAst right_;
};

struct unary_op
{
    explicit unary_op(const string& op)
        : op_(gs_op_map[op]), subject_(nil())
    {
        if (op_ == OT_NOP) throw ErrException(string("unrecognized operator:") + op);
    }

    OpType op_;
    ExpAst subject_;
};

struct func_op
{
    explicit func_op(const string& op)
        : op_(gs_op_map[op])
    {
        if (op_ == OT_NOP) throw ErrException(string("unrecognized operator:") + op);

        params_.reserve(5);
    }

    void AddParam(const ExpAst& param)
    {
        params_.push_back(param);
    }

    OpType op_;
    vector<ExpAst> params_;
};

struct string_op
{
    string_op() {}
    string_op(const char* s, const char* e): operand_(s, e) {}

    string operand_;
};

ExpAst& ExpAst::operator+=(const ExpAst& rhs)
{
    expr_ = binary_op(boost::get<string>(rhs.expr_), expr_);
    return *this;
}

ExpAst& ExpAst::operator-=(const ExpAst& rhs)
{
    binary_op& op = boost::get<binary_op>(this->expr_);
    op.right_ = rhs;
    return *this;
}

ExpAst& ExpAst::operator*=(const ExpAst& rhs)
{
    expr_ = func_op(boost::get<string>(expr_));
    return *this;
}

ExpAst& ExpAst::operator/=(const ExpAst& rhs)
{
    func_op& op = boost::get<func_op>(expr_);
    op.AddParam(rhs);
    return *this;
}

ExpAst& ExpAst::operator%=(const ExpAst& expr)
{
    expr_ = unary_op(boost::get<string>(expr.expr_));
    return *this;
}

ExpAst& ExpAst::operator^=(const ExpAst& expr)
{
    unary_op& op = boost::get<unary_op>(this->expr_);
    op.subject_ = expr;
    return *this;
}

struct ExpClosure: boost::spirit::closure<ExpClosure, ExpAst>
{
    member1 val_;
};

struct CalcGrammar: public grammar<CalcGrammar, ExpClosure::context_t>
{
    template <class scanner>
    struct definition
    {
        typedef rule<scanner, ExpClosure::context_t> rule_t;

        const rule_t& start() const { return top; }

        explicit definition(const CalcGrammar& self)
        {
            top = exp[self.val_ = arg1] >> end_p;

            exp = term[exp.val_ = arg1] >>
                *((str_p("+")[exp.val_ += construct_<string>(arg1, arg2)]
                            >> term[exp.val_ -= arg1]) |
                (str_p("-")[exp.val_ += construct_<string>(arg1, arg2)]
                            >> term[exp.val_ -= arg1]));

            term = factor[term.val_ = arg1] >>
                *((str_p("*")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("/")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("%")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("&")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("|")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("^")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("==")[term.val_ += construct_<string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]));

            factor =
                // parse string surrounded by a pair of double quote-mark, eg, "abc"
                lexeme_d['\"' >> (*(anychar_p - ch_p('\"')))
                [factor.val_ = construct_<string_op>(arg1, arg2)] >> '\"'] |
                // parse function call, eg, fun(3, 2, 1)
                func[factor.val_ = arg1] |
                // parse decimal digit into a double
                real_p[factor.val_ = arg1] |
                // parse a variable, eg, x
                id[factor.val_ = construct_<string>(arg1, arg2)] |
                '(' >> exp[factor.val_ = arg1] >> ')' |
                // parse a negate operation, eg, -x
                (str_p("-")[factor.val_ %= construct_<string>(arg1, arg2)]
                >> factor[factor.val_ ^= arg1]) |
                ('+' >> factor[factor.val_ = arg1]);

            func = id[func.val_ = construct_<string>(arg1, arg2)] >>
                ch_p('(')[func.val_ *= arg1] >> (exp[func.val_ /= arg1] % ',') >> ')';

            id  = (('$' | alpha_p | '_') >> *(alnum_p | '_'));
        }

        rule<scanner> id;
        rule_t exp, term, factor, top, func;
    };
};

struct AstWalker: public boost::static_visitor<OperandType>
{
    public:

        explicit AstWalker(const map<std::string, OperandType>* ref = NULL)
            : m_handler(NULL), m_refValue(ref)
        {
        }

        void SetHandler(FuncHandlerBase* func) { m_handler = func; }
        void SetVariable(const map<std::string, OperandType>* ref) { m_refValue = ref; }

        OperandType Walk(const ExpAst& ast) const
        {
            return boost::apply_visitor(*this, ast.expr_);
        }

        OperandType operator()(const ExpAst& ast) const
        {
            return boost::apply_visitor(*this, ast.expr_);
        }

        OperandType operator()(double d) const { return d; }

        OperandType operator()(const string_op& k) const
        {
            return k.operand_;
        }

        OperandType operator()(const string& k) const
        {
            if (!m_refValue) return k;

            map<string, OperandType>::const_iterator it = (*m_refValue).find(k);
            if (it != (*m_refValue).end()) return it->second;

            return k;
        }

        OperandType operator()(const unary_op& expr) const
        {
            OperandType a1 = boost::apply_visitor(*this, expr.subject_.expr_);
            return m_handler->Func1(expr.op_, a1);
        }

        OperandType operator()(const binary_op& expr) const
        {
            OperandType a1 = boost::apply_visitor(*this, expr.left_.expr_);
            OperandType a2 = boost::apply_visitor(*this, expr.right_.expr_);
            return m_handler->Func2(expr.op_, a1, a2);
        }

        OperandType operator()(const func_op& exp) const
        {
            size_t sz = exp.params_.size();
            assert(sz <= 4 && "function that takes more than 4 parameters is not supported");

            if (sz == 0) return m_handler->Func0(exp.op_);

            OperandType a[4];
            for (size_t i = 0; i < sz; ++i)
            {
                a[i] = boost::apply_visitor(*this, exp.params_[i].expr_);
            }

            if (sz == 1)
            {
                return m_handler->Func1(exp.op_, a[0]);
            }
            else if (sz == 2)
            {
                return m_handler->Func2(exp.op_, a[0], a[1]);
            }
            else if (sz == 3)
            {
                return m_handler->Func3(exp.op_, a[0], a[1], a[2]);
            }
            else
            {
                return m_handler->Func4(exp.op_, a[0], a[1], a[2], a[3]);
            }
        }

    private:

        FuncHandlerBase* m_handler;
        const std::map<std::string, OperandType>* m_refValue;
};

// default implementation of function handler
FuncHandlerBase::~FuncHandlerBase() {}

OperandType FuncHandlerBase::Func0(OpType op) const
{
    assert(0 && "unrecognized unary operator");
    return 0;
}

OperandType FuncHandlerBase::Func1(OpType op, OperandType a1) const
{
    if (op == OT_1_Neg) return -boost::get<double>(a1);
    else if (op == OT_1_Pos) return a1;

    assert(0 && "unrecognized unary operator");

    return 0;
}

OperandType FuncHandlerBase::Func2(OpType op, OperandType a1, OperandType a2) const
{
    try {
        switch (op)
        {
            case OT_2_Add:
                {
                    try {
                        return boost::get<double>(a1) + boost::get<double>(a2);
                    } catch (...) {
                        return boost::get<string>(a1) + boost::get<string>(a2);
                    }
                }
                break;
            case OT_2_Sub:
                {
                    return boost::get<double>(a1) - boost::get<double>(a2);
                }
                break;
            case OT_2_Mul:
                {
                    return boost::get<double>(a1) * boost::get<double>(a2);
                }
                break;
            case OT_2_Div:
                {
                    return boost::get<double>(a1) / boost::get<double>(a2);
                }
                break;
            case OT_2_Mod:
                {
                    return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) %
                            static_cast<long long>(boost::get<double>(a2)));
                }
                break;
            case OT_2_Xor:
                {
                    return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) ^
                            static_cast<long long>(boost::get<double>(a2)));
                }
                break;
            case OT_2_And:
                {
                    return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) &
                            static_cast<long long>(boost::get<double>(a2)));
                }
                break;
            case OT_2_Or:
                {
                    return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) |
                            static_cast<long long>(boost::get<double>(a2)));
                }
                break;
            case OT_2_Eq:
                {
                    return a1 == a2;
                }
                break;
            case OT_2_Left:
                {
                    string& s = boost::get<string>(a1);
                    size_t k = static_cast<size_t>(boost::get<double>(a2));
                    size_t len = s.size();

                    k = k < len? k : len;
                    return s.substr(0, k);
                }
                break;
            case OT_2_Right:
                {
                    string& s = boost::get<string>(a1);
                    size_t k = static_cast<size_t>(boost::get<double>(a2));
                    size_t len = s.size();

                    k = k < len? k : len;
                    size_t sp = len - k;
                    return s.substr(sp);
                }
                break;
            case OT_2_Concat:
                {
                    string& s1 = boost::get<string>(a1);
                    string& s2 = boost::get<string>(a2);
                    return s1 + s2;
                }
                break;
            default:
                {
                    assert(0 && "unrecognized binary operator");
                }
        }
    } catch (...) {
        ostringstream oss;
        oss.precision(16);
        oss << "Invaliad operation: " << gs_op_map2[op] << "(" << a1 << ", " << a2 << ")";
        throw ErrException(oss.str());
    }

    return 0;
}

OperandType FuncHandlerBase::Func3(OpType op, OperandType a1,
        OperandType a2, OperandType a3) const
{
    if (op == OT_3_If)
    {
        try {
            return boost::get<double>(a1)? a2 : a3;
        } catch(...) {
            return (boost::get<string>(a1)).empty()? a3:a2;
        }
    }

    assert(0 && "unrecognized function operator");
    return 0;
}

OperandType FuncHandlerBase::Func4(OpType op, OperandType a1,
        OperandType a2, OperandType a3, OperandType a4) const
{
    assert(0 && "unrecognized function operator");
    return 0;
}

class CalcParserImpl
{
    public:
        explicit CalcParserImpl(FuncHandlerBase* handler = NULL, bool delete_handler = false)
            : m_delete(delete_handler), m_handler(handler)
        {
            if (m_handler == NULL)
            {
                m_delete = true;
                m_handler = new FuncHandlerBase;
            }

            m_walker.SetHandler(m_handler);
        }

        ~CalcParserImpl()
        {
            if (m_delete) delete m_handler;
        }

        void SetHandler(FuncHandlerBase* handler, bool del)
        {
            if (m_delete && m_handler) delete m_handler;

            m_delete  = del;
            m_handler = handler;
        }

        bool ParseExpression(const std::string& exp, std::string& err)
        {
            try {
                parse_info<> pi = parse(exp.c_str(), m_grammar[var(m_ast) = arg1], space_p);
                if (pi.hit && pi.full) return true;

                ostringstream oss;
                oss << "Invalid expression, parser stopped at position "
                    << pi.stop - exp.c_str() << ":" << pi.stop;

                err += oss.str();
            } catch (exception& e) {
                err += string("Error occurs during parsing:") + e.what();
            } catch (...) {
                err += "Unknown error occurs while parsing the expression";
            }

            return false;
        }

        OperandType GenValue(const std::map<std::string,
                OperandType>& ref, std::string& err)
        {
            OperandType ret = nil();
            m_walker.SetVariable(&ref);
            try {
                ret = m_walker.Walk(m_ast);
            } catch (exception& e) {
                err += e.what();
            } catch (...) {
                err += "Unknown error occurs while walking the AST";
            }

            return ret;
        }

    private:
        bool m_delete;
        ExpAst m_ast;
        AstWalker m_walker;
        CalcGrammar m_grammar;
        FuncHandlerBase* m_handler;
};

CalculatorParser::CalculatorParser()
    : m_impl(new CalcParserImpl(NULL, true))
{
}

CalculatorParser::CalculatorParser(FuncHandlerBase* fun, bool delete_func)
    : m_impl(new CalcParserImpl(fun, delete_func))
{
}

CalculatorParser::~CalculatorParser()
{
    delete m_impl;
}

void CalculatorParser::SetHandler(FuncHandlerBase* handler, bool del)
{
    m_impl->SetHandler(handler, del);
}

bool CalculatorParser::ParseExpression(const string& exp, string& err)
{
    return m_impl->ParseExpression(exp, err);
}

OperandType CalculatorParser::GenValue(const map<string, OperandType>& ref, string& err)
{
    return m_impl->GenValue(ref, err);
}

}  // end namespace CalcParser

