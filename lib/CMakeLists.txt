# Recommend minimum version from https://cliutils.gitlab.io/modern-cmake/chapters/basics.html
cmake_minimum_required(VERSION 3.1...3.12)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(MondriaanRuntime VERSION 0.0.0
                         DESCRIPTION "Runtime for programs created by the LLVM-based Piet compiler"
				         LANGUAGES CXX)

# C++ settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src) 
add_subdirectory(test)
enable_testing()
add_test(NAME MyTest COMMAND Test)