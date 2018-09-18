#include <stdint.h>
#include <stack>

std::stack<uint32_t>& mondriaan_dump_stack();

extern "C" {
    void mondriaan_runtime_push(uint32_t);
    void mondriaan_runtime_duplicate();
    void mondriaan_runtime_out_char();
}
