#include <DotNyet/VM/VirtualMachine.hpp>
#include <DotNyet/Core/Exceptions.hpp>
#include <iostream>
#include <cstring>
#include <fmt/core.h>

namespace DotNyet::VM {
    VirtualMachine::VirtualMachine()
        : ip(0), logger("VM/Core") {}

    void VirtualMachine::LoadBytecode(std::vector<uint8_t> code) {
        bytecode = std::move(code);
        ip = 0;
    }

    int64_t VirtualMachine::ReadInt64(size_t pos) const {
        if (pos + 8 > bytecode.size()) {
            throw Core::RuntimeException("Unexpected end of bytecode reading int64");
        }
        int64_t val;
        std::memcpy(&val, &bytecode[pos], sizeof(int64_t));
        return val;
    }

    uint32_t VirtualMachine::ReadUInt32(size_t pos) const {
        if (pos + 4 > bytecode.size()) {
            throw Core::RuntimeException("Unexpected end of bytecode reading uint32");
        }
        uint32_t val;
        std::memcpy(&val, &bytecode[pos], sizeof(uint32_t));
        return val;
    }

    std::string VirtualMachine::ReadString(size_t pos, size_t len) const {
        if (pos + len > bytecode.size()) {
            throw Core::RuntimeException("Unexpected end of bytecode reading string");
        }
        return std::string(bytecode.begin() + pos, bytecode.begin() + pos + len);
    }

    void VirtualMachine::Run() {
        logger.Info("Starting execution with {} bytes of bytecode", bytecode.size());
    
        while (ip < bytecode.size()) {
            using namespace DotNyet::Bytecode;
            Opcode op = static_cast<Opcode>(bytecode[ip++]);
    
            logger.Debug("IP = {} | Executing opcode: 0x{:02X}", ip - 1, static_cast<uint8_t>(op));
    
            try {
                switch (op) {
                    case Opcode::NOP:
                        logger.Debug("NOP");
                        break;
    
                    case Opcode::PUSH: {
                        if (ip >= bytecode.size())
                            throw Core::RuntimeException("Unexpected end of bytecode reading PUSH type tag");
    
                        auto typeTag = static_cast<ValueTypeTag>(bytecode[ip++]);
                        logger.Debug("PUSH type tag: {}", static_cast<int>(typeTag));
    
                        switch (typeTag) {
                            case ValueTypeTag::Null:
                                logger.Debug("PUSH Null");
                                stack.Push(Types::Value());
                                break;
    
                            case ValueTypeTag::Integer: {
                                int64_t val = ReadInt64(ip);
                                ip += 8;
                                logger.Debug("PUSH Integer: {}", val);
                                stack.Push(Types::Value(val));
                                break;
                            }
    
                            case ValueTypeTag::Double: {
                                if (ip + 8 > bytecode.size())
                                    throw Core::RuntimeException("Unexpected end of bytecode reading double");
                                double val;
                                std::memcpy(&val, &bytecode[ip], sizeof(double));
                                ip += 8;
                                logger.Debug("PUSH Double: {}", val);
                                stack.Push(Types::Value(val));
                                break;
                            }
    
                            case ValueTypeTag::Boolean: {
                                if (ip >= bytecode.size())
                                    throw Core::RuntimeException("Unexpected end of bytecode reading bool");
                                bool val = bytecode[ip++] != 0;
                                logger.Debug("PUSH Boolean: {}", val);
                                stack.Push(Types::Value(val));
                                break;
                            }
    
                            case ValueTypeTag::String: {
                                uint32_t len = ReadUInt32(ip);
                                ip += 4;
                                std::string str = ReadString(ip, len);
                                ip += len;
                                logger.Debug("PUSH String: \"{}\"", str);
                                stack.Push(Types::Value(std::move(str)));
                                break;
                            }
    
                            default:
                                throw Core::RuntimeException(fmt::format("Unknown ValueTypeTag: {}", static_cast<int>(typeTag)));
                        }
                        break;
                    }
    
                    case Opcode::POP: {
                        logger.Debug("POP");
                        auto popped = stack.Pop();
                        logger.Debug("Popped value: {}", popped.ToString());
                        break;
                    }
    
                    case Opcode::ADD: {
                        logger.Debug("ADD");
                        auto b = stack.Pop();
                        auto a = stack.Pop();
                        logger.Debug("Operands: a = {}, b = {}", a.ToString(), b.ToString());
                    
                        auto result = a + b;
                        logger.Debug("Result: {}", result.ToString());
                    
                        stack.Push(result);
                        break;
                    }                    
    
                    case Opcode::PRINT: {
                        auto val = stack.Pop();
                        std::cout << val.ToString() << std::endl;
                        break;
                    }
    
                    default:
                        throw Core::RuntimeException(fmt::format("Unknown opcode: 0x{:02X}", static_cast<uint8_t>(op)));
                }
            } catch (const std::exception& e) {
                logger.Warn("Exception at ip={} opcode=0x{:02X}: {}", ip - 1, static_cast<uint8_t>(op), e.what());
                throw;
            }
        }
    
        logger.Info("Execution finished successfully.");
    }    

    Stack& VirtualMachine::GetStack() {
        return stack;
    }
}
