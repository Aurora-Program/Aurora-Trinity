#ifndef AURORA_NETWORK_H
#define AURORA_NETWORK_H

#include "aurora/trigate.h"

#define AURORA_NETWORK_MAX_CELLS 64u
#define AURORA_NETWORK_MAX_GATES 64u

typedef struct {
    size_t a;
    size_t b;
    size_t mode;
    size_t result;
    aurora_state state;
} aurora_network_gate;

typedef struct {
    aurora_domain cells[AURORA_NETWORK_MAX_CELLS];
    aurora_network_gate gates[AURORA_NETWORK_MAX_GATES];
    size_t cell_count;
    size_t gate_count;
    aurora_state state;
} aurora_network;

void aurora_network_init(aurora_network *network);
int aurora_network_add_cell(aurora_network *network, aurora_domain domain);
int aurora_network_add_gate(aurora_network *network,
                            size_t a,
                            size_t b,
                            size_t mode,
                            size_t result);

/*
 * Poda local monótona hasta punto fijo. CLOSED sólo significa cierre local de
 * los gates; no afirma por sí mismo unicidad de la configuración global.
 */
aurora_state aurora_network_propagate(aurora_network *network);

#endif
