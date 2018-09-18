#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <stack>

#include "../include/Runtime.h"

#include <iostream>

using namespace std;

struct TestFixture {
    TestFixture() : runtimeStack(mondriaan_dump_stack()) {}
    ~TestFixture() {
        while (!runtimeStack.empty()) {
            runtimeStack.pop();
        }
    }

    std::stack<uint32_t> &runtimeStack;
};

BOOST_FIXTURE_TEST_CASE( test_simple_push, TestFixture )
{
    mondriaan_runtime_push(42);

    auto stack = mondriaan_dump_stack();
    BOOST_CHECK( stack.size() == 1 );
    BOOST_CHECK( stack.top() == 42 );
}

BOOST_FIXTURE_TEST_CASE( test_duplicate_value, TestFixture )
{
    mondriaan_runtime_push(1);
    mondriaan_runtime_duplicate();

    auto stack = mondriaan_dump_stack();
    BOOST_CHECK( stack.size() == 2 );
    BOOST_CHECK( stack.top() == 1 );
    stack.pop();
    BOOST_CHECK( stack.top() == 1 );
}

BOOST_FIXTURE_TEST_CASE( test_duplicate_empty_stack, TestFixture )
{
    mondriaan_runtime_duplicate();

    auto stack = mondriaan_dump_stack();
    BOOST_CHECK( stack.empty() );
}
