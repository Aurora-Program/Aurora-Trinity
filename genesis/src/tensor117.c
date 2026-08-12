#include "genesis/tensor117.h"

#include <string.h>

void genesis_tensor117_clear(genesis_tensor117 *tensor) {
    if (tensor != NULL) {
        memset(tensor, 0, sizeof *tensor);
    }
}

aurora_domain genesis_tensor117_get(const genesis_tensor117 *tensor,
                                    size_t cell) {
    if (tensor == NULL || cell >= GENESIS_TENSOR117_CELLS) {
        return AURORA_DOMAIN_EMPTY;
    }
    size_t bit = cell * 3u;
    size_t byte = bit / 8u;
    unsigned shift = (unsigned)(bit % 8u);
    uint16_t pair = tensor->bytes[byte];
    if (byte + 1u < GENESIS_TENSOR117_BYTES) {
        pair |= (uint16_t)tensor->bytes[byte + 1u] << 8u;
    }
    return (aurora_domain)((pair >> shift) & 0x7u);
}

bool genesis_tensor117_set(genesis_tensor117 *tensor,
                           size_t cell,
                           aurora_domain domain) {
    if (tensor == NULL || cell >= GENESIS_TENSOR117_CELLS ||
        !aurora_domain_is_valid(domain)) {
        return false;
    }
    size_t bit = cell * 3u;
    size_t byte = bit / 8u;
    unsigned shift = (unsigned)(bit % 8u);
    uint16_t pair = tensor->bytes[byte];
    if (byte + 1u < GENESIS_TENSOR117_BYTES) {
        pair |= (uint16_t)tensor->bytes[byte + 1u] << 8u;
    }
    pair = (uint16_t)((pair & (uint16_t)~(0x7u << shift)) |
                      ((uint16_t)domain << shift));
    tensor->bytes[byte] = (uint8_t)(pair & 0xffu);
    if (byte + 1u < GENESIS_TENSOR117_BYTES) {
        tensor->bytes[byte + 1u] = (uint8_t)(pair >> 8u);
    }
    return true;
}

bool genesis_tensor117_is_valid(const genesis_tensor117 *tensor) {
    if (tensor == NULL || (tensor->bytes[14] & 0xe0u) != 0u) {
        return false;
    }
    for (size_t cell = 0u; cell < GENESIS_TENSOR117_CELLS; ++cell) {
        if (!aurora_domain_is_valid(genesis_tensor117_get(tensor, cell))) {
            return false;
        }
    }
    return true;
}

static bool write_triplet(genesis_tensor117 *output,
                          size_t offset,
                          const aurora_domain triplet[3]) {
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (!genesis_tensor117_set(output, offset + dimension,
                                   triplet[dimension])) {
            return false;
        }
    }
    return true;
}

bool genesis_tensor117_from_trinity(const aurora_tensor *arena,
                                    size_t root,
                                    genesis_tensor117 *output) {
    const aurora_tensor_node *top = aurora_tensor_get(arena, root);
    if (top == NULL || top->is_leaf || top->depth != 2u || output == NULL) {
        return false;
    }
    genesis_tensor117_clear(output);
    if (!write_triplet(output, 0u, top->superior.upper_ds)) {
        return false;
    }
    for (size_t branch = 0u; branch < 3u; ++branch) {
        const aurora_tensor_node *middle =
            aurora_tensor_get(arena, top->children[branch]);
        if (middle == NULL || middle->is_leaf || middle->depth != 1u ||
            !write_triplet(output, 3u + branch * 3u,
                           middle->superior.upper_ds)) {
            return false;
        }
        for (size_t leaf = 0u; leaf < 3u; ++leaf) {
            const aurora_tensor_node *bottom =
                aurora_tensor_get(arena, middle->children[leaf]);
            size_t leaf_number = branch * 3u + leaf;
            if (bottom == NULL || !bottom->is_leaf || bottom->depth != 0u ||
                !write_triplet(output, 12u + leaf_number * 3u,
                               bottom->superior.upper_ds)) {
                return false;
            }
        }
    }
    return genesis_tensor117_is_valid(output);
}

static bool equal_triplet(const aurora_domain left[3],
                          const genesis_tensor117 *right,
                          size_t offset) {
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (left[dimension] != genesis_tensor117_get(right,
                                                     offset + dimension)) {
            return false;
        }
    }
    return true;
}

bool genesis_tensor117_to_trinity(const genesis_tensor117 *input,
                                  aurora_tensor *arena,
                                  size_t *root) {
    if (!genesis_tensor117_is_valid(input) || arena == NULL || root == NULL) {
        return false;
    }
    size_t snapshot = arena->count;
    size_t leaves[9];
    for (size_t leaf = 0u; leaf < 9u; ++leaf) {
        aurora_domain triplet[3];
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            triplet[dimension] = genesis_tensor117_get(
                input, 12u + leaf * 3u + dimension);
        }
        int index = aurora_tensor_add_leaf(arena, triplet);
        if (index < 0) {
            arena->count = snapshot;
            return false;
        }
        leaves[leaf] = (size_t)index;
    }
    size_t middle[3];
    for (size_t branch = 0u; branch < 3u; ++branch) {
        int index = aurora_tensor_compose(
            arena, leaves[branch * 3u], leaves[branch * 3u + 1u],
            leaves[branch * 3u + 2u]);
        if (index < 0 ||
            !equal_triplet(arena->nodes[index].superior.upper_ds, input,
                           3u + branch * 3u)) {
            arena->count = snapshot;
            return false;
        }
        middle[branch] = (size_t)index;
    }
    int top = aurora_tensor_compose(arena, middle[0], middle[1], middle[2]);
    if (top < 0 || !equal_triplet(arena->nodes[top].superior.upper_ds, input,
                                  0u)) {
        arena->count = snapshot;
        return false;
    }
    *root = (size_t)top;
    return true;
}
