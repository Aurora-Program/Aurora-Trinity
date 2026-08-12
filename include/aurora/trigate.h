#ifndef AURORA_TRIGATE_H
#define AURORA_TRIGATE_H

#include "aurora/domain.h"

typedef enum {
    AURORA_CLOSED = 0,
    AURORA_CONTRADICTION = 1,
    AURORA_OPEN = 2
} aurora_state;

typedef enum {
    AURORA_DEDUCE_B = 0,
    AURORA_INFER_R = 1,
    AURORA_LEARN_M = 2
} aurora_direction;

/*
 * Paquete observable del TriGate. R es el resultado mayoritario, E expresa
 * cierre (0), incoherencia determinada (1) o ambigüedad (2), y O conserva
 * las posiciones desde las que la tripleta puede leerse sin autorreferencia.
 * Los tres campos son dominios para no destruir empates legítimos.
 */
typedef struct {
    aurora_domain r;
    aurora_domain e;
    aurora_domain o; /* ruta elegida para el intento actual */
    aurora_domain o_candidates; /* rutas conservadas para retroceso */
    aurora_state state;
} aurora_trigate_packet;

typedef struct {
    aurora_domain a;
    aurora_domain b;
    aurora_domain mode;
    aurora_domain result;
    aurora_direction direction;
    aurora_trigate_packet packet;
} aurora_trigate_resolution;

/* Majority3: si no hay mayoría, el resultado es el valor cerrado 2. */
uint8_t aurora_majority3(uint8_t a, uint8_t b, uint8_t mode);

/* Extiende Majority3 al producto cartesiano de tres dominios. */
aurora_domain aurora_majority3_domain(aurora_domain a,
                                      aurora_domain b,
                                      aurora_domain mode);

/*
 * Evalúa el paquete (R,E,O) usado al ordenar una tripleta. Para dominios
 * plurales agrega todos los testigos concretos y conserva la ambigüedad.
 */
aurora_trigate_packet aurora_trigate_order_packet(aurora_domain a,
                                                  aurora_domain b,
                                                  aurora_domain mode);

aurora_trigate_packet aurora_trigate_order_packet_at_phase(
    aurora_domain a,
    aurora_domain b,
    aurora_domain mode,
    aurora_domain phase);

/*
 * Proyecta E_C para una relación ya propagada. La dirección se conserva en
 * el contrato aunque la inferencia ordinaria sea hoy la única conectada a
 * las caras. El caso R=2 transporta el residual documentado de (2,2,x).
 */
aurora_domain aurora_trigate_residual(aurora_domain a,
                                      aurora_domain b,
                                      aurora_domain mode,
                                      aurora_domain result,
                                      aurora_direction direction,
                                      aurora_state propagation_state);

/*
 * Reejecuta la relación en la dirección C sobre copias de sus dominios. El
 * llamante deja abierto el dominio que quiere resolver; el resultado conserva
 * los dominios podados y el paquete observable correspondiente.
 */
aurora_trigate_resolution aurora_trigate_resolve(aurora_domain a,
                                                 aurora_domain b,
                                                 aurora_domain mode,
                                                 aurora_domain result,
                                                 aurora_direction direction);

/* Conversión explícita: aurora_state y E comparten semántica, no identidad accidental. */
aurora_domain aurora_e_from_state(aurora_state state);

/*
 * Impone R = Majority3(A,B,M) y poda los cuatro dominios hasta el punto fijo.
 * EMPTY significa contradicción; ALL significa falta de información, no el
 * valor 2. La función es bidireccional: también resuelve desde R hacia A/B/M.
 */
aurora_state aurora_trigate_propagate(aurora_domain *a,
                                      aurora_domain *b,
                                      aurora_domain *mode,
                                      aurora_domain *result);

#endif
