#include "vm.h"
#include "OpCode.h"

namespace ink {

void vm::Run()
{
    while (1)
    {
        short in = code_[ip_];
        switch (in)
        {
            case OP_MOVE:
            case OP_LOAD:
                {
                };
        }
    }
}

}

