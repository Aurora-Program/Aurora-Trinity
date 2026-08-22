#include "aurora/trigate.h"

aurora_domain aurora_e_from_state(aurora_state state) {
    switch (state) {
        case AURORA_CLOSED:
            return AURORA_DOMAIN_0;
        case AURORA_CONTRADICTION:
            return AURORA_DOMAIN_1;
        case AURORA_OPEN:
            return AURORA_DOMAIN_2;
    }
    return AURORA_DOMAIN_EMPTY;
}

uint8_t aurora_majority3(uint8_t a, uint8_t b, uint8_t mode) {
    if (a == b || a == mode) {
        return a;
    }
    if (b == mode) {
        return b;
    }
    return 2u;
}

aurora_domain aurora_majority3_domain(aurora_domain a,
                                      aurora_domain b,
                                      aurora_domain mode) {
    aurora_domain result = AURORA_DOMAIN_EMPTY;
    if (!aurora_domain_is_valid(a) || !aurora_domain_is_valid(b) ||
        !aurora_domain_is_valid(mode)) {
        return result;
    }
    for (uint8_t av = 0u; av < 3u; ++av) {
        for (uint8_t bv = 0u; bv < 3u; ++bv) {
            for (uint8_t mv = 0u; mv < 3u; ++mv) {
                if (aurora_domain_contains(a, av) &&
                    aurora_domain_contains(b, bv) &&
                    aurora_domain_contains(mode, mv)) {
                    result |= aurora_domain_singleton(
                        aurora_majority3(av, bv, mv));
                }
            }
        }
    }
    return result;
}

static aurora_domain select_orientation(aurora_domain candidates,
                                        aurora_domain phase) {
    if (!aurora_domain_is_valid(candidates)) return AURORA_DOMAIN_EMPTY;
    uint8_t start = 0u;
    for (uint8_t value = 0u; value < 3u; ++value) {
        if (phase == aurora_domain_singleton(value)) {
            start = value;
            break;
        }
    }
    for (uint8_t offset = 0u; offset < 3u; ++offset) {
        uint8_t candidate = (uint8_t)((start + offset) % 3u);
        if (aurora_domain_contains(candidates, candidate)) {
            return aurora_domain_singleton(candidate);
        }
    }
    return AURORA_DOMAIN_EMPTY;
}

aurora_trigate_packet aurora_trigate_order_packet_at_phase(
    aurora_domain a,
    aurora_domain b,
    aurora_domain mode,
    aurora_domain phase) {
    aurora_trigate_packet packet = {
        AURORA_DOMAIN_EMPTY, AURORA_DOMAIN_1, AURORA_DOMAIN_EMPTY,
        AURORA_DOMAIN_EMPTY, AURORA_CONTRADICTION};
    if (!aurora_domain_is_valid(a) || !aurora_domain_is_valid(b) ||
        !aurora_domain_is_valid(mode) || !aurora_domain_is_valid(phase)) {
        return packet;
    }

    aurora_domain inputs[3] = {a, b, mode};
    size_t witnesses = 0u;
    for (uint8_t av = 0u; av < 3u; ++av) {
        for (uint8_t bv = 0u; bv < 3u; ++bv) {
            for (uint8_t mv = 0u; mv < 3u; ++mv) {
                uint8_t values[3] = {av, bv, mv};
                if (!aurora_domain_contains(inputs[0], av) ||
                    !aurora_domain_contains(inputs[1], bv) ||
                    !aurora_domain_contains(inputs[2], mv)) {
                    continue;
                }
                uint8_t result = aurora_majority3(av, bv, mv);
                packet.r |= aurora_domain_singleton(result);
                for (uint8_t position = 0u; position < 3u; ++position) {
                    if (values[position] == result &&
                        values[position] != position) {
                        packet.o_candidates |=
                            aurora_domain_singleton(position);
                        ++witnesses;
                    }
                }
            }
        }
    }

    /*
     * Caso residual (2,2,2): la tripleta cerrada en 2 conserva las tres rutas
     * de lectura, incluida la posición 2 que la regla general descarta por
     * autorreferencia. Es deliberado: al no existir testigo legible sin
     * autorreferencia, la ventana permanece abierta con O=2.
     */
    if (a == AURORA_DOMAIN_2 && b == AURORA_DOMAIN_2 &&
        mode == AURORA_DOMAIN_2) {
        packet.o_candidates = AURORA_DOMAIN_ALL;
        packet.o = AURORA_DOMAIN_2;
        packet.e = AURORA_DOMAIN_2;
        packet.state = AURORA_OPEN;
        return packet;
    }

    if (packet.o_candidates == AURORA_DOMAIN_EMPTY) {
        packet.e = AURORA_DOMAIN_1;
        return packet;
    }

    packet.o = select_orientation(packet.o_candidates, phase);

    bool determined = witnesses == 1u &&
                      aurora_domain_is_singleton(packet.r) &&
                      aurora_domain_is_singleton(packet.o_candidates) &&
                      aurora_domain_is_singleton(a) &&
                      aurora_domain_is_singleton(b) &&
                      aurora_domain_is_singleton(mode);
    packet.state = determined ? AURORA_CLOSED : AURORA_OPEN;
    packet.e = determined ? AURORA_DOMAIN_0 : AURORA_DOMAIN_2;
    return packet;
}


aurora_trigate_packet aurora_trigate_order_packet(aurora_domain a,
                                                  aurora_domain b,
                                                  aurora_domain mode) {
    return aurora_trigate_order_packet_at_phase(a, b, mode,
                                                AURORA_DOMAIN_0);
}

static uint8_t singleton_value(aurora_domain domain) {
    for (uint8_t value = 0u; value < 3u; ++value) {
        if (domain == aurora_domain_singleton(value)) {
            return value;
        }
    }
    return 3u;
}

