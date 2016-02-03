#ifndef _CALCPARSER_H_
#define _CALCPARSER_H_

#include <map>
#include <string>

#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>

namespace CalcParser {

struct nil {};

inline bool operator==(const nil&, const nil&) { return true; }
inline std::ostream& operator<<(std::ostream& s, const nil&) { s << "nil"; return s; }

typedef boost::variant<nil, double, std::string> OperandType;

// function handler used by the parser to walk the generated ast
// FuncN() is the handler for operators that takes N parameters.
// currently, only the following operators are supported:
// binary operator: +, -, *, /, %, ^, &, |, ==, eg, a + b, a * b
// unary operator: +, -, eg, (-a), (+b)
// ternary operator: if, eg, if(f, a, b), if f is true, then return a, otherwise return b
// string operations: left("abcxy", 3) --> return "abc", right("abcde", 2) --> return "de",
// concat("ab", "cd") --> return "abcd"

inline bool IsMetaChar(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '&' ||
        c == '|' || c == '=' || c == '>' || c == '<' || c == '!';
}

enum OpType
{
    // OT_NOP must be 0
    OT_NOP = 0,

    OT_1_Start,
    OT_1_Pos,
    OT_1_Neg,
    OT_1_Abs,
    OT_1_End,

    OT_2_Start,
    OT_2_Add,
    OT_2_Sub,
    OT_2_Mul,
    OT_2_Div,
    OT_2_Mod,
    OT_2_Xor,
    OT_2_And,
    OT_2_Or,

    // relational
    OT_2_Eq,
    OT_2_Neq,  // non equal
    OT_2_GT,  // greater than
    OT_2_LT,  // less than
    OT_2_GET,  // greater equal than
    OT_2_LET,  // less equal than

    // logical
    OT_2_LAND,
    OT_2_LOR,

    OT_2_Left,
    OT_2_Right,
    OT_2_Concat,
    OT_2_End,

    OT_3_Start,
    OT_3_If,
    OT_3_End,
};

struct FuncHandlerBase
{
    virtual ~FuncHandlerBase();

    // function call with no paramter
    virtual OperandType Func0(OpType op) const;

    // function call with 1 paramter
    virtual OperandType Func1(OpType op, OperandType a1) const;

    // function call with 2 paramters
    virtual OperandType Func2(OpType op, OperandType a1, OperandType a2) const;

    // function call with 3 paramters
    virtual OperandType Func3(OpType op, OperandType a1,
            OperandType a2, OperandType a3) const;

    // function call with 4 paramters
    virtual OperandType Func4(OpType op, OperandType a1,
            OperandType a2, OperandType a3, OperandType a4) const;
};


// TODO: turn this calculator into a little language.
// need an intermediate symbol table.
class CalcParserImpl;
class CalculatorParser : boost::noncopyable {
public:
    CalculatorParser();
    explicit CalculatorParser(FuncHandlerBase* handler, bool del = false);

    ~CalculatorParser();

    FuncHandlerBase* GetHandler() const;
    void SetHandler(FuncHandlerBase* handler, bool del);

    // try to parse the given expression and generate a corresponding ast.
    bool ParseExpression(const std::string& exp, std::string& err);

    // parse the expression, and check whether the operand exists.
    bool ParseExpression(
        const std::string& exp, std::string& err, const std::map<std::string, OperandType>& ref);

    // execute the translated-expression(ast) and get the result.
    // ref contains the all variables used in the expression.
    OperandType GenValue(const std::map<std::string, OperandType>& ref, std::string& err);

    // combine parsing and excuting the ast into one call.
    OperandType GenValue(
        const std::string& exp, const std::map<std::string, OperandType>& ref, std::string& err) {
        if (!ParseExpression(exp, err)) return nil();

        return GenValue(ref, err);
    }

    // parse literal arithmetic expression, and return the result.
    // eg, exp = " 2 + 3 * 5", then return 17.
    OperandType GenLiteralValue(const std::string& exp, std::string& err) {
        std::map<std::string, OperandType> ref;
        return GenValue(exp, ref, err);
    }

private:
    CalcParserImpl* m_impl;
};

} // namespace CalcParser

#endif  // _CALCPARSER_H_

