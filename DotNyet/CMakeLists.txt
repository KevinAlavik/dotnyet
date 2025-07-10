add_executable(dotnyet
    src/Main.cpp
)

include_directories(
    "include/"
)

target_compile_options(dotnyet PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

if (UNIX)
    target_link_libraries(dotnyet PRIVATE stdc++)
endif()
