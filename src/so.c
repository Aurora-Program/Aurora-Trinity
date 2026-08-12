#include "aurora/so.h"

aurora_so aurora_so_seed(const aurora_domain ds[3]) {
    aurora_so so = {0};
    if (ds == NULL || aurora_order_triplet(ds).state ==
                          AURORA_CONTRADICTION) {
        so.state = AURORA_CONTRADICTION;
        return so;
    }
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        so.upper_ds[dimension] = ds[dimension];
        so.ds[dimension] = ds[dimension];
        so.de[dimension] = AURORA_DOMAIN_ALL;
        so.do_[dimension] = AURORA_DOMAIN_ALL;
    }
    so.state = AURORA_OPEN;
    return so;
}

aurora_so aurora_so_from_operation(const aurora_operation *operation) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_so_from_operation_at_do(operation, origin);
}

aurora_so aurora_so_from_operation_at_do(
    const aurora_operation *operation,
    const aurora_domain do_state[3]) {
    aurora_so so = {0};
    so.state = AURORA_CONTRADICTION;
    if (operation == NULL || do_state == NULL ||
        operation->state == AURORA_CONTRADICTION) {
        return so;
    }
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        so.ds[dimension] = operation->ds[dimension];
        so.de[dimension] = operation->de[dimension];
        so.do_[dimension] = operation->do_[dimension];
    }
    aurora_operation summary = aurora_operate_minimum_at_do(
        so.ds, so.de, so.do_, do_state);
    if (summary.state == AURORA_CONTRADICTION) {
        return so;
    }
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        so.upper_ds[dimension] = summary.ds[dimension];
    }
    /* La síntesis superior indexa K, pero no revoca un DE=000 ya cerrado. */
    so.state = operation->state;
    return so;
}

aurora_so aurora_so_relate(const aurora_so *first,
                           const aurora_so *second,
                           const aurora_so *third) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_so_relate_at_do(first, second, third, origin);
}

aurora_so aurora_so_relate_at_do(const aurora_so *first,
                                 const aurora_so *second,
                                 const aurora_so *third,
                                 const aurora_domain do_state[3]) {
    aurora_so result = {0};
    result.state = AURORA_CONTRADICTION;
    if (first == NULL || second == NULL || third == NULL ||
        do_state == NULL ||
        first->state == AURORA_CONTRADICTION ||
        second->state == AURORA_CONTRADICTION ||
        third->state == AURORA_CONTRADICTION) {
        return result;
    }

    aurora_operation relation = aurora_operate_minimum_at_do(
        first->upper_ds, second->upper_ds, third->upper_ds, do_state);
    return aurora_so_from_operation_at_do(&relation, do_state);
}
