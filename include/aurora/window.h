#ifndef AURORA_WINDOW_H
#define AURORA_WINDOW_H

#include "aurora/tensor.h"

typedef enum {
    AURORA_WINDOW_SLIDE = 0,
    AURORA_WINDOW_EMERGE = 1,
    AURORA_WINDOW_CARRY = 2
} aurora_window_action;

typedef struct {
    aurora_so relation;
    /* Orientación propia de U durante este intento de ventana. */
    aurora_domain u_orientation[3];
    aurora_state state;
    aurora_window_action action;
    int output;
} aurora_window_result;

/* Agrega DE: un 1 determinado domina, 000 cierra y el resto queda ambiguo. */
aurora_state aurora_window_state_from_de(const aurora_domain de[3]);

/* Evalúa sin modificar la arena; se usa para competir candidatas. */
aurora_window_result aurora_window_evaluate(const aurora_tensor *tensor,
                                            size_t first,
                                            size_t second,
                                            size_t third);

/* En las variantes at_do, DO_t fija también O_U para ese intento. */
aurora_window_result aurora_window_evaluate_at_do(
    const aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t third,
    const aurora_domain do_state[3]);

/*
 * Procesa tres tensores de igual nivel.
 * E=0 añade un tensor emergente; E=2 añade U* como carry;
 * E=1 no sintetiza A/B y ordena deslizar/sustituir.
 */
aurora_window_result aurora_window_process(aurora_tensor *tensor,
                                           size_t first,
                                           size_t second,
                                           size_t third);

aurora_window_result aurora_window_process_at_do(
    aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t third,
    const aurora_domain do_state[3]);

aurora_window_result aurora_window_evaluate_knowledge_at_do(
    const aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t knowledge,
    const aurora_domain do_state[3]);

aurora_window_result aurora_window_process_knowledge_at_do(
    aurora_tensor *tensor,
    size_t first,
    size_t second,
    size_t knowledge,
    const aurora_domain do_state[3]);

#endif
