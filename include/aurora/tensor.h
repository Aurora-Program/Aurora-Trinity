#ifndef AURORA_TENSOR_H
#define AURORA_TENSOR_H

#include "aurora/so.h"

#define AURORA_TENSOR_MAX_NODES 512u
#define AURORA_TENSOR_NO_CHILD ((size_t)-1)

typedef struct {
    aurora_so superior;
    /* O_U: orientación de la ventana que construyó esta salida U*. */
    aurora_domain window_orientation[3];
    size_t children[3];
    size_t depth; /* nivel de abstracción; solo la emergencia lo incrementa */
    size_t provenance_depth;
    size_t leaf_count;
    aurora_state state;
    bool has_window_orientation;
    bool is_leaf;
} aurora_tensor_node;

typedef struct {
    aurora_tensor_node nodes[AURORA_TENSOR_MAX_NODES];
    size_t count;
} aurora_tensor;

void aurora_tensor_init(aurora_tensor *tensor);

/* Añade una hoja. Devuelve su índice o -1 si la tripleta es imposible. */
int aurora_tensor_add_leaf(aurora_tensor *tensor,
                           const aurora_domain triplet[3]);

/* Compone tres nodos de igual profundidad conservando su procedencia. */
int aurora_tensor_compose(aurora_tensor *tensor,
                          size_t first,
                          size_t second,
                          size_t third);
int aurora_tensor_compose_at_do(aurora_tensor *tensor,
                                size_t first,
                                size_t second,
                                size_t third,
                                const aurora_domain do_state[3]);

/* Compone un candidato abierto sin ascender de nivel. */
int aurora_tensor_carry(aurora_tensor *tensor,
                        size_t first,
                        size_t second,
                        size_t third);
int aurora_tensor_carry_at_do(aurora_tensor *tensor,
                              size_t first,
                              size_t second,
                              size_t third,
                              const aurora_domain do_state[3]);

/* Materializa una relación ya reejecutada sin volver a calcularla. */
int aurora_tensor_materialize(aurora_tensor *tensor,
                              size_t first,
                              size_t second,
                              size_t third,
                              const aurora_so *superior,
                              const aurora_domain u_orientation[3],
                              bool emerge);

const aurora_tensor_node *aurora_tensor_get(const aurora_tensor *tensor,
                                            size_t index);

#endif
