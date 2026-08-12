#ifndef AURORA_BOOTSTRAP_H
#define AURORA_BOOTSTRAP_H

#include "aurora/so.h"

typedef struct {
    aurora_so knowledge;
    bool written;
    aurora_state state;
} aurora_bootstrap_result;

aurora_so aurora_so_empty(void);
bool aurora_so_is_empty(const aurora_so *so);

/* Aplica K=⊥, S=I, E_W=2 ⇒ K←I; en otro caso conserva K. */
aurora_bootstrap_result aurora_bootstrap_knowledge(
    const aurora_so *input,
    const aurora_so *current_knowledge,
    const aurora_so *output,
    aurora_state window_state);

#endif
