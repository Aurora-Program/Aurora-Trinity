#include "genesis/self_organization.h"

#include <string.h>

#define TOKENS 12u
#define GROUP_SIZE 4u
#define MAX_ROUNDS 20u

static size_t bits(aurora_domain domain) {
    size_t count = 0u;
    for (uint8_t bit = 0u; bit < 3u; ++bit)
        if ((domain & aurora_domain_singleton(bit)) != 0u) ++count;
    return count;
}

static size_t so_distance(const aurora_so *left, const aurora_so *right) {
    const aurora_domain *l[4] = {left->upper_ds, left->ds, left->de, left->do_};
    const aurora_domain *r[4] = {right->upper_ds, right->ds, right->de, right->do_};
    size_t distance = left->state == right->state ? 0u : 1u;
    for (size_t group = 0u; group < 4u; ++group)
        for (size_t dimension = 0u; dimension < 3u; ++dimension)
            distance += bits((aurora_domain)(l[group][dimension] ^
                                             r[group][dimension]));
    return distance;
}

static bool equal_so(const aurora_so *left, const aurora_so *right) {
    return so_distance(left, right) == 0u;
}

static void measure(const aurora_tensor *arena, const size_t states[TOKENS],
                    size_t *within, size_t *between,
                    size_t *within_pairs, size_t *between_pairs,
                    size_t *collisions) {
    *within = *between = *within_pairs = *between_pairs = *collisions = 0u;
    for (size_t left = 0u; left < TOKENS; ++left) {
        const aurora_so *left_so = &arena->nodes[states[left]].superior;
        for (size_t right = left + 1u; right < TOKENS; ++right) {
            const aurora_so *right_so = &arena->nodes[states[right]].superior;
            size_t distance = so_distance(left_so, right_so);
            if (left / GROUP_SIZE == right / GROUP_SIZE) {
                *within += distance;
                ++*within_pairs;
            } else {
                *between += distance;
                ++*between_pairs;
            }
            if (distance == 0u) ++*collisions;
        }
    }
}

bool genesis_self_organization_run(uint64_t seed, bool permute_identities,
                                   genesis_self_organization_metrics *metrics) {
    if (metrics == NULL) return false;
    memset(metrics, 0, sizeof *metrics);
    aurora_tensor arena;
    aurora_tensor_init(&arena);
    size_t states[TOKENS];
    for (size_t token = 0u; token < TOKENS; ++token) {
        size_t source = permute_identities ? (token * 5u + 3u) % TOKENS : token;
        genesis_tensor117 payload;
        if (!genesis_gr_generate(seed, source, &payload) ||
            !genesis_tensor117_to_trinity(&payload, &arena, &states[token]))
            return false;
    }
    size_t ignored_collisions;
    measure(&arena, states, &metrics->initial_within_distance,
            &metrics->initial_between_distance, &metrics->within_pairs,
            &metrics->between_pairs, &ignored_collisions);

    for (size_t round = 0u; round < MAX_ROUNDS; ++round) {
        size_t next[TOKENS];
        bool unchanged = true;
        for (size_t token = 0u; token < TOKENS; ++token) {
            size_t base = (token / GROUP_SIZE) * GROUP_SIZE;
            size_t offset = token % GROUP_SIZE;
            size_t second = base + (offset + 1u) % GROUP_SIZE;
            size_t third = base + (offset + 2u) % GROUP_SIZE;
            aurora_window_result result = aurora_window_process(
                &arena, states[token], states[second], states[third]);
            if (result.output < 0) return false;
            next[token] = (size_t)result.output;
            unchanged = unchanged && equal_so(
                &arena.nodes[states[token]].superior,
                &arena.nodes[next[token]].superior);
        }
        memcpy(states, next, sizeof states);
        metrics->rounds = round + 1u;
        if (unchanged) {
            metrics->stable = true;
            break;
        }
    }
    size_t final_within_pairs, final_between_pairs;
    measure(&arena, states, &metrics->final_within_distance,
            &metrics->final_between_distance, &final_within_pairs,
            &final_between_pairs, &metrics->final_collisions);
    if (final_within_pairs != metrics->within_pairs ||
        final_between_pairs != metrics->between_pairs) return false;
    metrics->tensor_nodes = arena.count;
    return true;
}
