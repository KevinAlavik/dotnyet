#include <DotNyet/Types/Value.hpp>
#include <DotNyet/Core/Exceptions.hpp>

namespace DotNyet::Types {
    Value::Value(int64_t i) : data(i) {}
    Value::Value(const std::string& s) : data(s) {}

    ValueType Value::Type() const {
        if (std::holds_alternative<int64_t>(data)) return ValueType::Integer;
        if (std::holds_alternative<std::string>(data)) return ValueType::String;
        return ValueType::Unknown;
    }

    std::string Value::ToString() const {
        switch (Type()) {
            case ValueType::Integer:
                return std::to_string(std::get<int64_t>(data));
            case ValueType::String:
                return std::get<std::string>(data);
            default:
                return "<unknown>";
        }
    }

    bool Value::IsInt() const {
        return std::holds_alternative<int64_t>(data);
    }

    bool Value::IsString() const {
        return std::holds_alternative<std::string>(data);
    }

    int64_t Value::AsInt() const {
        if (!IsInt()) throw DotNyet::VM::Core::TypeException("Value is not an int");
        return std::get<int64_t>(data);
    }

    const std::string& Value::AsString() const {
        if (!IsString()) throw DotNyet::VM::Core::TypeException("Value is not a string");
        return std::get<std::string>(data);
    }

    std::ostream& operator<<(std::ostream& os, const Value& val) {
        os << val.ToString();
        return os;
    }

}