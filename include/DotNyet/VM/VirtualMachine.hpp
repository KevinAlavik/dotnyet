#pragma once

#include <vector>
#include <memory>
#include <DotNyet/VM/Instructions.hpp>
#include <DotNyet/VM/Stack.hpp>
#include <Util/Log.hpp>

namespace DotNyet::VM {
    class VirtualMachine {
    public:
        VirtualMachine();

        void AddInstruction(std::unique_ptr<Instruction> instr);
        void Run();
        Stack& GetStack();

    private:
        std::vector<std::unique_ptr<Instruction>> instructions;
        Stack stack;
        size_t instructionPointer;

        Util::Logger logger;
    };
}
