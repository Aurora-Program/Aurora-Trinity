#ifndef AURORA_SEQUENCE_H
#define AURORA_SEQUENCE_H

#include "aurora/window.h"

typedef enum {
    AURORA_SEQUENCE_COMPLETE = 0,
    AURORA_SEQUENCE_PENDING = 1,
    AURORA_SEQUENCE_CAPACITY = 2,
    AURORA_SEQUENCE_INVALID = 3
} aurora_sequence_status;

typedef struct {
    size_t outputs[AURORA_TENSOR_MAX_NODES];
    size_t output_count;
    size_t pending[3];
    size_t pending_count;
    size_t input_consumed;
    size_t windows;
    size_t emergences;
    size_t carries;
    size_t slides;
    aurora_sequence_status status;
} aurora_sequence_result;

typedef struct {
    size_t roots[AURORA_TENSOR_MAX_NODES];
    size_t root_count;
    size_t pending[AURORA_TENSOR_MAX_NODES];
    size_t pending_count;
    size_t levels;
    size_t windows;
    size_t emergences;
    size_t carries;
    size_t slides;
    aurora_sequence_status status;
} aurora_hierarchy_result;

/* Recorre un nivel completo. Los índices de entrada deben tener igual nivel. */
aurora_sequence_result aurora_sequence_run_level(
    aurora_tensor *tensor,
    const size_t *input,
    size_t input_count);

/* Reaplica run_level a cada salida emergente hasta alcanzar las raíces. */
aurora_hierarchy_result aurora_sequence_run_hierarchy(
    aurora_tensor *tensor,
    const size_t *input,
    size_t input_count);

#endif
