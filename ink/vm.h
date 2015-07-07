#ifndef __INK_VM_H__
#define __INK_VM_H__

namespace ink {

class vm
{
    public:

        void Run();

    private:
        int ip_;
        char code_[];
};

}
#endif

