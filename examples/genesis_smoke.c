#include <genesis/random_origin.h>

#include <stdio.h>

int main(void) {
    genesis_tensor117 tensor;
    if (!genesis_gr_generate(42u, 1u, &tensor)) {
        return 1;
    }
    printf("Genesis G-R: %u bits, first byte 0x%02x\n",
           GENESIS_TENSOR117_BITS, tensor.bytes[0]);
    return 0;
}
