cmake_minimum_required(VERSION 3.16)
project(DotNyet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

file(GLOB_RECURSE DOTNYET_SOURCES
    src/*.cpp
    src/*.hpp
)

find_package(fmt REQUIRED)
add_executable(dotnyet ${DOTNYET_SOURCES})
target_link_libraries(dotnyet PRIVATE fmt::fmt)

if (WIN32)
    target_compile_definitions(dotnyet PRIVATE UNICODE _UNICODE)
endif()
