#include "CalcParser.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include <boost/assign.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <boost/spirit/version.hpp>

#if SPIRIT_VERSION <= 0x1806
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute.hpp>
namespace spirit_ns = boost::spirit;
#else
#include <boost/spirit/home/classic/core.hpp>
#include <boost/spirit/home/classic/attribute.hpp>
namespace spirit_ns = boost::spirit::classic;
#endif

namespace CalcParser {

struct ErrException: public std::exception
{
    std::string msg_;
    explicit ErrException(const std::string& s): msg_(s) {}
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
        , std::string
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

    Type tmp_;
    Type expr_;
};

static std::map<std::string, OpType> gs_op1_map = boost::assign::map_list_of
    ("+", OT_1_Pos)("-", OT_1_Neg);

static std::map<std::string, OpType> gs_op2_map = boost::assign::map_list_of("==", OT_2_Eq)
    ("+", OT_2_Add)("-", OT_2_Sub)("*", OT_2_Mul)("/", OT_2_Div)
    ("%", OT_2_Mod)("^", OT_2_Xor)("&", OT_2_And)("|", OT_2_Or)
    (">", OT_2_GT)(">=", OT_2_GET)("<", OT_2_LT)("<=", OT_2_LET)
    ("!=", OT_2_Neq)("&&", OT_2_LAND)("and", OT_2_LAND)("||", OT_2_LOR)("or", OT_2_LOR);

static std::map<std::string, OpType> gs_op_fun_map = boost::assign::map_list_of("if", OT_3_If)
    ("left", OT_2_Left)("right", OT_2_Right)("concat", OT_2_Concat)("abs", OT_1_Abs);

static std::map<OpType, std::string> gs_op_map_reverse = boost::assign::map_list_of(OT_2_Eq, "==")
    (OT_2_Add, "+")(OT_2_Sub, "-")(OT_2_Mul, "*")(OT_2_Div, "/")
    (OT_2_Mod, "%")(OT_2_Xor, "^")(OT_2_And, "&")(OT_2_Or, "|")
    (OT_2_Left, "left")(OT_2_Right, "right")(OT_2_Concat, "concat")
    (OT_1_Pos, "+")(OT_1_Neg, "-")(OT_3_If, "if")(OT_NOP, "nop")
    (OT_2_GT, ">")(OT_2_GET, ">=")(OT_2_LT, "<")(OT_2_LET, "<=")
    (OT_2_Neq, "!=")(OT_1_Abs, "abs")(OT_2_LAND, "&&")(OT_2_LOR, "||");

struct binary_op
{
    binary_op(const std::string& op, const ExpAst& left)
            : op_(gs_op2_map[op]), left_(left), right_(nil())
    {
        if (op_ == OT_NOP) throw ErrException(std::string("unrecognized operator:") + op);
    }

    OpType op_;
    ExpAst left_;
    ExpAst right_;
};

struct unary_op
{
    explicit unary_op(const std::string& op)
        : op_(gs_op1_map[op]), subject_(nil())
    {
        if (op_ == OT_NOP) throw ErrException(std::string("unrecognized operator:") + op);
    }

    OpType op_;
    ExpAst subject_;
};

struct func_op
{
    explicit func_op(const std::string& op)
        : op_(gs_op_fun_map[op])
    {
        if (op_ == OT_NOP) throw ErrException(std::string("unrecognized operator:") + op);

        params_.reserve(5);
    }

    void AddParam(const ExpAst& param)
    {
        params_.push_back(param);
    }

    OpType op_;
    std::vector<ExpAst> params_;
};

struct string_op
{
    string_op() {}
    string_op(const char* s, const char* e): operand_(s, e) {}

