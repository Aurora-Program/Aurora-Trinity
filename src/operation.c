#include "aurora/operation.h"

aurora_operation aurora_operate_minimum(const aurora_domain first[3],
                                        const aurora_domain second[3],
                                        const aurora_domain third[3]) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_operate_minimum_with_knowledge_at_do(
        first, second, third, NULL, origin);
}

aurora_operation aurora_operate_minimum_at_do(
    const aurora_domain first[3],
    const aurora_domain second[3],
    const aurora_domain third[3],
    const aurora_domain do_state[3]) {
    return aurora_operate_minimum_with_knowledge_at_do(
        first, second, third, NULL, do_state);
}

aurora_operation aurora_operate_minimum_with_knowledge(
    const aurora_domain first[3],
    const aurora_domain second[3],
    const aurora_domain third[3],
    const aurora_domain knowledge_ds[3]) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_operate_minimum_with_knowledge_at_do(
        first, second, third, knowledge_ds, origin);
}

aurora_operation aurora_operate_minimum_with_knowledge_at_do(
    const aurora_domain first[3],
    const aurora_domain second[3],
    const aurora_domain third[3],
    const aurora_domain knowledge_ds[3],
    const aurora_domain do_state[3]) {
    aurora_operation operation = {0};
    operation.state = AURORA_CONTRADICTION;
    if (first == NULL || second == NULL || third == NULL || do_state == NULL) {
        return operation;
    }

    const aurora_domain *triplets[3] = {first, second, third};
    for (size_t index = 0u; index < 3u; ++index) {
        operation.ordered[index] = aurora_order_triplet_at_phase(
            triplets[index], do_state[index]);
        if (operation.ordered[index].state == AURORA_CONTRADICTION) {
            return operation;
        }
    }

    aurora_network network;
    aurora_network_init(&network);
    int distilled_fn[3];
    int distilled_fo[3];
    int es_cells[3];
    int ds_cells[3];
    int emergence_gates[3];
    static const size_t left[3] = {0u, 1u, 2u};
    static const size_t right[3] = {1u, 2u, 0u};

    for (size_t pair = 0u; pair < 3u; ++pair) {
        int fo_a = aurora_network_add_cell(
            &network, operation.ordered[left[pair]].fo);
        int fo_b = aurora_network_add_cell(
            &network, operation.ordered[right[pair]].fo);
        int fn_a = aurora_network_add_cell(
            &network, operation.ordered[left[pair]].fn);
        int fn_b = aurora_network_add_cell(
            &network, operation.ordered[right[pair]].fn);
        int link = aurora_network_add_cell(&network, AURORA_DOMAIN_ALL);
        distilled_fo[pair] =
            aurora_network_add_cell(&network, AURORA_DOMAIN_ALL);
        distilled_fn[pair] =
            aurora_network_add_cell(&network, AURORA_DOMAIN_ALL);

        if (fo_a < 0 || fo_b < 0 || fn_a < 0 || fn_b < 0 || link < 0 ||
            distilled_fo[pair] < 0 || distilled_fn[pair] < 0 ||
            aurora_network_add_gate(&network, (size_t)fo_a, (size_t)fo_b,
                                    (size_t)link,
                                    (size_t)distilled_fo[pair]) < 0 ||
            aurora_network_add_gate(&network, (size_t)fn_a, (size_t)fn_b,
                                    (size_t)distilled_fn[pair],
                                    (size_t)link) < 0) {
            return operation;
        }
    }

    for (size_t index = 0u; index < 3u; ++index) {
        es_cells[index] = aurora_network_add_cell(
            &network, operation.ordered[index].es);
        if (es_cells[index] < 0) {
            return operation;
        }
    }
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        aurora_domain initial = knowledge_ds == NULL
                                    ? AURORA_DOMAIN_ALL
                                    : knowledge_ds[dimension];
        ds_cells[dimension] = aurora_network_add_cell(&network, initial);
        if (ds_cells[dimension] < 0) {
            return operation;
        }
    }
    emergence_gates[0] = aurora_network_add_gate(
        &network, (size_t)es_cells[0], (size_t)es_cells[1],
        (size_t)es_cells[2], (size_t)ds_cells[0]);
    emergence_gates[1] = aurora_network_add_gate(
        &network, (size_t)distilled_fn[0], (size_t)distilled_fn[1],
        (size_t)distilled_fn[2], (size_t)ds_cells[1]);
    emergence_gates[2] = aurora_network_add_gate(
        &network, (size_t)distilled_fo[0], (size_t)distilled_fo[1],
        (size_t)distilled_fo[2], (size_t)ds_cells[2]);
    if (emergence_gates[0] < 0 || emergence_gates[1] < 0 ||
        emergence_gates[2] < 0) {
        return operation;
    }

    if (aurora_network_propagate(&network) == AURORA_CONTRADICTION) {
        return operation;
    }

    for (size_t pair = 0u; pair < 3u; ++pair) {
        operation.pairs[pair].function =
            network.cells[distilled_fn[pair]];
        operation.pairs[pair].form = network.cells[distilled_fo[pair]];
    }

    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        operation.ds[dimension] = network.cells[ds_cells[dimension]];
        int group_cells[3];
        if (dimension == 0u) {
            group_cells[0] = es_cells[0];
            group_cells[1] = es_cells[1];
            group_cells[2] = es_cells[2];
        } else if (dimension == 1u) {
            group_cells[0] = distilled_fn[0];
            group_cells[1] = distilled_fn[1];
            group_cells[2] = distilled_fn[2];
        } else {
            group_cells[0] = distilled_fo[0];
            group_cells[1] = distilled_fo[1];
            group_cells[2] = distilled_fo[2];
        }
        aurora_domain group[3] = {
            network.cells[group_cells[0]], network.cells[group_cells[1]],
            network.cells[group_cells[2]]};
        operation.de[dimension] = aurora_trigate_residual(
            group[0], group[1], group[2],
            network.cells[ds_cells[dimension]], AURORA_INFER_R,
            network.gates[emergence_gates[dimension]].state);
        aurora_ordering emergence_order = aurora_order_triplet_at_phase(
            group, do_state[dimension]);
        if (emergence_order.state == AURORA_CONTRADICTION) {
            operation.state = AURORA_CONTRADICTION;
            return operation;
        }
        operation.do_[dimension] = emergence_order.orientation;
    }

    operation.state = AURORA_CLOSED;
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        /* Con R=2, E conserva el residual 0/1 y no clasifica por sí solo. */
        if (operation.de[dimension] == AURORA_DOMAIN_1 &&
            operation.ds[dimension] != AURORA_DOMAIN_2) {
            operation.state = AURORA_CONTRADICTION;
            return operation;
        }
        if (operation.ds[dimension] == AURORA_DOMAIN_2 ||
            operation.de[dimension] != AURORA_DOMAIN_0) {
            operation.state = AURORA_OPEN;
        }
    }
    return operation;
}
