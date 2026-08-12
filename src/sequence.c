#include "aurora/sequence.h"

static bool same_level(const aurora_tensor *tensor,
                       const size_t *input,
                       size_t input_count) {
    if (tensor == NULL || (input == NULL && input_count != 0u)) {
        return false;
    }
    if (input_count == 0u) {
        return true;
    }
    const aurora_tensor_node *first = aurora_tensor_get(tensor, input[0]);
    if (first == NULL) {
        return false;
    }
    for (size_t index = 1u; index < input_count; ++index) {
        const aurora_tensor_node *node = aurora_tensor_get(tensor, input[index]);
        if (node == NULL || node->depth != first->depth) {
            return false;
        }
    }
    return true;
}

aurora_sequence_result aurora_sequence_run_level(
    aurora_tensor *tensor,
    const size_t *input,
    size_t input_count) {
    aurora_sequence_result result = {0};
    result.status = AURORA_SEQUENCE_INVALID;
    if (!same_level(tensor, input, input_count)) {
        return result;
    }

    size_t position = 0u;
    int carry = -1;
    while (true) {
        size_t needed = carry >= 0 ? 2u : 3u;
        if (input_count - position < needed) {
            break;
        }
        if (tensor->count >= AURORA_TENSOR_MAX_NODES) {
            result.status = AURORA_SEQUENCE_CAPACITY;
            result.input_consumed = position;
            if (carry >= 0) {
                result.pending[result.pending_count++] = (size_t)carry;
            }
            return result;
        }

        size_t first = carry >= 0 ? (size_t)carry : input[position];
        size_t second = input[position + (carry >= 0 ? 0u : 1u)];
        size_t third = input[position + (carry >= 0 ? 1u : 2u)];
        aurora_window_result window =
            aurora_window_process(tensor, first, second, third);
        ++result.windows;

        if (window.action == AURORA_WINDOW_EMERGE) {
            result.outputs[result.output_count++] = (size_t)window.output;
            ++result.emergences;
            position += needed;
            carry = -1;
        } else if (window.action == AURORA_WINDOW_CARRY) {
            ++result.carries;
            position += needed;
            carry = window.output;
        } else {
            ++result.slides;
            if (carry >= 0) {
                /* El carry era la hipótesis contradictoria: se descarta y se
                 * reintentan los elementos originales, aún no consumidos. */
                carry = -1;
            } else {
                ++position;
            }
        }
    }

    result.input_consumed = position;
    if (carry >= 0) {
        result.pending[result.pending_count++] = (size_t)carry;
    }
    while (position < input_count) {
        result.pending[result.pending_count++] = input[position++];
    }
    result.status = result.pending_count == 0u ? AURORA_SEQUENCE_COMPLETE
                                                : AURORA_SEQUENCE_PENDING;
    return result;
}

aurora_hierarchy_result aurora_sequence_run_hierarchy(
    aurora_tensor *tensor,
    const size_t *input,
    size_t input_count) {
    aurora_hierarchy_result hierarchy = {0};
    hierarchy.status = AURORA_SEQUENCE_INVALID;
    if (input_count > AURORA_TENSOR_MAX_NODES ||
        (input == NULL && input_count != 0u)) {
        return hierarchy;
    }

    size_t current[AURORA_TENSOR_MAX_NODES];
    for (size_t index = 0u; index < input_count; ++index) {
        current[index] = input[index];
    }
    size_t current_count = input_count;

    while (current_count >= 3u) {
        aurora_sequence_result level =
            aurora_sequence_run_level(tensor, current, current_count);
        ++hierarchy.levels;
        hierarchy.windows += level.windows;
        hierarchy.emergences += level.emergences;
        hierarchy.carries += level.carries;
        hierarchy.slides += level.slides;
        if (level.status == AURORA_SEQUENCE_INVALID ||
            level.status == AURORA_SEQUENCE_CAPACITY) {
            hierarchy.status = level.status;
            return hierarchy;
        }
        for (size_t index = 0u; index < level.pending_count; ++index) {
            hierarchy.pending[hierarchy.pending_count++] = level.pending[index];
        }
        current_count = level.output_count;
        for (size_t index = 0u; index < current_count; ++index) {
            current[index] = level.outputs[index];
        }
        if (current_count == 0u) {
            break;
        }
    }

    for (size_t index = 0u; index < current_count; ++index) {
        hierarchy.roots[hierarchy.root_count++] = current[index];
    }
    hierarchy.status = hierarchy.pending_count == 0u
                           ? AURORA_SEQUENCE_COMPLETE
                           : AURORA_SEQUENCE_PENDING;
    return hierarchy;
}
