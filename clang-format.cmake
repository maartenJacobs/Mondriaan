file(GLOB_RECURSE CLANG_FORMAT_SRC
        src/*.cpp
        include/Piet.h
        main.cpp)

add_custom_target(
        clangformat
        COMMAND /usr/local/bin/clang-format
        -style=LLVM
        -i
        ${CLANG_FORMAT_SRC}
)
