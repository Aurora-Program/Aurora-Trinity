#include "genesis/random_origin.h"

static uint64_t splitmix64(uint64_t *state) {
    uint64_t value = (*state += UINT64_C(0x9e3779b97f4a7c15));
    value = (value ^ (value >> 30u)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27u)) * UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31u);
}

bool genesis_gr_generate(uint64_t public_seed,
                         uint64_t token_id,
                         genesis_tensor117 *output) {
    if (output == NULL) {
        return false;
    }
    for (uint64_t attempt = 0u; attempt < 4096u; ++attempt) {
        uint64_t state = public_seed ^
                         (token_id * UINT64_C(0xd6e8feb86659fd93)) ^ attempt;
        aurora_tensor arena;
        aurora_tensor_init(&arena);
        size_t leaves[9];
        bool valid = true;
        for (size_t leaf = 0u; leaf < 9u && valid; ++leaf) {
            aurora_domain triplet[3];
            do {
                for (size_t dimension = 0u; dimension < 3u; ++dimension) {
                    triplet[dimension] = aurora_domain_singleton(
                        (uint8_t)(splitmix64(&state) % 3u));
                }
            } while (aurora_order_triplet(triplet).state ==
                     AURORA_CONTRADICTION);
            int index = aurora_tensor_add_leaf(&arena, triplet);
            valid = index >= 0;
            leaves[leaf] = valid ? (size_t)index : 0u;
        }
        size_t middle[3];
        for (size_t branch = 0u; branch < 3u && valid; ++branch) {
            int index = aurora_tensor_compose(
                &arena, leaves[branch * 3u], leaves[branch * 3u + 1u],
                leaves[branch * 3u + 2u]);
            valid = index >= 0;
            middle[branch] = valid ? (size_t)index : 0u;
        }
        int root = valid ? aurora_tensor_compose(
                               &arena, middle[0], middle[1], middle[2])
                         : -1;
        if (root >= 0 &&
            aurora_order_triplet(arena.nodes[root].superior.upper_ds).state !=
                AURORA_CONTRADICTION &&
            genesis_tensor117_from_trinity(
                             &arena, (size_t)root, output)) {
            return true;
        }
    }
    return false;
}
