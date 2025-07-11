#pragma once

#include <variant>
#include <string>
#include <cstdint>
#include <iostream>
#include <fmt/core.h>

namespace DotNyet::Types {
    struct Value;

    using ValueData = std::variant<
        int64_t,        // Integer
        std::string     // String (UTF-8)
    >;

    enum class ValueType {
        Integer,
        String,
        Unknown
    };

    struct Value {
        ValueData data;

        Value() = default;
        explicit Value(int64_t i);
        explicit Value(const std::string& s);

        ValueType Type() const;

        std::string ToString() const;
        bool IsInt() const;
        bool IsString() const;

        int64_t AsInt() const;
        const std::string& AsString() const;
    };

    std::ostream& operator<<(std::ostream& os, const Value& val);
}
