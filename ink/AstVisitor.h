#ifndef __INK_AST_VISITOR_H__
#define __INK_AST_VISITOR_H__

namespace ink {

class AstIntExp;
class AstFloatExp;
class AstStringExp;
class AstFuncProtoExp;
class AstScopeStatementExp;
class AstFuncDefExp;
class AstFuncCallExp;
class AstArrayExp;
class AstArrayIndexExp;
class AstVarExp;
class AstUnaryExp;
class AstBinaryExp;
class AstRetExp;
class AstIfExp;
class AstTrueExp;
class AstWhileExp;
class AstForExp;
class AstErrInfo;
class AstBoolExp;

class VisitorBase
{
    public:
        virtual int64_t Visit(AstIntExp*) {}
        virtual int64_t Visit(AstBoolExp*) {}
        virtual int64_t Visit(AstFloatExp*) {}
        virtual int64_t Visit(AstStringExp*) {}
        virtual int64_t Visit(AstFuncProtoExp*) {}
        virtual int64_t Visit(AstScopeStatementExp*) {}
        virtual int64_t Visit(AstFuncDefExp*) {}
        virtual int64_t Visit(AstFuncCallExp*) {}
        virtual int64_t Visit(AstArrayExp*) {}
        virtual int64_t Visit(AstArrayIndexExp*) {}
        virtual int64_t Visit(AstVarExp*) {}
        virtual int64_t Visit(AstUnaryExp*) {}
        virtual int64_t Visit(AstBinaryExp*) {}
        virtual int64_t Visit(AstRetExp*) {}
        virtual int64_t Visit(AstIfExp*) {}
        virtual int64_t Visit(AstTrueExp*) {}
        virtual int64_t Visit(AstWhileExp*) {}
        virtual int64_t Visit(AstForExp*) {}
        virtual int64_t Visit(AstErrInfo*) {}
};

} // end namespace

#endif

