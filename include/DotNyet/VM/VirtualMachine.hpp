#pragma once

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
        Util::Logger logger;

        int64_t ReadInt64(size_t pos) const;
        uint32_t ReadUInt32(size_t pos) const;
        std::string ReadString(size_t pos, size_t len) const;
    };
}
