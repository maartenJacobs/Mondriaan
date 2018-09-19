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

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${TEST_SOURCE_DIR}/src
        ${TEST_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
        )
add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(Test ParserTest.cpp
        ../../src/Image.cpp
        ../../src/Parser.cpp
        ../../src/Graph.cpp
        )
target_link_libraries(Test
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${llvm_libs}
        )