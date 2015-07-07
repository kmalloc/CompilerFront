#ifndef __INK_OPCODE_H__
#define __INK_OPCODE_H__

namespace ink {

enum InkOpCode
{
    OP_MOVE,
    OP_LOAD, // load local value
    OP_GLOAD, // load a global value
    OP_STORE, // store value to local
    OP_GSTORE, // store value to global
    OP_JUMP,
    OP_EQ,
    OP_LT,
    OP_GT,
    OP_TEST,
    OP_CALL,
    OP_PUSH,
    OP_POP,
    OP_RET,

    // arithmetic
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_BW_OR,
    OP_BW_XOR,
    OP_BW_ADN,
    OP_BW_SHL,
    OP_BW_SHR,
    OP_BW_INV, //~
};

class CodeGen
{
    public:
        // input ast, and compiler context
        CodeGen();
};

}

#endif

