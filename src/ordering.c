#include "aurora/ordering.h"

static void add_witness(aurora_ordering *ordering,
                        const uint8_t values[3],
                        uint8_t es_position) {
    uint8_t fn_position = values[es_position];
    uint8_t fo_position = (uint8_t)(3u - es_position - fn_position);

    ordering->es |= aurora_domain_singleton(values[es_position]);
    ordering->fn |= aurora_domain_singleton(values[fn_position]);
    ordering->fo |= aurora_domain_singleton(values[fo_position]);
}

aurora_ordering aurora_order_triplet(const aurora_domain triplet[3]) {
    return aurora_order_triplet_at_phase(triplet, AURORA_DOMAIN_0);
}

aurora_ordering aurora_order_triplet_at_phase(
    const aurora_domain triplet[3],
    aurora_domain phase) {
    aurora_ordering ordering = {
        AURORA_DOMAIN_EMPTY, AURORA_DOMAIN_EMPTY, AURORA_DOMAIN_1,
        AURORA_DOMAIN_EMPTY,
        AURORA_DOMAIN_EMPTY, AURORA_DOMAIN_EMPTY, AURORA_CONTRADICTION};
    if (triplet == NULL || !aurora_domain_is_valid(triplet[0]) ||
        !aurora_domain_is_valid(triplet[1]) ||
        !aurora_domain_is_valid(triplet[2])) {
        return ordering;
    }

    aurora_trigate_packet packet = aurora_trigate_order_packet_at_phase(
        triplet[0], triplet[1], triplet[2], phase);
    if (packet.state == AURORA_CONTRADICTION) {
        return ordering;
    }
    ordering.orientation = packet.o;
    ordering.alternatives = packet.o_candidates;

    for (uint8_t a = 0u; a < 3u; ++a) {
        for (uint8_t b = 0u; b < 3u; ++b) {
            for (uint8_t c = 0u; c < 3u; ++c) {
                uint8_t values[3] = {a, b, c};
                if (!aurora_domain_contains(triplet[0], a) ||
                    !aurora_domain_contains(triplet[1], b) ||
                    !aurora_domain_contains(triplet[2], c)) {
                    continue;
                }
                uint8_t majority = aurora_majority3(a, b, c);
                for (uint8_t position = 0u; position < 3u; ++position) {
                    if (aurora_domain_contains(packet.o_candidates, position) &&
                        values[position] == majority &&
                        values[position] != position) {
                        add_witness(&ordering, values, position);
                    }
                }
            }
        }
    }

    if (ordering.alternatives == AURORA_DOMAIN_EMPTY) {
        return ordering;
    }

    ordering.state = packet.state;
    ordering.e = packet.e;
    return ordering;
}

void aurora_do_cursor_init(aurora_do_cursor *cursor) {
    if (cursor == NULL) return;
    cursor->visited = 0u;
    cursor->attempts = 0u;
    cursor->state[0] = AURORA_DOMAIN_0;
    cursor->state[1] = AURORA_DOMAIN_0;
    cursor->state[2] = AURORA_DOMAIN_0;
    cursor->exhausted = false;
}

bool aurora_do_cursor_next(aurora_do_cursor *cursor) {
    if (cursor == NULL || cursor->exhausted ||
        cursor->attempts >= AURORA_DO_STATE_COUNT) {
        if (cursor != NULL) cursor->exhausted = true;
        return false;
    }
    size_t encoded =
        (cursor->attempts * AURORA_DO_FIBONACCI_STRIDE) %
        AURORA_DO_STATE_COUNT;
    uint32_t bit = UINT32_C(1) << encoded;
    if ((cursor->visited & bit) != 0u) {
        cursor->exhausted = true;
        return false;
    }
    cursor->visited |= bit;
    cursor->state[0] = aurora_domain_singleton((uint8_t)(encoded / 9u));
    cursor->state[1] =
        aurora_domain_singleton((uint8_t)((encoded / 3u) % 3u));
    cursor->state[2] = aurora_domain_singleton((uint8_t)(encoded % 3u));
    ++cursor->attempts;
    if (cursor->attempts == AURORA_DO_STATE_COUNT) cursor->exhausted = true;
    return true;
}
