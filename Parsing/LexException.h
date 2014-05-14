#ifndef LEX_EXCEPTION_H_
#define LEX_EXCEPTION_H_

#include <string>
#include <exception>

class LexErrException: public std::exception
{
    public:

        LexErrException(const char* pos, const char* what)
            :what_(what), errPosition_(pos)
        {
        }

        ~LexErrException() throw() {}

        virtual const char* what() const throw()
        {
            return what_.c_str();
        }

    private:

        std::string what_;
        const char* errPosition_;
};

#endif