aurora_domain aurora_trigate_residual(aurora_domain a,
                                      aurora_domain b,
                                      aurora_domain mode,
                                      aurora_domain result,
                                      aurora_direction direction,
                                      aurora_state propagation_state) {
    if (direction > AURORA_LEARN_M ||
        propagation_state == AURORA_CONTRADICTION) {
        return AURORA_DOMAIN_1;
    }
    if (propagation_state == AURORA_OPEN) {
        return AURORA_DOMAIN_2;
    }

    uint8_t av = singleton_value(a);
    uint8_t bv = singleton_value(b);
    uint8_t mv = singleton_value(mode);
    uint8_t rv = singleton_value(result);
    if (av > 2u || bv > 2u || mv > 2u || rv > 2u) {
        return AURORA_DOMAIN_2;
    }

    if (rv == 2u) {
        uint8_t values[3] = {av, bv, mv};
        size_t twos = 0u;
        uint8_t residual = 2u;
        for (size_t index = 0u; index < 3u; ++index) {
            if (values[index] == 2u) {
                ++twos;
            } else {
                residual = values[index];
            }
        }
        if (twos == 2u) {
            return aurora_domain_singleton(residual);
        }
        if (twos == 3u || (av != bv && av != mv && bv != mv)) {
            return AURORA_DOMAIN_2;
        }
    }
    return AURORA_DOMAIN_0;
}

aurora_state aurora_trigate_propagate(aurora_domain *a,
                                      aurora_domain *b,
                                      aurora_domain *mode,
                                      aurora_domain *result) {
    if (a == NULL || b == NULL || mode == NULL || result == NULL ||
        !aurora_domain_is_valid(*a) || !aurora_domain_is_valid(*b) ||
        !aurora_domain_is_valid(*mode) || !aurora_domain_is_valid(*result)) {
        return AURORA_CONTRADICTION;
    }

    aurora_domain support_a = AURORA_DOMAIN_EMPTY;
    aurora_domain support_b = AURORA_DOMAIN_EMPTY;
    aurora_domain support_m = AURORA_DOMAIN_EMPTY;
    aurora_domain support_r = AURORA_DOMAIN_EMPTY;

    for (uint8_t av = 0u; av < 3u; ++av) {
        for (uint8_t bv = 0u; bv < 3u; ++bv) {
            for (uint8_t mv = 0u; mv < 3u; ++mv) {
                uint8_t rv = aurora_majority3(av, bv, mv);
                if (aurora_domain_contains(*a, av) &&
                    aurora_domain_contains(*b, bv) &&
                    aurora_domain_contains(*mode, mv) &&
                    aurora_domain_contains(*result, rv)) {
                    support_a |= aurora_domain_singleton(av);
                    support_b |= aurora_domain_singleton(bv);
                    support_m |= aurora_domain_singleton(mv);
                    support_r |= aurora_domain_singleton(rv);
                }
            }
        }
    }

    if (support_a == AURORA_DOMAIN_EMPTY) {
        *a = *b = *mode = *result = AURORA_DOMAIN_EMPTY;
        return AURORA_CONTRADICTION;
    }

    *a = aurora_domain_intersection(*a, support_a);
    *b = aurora_domain_intersection(*b, support_b);
    *mode = aurora_domain_intersection(*mode, support_m);
    *result = aurora_domain_intersection(*result, support_r);

    return aurora_domain_is_singleton(*a) &&
                   aurora_domain_is_singleton(*b) &&
                   aurora_domain_is_singleton(*mode) &&
                   aurora_domain_is_singleton(*result)
               ? AURORA_CLOSED
               : AURORA_OPEN;
}

aurora_trigate_resolution aurora_trigate_resolve(aurora_domain a,
                                                 aurora_domain b,
                                                 aurora_domain mode,
                                                 aurora_domain result,
                                                 aurora_direction direction) {
    aurora_trigate_resolution resolution = {0};
    resolution.a = a;
    resolution.b = b;
    resolution.mode = mode;
    resolution.result = result;
    resolution.direction = direction;
    resolution.packet.e = AURORA_DOMAIN_1;
    resolution.packet.state = AURORA_CONTRADICTION;
    if (direction > AURORA_LEARN_M) {
        return resolution;
    }

    aurora_state propagation_state = aurora_trigate_propagate(
        &resolution.a, &resolution.b, &resolution.mode, &resolution.result);
    resolution.packet.r = resolution.result;
    if (propagation_state == AURORA_CONTRADICTION) {
        resolution.packet.e = AURORA_DOMAIN_1;
        resolution.packet.state = AURORA_CONTRADICTION;
        return resolution;
    }

    aurora_domain target = direction == AURORA_DEDUCE_B
                               ? resolution.b
                               : direction == AURORA_INFER_R
                                     ? resolution.result
                                     : resolution.mode;
    if (!aurora_domain_is_singleton(target)) {
        resolution.packet.e = AURORA_DOMAIN_2;
        resolution.packet.state = AURORA_OPEN;
    } else if (resolution.result == AURORA_DOMAIN_2) {
        resolution.packet.e = aurora_trigate_residual(
            resolution.a, resolution.b, resolution.mode, resolution.result,
            direction, propagation_state);
        resolution.packet.state = AURORA_OPEN;
    } else {
        resolution.packet.e = AURORA_DOMAIN_0;
        resolution.packet.state = AURORA_CLOSED;
    }

    aurora_trigate_packet order = aurora_trigate_order_packet_at_phase(
        resolution.a, resolution.b, resolution.mode,
        aurora_domain_singleton((uint8_t)direction));
    resolution.packet.o = order.o;
    resolution.packet.o_candidates = order.o_candidates;
    return resolution;
}
