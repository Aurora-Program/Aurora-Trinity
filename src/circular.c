#include "aurora/circular.h"
#include "aurora/network.h"

#include <string.h>

void aurora_circular_init(aurora_circular_network *network) {
    if (network != NULL) memset(network, 0, sizeof *network);
}

int aurora_circular_add_variable(aurora_circular_network *network,
                                 aurora_domain domain) {
    if (network == NULL || !aurora_domain_is_valid(domain) ||
        network->variable_count >= AURORA_CIRCULAR_MAX_VARIABLES) return -1;
    size_t index = network->variable_count++;
    network->domains[index] = domain;
    return (int)index;
}

bool aurora_circular_add_gate(aurora_circular_network *network,
                              size_t a, size_t b, size_t mode, size_t result) {
    if (network == NULL || network->gate_count >= AURORA_CIRCULAR_MAX_GATES ||
        a >= network->variable_count || b >= network->variable_count ||
        mode >= network->variable_count || result >= network->variable_count)
        return false;
    aurora_circular_gate *gate = &network->gates[network->gate_count++];
    gate->variables[0] = a;
    gate->variables[1] = b;
    gate->variables[2] = mode;
    gate->variables[3] = result;
    return true;
}

static bool model_is_valid(const aurora_circular_network *network,
                           const uint8_t *values) {
    for (size_t index = 0u; index < network->gate_count; ++index) {
        const aurora_circular_gate *gate = &network->gates[index];
        if (aurora_majority3(values[gate->variables[0]],
                             values[gate->variables[1]],
                             values[gate->variables[2]]) !=
            values[gate->variables[3]]) return false;
    }
    return true;
}

static void enumerate(const aurora_circular_network *network,
                      size_t variable, uint8_t *values,
                      aurora_circular_solution *solution) {
    if (variable == network->variable_count) {
        if (!model_is_valid(network, values)) return;
        ++solution->model_count;
        for (size_t index = 0u; index < network->variable_count; ++index)
            solution->supported[index] |=
                aurora_domain_singleton(values[index]);
        return;
    }
    for (uint8_t value = 0u; value < 3u; ++value) {
        if (aurora_domain_contains(network->domains[variable], value)) {
            values[variable] = value;
            enumerate(network, variable + 1u, values, solution);
        }
    }
}

aurora_circular_solution aurora_circular_solve(
    const aurora_circular_network *network) {
    aurora_circular_solution solution;
    memset(&solution, 0, sizeof solution);
    solution.state = AURORA_CONTRADICTION;
    if (network == NULL || network->variable_count == 0u ||
        network->variable_count > AURORA_CIRCULAR_MAX_VARIABLES) return solution;
    for (size_t index = 0u; index < network->variable_count; ++index)
        if (!aurora_domain_is_valid(network->domains[index])) return solution;
    uint8_t values[AURORA_CIRCULAR_MAX_VARIABLES] = {0};
    enumerate(network, 0u, values, &solution);
    solution.state = solution.model_count == 0u ? AURORA_CONTRADICTION
                   : solution.model_count == 1u ? AURORA_CLOSED
                                                : AURORA_OPEN;
    return solution;
}

static bool build_ring(const uint8_t fixed[6], const aurora_domain outputs[3],
                       aurora_circular_network *exact,
                       aurora_network *local) {
    aurora_circular_init(exact);
    aurora_network_init(local);
    for (size_t index = 0u; index < 6u; ++index) {
        aurora_domain domain = aurora_domain_singleton(fixed[index]);
        if (aurora_circular_add_variable(exact, domain) < 0 ||
            aurora_network_add_cell(local, domain) < 0) return false;
    }
    for (size_t index = 0u; index < 3u; ++index) {
        if (aurora_circular_add_variable(exact, outputs[index]) < 0 ||
            aurora_network_add_cell(local, outputs[index]) < 0) return false;
    }
    const size_t modes[3] = {8u, 6u, 7u};
    for (size_t gate = 0u; gate < 3u; ++gate) {
        size_t a = 2u * gate, b = a + 1u, result = 6u + gate;
        if (!aurora_circular_add_gate(exact, a, b, modes[gate], result) ||
            aurora_network_add_gate(local, a, b, modes[gate], result) < 0)
            return false;
    }
    return true;
}

