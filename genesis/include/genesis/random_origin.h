#ifndef GENESIS_RANDOM_ORIGIN_H
#define GENESIS_RANDOM_ORIGIN_H

#include "genesis/tensor117.h"

/* G-R: la misma seed pública y token producen siempre el mismo tensor. */
bool genesis_gr_generate(uint64_t public_seed,
                         uint64_t token_id,
                         genesis_tensor117 *output);

#endif
