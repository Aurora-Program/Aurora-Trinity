#include "genesis/benchmark.h"

#include <string.h>

#define VOCABULARY 12u
#define TRAINING 6u
#define HELDOUT 6u

static const uint32_t TRAINING_PHRASES[TRAINING][3] = {
    {0u, 1u, 2u}, {1u, 2u, 3u},
    {4u, 5u, 6u}, {5u, 6u, 7u},
    {8u, 9u, 10u}, {9u, 10u, 11u}
};

static const uint32_t HELDOUT_PHRASES[HELDOUT][3] = {
    {0u, 2u, 3u}, {0u, 1u, 3u},
    {4u, 6u, 7u}, {4u, 5u, 7u},
    {8u, 10u, 11u}, {8u, 9u, 11u}
};

static bool initialize_vocabulary(uint64_t seed,
                                  bool shuffled,
                                  aurora_tensor *arena,
                                  uint32_t token_ids[VOCABULARY],
                                  size_t roots[VOCABULARY]) {
    aurora_tensor_init(arena);
    for (size_t token = 0u; token < VOCABULARY; ++token) {
        size_t source = shuffled ? (token * 5u + 3u) % VOCABULARY : token;
        genesis_tensor117 payload;
        token_ids[token] = (uint32_t)token;
        if (!genesis_gr_generate(seed, source, &payload) ||
            !genesis_tensor117_to_trinity(&payload, arena, &roots[token])) {
            return false;
        }
    }
    return true;
}

static void count_state(const aurora_tensor *arena,
                        int output,
                        genesis_gr_metrics *metrics) {
    const aurora_tensor_node *node =
        output < 0 ? NULL : aurora_tensor_get(arena, (size_t)output);
    aurora_state state = node == NULL
                             ? AURORA_CONTRADICTION
                             : aurora_window_state_from_de(node->superior.de);
    if (state == AURORA_CLOSED) {
        ++metrics->closed;
    } else if (state == AURORA_OPEN) {
        ++metrics->open;
    } else {
        ++metrics->contradictions;
    }
}

static bool equal_so(const aurora_so *left, const aurora_so *right) {
    return left->state == right->state &&
           memcmp(left->upper_ds, right->upper_ds, sizeof left->upper_ds) == 0 &&
           memcmp(left->ds, right->ds, sizeof left->ds) == 0 &&
           memcmp(left->de, right->de, sizeof left->de) == 0 &&
           memcmp(left->do_, right->do_, sizeof left->do_) == 0;
}

bool genesis_gr_context_benchmark(uint64_t public_seed,
                                  bool shuffled_control,
                                  genesis_gr_metrics *metrics) {
    if (metrics == NULL) {
        return false;
    }
    memset(metrics, 0, sizeof *metrics);
    metrics->training_phrases = TRAINING;
    metrics->heldout_phrases = HELDOUT;
    aurora_tensor arena;
    aurora_dictionary dictionary;
    uint32_t token_ids[VOCABULARY];
    size_t roots[VOCABULARY];
    aurora_dictionary_init(&dictionary);
    if (!initialize_vocabulary(public_seed, shuffled_control, &arena,
                               token_ids, roots)) {
        return false;
    }

    for (size_t phrase = 0u; phrase < TRAINING; ++phrase) {
        aurora_translation translation = aurora_translate(
            &dictionary, &arena, TRAINING_PHRASES[phrase], 3u,
            token_ids, roots, VOCABULARY, true);
        if (translation.status != AURORA_TRANSLATION_OK ||
            translation.output < 0) {
            return false;
        }
        metrics->raw_windows += translation.windows;
        count_state(&arena, translation.output, metrics);
        int entry = aurora_dictionary_propose(
            &dictionary, &arena, TRAINING_PHRASES[phrase], 3u,
            (size_t)translation.output);
        if (entry < 0 || !aurora_dictionary_used_in_output(
                             &dictionary, &arena, (size_t)entry,
                             (size_t)translation.output,
                             AURORA_LEARN_CRYSTALLIZE)) {
            return false;
        }
    }

    for (size_t phrase = 0u; phrase < TRAINING; ++phrase) {
        aurora_translation translation = aurora_translate(
            &dictionary, &arena, TRAINING_PHRASES[phrase], 3u,
            token_ids, roots, VOCABULARY, true);
        if (translation.status != AURORA_TRANSLATION_OK) {
            return false;
        }
        metrics->repeated_windows += translation.windows;
    }

    aurora_domain heldout_roots[HELDOUT][3];
    aurora_so heldout_sos[HELDOUT];
    size_t heldout_count = 0u;
    for (size_t phrase = 0u; phrase < HELDOUT; ++phrase) {
        aurora_translation translation = aurora_translate(
            &dictionary, &arena, HELDOUT_PHRASES[phrase], 3u,
            token_ids, roots, VOCABULARY, true);
        if (translation.status != AURORA_TRANSLATION_OK ||
            translation.output < 0) {
            return false;
        }
        metrics->heldout_windows += translation.windows;
        const aurora_tensor_node *node =
            aurora_tensor_get(&arena, (size_t)translation.output);
        count_state(&arena, translation.output, metrics);
        for (size_t previous = 0u; previous < heldout_count; ++previous) {
            if (memcmp(heldout_roots[previous], node->superior.upper_ds,
                       sizeof heldout_roots[previous]) == 0) {
                ++metrics->heldout_root_collisions;
                break;
            }
        }
        for (size_t previous = 0u; previous < heldout_count; ++previous) {
            if (equal_so(&heldout_sos[previous], &node->superior)) {
                ++metrics->heldout_so_collisions;
                break;
            }
        }
        memcpy(heldout_roots[heldout_count++], node->superior.upper_ds,
               sizeof node->superior.upper_ds);
        heldout_sos[heldout_count - 1u] = node->superior;
    }
    metrics->dictionary_entries = dictionary.count;
    return true;
}
