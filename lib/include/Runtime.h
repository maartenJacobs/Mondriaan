#include <stack>
#include <stdint.h>

std::stack<uint32_t> &mondriaan_dump_stack();

extern "C" {
void mondriaan_runtime_push(uint32_t);
void mondriaan_runtime_duplicate();
void mondriaan_runtime_out_char();
void mondriaan_runtime_out_number();
uint8_t mondriaan_runtime_pointer();
void mondriaan_runtime_in_number();
void mondriaan_runtime_multiply();
void mondriaan_runtime_divide();
void mondriaan_runtime_roll();
}