bool aurora_circular_ring_census(aurora_circular_census *census) {
    if (census == NULL) return false;
    memset(census, 0, sizeof *census);
    for (size_t encoded = 0u; encoded < 729u * 343u; ++encoded) {
        size_t value = encoded;
        uint8_t fixed[6];
        for (size_t index = 0u; index < 6u; ++index) {
            fixed[index] = (uint8_t)(value % 3u);
            value /= 3u;
        }
        aurora_domain outputs[3];
        for (size_t index = 0u; index < 3u; ++index) {
            outputs[index] = (aurora_domain)(1u + value % 7u);
            value /= 7u;
        }
        aurora_circular_network exact_network;
        aurora_network local_network;
        if (!build_ring(fixed, outputs, &exact_network, &local_network))
            return false;
        aurora_circular_solution exact =
            aurora_circular_solve(&exact_network);
        aurora_state local = aurora_network_propagate(&local_network);
        ++census->networks;
        if (exact.state == AURORA_CONTRADICTION)
            ++census->global_contradictions;
        else if (exact.state == AURORA_CLOSED)
            ++census->global_unique;
        else
            ++census->global_open;
        if (local == AURORA_CONTRADICTION)
            ++census->local_contradictions;
        else if (local == AURORA_CLOSED)
            ++census->local_closed;
        else
            ++census->local_open;
        if (exact.state == AURORA_CLOSED && local != AURORA_CLOSED)
            ++census->unique_missed_locally;
        if (exact.state == AURORA_CONTRADICTION &&
            local != AURORA_CONTRADICTION)
            ++census->contradictions_missed_locally;
        if (exact.state != AURORA_CONTRADICTION &&
            local != AURORA_CONTRADICTION) {
            for (size_t index = 6u; index < 9u; ++index) {
                aurora_domain local_domain = local_network.cells[index];
                aurora_domain exact_domain = exact.supported[index];
                if ((exact_domain & local_domain) != exact_domain)
                    ++census->unsound_local_pruning;
                size_t local_size = aurora_domain_size(local_domain);
                size_t exact_size = aurora_domain_size(exact_domain);
                if (local_size > exact_size)
                    census->values_pruned_globally += local_size - exact_size;
            }
        }
    }
    return true;
}

static uint64_t next_random(uint64_t *state) {
    uint64_t value = *state;
    value ^= value >> 12;
    value ^= value << 25;
    value ^= value >> 27;
    *state = value;
    return value * UINT64_C(2685821657736338717);
}

bool aurora_circular_sample_census(uint64_t seed, size_t samples,
                                   aurora_circular_census *census) {
    if (census == NULL || samples == 0u) return false;
    memset(census, 0, sizeof *census);
    uint64_t random = seed == 0u ? UINT64_C(1) : seed;
    for (size_t sample = 0u; sample < samples; ++sample) {
        aurora_circular_network exact_network;
        aurora_network local_network;
        aurora_circular_init(&exact_network);
        aurora_network_init(&local_network);
        for (size_t variable = 0u; variable < 6u; ++variable) {
            aurora_domain domain =
                (aurora_domain)(1u + next_random(&random) % 7u);
            if (aurora_circular_add_variable(&exact_network, domain) < 0 ||
                aurora_network_add_cell(&local_network, domain) < 0)
                return false;
        }
        for (size_t gate = 0u; gate < 4u; ++gate) {
            size_t permutation[6] = {0u, 1u, 2u, 3u, 4u, 5u};
            for (size_t right = 5u; right > 0u; --right) {
                size_t selected = next_random(&random) % (right + 1u);
                size_t temporary = permutation[right];
                permutation[right] = permutation[selected];
                permutation[selected] = temporary;
            }
            if (!aurora_circular_add_gate(
                    &exact_network, permutation[0], permutation[1],
                    permutation[2], permutation[3]) ||
                aurora_network_add_gate(
                    &local_network, permutation[0], permutation[1],
                    permutation[2], permutation[3]) < 0) return false;
        }
        aurora_circular_solution exact =
            aurora_circular_solve(&exact_network);
        aurora_state local = aurora_network_propagate(&local_network);
        ++census->networks;
        if (exact.state == AURORA_CONTRADICTION)
            ++census->global_contradictions;
        else if (exact.state == AURORA_CLOSED)
            ++census->global_unique;
        else ++census->global_open;
        if (local == AURORA_CONTRADICTION)
            ++census->local_contradictions;
        else if (local == AURORA_CLOSED)
            ++census->local_closed;
        else ++census->local_open;
        if (exact.state == AURORA_CLOSED && local != AURORA_CLOSED)
            ++census->unique_missed_locally;
        if (exact.state == AURORA_CONTRADICTION &&
            local != AURORA_CONTRADICTION)
            ++census->contradictions_missed_locally;
        if (local == AURORA_CLOSED && exact.state != AURORA_CLOSED)
            ++census->false_local_closure;
        if (exact.state != AURORA_CONTRADICTION &&
            local != AURORA_CONTRADICTION) {
            for (size_t index = 0u; index < 6u; ++index) {
                aurora_domain local_domain = local_network.cells[index];
                aurora_domain exact_domain = exact.supported[index];
                if ((exact_domain & local_domain) != exact_domain)
                    ++census->unsound_local_pruning;
                size_t local_size = aurora_domain_size(local_domain);
                size_t exact_size = aurora_domain_size(exact_domain);
                if (local_size > exact_size)
                    census->values_pruned_globally += local_size - exact_size;
            }
        } else if (exact.state != AURORA_CONTRADICTION) {
            ++census->unsound_local_pruning;
        }
    }
    return true;
}
