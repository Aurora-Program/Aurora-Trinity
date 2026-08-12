#ifndef AURORA_CIRCULAR_H
#define AURORA_CIRCULAR_H

#include "aurora/trigate.h"

#define AURORA_CIRCULAR_MAX_VARIABLES 12u
#define AURORA_CIRCULAR_MAX_GATES 32u

typedef struct {
    size_t variables[4]; /* a, b, mode, result */
} aurora_circular_gate;

typedef struct {
    aurora_domain domains[AURORA_CIRCULAR_MAX_VARIABLES];
    size_t variable_count;
    aurora_circular_gate gates[AURORA_CIRCULAR_MAX_GATES];
    size_t gate_count;
} aurora_circular_network;

typedef struct {
    aurora_state state;
    size_t model_count;
    aurora_domain supported[AURORA_CIRCULAR_MAX_VARIABLES];
} aurora_circular_solution;

typedef struct {
    size_t networks;
    size_t global_contradictions;
    size_t global_unique;
    size_t global_open;
    size_t local_contradictions;
    size_t local_closed;
    size_t local_open;
    size_t unique_missed_locally;
    size_t contradictions_missed_locally;
    size_t values_pruned_globally;
    size_t unsound_local_pruning;
    size_t false_local_closure;
} aurora_circular_census;

void aurora_circular_init(aurora_circular_network *network);
int aurora_circular_add_variable(aurora_circular_network *network,
                                 aurora_domain domain);
bool aurora_circular_add_gate(aurora_circular_network *network,
                              size_t a, size_t b, size_t mode, size_t result);

/*
 * Resuelve directamente la conjunción simultánea de restricciones originales.
 * No agrega decisiones ni salidas previamente adoptadas por los TriGates:
 * 0 modelos = contradicción, 1 = unicidad, >1 = apertura.
 */
aurora_circular_solution aurora_circular_solve(
    const aurora_circular_network *network);

/* Censo exhaustivo de 3^6 entradas fijas por 7^3 dominios de resultado. */
bool aurora_circular_ring_census(aurora_circular_census *census);
bool aurora_circular_sample_census(uint64_t seed, size_t samples,
                                   aurora_circular_census *census);

#endif
