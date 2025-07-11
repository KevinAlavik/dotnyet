#pragma once

#include <cstdint>

namespace DotNyet::Bytecode {
    enum class Opcode : uint8_t {
        // Stack manipulation opcodes
        NOP   = 0x00,
        PUSH  = 0x01,
        POP   = 0x02,
        CMP   = 0x03,

        // Control flow / function call
        DEF   = 0x10,
        CALL  = 0x11,
        RET   = 0x12,

        // Memory access
        STORE = 0x20,
        LOAD  = 0x21,

        // Jump instructions
        JMP   = 0x30,
        JZ    = 0x31,
        JNZ   = 0x32,

        // Miscellaneous
        HALT  = 0x40,

        // I/O opcodes
        PRINT = 0x50,
        INPUT = 0x51,

        // Math
        ADD   = 0x60,
        SUB   = 0x61
    };

    enum class ValueTypeTag : uint8_t {
        Null = 0,
        Integer = 1,
        Double = 2,
        Boolean = 3,
        String = 4,
    };
}
