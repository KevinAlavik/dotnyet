#include <DotNyet/VM/Stack.hpp>
#include <DotNyet/Core/Exceptions.hpp>

namespace DotNyet::VM {

    Stack::Stack()
        : logger("VM/Stack")
    {}

    void Stack::Push(const Types::Value& val) {
        stack.push_back(val);
    }

    Types::Value Stack::Pop() {
        if (stack.empty()) {
            logger.Warn("Stack underflow on Pop");
            throw Core::StackException("Pop called on empty stack");
        }
        Types::Value val = stack.back();
        stack.pop_back();
        return val;
    }

    const Types::Value& Stack::Peek(size_t depth) const {
        if (depth >= stack.size()) {
            logger.Warn("Stack access out of bounds at depth {}", depth);
            throw Core::StackException("Peek access out of bounds");
        }
        const auto& val = stack[stack.size() - 1 - depth];
        return val;
    }

    size_t Stack::Size() const {
        return stack.size();
    }
}
