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
add_executable(test test.cpp
        src/Image.cpp
        src/Reader.cpp
        src/PNG.cpp
        src/Parser.cpp
        src/Graph.cpp)

# Include PNG
find_package(PNG MODULE REQUIRED)
target_link_libraries(test PUBLIC PNG::PNG)
include_directories(${PNG_INCLUDE_DIRS})
link_directories(${PNG_LIBRARY_DIRS})

# Configure debug builds (so many segmentation faults)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wpedantic -g -O0")
