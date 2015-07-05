#ifndef __INK_PARSER_H__
#define __INK_PARSER_H__

#include "Ast.h"
#include "Lexer.h"

#include "Noncopyable.h"

#include <string>

namespace ink {

class AstBase;

class Parser: public noncopyable
{
    public:
        Parser(const std::string& buff, const std::string& file);
        explicit Parser(const std::string& file);

        std::string StartParsing();
        std::vector<AstBasePtr>& GetResult() { return res_; }
        void SetBuffer(const std::string& buff) { buff_ = buff; }

    private:
        AstBasePtr ParsePrimary();
        AstBasePtr ParseExpression();

        // followings are primary expression.
        AstBasePtr ParseIntExp();
        AstBasePtr ParseFloatExp();
        AstBasePtr ParseParenExp();
        AstBasePtr ParseStringExp();
        AstBasePtr ParseIdentifierExp();

        AstBasePtr ParseUaryExp(TokenType op);
        AstBasePtr ParseBinaryExp(int prec, const AstBasePtr& lhs);

        AstBasePtr ParseFuncProtoExp();
        AstBasePtr ParseFuncDefExp();
        AstBasePtr ParseFuncCallExp(const std::string& fun);
        AstBasePtr ParseArrayExp();
        AstBasePtr ParseArrIndexExp(const std::string& arr);

        AstBasePtr ParseExternExp();
        AstBasePtr ParseFuncRetExp();
        AstBasePtr ParseClassDefExp();
        AstBasePtr ParseIfExp();
        AstBasePtr ParseWhileExp();
        AstBasePtr ParseForExp();

        AstBasePtr ReportError(const char* msg);
        AstScopeStatementExpPtr ParseScopeStatement();

    private:
        Lexer lex_;
        std::string file_;
        std::string buff_;

        std::vector<AstBasePtr> res_;
};

} // end ink

#endif