    std::string operand_;
};

// get a binary operator, cache it until we get right operand.
ExpAst& ExpAst::operator+=(const ExpAst& rhs)
{
    tmp_ = boost::get<std::string>(rhs.expr_);
    return *this;
}

// get a right operand, now create a binary node.
ExpAst& ExpAst::operator-=(const ExpAst& rhs)
{
    expr_ = binary_op(boost::get<std::string>(tmp_), expr_);
    binary_op& op = boost::get<binary_op>(expr_);
    op.right_ = rhs;
    return *this;
}

ExpAst& ExpAst::operator*=(const ExpAst& rhs)
{
    expr_ = func_op(boost::get<std::string>(expr_));
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
    expr_ = unary_op(boost::get<std::string>(expr.expr_));
    return *this;
}

ExpAst& ExpAst::operator^=(const ExpAst& expr)
{
    unary_op& op = boost::get<unary_op>(this->expr_);
    op.subject_ = expr;
    return *this;
}

struct ExpClosure: spirit_ns::closure<ExpClosure, ExpAst>
{
    member1 val_;
};

struct CalcGrammar: public spirit_ns::grammar<CalcGrammar, ExpClosure::context_t>
{
    template <class scanner>
    struct definition
    {
        typedef spirit_ns::rule<scanner, ExpClosure::context_t> rule_t;

        const rule_t& start() const { return top; }

        explicit definition(const CalcGrammar& self)
        {
            using namespace phoenix;
            using namespace spirit_ns;

            top = log[self.val_ = arg1] >> end_p;

            // logical operator
            log = blog[log.val_ = arg1] >>
                *((str_p("&&")[log.val_ += construct_<std::string>(arg1, arg2)]
                            >> blog[log.val_ -= arg1]) |
                (str_p("||")[log.val_ += construct_<std::string>(arg1, arg2)]
                            >> blog[log.val_ -= arg1]) |
                (str_p("and")[log.val_ += construct_<std::string>(arg1, arg2)]
                            >> blog[log.val_ -= arg1]) |
                (str_p("or")[log.val_ += construct_<std::string>(arg1, arg2)]
                            >> blog[log.val_ -= arg1]));

            // bitwise logical
            blog = rel[blog.val_ = arg1] >>
                *((str_p("&")[blog.val_ += construct_<std::string>(arg1, arg2)]
                            >> rel[blog.val_ -= arg1]) |
                (str_p("|")[blog.val_ += construct_<std::string>(arg1, arg2)]
                            >> rel[blog.val_ -= arg1]) |
                (str_p("^")[blog.val_ += construct_<std::string>(arg1, arg2)]
                            >> rel[blog.val_ -= arg1]));

            // relational operators
            rel = exp[rel.val_ = arg1] >>
                *((str_p(">=")[rel.val_ += construct_<std::string>(arg1, arg2)]
                            >> exp[rel.val_ -= arg1]) |
                (str_p(">")[rel.val_ += construct_<std::string>(arg1, arg2)]
                            >> exp[rel.val_ -= arg1]) |
                (str_p("<=")[rel.val_ += construct_<std::string>(arg1, arg2)]
                            >> exp[rel.val_ -= arg1]) |
                (str_p("<")[rel.val_ += construct_<std::string>(arg1, arg2)]
                            >> exp[rel.val_ -= arg1]) |
                (str_p("==")[rel.val_ += construct_<std::string>(arg1, arg2)]
                            >> exp[rel.val_ -= arg1]) |
                (str_p("!=")[rel.val_ += construct_<std::string>(arg1, arg2)]
                            >> exp[rel.val_ -= arg1]));

            exp = term[exp.val_ = arg1] >>
                *((str_p("+")[exp.val_ += construct_<std::string>(arg1, arg2)]
                            >> term[exp.val_ -= arg1]) |
                (str_p("-")[exp.val_ += construct_<std::string>(arg1, arg2)]
                            >> term[exp.val_ -= arg1]));

            term = factor[term.val_ = arg1] >>
                *((str_p("*")[term.val_ += construct_<std::string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("/")[term.val_ += construct_<std::string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]) |
                (str_p("%")[term.val_ += construct_<std::string>(arg1, arg2)]
                            >> factor[term.val_ -= arg1]));

            factor =
                // parse string surrounded by a pair of double quote-mark, eg, "abc"
                lexeme_d['\"' >> (*(anychar_p - ch_p('\"')))
                [factor.val_ = construct_<string_op>(arg1, arg2)] >> '\"'] |
                lexeme_d['\'' >> (*(anychar_p - ch_p('\'')))
                [factor.val_ = construct_<string_op>(arg1, arg2)] >> '\''] |
                // parse function call, eg, fun(3, 2, 1)
                func[factor.val_ = arg1] |
                // parse decimal digit into a double
                real_p[factor.val_ = arg1] |
                // parse a variable, eg, x
                id[factor.val_ = construct_<std::string>(arg1, arg2)] |
                '(' >> log[factor.val_ = arg1] >> ')' |
                // parse a negate operation, eg, -x
                (str_p("-")[factor.val_ %= construct_<std::string>(arg1, arg2)]
                >> factor[factor.val_ ^= arg1]) |
                ('+' >> factor[factor.val_ = arg1]);

            func = id[func.val_ = construct_<std::string>(arg1, arg2)] >>
                ch_p('(')[func.val_ *= arg1] >> !(log[func.val_ /= arg1] % ',') >> ')';

            id  = (('$' | alpha_p | '_') >> *(alnum_p | '_'));
        }

        spirit_ns::rule<scanner> id;
        rule_t log, blog, rel, exp, term, factor, top, func;
    };
};

struct AstWalker: public boost::static_visitor<OperandType>
{
    public:

