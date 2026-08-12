#ifndef AURORA_OPERATION_H
#define AURORA_OPERATION_H

#include "aurora/network.h"
#include "aurora/ordering.h"

typedef struct {
    aurora_domain function;
    aurora_domain form;
} aurora_distilled_pair;

typedef struct {
    aurora_ordering ordered[3];
    aurora_distilled_pair pairs[3];
    aurora_domain ds[3]; /* ES, FN, FO */
    aurora_domain de[3]; /* estados de las tres emergencias */
    aurora_domain do_[3]; /* O de las emergencias ES, FN y FO */
    aurora_state state;
} aurora_operation;

/* Ejecuta ordenación, destilador circular y emergencia sobre tres tripletas. */
aurora_operation aurora_operate_minimum(const aurora_domain first[3],
                                        const aurora_domain second[3],
                                        const aurora_domain third[3]);

aurora_operation aurora_operate_minimum_at_do(
    const aurora_domain first[3],
    const aurora_domain second[3],
    const aurora_domain third[3],
    const aurora_domain do_state[3]);

/* Igual que la operación mínima, pero permite clavar dominios esperados de DS. */
aurora_operation aurora_operate_minimum_with_knowledge(
    const aurora_domain first[3],
    const aurora_domain second[3],
    const aurora_domain third[3],
    const aurora_domain knowledge_ds[3]);

aurora_operation aurora_operate_minimum_with_knowledge_at_do(
    const aurora_domain first[3],
    const aurora_domain second[3],
    const aurora_domain third[3],
    const aurora_domain knowledge_ds[3],
    const aurora_domain do_state[3]);

#endif
