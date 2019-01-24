file(GLOB_RECURSE CLANG_FORMAT_SRC
        src/*.cpp
        include/Runtime.h
        test/*.cpp)

find_program(CLANG_FORMAT NAMES clang-format)

add_custom_target(
        clangformat
        COMMAND ${CLANG_FORMAT}
        -style=LLVM
        -i
        ${CLANG_FORMAT_SRC}
)
