message("Configuring unit tests")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
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