#ifndef AURORA_ORDERING_H
#define AURORA_ORDERING_H

#include "aurora/trigate.h"

typedef struct {
    /* O elegido para el intento; alternatives conserva las demás rutas. */
    aurora_domain orientation;
    aurora_domain alternatives;
    /* E de la ordenación: 0 cierre, 1 imposible, 2 ambigua. */
    aurora_domain e;
    aurora_domain es;
    aurora_domain fn;
    aurora_domain fo;
    aurora_state state;
} aurora_ordering;

/*
 * Ordena una tripleta sin destruir la ambigüedad.
 *
 * El TriGate produce primero (R,E,O). Para cada testigo compatible de O:
 *   1. ES toma R, el valor mayoritario de la tripleta.
 *   2. La celda ES contiene el índice de FN.
 *   3. ES no puede apuntarse a sí misma.
 *   4. FO es la posición restante.
 *
 * Los campos de salida son la unión de todos los testigos válidos.
 */
aurora_ordering aurora_order_triplet(const aurora_domain triplet[3]);

aurora_ordering aurora_order_triplet_at_phase(
    const aurora_domain triplet[3],
    aurora_domain phase);

#define AURORA_DO_STATE_COUNT 27u
#define AURORA_DO_FIBONACCI_STRIDE 7u /* F(9)=34 ≡ 7 (mod 27) */

typedef struct {
    uint32_t visited;
    size_t attempts;
    aurora_domain state[3];
    bool exhausted;
} aurora_do_cursor;

void aurora_do_cursor_init(aurora_do_cursor *cursor);
bool aurora_do_cursor_next(aurora_do_cursor *cursor);

#endif
