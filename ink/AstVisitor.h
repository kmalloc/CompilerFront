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
        virtual uint32_t Visit(AstIntExp*) {}
        virtual uint32_t Visit(AstBoolExp*) {}
        virtual uint32_t Visit(AstFloatExp*) {}
        virtual uint32_t Visit(AstStringExp*) {}
        virtual uint32_t Visit(AstFuncProtoExp*) {}
        virtual uint32_t Visit(AstScopeStatementExp*) {}
        virtual uint32_t Visit(AstFuncDefExp*) {}
        virtual uint32_t Visit(AstFuncCallExp*) {}
        virtual uint32_t Visit(AstArrayExp*) {}
        virtual uint32_t Visit(AstArrayIndexExp*) {}
        virtual uint32_t Visit(AstVarExp*) {}
        virtual uint32_t Visit(AstUnaryExp*) {}
        virtual uint32_t Visit(AstBinaryExp*) {}
        virtual uint32_t Visit(AstRetExp*) {}
        virtual uint32_t Visit(AstIfExp*) {}
        virtual uint32_t Visit(AstTrueExp*) {}
        virtual uint32_t Visit(AstWhileExp*) {}
        virtual uint32_t Visit(AstForExp*) {}
        virtual uint32_t Visit(AstErrInfo*) {}
};

} // end namespace

#endif

