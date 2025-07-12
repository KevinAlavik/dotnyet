#include <Util/Log.hpp>
#include <DotNyet/VM/VirtualMachine.hpp>
#include <DotNyet/Core/Exceptions.hpp>
#include <DotNyet/Bytecode/Opcodes.hpp>

#include <fstream>
#include <print>
#include <vector>
#include <string>
#include <exception>
#include <typeinfo>
#include <cstring>
#include <memory>
#include <Util/Demangle.hpp>
#include <getopt.h>

constexpr char NYET_MAGIC[4] = {'N', 'Y', 'E', 'T'};
constexpr uint8_t NYET_VERSION = 0x01;

Util::Logger logger("Main");

void print_usage(const char* prog_name) {
    std::printf("Usage: %s [options] <bytecode file> [-- args...]\n", prog_name);
    std::printf("Options:\n");
    std::printf("  -h, --help             Show this help message and exit\n");
    std::printf("  -v, --version          Show version information and exit\n");
    std::printf("  -l, --log-level=LEVEL  Set logging level (debug, info, warn, error)\n");
    std::printf("  -n, --no-verify        Disable bytecode verification\n");
}

void print_version() {
    std::printf(".NYET v1.0\n");
    std::printf("Copyright (C) 2025 Kevin Alavik\n");
    std::printf("License: Apache License, Version 2.0\n");
    std::printf("Git commit: %s\n", GIT_HASH);
    std::printf("This is free software; you are free to change and redistribute it.\n");
    std::printf("There is NO WARRANTY, to the extent permitted by law.\n");
}

void prog(const std::string& filename, const std::string& args = "", bool verify_bytecode = true) {
    using namespace DotNyet::VM::Core;

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw BytecodeFormatException("Failed to open bytecode file: " + filename);
    }

    if (verify_bytecode) {
        char magic[4];
        file.read(magic, 4);
        if (file.gcount() != 4 || std::memcmp(magic, NYET_MAGIC, 4) != 0) {
            logger.Warn("Invalid bytecode file: missing NYET magic header, proceeding without verification");
            file.seekg(0, std::ios::beg);
        } else {
            uint8_t version = 0;
            file.read(reinterpret_cast<char*>(&version), 1);
            if (file.gcount() != 1 || version != NYET_VERSION) {
                logger.Warn("Invalid bytecode file: unsupported version {}, proceeding without verification", version);
                file.seekg(4, std::ios::beg);
            }
        }
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
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"log-level", required_argument, 0, 'l'},
        {"no-verify", no_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    int opt;
    bool verify_bytecode = true;
    std::string filename;
    std::string argString;

    while ((opt = getopt_long(argc, argv, "hvl:n", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'v':
                print_version();
                return 0;
            case 'l':
                {
                    std::string level(optarg);
                    if (level == "debug") {
                        Util::Logger::SetLogLevel(Util::Logger::Level::Debug);
                    } else if (level == "info") {
                        Util::Logger::SetLogLevel(Util::Logger::Level::Info);
                    } else if (level == "warn") {
                        Util::Logger::SetLogLevel(Util::Logger::Level::Warn);
                    } else if (level == "error") {
                        Util::Logger::SetLogLevel(Util::Logger::Level::Error);
                    } else {
                        logger.Error("Invalid log level: {}. Available levels: debug, info, warn, error", level);
                        return 1;
                    }
                }
                break;
            case 'n':
                verify_bytecode = false;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    bool afterDoubleDash = false;
    for (int i = optind; i < argc; ++i) {
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

    if (filename.empty()) {
        logger.Error("No bytecode file specified");
        print_usage(argv[0]);
        return 1;
    }

    try {
        prog(filename, argString, verify_bytecode);
    } catch (const std::exception& e) {
        logger.Error("Exception caught [{}]: {}", demangle(typeid(e).name()).c_str(), e.what());
        return 1;
    }

    return 0;
}