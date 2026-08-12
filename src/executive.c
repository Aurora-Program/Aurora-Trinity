#include "aurora/executive.h"

#include <stdint.h>

static const uint8_t TARGETS[3][3] = {
    {0u, 1u, 2u}, /* C4: diferenciación */
    {0u, 0u, 0u}, /* C5: vector residual nulo */
    {0u, 0u, 0u}  /* C6: trayectoria mínima */
};

aurora_harmonizer aurora_harmonizer_default(void) {
    return (aurora_harmonizer){1u, 1u, 1u};
}

size_t aurora_face_distance(const aurora_domain face[3],
                            const uint8_t target[3]) {
    if (face == NULL || target == NULL) {
        return SIZE_MAX;
    }
    size_t distance = 0u;
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (!aurora_domain_is_valid(face[dimension])) {
            return SIZE_MAX;
        }
        if (!aurora_domain_contains(face[dimension], target[dimension])) {
            ++distance;
        }
    }
    return distance;
}

static bool exact_target(const aurora_domain face[3],
                         const uint8_t target[3]) {
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (face[dimension] != aurora_domain_singleton(target[dimension])) {
            return false;
        }
    }
    return true;
}

aurora_domain aurora_harmonize_face(const aurora_domain face[3],
                                    const uint8_t target[3],
                                    size_t near,
                                    size_t *distance) {
    if (distance == NULL) {
        return AURORA_DOMAIN_0;
    }
    *distance = aurora_face_distance(face, target);
    if (*distance == SIZE_MAX) {
        return AURORA_DOMAIN_0;
    }
    (void)near;
    if (exact_target(face, target)) return AURORA_DOMAIN_0;
    if (*distance == 0u) return AURORA_DOMAIN_2;
    return AURORA_DOMAIN_1;
}

static aurora_state state_from_e(aurora_domain e) {
    if (e == AURORA_DOMAIN_0) return AURORA_CLOSED;
    if (e == AURORA_DOMAIN_1) return AURORA_CONTRADICTION;
    return AURORA_OPEN;
}

aurora_direction aurora_executive_resolve_faces(aurora_state c4,
                                                aurora_state c5,
                                                aurora_domain do_phase) {
    if (c5 == AURORA_CLOSED) return AURORA_INFER_R;
    if (c5 == AURORA_CONTRADICTION) return AURORA_LEARN_M;
    if (c4 == AURORA_CLOSED) return AURORA_INFER_R;
    if (c4 == AURORA_CONTRADICTION) return AURORA_DEDUCE_B;
    for (uint8_t phase = 0u; phase < 3u; ++phase) {
        if (do_phase == aurora_domain_singleton(phase)) {
            return (aurora_direction)phase;
        }
    }
    return AURORA_INFER_R;
}

static void make_seed_so(aurora_so *seed,
                         const aurora_domain deltas[3],
                         aurora_state state) {
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        seed->upper_ds[dimension] = deltas[dimension];
        seed->ds[dimension] = deltas[dimension];
        seed->de[dimension] = AURORA_DOMAIN_ALL;
        seed->do_[dimension] = AURORA_DOMAIN_ALL;
    }
    seed->state = state;
}

aurora_executive aurora_executive_decide(
    const aurora_so *input,
    const aurora_so *knowledge,
    const aurora_so *output,
    const aurora_harmonizer *harmonizer) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_executive_decide_at_do(
        input, knowledge, output, harmonizer, origin, false);
}

aurora_executive aurora_executive_decide_at_do(
    const aurora_so *input,
    const aurora_so *knowledge,
    const aurora_so *output,
    const aurora_harmonizer *harmonizer,
    const aurora_domain do_state[3],
    bool do_exhausted) {
    aurora_executive executive = {0};
    executive.state = AURORA_CONTRADICTION;
    executive.direction = AURORA_LEARN_NONE;
    executive.timing = AURORA_EXECUTIVE_DESIST;
    if (input == NULL || knowledge == NULL || output == NULL ||
        do_state == NULL) {
        return executive;
    }
    aurora_harmonizer defaults = aurora_harmonizer_default();
    const aurora_harmonizer *config = harmonizer == NULL ? &defaults
                                                         : harmonizer;
    const aurora_domain *channels[3][3] = {
        {input->ds, knowledge->ds, output->ds},
        {input->de, knowledge->de, output->de},
        {input->do_, knowledge->do_, output->do_}};
    for (size_t face = 0u; face < 3u; ++face) {
        aurora_operation relation = aurora_operate_minimum_at_do(
            channels[face][0], channels[face][1], channels[face][2],
            do_state);
        if (relation.state == AURORA_CONTRADICTION) {
            return executive;
        }
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            executive.faces[face][dimension] = relation.ds[dimension];
        }
    }

    const size_t near[3] = {config->near_ds, config->near_de,
                            config->near_do};
    for (size_t face = 0u; face < 3u; ++face) {
        executive.deltas[face] = aurora_harmonize_face(
            executive.faces[face], TARGETS[face], near[face],
            &executive.distances[face]);
        executive.face_states[face] = state_from_e(executive.deltas[face]);
    }

    executive.r = aurora_majority3_domain(
        executive.deltas[0], executive.deltas[1], executive.deltas[2]);
    aurora_ordering order = aurora_order_triplet_at_phase(
        executive.deltas, do_state[2]);
    executive.state = executive.face_states[1] == AURORA_OPEN
                          ? order.state
                          : executive.face_states[1];
    executive.e = aurora_e_from_state(executive.state);
    executive.o = order.orientation;
    executive.resolution = aurora_executive_resolve_faces(
        executive.face_states[0], executive.face_states[1], do_state[2]);
    make_seed_so(&executive.seed, executive.deltas, executive.state);

    if (executive.state == AURORA_CLOSED) {
        executive.direction = AURORA_LEARN_CRYSTALLIZE;
    } else if (executive.resolution == AURORA_LEARN_M) {
        executive.direction = AURORA_LEARN_TEMPORARY;
    }
    executive.timing = do_exhausted
                           ? AURORA_EXECUTIVE_DESIST
                           : executive.state == AURORA_CLOSED ||
                                     executive.state == AURORA_CONTRADICTION
                                 ? AURORA_EXECUTIVE_ACT
                                 : AURORA_EXECUTIVE_CONTINUE;
    return executive;
}
