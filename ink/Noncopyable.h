#ifndef __INK_NONCOPYABLE_H__
#define __INK_NONCOPYABLE_H__

namespace ink {

class noncopyable
{
    protected:
        noncopyable() {}
        ~noncopyable() {}

    private:
        noncopyable(const noncopyable& other);
        noncopyable& operator=(const noncopyable&);
};

}

#endif

