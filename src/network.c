#include "aurora/network.h"

void aurora_network_init(aurora_network *network) {
    if (network == NULL) {
        return;
    }
    network->cell_count = 0u;
    network->gate_count = 0u;
    network->state = AURORA_OPEN;
}

int aurora_network_add_cell(aurora_network *network, aurora_domain domain) {
    if (network == NULL || !aurora_domain_is_valid(domain) ||
        network->cell_count >= AURORA_NETWORK_MAX_CELLS) {
        return -1;
    }
    size_t index = network->cell_count++;
    network->cells[index] = domain;
    return (int)index;
}

int aurora_network_add_gate(aurora_network *network,
                            size_t a,
                            size_t b,
                            size_t mode,
                            size_t result) {
    if (network == NULL || network->gate_count >= AURORA_NETWORK_MAX_GATES ||
        a >= network->cell_count || b >= network->cell_count ||
        mode >= network->cell_count || result >= network->cell_count) {
        return -1;
    }
    size_t index = network->gate_count++;
    network->gates[index] =
        (aurora_network_gate){a, b, mode, result, AURORA_OPEN};
    return (int)index;
}

aurora_state aurora_network_propagate(aurora_network *network) {
    if (network == NULL) {
        return AURORA_CONTRADICTION;
    }

    bool changed;
    do {
        changed = false;
        for (size_t index = 0u; index < network->gate_count; ++index) {
            aurora_network_gate *gate = &network->gates[index];
            aurora_domain before[4] = {network->cells[gate->a],
                                       network->cells[gate->b],
                                       network->cells[gate->mode],
                                       network->cells[gate->result]};
            gate->state = aurora_trigate_propagate(
                &network->cells[gate->a], &network->cells[gate->b],
                &network->cells[gate->mode], &network->cells[gate->result]);
            if (gate->state == AURORA_CONTRADICTION) {
                network->state = AURORA_CONTRADICTION;
                return network->state;
            }
            changed = changed || before[0] != network->cells[gate->a] ||
                      before[1] != network->cells[gate->b] ||
                      before[2] != network->cells[gate->mode] ||
                      before[3] != network->cells[gate->result];
        }
    } while (changed);

    network->state = AURORA_CLOSED;
    for (size_t index = 0u; index < network->gate_count; ++index) {
        aurora_network_gate *gate = &network->gates[index];
        gate->state = aurora_trigate_propagate(
            &network->cells[gate->a], &network->cells[gate->b],
            &network->cells[gate->mode], &network->cells[gate->result]);
        if (gate->state != AURORA_CLOSED) {
            network->state = AURORA_OPEN;
        }
    }
    return network->state;
}
