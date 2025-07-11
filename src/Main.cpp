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
#include <memory>
#include <Util/Demangle.hpp>

constexpr char NYET_MAGIC[4] = {'N', 'Y', 'E', 'T'};
constexpr uint8_t NYET_VERSION = 0x01;

Util::Logger logger("Main");

void prog(const std::string& filename, const std::string& args = "") {
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

    if (!args.empty()) {
        vm.GetStack().Push(DotNyet::Types::Value(args));
    } else {
        vm.GetStack().Push(DotNyet::Types::Value(std::string()));
    }

    vm.Run();
}


int main(int argc, char* argv[]) {
    Util::Logger logger("Main");
    logger.Info(".NYET v1.0");

    if (argc < 2) {
        logger.Error("Usage: {} <bytecode file> [-- args...]", argv[0]);
        return 1;
    }

    std::string filename;
    std::string argString;

    bool afterDoubleDash = false;
    for (int i = 1; i < argc; ++i) {
        if (!afterDoubleDash) {
            if (std::strcmp(argv[i], "--") == 0) {
                afterDoubleDash = true;
            } else if (filename.empty()) {
                filename = argv[i];
            } else {
                logger.Error("Unexpected argument before --: {}", argv[i]);
                return 1;
            }
        } else {
            if (!argString.empty()) argString += " ";
            argString += argv[i];
        }
    }

    try {
        prog(filename, argString);
    } catch (const std::exception& e) {
        logger.Error("Exception caught [{}]: {}", demangle(typeid(e).name()).c_str(), e.what());
        return 1;
    }

    return 0;
}
