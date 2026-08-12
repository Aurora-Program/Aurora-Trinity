#include "aurora/bootstrap.h"

#include <string.h>

aurora_so aurora_so_empty(void) {
    aurora_so empty = {0};
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        empty.upper_ds[dimension] = AURORA_DOMAIN_ALL;
        empty.ds[dimension] = AURORA_DOMAIN_ALL;
        empty.de[dimension] = AURORA_DOMAIN_ALL;
        empty.do_[dimension] = AURORA_DOMAIN_ALL;
    }
    empty.state = AURORA_OPEN;
    return empty;
}

bool aurora_so_is_empty(const aurora_so *so) {
    if (so == NULL || so->state != AURORA_OPEN) {
        return false;
    }
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (so->upper_ds[dimension] != AURORA_DOMAIN_ALL ||
            so->ds[dimension] != AURORA_DOMAIN_ALL ||
            so->de[dimension] != AURORA_DOMAIN_ALL ||
            so->do_[dimension] != AURORA_DOMAIN_ALL) {
            return false;
        }
    }
    return true;
}

static bool same_so(const aurora_so *left, const aurora_so *right) {
    return left->state == right->state &&
           memcmp(left->upper_ds, right->upper_ds,
                  sizeof left->upper_ds) == 0 &&
           memcmp(left->ds, right->ds, sizeof left->ds) == 0 &&
           memcmp(left->de, right->de, sizeof left->de) == 0 &&
           memcmp(left->do_, right->do_, sizeof left->do_) == 0;
}

aurora_bootstrap_result aurora_bootstrap_knowledge(
    const aurora_so *input,
    const aurora_so *current_knowledge,
    const aurora_so *output,
    aurora_state window_state) {
    aurora_bootstrap_result result = {0};
    result.state = AURORA_CONTRADICTION;
    if (input == NULL || current_knowledge == NULL || output == NULL) {
        return result;
    }
    result.knowledge = *current_knowledge;
    result.state = window_state;
    if (aurora_so_is_empty(current_knowledge) &&
        window_state == AURORA_OPEN && same_so(input, output)) {
        result.knowledge = *input;
        result.written = true;
    }
    return result;
}
