#pragma once

#include <cstdint>

namespace DotNyet::Bytecode {
    enum class Opcode : uint8_t {
        NOP = 0x00,
        PUSH = 0x01,
        POP = 0x02,
        ADD = 0x03,
        PRINT = 0x04,
    };

    enum class ValueTypeTag : uint8_t {
        Null = 0,
        Integer = 1,
        Double = 2,
        Boolean = 3,
        String = 4,
    };
}
