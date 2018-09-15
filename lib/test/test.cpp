#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <stack>

#include "../include/Runtime.h"

BOOST_AUTO_TEST_CASE( test1 )
{
    mondriaan_runtime_push(42);

    auto stack = mondriaan_dump_stack();
    BOOST_CHECK( stack.size() == 1 );
    BOOST_CHECK( stack.top() == 42 );
}
