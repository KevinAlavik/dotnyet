#pragma once

#include <variant>
#include <string>
#include <cstdint>
#include <iostream>
#include <fmt/core.h>
#include <DotNyet/Core/Exceptions.hpp>

namespace DotNyet::Types {
    struct Value;

    using ValueData = std::variant<
        std::monostate,  // represents null/none
        int64_t,         // Integer
        double,          // Floating point
        bool,            // Boolean
        std::string      // String (UTF-8)
    >;

    enum class ValueType {
        Null,
        Integer,
        Double,
        Boolean,
        String,
        Unknown
    };

    struct Value {
        ValueData data;

        Value() = default;
        explicit Value(int64_t i);
        explicit Value(double d);
        explicit Value(bool b);
        explicit Value(const std::string& s);

        ValueType Type() const;

        std::string ToString() const;
        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsBool() const;
        bool IsString() const;

        int64_t AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
        
        bool IsTruthy() const;
    };

    Value operator+(const Value& lhs, const Value& rhs);
    std::ostream& operator<<(std::ostream& os, const Value& val);
}

// fmt formatters for Value and ValueType
template <>
struct fmt::formatter<DotNyet::Types::Value> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const DotNyet::Types::Value& v, FormatContext& ctx) {
        return fmt::formatter<std::string>::format(v.ToString(), ctx);
    }
};

template <>
struct fmt::formatter<DotNyet::Types::ValueType> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const DotNyet::Types::ValueType& t, FormatContext& ctx) const {
        using DotNyet::Types::ValueType;
        std::string name;
        switch (t) {
            case ValueType::Null: name = "Null"; break;
            case ValueType::Integer: name = "Integer"; break;
            case ValueType::Double: name = "Double"; break;
            case ValueType::Boolean: name = "Boolean"; break;
            case ValueType::String: name = "String"; break;
            default: name = "Unknown"; break;
        }
        return fmt::formatter<std::string>::format(name, ctx);
    }
};
