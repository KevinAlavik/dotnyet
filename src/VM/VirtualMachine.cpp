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
        functionTable.clear();
        LoadFunctionTable();
    }

    int64_t VirtualMachine::ReadInt64(size_t pos) const {
        if (pos + 8 > bytecode.size())
            throw Core::RuntimeException("Unexpected end of bytecode reading int64");
        int64_t val;
        std::memcpy(&val, &bytecode[pos], sizeof(int64_t));
        return val;
    }

    uint32_t VirtualMachine::ReadUInt32(size_t pos) const {
        if (pos + 4 > bytecode.size())
            throw Core::RuntimeException("Unexpected end of bytecode reading uint32");
        uint32_t val;
        std::memcpy(&val, &bytecode[pos], sizeof(uint32_t));
        return val;
    }

    std::string VirtualMachine::ReadString(size_t pos, size_t len) const {
        if (pos + len > bytecode.size())
            throw Core::RuntimeException("Unexpected end of bytecode reading string");
        return std::string(bytecode.begin() + pos, bytecode.begin() + pos + len);
    }

    void VirtualMachine::LoadFunctionTable() {
        using namespace DotNyet::Bytecode;
        size_t pos = 0;

        while (pos < bytecode.size()) {
            Opcode op = static_cast<Opcode>(bytecode[pos]);

            if (op == Opcode::DEF) {
                pos++; // consume DEF opcode
                if (pos + 4 > bytecode.size())
                    throw Core::RuntimeException("Unexpected end of bytecode reading DEF name length");

                uint32_t nameLen = ReadUInt32(pos);
                pos += 4;

                if (pos + nameLen > bytecode.size())
                    throw Core::RuntimeException("Unexpected end of bytecode reading DEF name");

                std::string name = ReadString(pos, nameLen);
                pos += nameLen;

                functionTable[name] = pos;

                size_t scanPos = pos;
                while (scanPos < bytecode.size()) {
                    Opcode scanOp = static_cast<Opcode>(bytecode[scanPos]);
                    if (scanOp == Opcode::DEF)
                        break;
                    scanPos++;

                    switch (scanOp) {
                        case Opcode::PUSH: {
                            if (scanPos >= bytecode.size())
                                throw Core::RuntimeException("Unexpected end of bytecode reading PUSH type tag");

                            auto tag = static_cast<ValueTypeTag>(bytecode[scanPos++]);

                            switch (tag) {
                                case ValueTypeTag::Null:
                                    break;
                                case ValueTypeTag::Integer:
                                case ValueTypeTag::Double:
                                    scanPos += 8;
                                    break;
                                case ValueTypeTag::Boolean:
                                    scanPos += 1;
                                    break;
                                case ValueTypeTag::String: {
                                    if (scanPos + 4 > bytecode.size())
                                        throw Core::RuntimeException("Unexpected end of bytecode reading PUSH string length");
                                    uint32_t strLen = ReadUInt32(scanPos);
                                    scanPos += 4 + strLen;
                                    break;
                                }
                                default:
                                    throw Core::RuntimeException(fmt::format("Unknown PUSH ValueTypeTag {}", static_cast<int>(tag)));
                            }
                            break;
                        }

                        case Opcode::STORE:
                        case Opcode::LOAD: {
                            scanPos += 4;
                            break;
                        }

                        case Opcode::CALL: {
                            if (scanPos + 4 > bytecode.size())
                                throw Core::RuntimeException("Unexpected end of bytecode reading CALL name length");
                            uint32_t callNameLen = ReadUInt32(scanPos);
                            scanPos += 4 + callNameLen;
                            break;
                        }

                        case Opcode::JMP:
                        case Opcode::JZ:
                        case Opcode::JNZ: {
                            scanPos += 4;
                            break;
                        }

                        case Opcode::HALT:
                        case Opcode::NOP:
                        case Opcode::POP:
                        case Opcode::ADD:
                        case Opcode::PRINT:
                        case Opcode::RET:
                            break;

                        default:
                            throw Core::RuntimeException(fmt::format("Unknown opcode during function scan: 0x{:02X}", static_cast<uint8_t>(scanOp)));
                    }
                }

                pos = scanPos;
            } else {
                break;
            }
        }
    }

    void VirtualMachine::Run() {
        logger.Info("Starting execution with {} bytes of bytecode", bytecode.size());

        // Check for 'main' function
        auto it = functionTable.find("main");
        if (it == functionTable.end()) {
            throw Core::RuntimeException("No 'main' function defined");
        }

        // Simulate CALL to 'main'
        callStack.push_back(bytecode.size()); // Push end of bytecode as return address
        ip = it->second; // Set IP to main function

        while (ip < bytecode.size()) {
            using namespace DotNyet::Bytecode;
            Opcode op = static_cast<Opcode>(bytecode[ip++]);

            logger.Debug("IP = {} | Executing opcode: 0x{:02X}", ip - 1, static_cast<uint8_t>(op));

            try {
                switch (op) {
                    case Opcode::HALT:
                        logger.Debug("HALT");
                        return;

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
                                uint32_t len = ReadUInt32(ip); ip += 4;
                                std::string str = ReadString(ip, len); ip += len;
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

                    case Opcode::DEF: {
                        uint32_t nameLen = ReadUInt32(ip);
                        ip += 4;
                        std::string name = ReadString(ip, nameLen);
                        ip += nameLen;
                        logger.Debug("Skipping DEF function '{}'", name);
                        break;
                    }

                    case Opcode::CALL: {
                        uint32_t nameLen = ReadUInt32(ip);
                        ip += 4;
                        std::string name = ReadString(ip, nameLen);
                        ip += nameLen;

                        auto it = functionTable.find(name);
                        if (it == functionTable.end())
                            throw Core::RuntimeException(fmt::format("Unknown function '{}'", name));

                        logger.Debug("CALL function '{}'", name);
                        callStack.push_back(ip);
                        ip = it->second;
                        break;
                    }

                    case Opcode::RET: {
                        if (callStack.empty())
                            throw Core::RuntimeException("RET with empty call stack");

                        ip = callStack.back();
                        callStack.pop_back();
                        logger.Debug("RET to {}", ip);
                        break;
                    }

                    case Opcode::STORE: {
                        uint32_t address = ReadUInt32(ip);
                        ip += 4;
                    
                        Types::Value val = stack.Pop();
                        logger.Debug("STORE at address {}: {}", address, val.ToString());
                
                        memory[address] = val;
                        break;
                    }
                    
                    case Opcode::LOAD: {
                        uint32_t address = ReadUInt32(ip);
                        ip += 4;
                    
                        auto it = memory.find(address);
                        if (it == memory.end())
                            throw Core::RuntimeException(fmt::format("No value stored at address {}", address));
                    
                        logger.Debug("LOAD from address {}: {}", address, it->second.ToString());
                        stack.Push(it->second);
                        break;
                    }                    

                    case Opcode::JMP: {
                        uint32_t target = ReadUInt32(ip); ip += 4;
                        logger.Debug("JMP to {}", target);
                        ip = target;
                        break;
                    }

                    case Opcode::JZ: {
                        uint32_t target = ReadUInt32(ip); ip += 4;
                        Types::Value cond = stack.Pop();
                        if (!cond.IsTruthy()) {
                            logger.Debug("JZ to {}", target);
                            ip = target;
                        } else {
                            logger.Debug("JZ skipped");
                        }
                        break;
                    }

                    case Opcode::JNZ: {
                        uint32_t target = ReadUInt32(ip); ip += 4;
                        Types::Value cond = stack.Pop();
                        if (cond.IsTruthy()) {
                            logger.Debug("JNZ to {}", target);
                            ip = target;
                        } else {
                            logger.Debug("JNZ skipped");
                        }
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