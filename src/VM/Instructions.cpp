#include <DotNyet/VM/Instructions.hpp>
#include <DotNyet/VM/Stack.hpp>
#include <DotNyet/Core/Exceptions.hpp>
#include <iostream>

namespace DotNyet::VM {
    void PushInstruction::Execute(Stack& stack) const {
        stack.Push(value);
    }

    void PopInstruction::Execute(Stack& stack) const {
        stack.Pop();
    }

    void AddInstruction::Execute(Stack& stack) const {
        auto b = stack.Pop();
        auto a = stack.Pop();

        if (!a.IsInt() || !b.IsInt()) {
            throw Core::RuntimeException("ADD requires two integer values");
        }
        int64_t sum = a.AsInt() + b.AsInt();
        stack.Push(Types::Value(sum));
    }

    void PrintInstruction::Execute(Stack& stack) const {
        auto val = stack.Pop();
        std::cout << val.ToString() << std::endl;
    }
}
