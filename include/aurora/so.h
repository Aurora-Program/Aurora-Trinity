#ifndef AURORA_SO_H
#define AURORA_SO_H

#include "aurora/operation.h"

typedef struct {
    aurora_domain upper_ds[3];
    aurora_domain ds[3];
    aurora_domain de[3];
    aurora_domain do_[3];
    aurora_state state;
} aurora_so;

/* SO inicial: conserva el DS dado y deja DE/DO explícitamente abiertos. */
aurora_so aurora_so_seed(const aurora_domain ds[3]);

/* Convierte DS/DE/DO de una operación en una SO y sintetiza su DS superior. */
aurora_so aurora_so_from_operation(const aurora_operation *operation);
aurora_so aurora_so_from_operation_at_do(
    const aurora_operation *operation,
    const aurora_domain do_state[3]);

/* Relaciona los DS superiores de tres SO y sintetiza una nueva SO completa. */
aurora_so aurora_so_relate(const aurora_so *first,
                           const aurora_so *second,
                           const aurora_so *third);
aurora_so aurora_so_relate_at_do(const aurora_so *first,
                                 const aurora_so *second,
                                 const aurora_so *third,
                                 const aurora_domain do_state[3]);

#endif
