#ifndef __INK_TYPES_H__
#define __INK_TYPES_H__

#include <unordered_map>

enum ObjType
{
    OT_STR,
    OT_FLOAT,
    OT_INT,
    OT_NONE,
};

//  gc object
class Value
{
public:
    Value(): type_(OT_NONE) {}

private:
    union V
    {
        char* s_;
        void* p_; // table??
        double f_;
        int64_t i_;
    }; // anonymous union

    // discriminated union
    ObjType type_;
};

// table impl mimics that in lua
struct Table
{
    std::unordered_map<std::string, Value> map_;
};


#endif  // end __INK_TYPES_H__

