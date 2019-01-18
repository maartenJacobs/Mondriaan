# Recommend minimum version from https://cliutils.gitlab.io/modern-cmake/chapters/basics.html
cmake_minimum_required(VERSION 3.1...3.12)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(Mondriaan VERSION 0.0.0
                  DESCRIPTION "LLVM-based Piet compiler"
				  LANGUAGES CXX)

# C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define executable and sources
add_executable(mondriaan main.cpp
        src/Image.cpp
        src/Reader.cpp
        src/PNG.cpp
        src/Parser.cpp
        src/Graph.cpp
        src/Translator.cpp
        src/ColorTransition.cpp
        src/DirectionPoint.cpp)

# Define tests
add_subdirectory(test/unit)
enable_testing()
add_test(NAME MondriaanUnitTest COMMAND Test)

# Include PNG
find_package(PNG MODULE REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
link_directories(${PNG_LIBRARY_DIRS})

# Include LLVM
# Copied from https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
llvm_map_components_to_libnames(llvm_libs all)
message(STATUS "Using llvm libs: ${llvm_libs}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${llvm_libs})


target_link_libraries(mondriaan PUBLIC PNG::PNG ${llvm_libs})

# Configure debug builds (so many segmentation faults)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wpedantic -g -O0")

# Include third-party tools.
include(clang-format.cmake)
