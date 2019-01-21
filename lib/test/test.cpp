#define BOOST_TEST_MAIN
#include <array>
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

BOOST_FIXTURE_TEST_CASE(test_simple_push, TestFixture) {
  mondriaan_runtime_push(42);

  auto stack = mondriaan_dump_stack();
  BOOST_CHECK(stack.size() == 1);
  BOOST_CHECK(stack.top() == 42);
}

BOOST_FIXTURE_TEST_CASE(test_duplicate_value, TestFixture) {
  mondriaan_runtime_push(1);
  mondriaan_runtime_duplicate();

  auto stack = mondriaan_dump_stack();
  BOOST_CHECK(stack.size() == 2);
  BOOST_CHECK(stack.top() == 1);
  stack.pop();
  BOOST_CHECK(stack.top() == 1);
}

BOOST_FIXTURE_TEST_CASE(test_duplicate_empty_stack, TestFixture) {
  mondriaan_runtime_duplicate();

  auto stack = mondriaan_dump_stack();
  BOOST_CHECK(stack.empty());
}

BOOST_FIXTURE_TEST_CASE(test_out_char, TestFixture) {
  mondriaan_runtime_push((uint32_t)'a');
  mondriaan_runtime_out_char();

  // TODO: figure out how to mock I/O.

  auto stack = mondriaan_dump_stack();
  BOOST_CHECK(stack.empty());
}

BOOST_FIXTURE_TEST_CASE(test_out_char_empty_stack, TestFixture) {
  mondriaan_runtime_out_char();

  // TODO: figure out how to mock I/O.

  auto stack = mondriaan_dump_stack();
  BOOST_CHECK(stack.empty());
}

BOOST_FIXTURE_TEST_CASE(test_roll_of_depth_1, TestFixture) {
  // Push values into stack: 1-5 in ascending order.
  mondriaan_runtime_push(5);
  mondriaan_runtime_push(4);
  mondriaan_runtime_push(3);
  mondriaan_runtime_push(2);
  mondriaan_runtime_push(1);

  // Push the top value down by depth 4.
  mondriaan_runtime_push(4); // depth
  mondriaan_runtime_push(1); // rolls

  mondriaan_runtime_roll();

  // Confirm that the top value has been rolled.
  auto stack = mondriaan_dump_stack();
  BOOST_CHECK_EQUAL(5, stack.size());
  std::array<uint32_t, 5> expected{2, 3, 4, 1, 5};
  for (uint32_t expectedIndex = 0; expectedIndex < 5; expectedIndex++) {
    BOOST_CHECK_EQUAL(expected[expectedIndex], stack.top());
    stack.pop();
  }
}

BOOST_FIXTURE_TEST_CASE(test_roll_of_depth_2, TestFixture) {
  // Push values into stack: 1-5 in ascending order.
  mondriaan_runtime_push(5);
  mondriaan_runtime_push(4);
  mondriaan_runtime_push(3);
  mondriaan_runtime_push(2);
  mondriaan_runtime_push(1);

  // Push the 2 top values down by depth 4.
  mondriaan_runtime_push(4); // depth
  mondriaan_runtime_push(2); // rolls

  mondriaan_runtime_roll();

  // Confirm that the 2 top values have been rolled.
  auto stack = mondriaan_dump_stack();
  BOOST_CHECK_EQUAL(5, stack.size());
  std::array<uint32_t, 5> expected{3, 4, 1, 2, 5};
  for (uint32_t expectedIndex = 0; expectedIndex < 5; expectedIndex++) {
    BOOST_CHECK_EQUAL(expected[expectedIndex], stack.top());
    stack.pop();
  }
}