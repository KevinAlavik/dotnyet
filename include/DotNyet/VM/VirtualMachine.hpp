#pragma once

#include <vector>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <string>
#include <DotNyet/Types/Value.hpp>
#include <DotNyet/VM/Stack.hpp>
#include <DotNyet/Bytecode/Opcodes.hpp>
#include <Util/Log.hpp>

namespace DotNyet::VM {
    class VirtualMachine {
    public:
        VirtualMachine();

        void LoadBytecode(std::vector<uint8_t> bytecode);
        void Run();
        Stack& GetStack();

    private:
        std::vector<uint8_t> bytecode;
        size_t ip = 0;
        Stack stack;
        std::vector<size_t> callStack;
        std::unordered_map<std::string, size_t> functionTable;
        std::unordered_map<uint32_t, Types::Value> memory;
        Util::Logger logger;

        int64_t ReadInt64(size_t pos) const;
        uint32_t ReadUInt32(size_t pos) const;
        std::string ReadString(size_t pos, size_t len) const;

        void LoadFunctionTable();
    };
}