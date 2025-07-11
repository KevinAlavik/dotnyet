#pragma once

#include <vector>
#include <DotNyet/Types/Value.hpp>
#include <Util/Log.hpp>

namespace DotNyet::VM {
    class Stack {
    public:
        Stack();

        void Push(const Types::Value& val);
        Types::Value Pop();
        const Types::Value& Peek(size_t depth = 0) const;
        size_t Size() const;

    private:
        std::vector<Types::Value> stack;
        Util::Logger logger;
    };

}
