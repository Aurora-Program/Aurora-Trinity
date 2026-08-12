#include "aurora/translation.h"

static int find_simple(const uint32_t *simple_tokens,
                       size_t simple_count,
                       uint32_t token) {
    for (size_t index = 0u; index < simple_count; ++index) {
        if (simple_tokens[index] == token) {
            return (int)index;
        }
    }
    return -1;
}

static aurora_translation lexicalize(
    aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    const uint32_t *tokens,
    size_t token_count,
    const uint32_t *simple_tokens,
    const size_t *simple_tensors,
    size_t simple_count,
    size_t max_length) {
    aurora_translation result = {0};
    result.status = AURORA_TRANSLATION_OK;
    result.output = -1;
    size_t position = 0u;
    while (position < token_count) {
        int entry = aurora_dictionary_lookup_limit(
            dictionary, tokens + position, token_count - position, max_length);
        if (entry >= 0) {
            const aurora_dictionary_entry *match = &dictionary->entries[entry];
            if (aurora_tensor_get(tensor, match->tensor) == NULL) {
                result.status = AURORA_TRANSLATION_INCOHERENT;
                return result;
            }
            result.tensors[result.count++] = match->tensor;
            ++result.dictionary_hits;
            if (match->token_count > result.max_length_used) {
                result.max_length_used = match->token_count;
            }
            position += match->token_count;
            continue;
        }
        int simple = find_simple(simple_tokens, simple_count, tokens[position]);
        if (simple < 0 || aurora_tensor_get(tensor, simple_tensors[simple]) == NULL) {
            result.status = AURORA_TRANSLATION_UNKNOWN;
            return result;
        }
        result.tensors[result.count++] = simple_tensors[simple];
        ++result.simple_hits;
        if (result.max_length_used == 0u) {
            result.max_length_used = 1u;
        }
        ++position;
    }
    return result;
}

static bool coherent_roots(const aurora_tensor *tensor,
                           const aurora_translation *translation) {
    for (size_t index = 0u; index < translation->count; ++index) {
        const aurora_tensor_node *node =
            aurora_tensor_get(tensor, translation->tensors[index]);
        if (node == NULL || aurora_order_triplet(node->superior.upper_ds).state ==
                                AURORA_CONTRADICTION) {
            return false;
        }
    }
    return true;
}

aurora_translation aurora_translate(
    aurora_dictionary *dictionary,
    aurora_tensor *tensor,
    const uint32_t *tokens,
    size_t token_count,
    const uint32_t *simple_tokens,
    const size_t *simple_tensors,
    size_t simple_count,
    bool validate) {
    aurora_translation failure = {0};
    failure.status = AURORA_TRANSLATION_UNKNOWN;
    failure.output = -1;
    if (dictionary == NULL || tensor == NULL || tokens == NULL ||
        simple_tokens == NULL || simple_tensors == NULL ||
        token_count > AURORA_TENSOR_MAX_NODES) {
        return failure;
    }

    size_t max_length = token_count > AURORA_DICTIONARY_MAX_TOKENS
                            ? AURORA_DICTIONARY_MAX_TOKENS
                            : token_count;
    size_t backoffs = 0u;
    while (max_length > 0u) {
        size_t tensor_snapshot = tensor->count;
        aurora_translation result = lexicalize(
            dictionary, tensor, tokens, token_count, simple_tokens,
            simple_tensors, simple_count, max_length);
        result.backoffs = backoffs;
        if (result.status != AURORA_TRANSLATION_OK || !validate) {
            return result;
        }

        bool coherent = coherent_roots(tensor, &result);
        aurora_hierarchy_result hierarchy = {0};
        if (coherent) {
            hierarchy = aurora_sequence_run_hierarchy(
                tensor, result.tensors, result.count);
            result.windows = hierarchy.windows;
            if (hierarchy.root_count + hierarchy.pending_count == 1u) {
                result.output = hierarchy.root_count == 1u
                                    ? (int)hierarchy.roots[0]
                                    : (int)hierarchy.pending[0];
            }
            coherent = hierarchy.status != AURORA_SEQUENCE_INVALID &&
                       hierarchy.status != AURORA_SEQUENCE_CAPACITY &&
                       hierarchy.slides == 0u;
        }
        if (coherent) {
            return result;
        }
        tensor->count = tensor_snapshot;
        if (result.max_length_used <= 1u) {
            result.status = AURORA_TRANSLATION_INCOHERENT;
            return result;
        }
        max_length = result.max_length_used - 1u;
        ++backoffs;
    }
    failure.status = AURORA_TRANSLATION_INCOHERENT;
    return failure;
}
