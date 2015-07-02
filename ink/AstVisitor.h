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

class VisitorBase
{
    public:
        virtual void Visit(AstIntExp* t) = 0;
        virtual void Visit(AstFloatExp* t) = 0;
        virtual void Visit(AstStringExp* t) = 0;
        virtual void Visit(AstFuncProtoExp* t) = 0;
        virtual void Visit(AstScopeStatementExp* t) = 0;
        virtual void Visit(AstFuncDefExp* t) = 0;
        virtual void Visit(AstFuncCallExp* t) = 0;
        virtual void Visit(AstArrayExp* t) = 0;
        virtual void Visit(AstArrayIndexExp* t) = 0;
        virtual void Visit(AstVarExp* t) = 0;
        virtual void Visit(AstUnaryExp* t) = 0;
        virtual void Visit(AstBinaryExp* t) = 0;
        virtual void Visit(AstRetExp* t) = 0;
        virtual void Visit(AstIfExp* t) = 0;
        virtual void Visit(AstTrueExp* t) = 0;
        virtual void Visit(AstWhileExp* t) = 0;
        virtual void Visit(AstForExp* t) = 0;
};


} // end namespace

#endif

