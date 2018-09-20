#include <stdint.h>
#include <stack>
#include <iostream>

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
//      Otherwise, decrement the next stack index by 1 and return the value at the
//      new index.
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
//      Pop the top value. If the value is 0 mod 4, then take the branch matching
//      the current direction. If the value is 1 mod 4, then take the branch matching
//      the next direction with a matching CC but the next DP. And so forth.
//  switch
//      Validation: next stack index > 0
//      Pop the top value. If the value is 0 mod 2, then take the branch matching
//      the current direction. If the value is 1 mod 2, then take the branch matching
//      the next direction with matching DP but the next CC.
//
//  INPUT:
//  in (char), in (number)
//  out (char), out (number)

std::stack<uint32_t> stack;

std::stack<uint32_t>& mondriaan_dump_stack() {
    return stack;
}

extern "C" {
    void mondriaan_runtime_push(uint32_t value) {
        stack.push(value);
    }

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

        std::cout << (char) stack.top();
        stack.pop();
    }
}
