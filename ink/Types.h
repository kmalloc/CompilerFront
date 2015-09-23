#ifndef __INK_TYPES_H__
#define __INK_TYPES_H__

#include <unordered_map>

enum ObjType
{
};

//  gc object
struct Value
{
    union V
    {
        char* s_;
        void* p_; // table??
        double f_;
        int64_t i_;
    } v_; // or anonymous union?

    ObjType type_;
};

// table impl mimics that in lua
struct Table
{
    std::unordered_map<std::string, Value> map_;
};


#endif  // end __INK_TYPES_H__

