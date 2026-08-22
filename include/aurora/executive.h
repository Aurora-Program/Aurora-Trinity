#ifndef AURORA_EXECUTIVE_H
#define AURORA_EXECUTIVE_H

#include "aurora/so.h"

typedef struct {
    /*
     * Umbrales declarados por contrato (near_ds, near_de, near_do). Hoy no
     * alteran el resultado de aurora_harmonize_face, que clasifica cierre
     * exacto (0), distancia no nula (1) y apertura (2). Quedan reservados
     * para una futura calibración de cercanía.
     */
    size_t near_ds;
    size_t near_de;
    size_t near_do;
} aurora_harmonizer;

typedef enum {
    AURORA_LEARN_NONE = 0,
    AURORA_LEARN_CRYSTALLIZE = 1,
    AURORA_LEARN_TEMPORARY = 2
} aurora_learning_direction;

typedef enum {
    AURORA_EXECUTIVE_DESIST = 0,
    AURORA_EXECUTIVE_ACT = 1,
    AURORA_EXECUTIVE_CONTINUE = 2
} aurora_executive_timing;

typedef struct {
    aurora_domain faces[3][3]; /* C4=DS, C5=DE, C6=DO */
    size_t distances[3];
    aurora_domain deltas[3];
    aurora_so seed;
    aurora_domain r;
    aurora_domain e;
    aurora_domain o;
    aurora_state face_states[3];
    aurora_direction resolution;
    aurora_learning_direction direction;
    aurora_executive_timing timing;
    aurora_state state;
} aurora_executive;

aurora_harmonizer aurora_harmonizer_default(void);

/* Distancia mínima de Hamming entre una cara de dominios y un objetivo. */
size_t aurora_face_distance(const aurora_domain face[3],
                            const uint8_t target[3]);

aurora_domain aurora_harmonize_face(const aurora_domain face[3],
                                    const uint8_t target[3],
                                    size_t near,
                                    size_t *distance);

aurora_executive aurora_executive_decide(
    const aurora_so *input,
    const aurora_so *knowledge,
    const aurora_so *output,
    const aurora_harmonizer *harmonizer);

aurora_direction aurora_executive_resolve_faces(aurora_state c4,
                                                aurora_state c5,
                                                aurora_domain do_phase);

aurora_executive aurora_executive_decide_at_do(
    const aurora_so *input,
    const aurora_so *knowledge,
    const aurora_so *output,
    const aurora_harmonizer *harmonizer,
    const aurora_domain do_state[3],
    bool do_exhausted);

#endif
