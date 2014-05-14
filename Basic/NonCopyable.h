#ifndef _H_NON_COPYABLE_H_
#define _H_NON_COPYABLE_H_

#define DECLARE_NONCOPYABLE(cs) \
    private:    \
        cs(const cs&);\
        const cs& operator=(const cs&);


class NonCopyable
{
    protected:
        NonCopyable(){};
        ~NonCopyable(){};
    private:
        NonCopyable(const NonCopyable&);
        const NonCopyable& operator=(const NonCopyable&);
};

#endif

