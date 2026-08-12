#ifndef GENESIS_TENSOR117_H
#define GENESIS_TENSOR117_H

#include <trinity/trinity.h>

#define GENESIS_TENSOR117_CELLS 39u
#define GENESIS_TENSOR117_BITS 117u
#define GENESIS_TENSOR117_BYTES 15u

typedef struct {
    uint8_t bytes[GENESIS_TENSOR117_BYTES];
} genesis_tensor117;

void genesis_tensor117_clear(genesis_tensor117 *tensor);
aurora_domain genesis_tensor117_get(const genesis_tensor117 *tensor,
                                    size_t cell);
bool genesis_tensor117_set(genesis_tensor117 *tensor,
                           size_t cell,
                           aurora_domain domain);
bool genesis_tensor117_is_valid(const genesis_tensor117 *tensor);

/* Árbol canónico: nivel 1 [0..2], nivel 2 [3..11], nivel 3 [12..38]. */
bool genesis_tensor117_from_trinity(const aurora_tensor *arena,
                                    size_t root,
                                    genesis_tensor117 *output);
bool genesis_tensor117_to_trinity(const genesis_tensor117 *input,
                                  aurora_tensor *arena,
                                  size_t *root);

#endif