        explicit AstWalker(const std::map<std::string, OperandType>* ref = NULL)
            : m_reportUnknowVar(false), m_handler(NULL), m_refValue(ref)
        {
        }

        void SetHandler(FuncHandlerBase* func) { m_handler = func; }

        void SetReportUnknowVar(bool f) { m_reportUnknowVar = f; }
        void SetVariable(const std::map<std::string, OperandType>* ref) { m_refValue = ref; }

        OperandType Walk(const ExpAst& ast) const
        {
            return boost::apply_visitor(*this, ast.expr_);
        }

        OperandType operator()(nil) const
        {
            assert(0);
            return 0;
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

        OperandType operator()(const std::string& k) const
        {
            if (!m_refValue) return k;

            std::map<std::string, OperandType>::const_iterator it = (*m_refValue).find(k);
            if (it != (*m_refValue).end()) return it->second;

            if (m_reportUnknowVar) throw ErrException(std::string("Unknown operand:") + k);

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
        bool m_reportUnknowVar;
        FuncHandlerBase* m_handler;
        const std::map<std::string, OperandType>* m_refValue;
};

// default implementation of function handler
FuncHandlerBase::~FuncHandlerBase() {}

OperandType FuncHandlerBase::Func0(OpType op) const
{
    std::string err = "parameters mismatched for function:" + gs_op_map_reverse[op];
    throw ErrException(err);

    return 0;
}

OperandType FuncHandlerBase::Func1(OpType op, OperandType a1) const
{
    try {
        switch (op) {
            case OT_1_Neg: return -boost::get<double>(a1);
            case OT_1_Pos: return a1;
            case OT_1_Abs: return fabs(boost::get<double>(a1));
            default: break;
        }
    } catch (...) {
    }

    std::string err = "invalid operand for function:" + gs_op_map_reverse[op];
    throw ErrException(err);

    return 0;
}

static inline OperandType LogicalOperation(OpType op, OperandType a1, OperandType a2)
{
    bool left = false, right = false;

    try {
        left = boost::get<double>(a1);
    } catch (...) {
        left = !boost::get<std::string>(a1).empty();
    }

    try {
        right = boost::get<double>(a2);
    } catch (...) {
        right = !boost::get<std::string>(a2).empty();
    }

    switch (op)
    {
        case OT_2_LAND: return left && right;
        case OT_2_LOR: return left || right;
        default: throw ErrException("invalid logical operator");
    }

    return 0;
}

static inline OperandType CalcVariantType(OpType op, OperandType a1, OperandType a2)
{
    switch (op)
    {
        case OT_2_Add:
            {
                try {
                    return boost::get<double>(a1) + boost::get<double>(a2);
                } catch (...) {
                    return boost::get<std::string>(a1) + boost::get<std::string>(a2);
                }
            }
        case OT_2_GT:
            {
                try {
                    return boost::get<double>(a1) > boost::get<double>(a2);
                } catch (...) {
                    return boost::get<std::string>(a1) > boost::get<std::string>(a2);
                }
            };
        case OT_2_GET:
            {
                try {
                    return boost::get<double>(a1) >= boost::get<double>(a2);
                } catch (...) {
                    return boost::get<std::string>(a1) >= boost::get<std::string>(a2);
                }
            };
        case OT_2_LT:
            {
                try {
                    return boost::get<double>(a1) < boost::get<double>(a2);
                } catch (...) {
                    return boost::get<std::string>(a1) < boost::get<std::string>(a2);
                }
            };
        case OT_2_LET:
            {
                try {
                    return boost::get<double>(a1) <= boost::get<double>(a2);
                } catch (...) {
                    return boost::get<std::string>(a1) <= boost::get<std::string>(a2);
                }
            };
        default:
            assert(0);
            return nil();
    }
}

OperandType FuncHandlerBase::Func2(OpType op, OperandType a1, OperandType a2) const
{
    try {
        switch (op)
        {
            case OT_2_Add:
                return CalcVariantType(op, a1, a2);
            case OT_2_Sub:
                return boost::get<double>(a1) - boost::get<double>(a2);
            case OT_2_Mul:
                return boost::get<double>(a1) * boost::get<double>(a2);
            case OT_2_Div:
                return boost::get<double>(a1) / boost::get<double>(a2);
            case OT_2_Mod:
                return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) %
                        static_cast<long long>(boost::get<double>(a2)));
            case OT_2_Xor:
                return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) ^
                        static_cast<long long>(boost::get<double>(a2)));
            case OT_2_And:
                return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) &
                        static_cast<long long>(boost::get<double>(a2)));
            case OT_2_Or:
                return static_cast<double>(static_cast<long long>(boost::get<double>(a1)) |
                        static_cast<long long>(boost::get<double>(a2)));
            case OT_2_GT:
            case OT_2_GET:
            case OT_2_LT:
            case OT_2_LET:
                return CalcVariantType(op, a1, a2);
            case OT_2_Eq:
                return a1 == a2;
            case OT_2_Neq:
                return !(a1 == a2);
            case OT_2_Left:
                {
                    std::string& s = boost::get<std::string>(a1);
                    size_t k = static_cast<size_t>(boost::get<double>(a2));
                    size_t len = s.size();

                    k = k < len? k : len;
                    return s.substr(0, k);
                }
            case OT_2_Right:
                {
                    std::string& s = boost::get<std::string>(a1);
                    size_t k = static_cast<size_t>(boost::get<double>(a2));
                    size_t len = s.size();

                    k = k < len? k : len;
                    size_t sp = len - k;
                    return s.substr(sp);
                }
            case OT_2_Concat:
                {
                    std::string& s1 = boost::get<std::string>(a1);
                    std::string& s2 = boost::get<std::string>(a2);
                    return s1 + s2;
                }
            case OT_2_LAND:
            case OT_2_LOR:
                return LogicalOperation(op, a1, a2);
            default:
                throw "invalid parameters";
        }
    } catch (...) {
        std::ostringstream oss;
        oss.precision(16);
        oss << "Invalid operation: " << gs_op_map_reverse[op] << "(" << a1 << ", " << a2 << ")";
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
            return (boost::get<std::string>(a1)).empty()? a3:a2;
        }
    }

    std::string err = "parameters mismatched for function:" + gs_op_map_reverse[op];
    throw ErrException(err);

    return 0;
}

