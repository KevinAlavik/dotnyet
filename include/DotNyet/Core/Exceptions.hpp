#pragma once

#include <stdexcept>
#include <string>

namespace DotNyet::VM::Core {
    class VMException : public std::runtime_error {
    public:
        explicit VMException(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    class StackException : public VMException {
    public:
        explicit StackException(const std::string& msg)
            : VMException("StackException: " + msg) {}
    };

    class TypeException : public VMException {
    public:
        explicit TypeException(const std::string& msg)
            : VMException("TypeException: " + msg) {}
    };

    class RuntimeException : public VMException {
    public:
        explicit RuntimeException(const std::string& msg)
            : VMException("RuntimeException: " + msg) {}
    };

    class BytecodeFormatException : public VMException {
    public:
        explicit BytecodeFormatException(const std::string& msg)
            : VMException("BytecodeFormatException: " + msg) {}
    };
}
