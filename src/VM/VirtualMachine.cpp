#include <DotNyet/VM/VirtualMachine.hpp>
#include <DotNyet/Core/Exceptions.hpp>

namespace DotNyet::VM {
    VirtualMachine::VirtualMachine()
        : instructionPointer(0), logger("VM") {}

    void VirtualMachine::AddInstruction(std::unique_ptr<Instruction> instr) {
        instructions.push_back(std::move(instr));
        logger.Debug("Instruction added, total count: {}", instructions.size());
    }

    void VirtualMachine::Run() {
        while (instructionPointer < instructions.size()) {
            auto& instr = instructions[instructionPointer];
            try {
                instr->Execute(stack);
            } catch (const std::exception& e) {
                logger.Warn("Exception at instruction {}: {}", instructionPointer, e.what());
                throw;
            }

            instructionPointer++;
        }
    }

    Stack& VirtualMachine::GetStack() {
        return stack;
    }
}