OperandType FuncHandlerBase::Func4(OpType op, OperandType a1,
        OperandType a2, OperandType a3, OperandType a4) const
{
    std::string err = "parameters mismatched for function:" + gs_op_map_reverse[op];
    throw ErrException(err);

    return 0;
}

struct FuncHandlerForVerify: FuncHandlerBase
{
    virtual OperandType Func0(OpType op) const
    {
        throw ErrException("Parameters mismatched for function:" + gs_op_map_reverse[op]);
        return nil();
    }

    virtual OperandType Func1(OpType op, OperandType a1) const
    {
        if (op <= OT_1_Start || op >= OT_1_End)
        {
            throw ErrException("Parameters mismatched for function:" + gs_op_map_reverse[op]);
        }
        return nil();
    }

    virtual OperandType Func2(OpType op, OperandType a1, OperandType a2) const
    {
        if (op <= OT_2_Start || op >= OT_2_End)
        {
            throw ErrException("Parameters mismatched for function:" + gs_op_map_reverse[op]);
        }
        return nil();
    }

    virtual OperandType Func3(OpType op, OperandType a1,
            OperandType a2, OperandType a3) const
    {
        if (op <= OT_3_Start || op >= OT_3_End)
        {
            throw ErrException("Parameters mismatched for function:" + gs_op_map_reverse[op]);
        }
        return nil();
    }

