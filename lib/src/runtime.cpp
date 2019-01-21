#include <iostream>
#include <stack>
#include <stdint.h>
#include <vector>

// Register operations:
//  PRIMITIVES:
//  push i32
//      push should check if the next stack index is greater than the
//      stack size; if it is, the stack should first be extended by 10 items.
//      Then set the next stack index to the pushed value.
//      Finally increment the next stack index.
//  pop
//      pop should check if the next stack index is greater than 0; if it's not,
//      skip the pop command.
//      Otherwise, decrement the next stack index by 1 and return the value at
//      the new index.
//
//  COMPOUNDS:
//  add
//      Validation: next stack index > 1
//      pop + pop + push
//  subtract
//      Validation: next stack index > 1
//  multiply
//      Validation: next stack index > 1
//  divide
//      Validation: next stack index > 1
//  mod
//      Validation: next stack index > 1
//  not
//      Validation: next stack index > 0
//  greater
//      Validation: next stack index > 1
//  roll
//      Validation: next stack index > 2 and stack[-2] >= 0
//      Roll pops the stack and uses the stack value as the number of rolls.
//      Then the stack is popped again and the stack value is used as the depth.
//      Select the first n top values of the stack, where n is the absolute
//      number of rolls. If the number of rolls is greater than 0, move the
//      selected values down the stack by the depth.
//      If the number of rolls is less than 0, move the selected values up the
//      stack, starting from the bottom, by the depth.
//      If the number of rolls is 0, do nothing.
//
//  CONTROL FLOW:
//  pointer
//      Validation: next stack index > 0
//      Pop the top value. If the value is 0 mod 4, then take the branch
//      matching the current direction. If the value is 1 mod 4, then take the
//      branch matching the next direction with a matching CC but the next DP.
//      And so forth.
//  switch
//      Validation: next stack index > 0
//      Pop the top value. If the value is 0 mod 2, then take the branch
//      matching the current direction. If the value is 1 mod 2, then take the
//      branch matching the next direction with matching DP but the next CC.
//
//  INPUT:
//  in (char), in (number)
//  out (char), out (number)

typedef uint32_t piet_int;

std::stack<piet_int> stack;

std::stack<piet_int> &mondriaan_dump_stack() { return stack; }

extern "C" {
void mondriaan_runtime_push(piet_int value) { stack.push(value); }

void mondriaan_runtime_duplicate() {
  if (stack.empty()) {
    return;
  }

  stack.push(stack.top());
}

void mondriaan_runtime_out_char() {
  if (stack.empty()) {
    return;
  }

  std::cout << (char)stack.top();
  stack.pop();
}

void mondriaan_runtime_out_number() {
  if (stack.empty()) {
    return;
  }

  std::cout << stack.top();
  stack.pop();
}

uint8_t mondriaan_runtime_pointer() {
  if (stack.empty()) {
    return 0;
  }

  auto top = stack.top();
  stack.pop();

  return (uint8_t)(top % 4);
}

void mondriaan_runtime_in_number() {
  std::string line;
  std::getline(std::cin, line);
  try {
    stack.push((piet_int)std::stoul(line));
  } catch (...) {
    // Ignore
    return;
  }
}

void mondriaan_runtime_multiply() {
  if (stack.size() < 2) {
    return;
  }

  auto op1 = stack.top();
  stack.pop();
  auto op2 = stack.top();
  stack.pop();

  stack.push(op1 * op2);
}

void mondriaan_runtime_divide() {
  if (stack.size() < 2) {
    return;
  }

  auto divisor = stack.top();
  stack.pop();
  auto dividend = stack.top();
  stack.pop();

  stack.push(dividend / divisor);
}

void mondriaan_runtime_roll() {
  // A roll is only useful if there exists:
  // 1st = number of rolls
  // 2nd = depth of rolls
  // 3rd and more = values to roll. A single value is rolled to itself.
  if (stack.size() < 3) {
    return;
  }

  auto rolls = stack.top();
  stack.pop();
  auto depth = stack.top();
  stack.pop();

  // A roll can have no effect at all.
  if (rolls > depth) {
    rolls %= depth;
  }
  if (rolls == 0) {
    std::cout << "Zero rolls!" << std::endl;
    return;
  }
  if (depth > stack.size()) {
    std::cout << "Greater than stack size?" << std::endl;
    return;
  }

  // Apply the roll by collecting the relevant values, rearranging them
  // and then reinserting them.
  std::vector<piet_int> rollValues{};
  for (uint32_t valuePop = 0; valuePop < depth; valuePop++) {
    rollValues.push_back(stack.top());
    stack.pop();
  }

  for (uint32_t roll = 0; roll < rolls; roll++) {
    piet_int top = rollValues[0];
    rollValues.erase(rollValues.begin());
    rollValues.push_back(top);
  }

  for (uint32_t insertIndex = depth; insertIndex > 0; insertIndex--) {
    stack.push(rollValues[insertIndex - 1]);
  }
}
}
