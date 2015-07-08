#include "vm.h"

#include "OpCode.h"

#include <stack>
#include <memory>

namespace ink {

class Frame
{

};
typedef std::shared_ptr<Frame> FramePtr;

class Stack
{
    public:
        Stack();
        ~Stack();

        void Pop() { stack_.pop(); }
        void Push(FramePtr f) { stack_.push(f); }
        FramePtr GetTop() const { return stack_.top(); }

    private:
        std::stack<FramePtr> stack_;
};

class Runtime
{
    template<class T>
    T* GetValue(size_t p) { return static_cast<T*>(gvar_ + p); }

    private:
        char gvar_[];
};

void vm::Run()
{
    while (1)
    {
        // TODO, use direct-threaded dispatch to improve performance
        short in = code_[ip_];
        switch (in)
        {
            case OP_MOV:
            case OP_LAD:
                {
                };
        }
    }
}

}

