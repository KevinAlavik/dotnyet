#include <Util/Log.hpp>
#include <DotNyet/VM/VirtualMachine.hpp>
#include <DotNyet/Core/Exceptions.hpp>
#include <DotNyet/Bytecode/Opcodes.hpp>

#include <fstream>
#include <vector>
#include <string>
#include <exception>
#include <typeinfo>
#include <cstring>
#include <cxxabi.h>
#include <memory>

/* UTIL, TODO: MOVE */
std::string demangle(const char* mangledName) {
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res{
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    };
    return (status == 0) ? res.get() : mangledName;
}

constexpr char NYET_MAGIC[4] = {'N', 'Y', 'E', 'T'};
constexpr uint8_t NYET_VERSION = 0x01;

Util::Logger logger("Main");

void prog(const std::string& filename) {
    using namespace DotNyet::VM::Core;

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw BytecodeFormatException("Failed to open bytecode file: " + filename);
    }

    char magic[4];
    file.read(magic, 4);
    if (file.gcount() != 4 || std::memcmp(magic, NYET_MAGIC, 4) != 0) {
        throw BytecodeFormatException("Invalid bytecode file: missing NYET magic header");
    }

    uint8_t version = 0;
    file.read(reinterpret_cast<char*>(&version), 1);
    if (file.gcount() != 1) {
        throw BytecodeFormatException("Invalid bytecode file: missing version byte");
    }
    if (version != NYET_VERSION) {
        throw BytecodeFormatException("Unsupported bytecode version: " + std::to_string(version));
    }

    std::vector<uint8_t> program(std::istreambuf_iterator<char>(file), {});
    DotNyet::VM::VirtualMachine vm;
    vm.LoadBytecode(std::move(program));
    vm.Run();
}

int main(int argc, char* argv[]) {
    Util::Logger logger("Main");
    logger.Info(".NYET v1.0");

    if (argc != 2) {
        logger.Error("Usage: {} <bytecode file>", argv[0]);
        return 1;
    }

    try {
        prog(argv[1]);
    } catch (const std::exception& e) {
        logger.Error("Exception caught [{}]: {}", demangle(typeid(e).name()).c_str(), e.what());
        return 1;
    }

    return 0;
}
