#ifndef __INK_OPCODE_H__
#define __INK_OPCODE_H__

#include "Parser.h"

#include <string>

namespace ink {

/*
register based instruction modeling lua byte code.
instructions are all 4 byte long, 3 address code

 |op(6 bits)|A(9 bits)|B(9 bits)|C(8 bits)|
*/

using ins_t = uint32_t;

constexpr uint32_t GetReg64Num() { return 32; }
constexpr uint32_t GetReg32Num() { return 128; }

constexpr uint32_t InsSize() { return 32; }
constexpr uint32_t InsOpSize() { return 6; }
constexpr uint32_t InsOpASize() { return 9; }
constexpr uint32_t InsOpBSize() { return 9; }
constexpr uint32_t InsOpCSize() { return 8; }

constexpr uint32_t InsOpPos() { return 0; }
constexpr uint32_t InsAPos() { return InsOpPos() + InsOpSize(); }
constexpr uint32_t InsBPos() { return InsAPos() + InsOpASize(); }
constexpr uint32_t InsCPos() { return InsBPos() + InsOpBSize(); }

constexpr uint32_t MaxOpAddr() { return 1 << InsOpBSize(); }

enum OpCode
{
    OP_NOP,
    OP_MOV,
    /*
     * load instruction in form of: |op|A|B|
     * meaning: load pointer of value stored in address A(which may be a local addr or a global addr) to register B.
     */
    OP_LDL, // load a local variable
    OP_LDG, // load a global variable
    OP_LDK, // load literal int
    OP_LDS, // load literal string
    OP_LDF, // load literal float

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

