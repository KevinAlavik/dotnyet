#pragma once

#include <DotNyet/Types/Value.hpp>
#include <DotNyet/VM/Stack.hpp>

namespace DotNyet::VM {

    class Instruction {
    public:
        virtual ~Instruction() = default;
        virtual void Execute(Stack& stack) const = 0;
    };

    // NOP instruction — does nothing
    class NopInstruction : public Instruction {
    public:
        void Execute(Stack& /*stack*/) const override {
            // no-op
        }
    };

    // PUSH instruction — pushes any Value constant
    class PushInstruction : public Instruction {
        Types::Value value;
    public:
        explicit PushInstruction(Types::Value val) : value(std::move(val)) {}
        void Execute(Stack& stack) const override;
    };

    // POP instruction — pops top of stack and discards
    class PopInstruction : public Instruction {
    public:
        void Execute(Stack& stack) const override;
    };

    // ADD instruction — pops two ints, pushes their sum
    class AddInstruction : public Instruction {
    public:
        void Execute(Stack& stack) const override;
    };

    // PRINT instruction — pops and prints value
    class PrintInstruction : public Instruction {
    public:
        void Execute(Stack& stack) const override;
    };

}
