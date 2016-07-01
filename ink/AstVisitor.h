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
        virtual uint32_t Visit(AstIntExp*) { return 0; }
        virtual uint32_t Visit(AstBoolExp*) { return 0; }
        virtual uint32_t Visit(AstFloatExp*) { return 0; }
        virtual uint32_t Visit(AstStringExp*) { return 0; }
        virtual uint32_t Visit(AstFuncProtoExp*) { return 0; }
        virtual uint32_t Visit(AstScopeStatementExp*) { return 0; }
        virtual uint32_t Visit(AstFuncDefExp*) { return 0; }
        virtual uint32_t Visit(AstFuncCallExp*) { return 0; }
        virtual uint32_t Visit(AstArrayExp*) { return 0; }
        virtual uint32_t Visit(AstArrayIndexExp*) { return 0; }
        virtual uint32_t Visit(AstVarExp*) { return 0; }
        virtual uint32_t Visit(AstUnaryExp*) { return 0; }
        virtual uint32_t Visit(AstBinaryExp*) { return 0; }
        virtual uint32_t Visit(AstRetExp*) { return 0; }
        virtual uint32_t Visit(AstIfExp*) { return 0; }
        virtual uint32_t Visit(AstTrueExp*) { return 0; }
        virtual uint32_t Visit(AstWhileExp*) { return 0; }
        virtual uint32_t Visit(AstForExp*) { return 0; }
        virtual uint32_t Visit(AstErrInfo*) { return 0; }
};

} // end namespace

#endif

