#ifndef __INK_TYPES_H__
#define __INK_TYPES_H__

#include "Basic/Variant.h"

#include <vector>
#include <unordered_map>

namespace ink {

    enum ObjType
    {
        OT_NIL,
        OT_BOOL,
        OT_INT,
        OT_FLOAT,
        OT_STR,
        OT_TABLE,
    };

    // table impl mimics that in lua
    template<typename ...TS>
    struct Table
    {
        using type = Variant<TS...>;

        template<typename T>
        void SetValue(std::string n, T&& v)
        {
            static_assert(VariantHelper::TypeExist<T, TS...>::exist, "wrong type to store in table.");
            data_[std::move(n)] = std::forward<T>(v);
        }

    private:
        std::unordered_map<std::string, type> data_;
    };

    using InkTable = Table<int64_t, double, std::string>;

    template<typename T> struct TypeTrait
    {
        static constexpr ObjType type = OT_NIL;
    };

    template<> struct TypeTrait<bool>
    {
        static constexpr ObjType type = OT_BOOL;
    };

    template<> struct TypeTrait<int64_t>
    {
        static constexpr ObjType type = OT_INT;
    };

    template<> struct TypeTrait<float>
    {
        static constexpr ObjType type = OT_FLOAT;
    };

    template<> struct TypeTrait<double>
    {
        static constexpr ObjType type = OT_FLOAT;
    };

    template<> struct TypeTrait<std::string>
    {
        static constexpr ObjType type = OT_STR;
    };

    template<> struct TypeTrait<const char*>
    {
        static constexpr ObjType type = OT_STR;
    };

    template<> struct TypeTrait<InkTable>
    {
        static constexpr ObjType type = OT_TABLE;
    };

    // value has type, not variable.
    class Value
    {
    public:

        Value(): type_(OT_NIL) {}

        template<typename T>
        Value(T&& v)
        {
            static_assert(TypeTrait<typename std::decay<T>::type>::type, "invalid value type for Value.");

            val_ = std::forward<T>(v);
            type_ = TypeTrait<typename std::decay<T>::type>::type;
        }

        ObjType GetType() const { return type_; }

        template<typename T>
        const T& GetValue() const { return val_.GetConstRef<T>(); }

    private:
        ObjType type_;
        Variant<int64_t, double, std::string, InkTable> val_;
    };

    struct ConstPool
    {
        size_t AddConst(int64_t v)
        {
            size_t ret;
            auto pred = [v](const Value& it) { return it.GetValue<int64_t>() == v; };
            auto pos = std::find_if(int_.begin(), int_.end(), pred);

            if (pos == int_.end())
            {
                ret = int_.size();
                int_.push_back(Value(v));
            }
            else
            {
                ret = pos - int_.begin();
            }

            return ret;
        }

        size_t AddConst(double v)
        {
            size_t ret;

            // yes, float comparision.
            auto pred = [v](const Value& it) { return it.GetValue<double>() == v; };
            auto pos = std::find_if(float_.begin(), float_.end(), pred);

            if (pos == float_.end())
            {
                ret = float_.size();
                float_.push_back(Value(v));
            }
            else
            {
                ret = pos - float_.begin();
            }

            return ret;
        }

        size_t AddConst(std::string v)
        {
            size_t ret;

            auto pred = [v](const Value& it) { return it.GetValue<std::string>() == v; };
            auto pos = std::find_if(str_.begin(), str_.end(), pred);

            if (pos == str_.end())
            {
                ret = str_.size();
                str_.push_back(Value(v));
            }
            else
            {
                ret = pos - str_.begin();
            }

            return ret;
        }

        std::vector<Value> int_;
        std::vector<Value> str_;
        std::vector<Value> float_;
    };

} // end namespace ink

#endif  // end __INK_TYPES_H__
