#ifndef __INK_OPCODE_H__
#define __INK_OPCODE_H__

#include "Parser.h"

#include <string>

namespace ink {

/*
register based instruction modeling lua byte code.
instructions are all 4 byte long, 3 address code
*/

enum OpCode
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
    OP_LE, // less equal
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
    OP_AND,
    OP_SHL,
    OP_SHR,
    OP_INV, //~

    OP_NEW_TABLE, // create table
    OP_SET_TABLE, // set table

    // maximum instruction.
    OP_MAX = (1 << 6),
};

class CodeGen
{
    public:
        CodeGen();
        ~CodeGen();

        void SetParser(const ParserPtr& p) { parser_ = p; }
        std::string StartGenCode(unsigned char* buff, size_t sz);

    private:
        ParserPtr parser_;
};

} // end namespace

#endif

