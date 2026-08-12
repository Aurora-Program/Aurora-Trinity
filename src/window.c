#include "aurora/window.h"

static const aurora_domain ORIGIN[3] = {
    AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};

static const aurora_domain *inherited_u_orientation(
    const aurora_tensor *tensor,
    size_t first) {
    const aurora_tensor_node *node = aurora_tensor_get(tensor, first);
    return node != NULL && node->has_window_orientation
               ? node->window_orientation
               : ORIGIN;
}

static void set_u_orientation(aurora_window_result *result,
                              const aurora_domain orientation[3]) {
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        result->u_orientation[dimension] = orientation[dimension];
    }
}

static bool valid_u_orientation(const aurora_domain orientation[3]) {
    if (orientation == NULL) return false;
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (!aurora_domain_is_valid(orientation[dimension])) return false;
    }
    return true;
}

aurora_state aurora_window_state_from_de(const aurora_domain de[3]) {
    if (de == NULL) {
        return AURORA_CONTRADICTION;
    }
    bool all_closed = true;
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (!aurora_domain_is_valid(de[dimension]) ||
            de[dimension] == AURORA_DOMAIN_1) {
            return AURORA_CONTRADICTION;
        }
        if (de[dimension] != AURORA_DOMAIN_0) {
            all_closed = false;
        }
    }
    return all_closed ? AURORA_CLOSED : AURORA_OPEN;
}

aurora_window_result aurora_window_process(aurora_tensor *tensor,
                                           size_t first,
                                           size_t second,
                                           size_t third) {
    return aurora_window_process_at_do(
        tensor, first, second, third,
        inherited_u_orientation(tensor, first));
}

aurora_window_result aurora_window_evaluate(const aurora_tensor *tensor,
                                            size_t first,
                                            size_t second,
                                            size_t third) {
    return aurora_window_evaluate_at_do(
        tensor, first, second, third,
        inherited_u_orientation(tensor, first));
}

aurora_window_result aurora_window_evaluate_at_do(
    const aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t third,
    const aurora_domain do_state[3]) {
    aurora_window_result result = {0};
    result.state = AURORA_CONTRADICTION;
    result.action = AURORA_WINDOW_SLIDE;
    result.output = -1;
    set_u_orientation(&result, ORIGIN);
    if (!valid_u_orientation(do_state)) return result;
    set_u_orientation(&result, do_state);
    const aurora_tensor_node *a = aurora_tensor_get(tensor, first);
    const aurora_tensor_node *b = aurora_tensor_get(tensor, second);
    const aurora_tensor_node *c = aurora_tensor_get(tensor, third);
    if (a == NULL || b == NULL || c == NULL ||
        a->depth != b->depth || a->depth != c->depth) {
        return result;
    }

    result.relation = aurora_so_relate_at_do(
        &a->superior, &b->superior, &c->superior, do_state);
    if (result.relation.state == AURORA_CONTRADICTION) {
        return result;
    }
    /* relation.state ya interpreta conjuntamente DS/R y DE; DE aislado no
     * basta cuando R=2 transporta un residual 0/1. */
    result.state = result.relation.state;
    if (result.state == AURORA_CONTRADICTION) {
        return result;
    }

    result.action = result.state == AURORA_CLOSED ? AURORA_WINDOW_EMERGE
                                                   : AURORA_WINDOW_CARRY;
    return result;
}

aurora_window_result aurora_window_process_at_do(
    aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t third,
    const aurora_domain do_state[3]) {
    aurora_window_result result = aurora_window_evaluate_at_do(
        tensor, first, second, third, do_state);
    if (result.state == AURORA_CONTRADICTION || tensor == NULL) {
        return result;
    }
    result.output = result.action == AURORA_WINDOW_EMERGE
                        ? aurora_tensor_compose_at_do(
                              tensor, first, second, third, do_state)
                        : aurora_tensor_carry_at_do(
                              tensor, first, second, third, do_state);
    if (result.output < 0) {
        result.state = AURORA_CONTRADICTION;
        result.action = AURORA_WINDOW_SLIDE;
    }
    return result;
}

aurora_window_result aurora_window_evaluate_knowledge_at_do(
    const aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t knowledge,
    const aurora_domain do_state[3]) {
    aurora_window_result result = {0};
    result.state = AURORA_CONTRADICTION;
    result.action = AURORA_WINDOW_SLIDE;
    result.output = -1;
    set_u_orientation(&result, ORIGIN);
    if (!valid_u_orientation(do_state)) return result;
    set_u_orientation(&result, do_state);
    const aurora_tensor_node *a = aurora_tensor_get(tensor, first);
    const aurora_tensor_node *b = aurora_tensor_get(tensor, second);
    const aurora_tensor_node *k = aurora_tensor_get(tensor, knowledge);
    if (a == NULL || b == NULL || k == NULL ||
        a->depth != b->depth || a->depth != k->depth) {
        return result;
    }

    const aurora_domain unknown[3] = {
        AURORA_DOMAIN_ALL, AURORA_DOMAIN_ALL, AURORA_DOMAIN_ALL};
    aurora_operation operation =
        aurora_operate_minimum_with_knowledge_at_do(
            a->superior.upper_ds, b->superior.upper_ds, unknown,
            k->superior.upper_ds, do_state);
    if (operation.state == AURORA_CONTRADICTION) return result;
    result.relation = aurora_so_from_operation_at_do(&operation, do_state);
    result.state = result.relation.state;
    if (result.state == AURORA_CONTRADICTION) return result;
    result.action = result.state == AURORA_CLOSED ? AURORA_WINDOW_EMERGE
                                                   : AURORA_WINDOW_CARRY;
    return result;
}

aurora_window_result aurora_window_process_knowledge_at_do(
    aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t knowledge,
    const aurora_domain do_state[3]) {
    aurora_window_result result = aurora_window_evaluate_knowledge_at_do(
        tensor, first, second, knowledge, do_state);
    if (result.state != AURORA_CLOSED || tensor == NULL) return result;
    result.output = aurora_tensor_materialize(
        tensor, first, second, knowledge, &result.relation,
        result.u_orientation, true);
    if (result.output < 0) {
        result.state = AURORA_CONTRADICTION;
        result.action = AURORA_WINDOW_SLIDE;
    }
    return result;
}
