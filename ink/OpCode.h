#ifndef __INK_OPCODE_H__
#define __INK_OPCODE_H__

#include "Parser.h"

namespace ink {

enum InkOpCode
{
    OP_NOP,
    OP_MOV,
    OP_LAD, // load local value
    OP_LDK, // load literal int
    OP_LDS, // load literal string
    OP_LDF, // load literal float
    OP_GLD, // load a global value
    OP_ST, // store value to local
    OP_GST, // store value to global
    OP_JMP,
    OP_EQ,
    OP_LT,
    OP_GT,
    OP_TEST,
    OP_CALL,
    OP_RET,
    OP_LOOP,

    // arithmetic
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_FADD,
    OP_FSUB,
    OP_FMUL,
    OP_DIV,
    OP_POW,
    OP_NOT,
    OP_OR,
    OP_XOR,
    OP_ADN,
    OP_SHL,
    OP_SHR,
    OP_INV, //~
};

class CodeGen
{
    public:
        // input ast, and compiler context
        CodeGen();
        ~CodeGen();

        void StartGenCode();
        void SetParser(const ParserPtr& p) { parser_ = p; }

    private:
        ParserPtr parser_;
};

}

#endif

