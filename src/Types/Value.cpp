#include <DotNyet/Types/Value.hpp>

namespace DotNyet::Types {

    Value::Value(int64_t i) : data(i) {}
    Value::Value(double d) : data(d) {}
    Value::Value(bool b) : data(b) {}
    Value::Value(const std::string& s) : data(s) {}

    ValueType Value::Type() const {
        if (std::holds_alternative<std::monostate>(data)) return ValueType::Null;
        if (std::holds_alternative<int64_t>(data)) return ValueType::Integer;
        if (std::holds_alternative<double>(data)) return ValueType::Double;
        if (std::holds_alternative<bool>(data)) return ValueType::Boolean;
        if (std::holds_alternative<std::string>(data)) return ValueType::String;
        return ValueType::Unknown;
    }

    std::string Value::ToString() const {
        switch (Type()) {
            case ValueType::Null:
                return "null";
            case ValueType::Integer:
                return std::to_string(std::get<int64_t>(data));
            case ValueType::Double:
                return std::to_string(std::get<double>(data));
            case ValueType::Boolean:
                return std::get<bool>(data) ? "true" : "false";
            case ValueType::String:
                return std::get<std::string>(data);
            default:
                return "<unknown>";
        }
    }

    bool Value::IsNull() const {
        return std::holds_alternative<std::monostate>(data);
    }

    bool Value::IsInt() const {
        return std::holds_alternative<int64_t>(data);
    }

    bool Value::IsDouble() const {
        return std::holds_alternative<double>(data);
    }

    bool Value::IsBool() const {
        return std::holds_alternative<bool>(data);
    }

    bool Value::IsString() const {
        return std::holds_alternative<std::string>(data);
    }

    int64_t Value::AsInt() const {
        if (!IsInt()) throw DotNyet::VM::Core::TypeException("Value is not an int");
        return std::get<int64_t>(data);
    }

    double Value::AsDouble() const {
        if (!IsDouble()) throw DotNyet::VM::Core::TypeException("Value is not a double");
        return std::get<double>(data);
    }

    bool Value::AsBool() const {
        if (!IsBool()) throw DotNyet::VM::Core::TypeException("Value is not a bool");
        return std::get<bool>(data);
    }

    const std::string& Value::AsString() const {
        if (!IsString()) throw DotNyet::VM::Core::TypeException("Value is not a string");
        return std::get<std::string>(data);
    }

    bool Value::IsTruthy() const {
        switch (Type()) {
            case ValueType::Null: return false;
            case ValueType::Boolean: return data.index() == 3 && std::get<bool>(data);
            case ValueType::Integer: return data.index() == 1 && std::get<int64_t>(data) != 0;
            case ValueType::Double: return data.index() == 2 && std::get<double>(data) != 0.0;
            case ValueType::String: return data.index() == 4 && !std::get<std::string>(data).empty();
            default: return false;
        }
    }
    
    Value operator+(const Value& lhs, const Value& rhs) {
        if (lhs.IsNull() || rhs.IsNull()) {
            throw DotNyet::VM::Core::RuntimeException("Cannot add null values");
        }

        if (lhs.IsInt() && rhs.IsInt()) {
            return Value(lhs.AsInt() + rhs.AsInt());
        }
        
        if (lhs.IsDouble() && rhs.IsDouble()) {
            return Value(lhs.AsDouble() + rhs.AsDouble());
        }

        if (lhs.IsInt() && rhs.IsDouble() || lhs.IsDouble() && rhs.IsInt()) {
            return Value(lhs.AsDouble() + rhs.AsDouble());
        }
        
        if (lhs.IsString() && rhs.IsString()) {
            return Value(lhs.AsString() + rhs.AsString());
        }

        if (lhs.IsString() && rhs.IsInt()) {
            return Value(lhs.AsString() + std::to_string(rhs.AsInt()));
        }
        
        if (lhs.IsString() && rhs.IsDouble()) {
            return Value(lhs.AsString() + std::to_string(rhs.AsDouble()));
        }

        if (lhs.IsDouble() && rhs.IsString()) {
            return Value(std::to_string(lhs.AsDouble()) + rhs.AsString());
        }

        if (lhs.IsInt() && rhs.IsString()) {
            return Value(std::to_string(lhs.AsInt()) + rhs.AsString());
        }

        throw DotNyet::VM::Core::RuntimeException(fmt::format(
            "Unsupported addition operand types: {} and {}",
            lhs.Type(), rhs.Type()
        ));
    }

    Value operator-(const Value& lhs, const Value& rhs) {
        if (lhs.IsNull() || rhs.IsNull()) {
            throw DotNyet::VM::Core::RuntimeException("Cannot subtract null values");
        }

        if (lhs.IsInt() && rhs.IsInt()) {
            return Value(lhs.AsInt() - rhs.AsInt());
        }

        throw DotNyet::VM::Core::RuntimeException(fmt::format(
            "Unsupported subtraction operand types: {} and {}",
            lhs.Type(), rhs.Type()
        ));
    }

    std::ostream& operator<<(std::ostream& os, const Value& val) {
        os << val.ToString();
        return os;
    }

}
