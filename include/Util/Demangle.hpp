#pragma once
#if defined(__GNUG__)
#include <memory>
#include <cstdlib>
#include <cxxabi.h>
#elif defined(_MSC_VER)
#include <windows.h>
#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

std::string demangle(const char* mangledName) {
#if defined(__GNUG__)
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res{
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    };
    return (status == 0 && res) ? res.get() : mangledName;

#elif defined(_MSC_VER)
    char demangled[1024];
    if (UnDecorateSymbolName(mangledName, demangled, sizeof(demangled), UNDNAME_COMPLETE)) {
        return std::string(demangled);
    }
    else {
        return mangledName;
    }

#else
    // No demangling available
    return mangledName;
#endif
}