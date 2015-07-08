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
        virtual void Visit(AstIntExp*) {}
        virtual void Visit(AstBoolExp*) {}
        virtual void Visit(AstFloatExp*) {}
        virtual void Visit(AstStringExp*) {}
        virtual void Visit(AstFuncProtoExp*) {}
        virtual void Visit(AstScopeStatementExp*) {}
        virtual void Visit(AstFuncDefExp*) {}
        virtual void Visit(AstFuncCallExp*) {}
        virtual void Visit(AstArrayExp*) {}
        virtual void Visit(AstArrayIndexExp*) {}
        virtual void Visit(AstVarExp*) {}
        virtual void Visit(AstUnaryExp*) {}
        virtual void Visit(AstBinaryExp*) {}
        virtual void Visit(AstRetExp*) {}
        virtual void Visit(AstIfExp*) {}
        virtual void Visit(AstTrueExp*) {}
        virtual void Visit(AstWhileExp*) {}
        virtual void Visit(AstForExp*) {}
        virtual void Visit(AstErrInfo*) {}
};

} // end namespace

#endif