    virtual OperandType Func4(OpType op, OperandType a1,
            OperandType a2, OperandType a3, OperandType a4) const
    {
        throw ErrException("Parameters mismatched for function:" + gs_op_map_reverse[op]);
        return nil();
    }
};

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

        FuncHandlerBase* GetHandler() const { return m_handler; }

        bool ParseExpression(const std::string& exp, std::string& err)
        {
            using namespace phoenix;
            using namespace spirit_ns;

            try {
                parse_info<> pi = parse(exp.c_str(), m_grammar[var(m_ast) = arg1], space_p);
                if (pi.hit && pi.full) return true;

                std::ostringstream oss;
                oss << "Invalid expression, parser stopped at position "
                    << pi.stop - exp.c_str() << ":" << pi.stop;

                err += oss.str();
            } catch (std::exception& e) {
                err += std::string("Error occurs during parsing:") + e.what();
            } catch (...) {
                err += "Unknown error occurs while parsing the expression";
            }

            return false;
        }

        bool ParseExpression(const std::string& exp, std::string& err,
                const std::map<std::string, OperandType>& ref)
        {
            bool ret = ParseExpression(exp, err);

            if (!ret) return ret;

            FuncHandlerForVerify verify;

            m_walker.SetHandler(&verify);
            m_walker.SetVariable(&ref);
            m_walker.SetReportUnknowVar(true);

            try {
                m_walker.Walk(m_ast);
            } catch (std::exception& e) {
                ret = false;
                err += e.what();
            } catch (...) {
                ret = false;
            }

            m_walker.SetHandler(m_handler);
            m_walker.SetReportUnknowVar(false);
            return ret;
        }

        OperandType GenValue(const std::map<std::string,
                OperandType>& ref, std::string& err)
        {
            OperandType ret = nil();
            m_walker.SetVariable(&ref);
            try {
                ret = m_walker.Walk(m_ast);
            } catch (std::exception& e) {
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

FuncHandlerBase* CalculatorParser::GetHandler() const
{
    return m_impl->GetHandler();
}

bool CalculatorParser::ParseExpression(const std::string& exp, std::string& err)
{
    return m_impl->ParseExpression(exp, err);
}

bool CalculatorParser::ParseExpression(const std::string& exp, std::string& err,
        const std::map<std::string, OperandType>& ref)
{
    return m_impl->ParseExpression(exp, err, ref);
}

OperandType CalculatorParser::GenValue(const std::map<std::string, OperandType>& ref, std::string& err)
{
    return m_impl->GenValue(ref, err);
}

}  // end namespace CalcParser

