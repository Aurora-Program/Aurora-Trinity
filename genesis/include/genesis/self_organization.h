#ifndef GENESIS_SELF_ORGANIZATION_H
#define GENESIS_SELF_ORGANIZATION_H

#include "genesis/random_origin.h"

typedef struct {
    size_t rounds;
    bool stable;
    size_t initial_within_distance;
    size_t initial_between_distance;
    size_t final_within_distance;
    size_t final_between_distance;
    size_t within_pairs;
    size_t between_pairs;
    size_t final_collisions;
    size_t tensor_nodes;
} genesis_self_organization_metrics;

/* Formación puramente numérica; los grupos sólo se consultan al medir. */
bool genesis_self_organization_run(uint64_t seed, bool permute_identities,
                                   genesis_self_organization_metrics *metrics);

#endif
