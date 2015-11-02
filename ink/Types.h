#ifndef __INK_TYPES_H__
#define __INK_TYPES_H__

#include "Basic/Variant.h"

#include <unordered_map>

namespace ink {

    enum ObjType
    {
        OT_NONE,
        OT_INT,
        OT_FLOAT,
        OT_STR,
        OT_TABLE,
    };

    // table impl mimics that in lua
    struct Table
    {
    };

    //  gc object
    class Value
    {
    public:
        Value()
        {

        }

    private:

        Variant<int64_t, double, std::string, Table> val_;
    };

} // end namespace ink

#endif  // end __INK_TYPES_H__
