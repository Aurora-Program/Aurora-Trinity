#ifndef GENESIS_BENCHMARK_H
#define GENESIS_BENCHMARK_H

#include "genesis/random_origin.h"

typedef struct {
    size_t training_phrases;
    size_t heldout_phrases;
    size_t raw_windows;
    size_t repeated_windows;
    size_t heldout_windows;
    size_t closed;
    size_t open;
    size_t contradictions;
    size_t heldout_root_collisions;
    size_t heldout_so_collisions;
    size_t dictionary_entries;
} genesis_gr_metrics;

/* Ejecuta el mismo currículo con asignación normal o token↔tensor barajada. */
bool genesis_gr_context_benchmark(uint64_t public_seed,
                                  bool shuffled_control,
                                  genesis_gr_metrics *metrics);

#endif
