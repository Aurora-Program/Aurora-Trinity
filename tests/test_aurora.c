#include "trinity/trinity.h"

#include <stdio.h>
#include <stdlib.h>

static int checks;

#define CHECK(condition)                                                        \
    do {                                                                        \
        ++checks;                                                               \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,           \
                    #condition);                                                \
            exit(EXIT_FAILURE);                                                 \
        }                                                                       \
    } while (0)

static uint8_t expected_majority(uint8_t a, uint8_t b, uint8_t m) {
    int counts[3] = {0, 0, 0};
    ++counts[a];
    ++counts[b];
    ++counts[m];
    for (uint8_t value = 0u; value < 3u; ++value) {
        if (counts[value] >= 2) {
            return value;
        }
    }
    return 2u;
}

static uint8_t expected_r2_residual(uint8_t a, uint8_t b, uint8_t m) {
    uint8_t values[3] = {a, b, m};
    size_t twos = 0u;
    uint8_t residual = 2u;
    for (size_t index = 0u; index < 3u; ++index) {
        if (values[index] == 2u) {
            ++twos;
        } else {
            residual = values[index];
        }
    }
    return twos == 2u ? residual : 2u;
}

static void test_domains(void) {
    volatile aurora_domain closed_two = AURORA_DOMAIN_2;
    volatile aurora_domain unknown = AURORA_DOMAIN_ALL;
    CHECK(!aurora_domain_is_valid(AURORA_DOMAIN_EMPTY));
    CHECK(aurora_domain_is_valid(AURORA_DOMAIN_ALL));
    CHECK(aurora_domain_is_singleton(AURORA_DOMAIN_2));
    CHECK(!aurora_domain_is_singleton(AURORA_DOMAIN_ALL));
    CHECK(closed_two != unknown);
    CHECK(aurora_domain_size(AURORA_DOMAIN_ALL) == 3u);
    CHECK(aurora_domain_size(AURORA_DOMAIN_0 | AURORA_DOMAIN_2) == 2u);
    CHECK(aurora_domain_intersection(AURORA_DOMAIN_0 | AURORA_DOMAIN_2,
                                     AURORA_DOMAIN_1 | AURORA_DOMAIN_2) ==
          AURORA_DOMAIN_2);
    volatile uint8_t closed_state = (uint8_t)AURORA_CLOSED;
    volatile uint8_t incoherent_state = (uint8_t)AURORA_CONTRADICTION;
    volatile uint8_t ambiguous_state = (uint8_t)AURORA_OPEN;
    CHECK(closed_state == 0u);
    CHECK(incoherent_state == 1u);
    CHECK(ambiguous_state == 2u);
    CHECK(aurora_e_from_state(AURORA_CLOSED) == AURORA_DOMAIN_0);
    CHECK(aurora_e_from_state(AURORA_CONTRADICTION) == AURORA_DOMAIN_1);
    CHECK(aurora_e_from_state(AURORA_OPEN) == AURORA_DOMAIN_2);
    for (uint8_t value = 0u; value < 3u; ++value) {
        aurora_domain singleton = aurora_domain_singleton(value);
        CHECK(aurora_domain_is_singleton(singleton));
        CHECK(aurora_domain_contains(singleton, value));
    }
}

static void test_trigate_order_packets(void) {
    int coherent = 0, ambiguous = 0, incoherent = 0;
    for (uint8_t a = 0u; a < 3u; ++a) {
        for (uint8_t b = 0u; b < 3u; ++b) {
            for (uint8_t m = 0u; m < 3u; ++m) {
                aurora_trigate_packet packet = aurora_trigate_order_packet(
                    aurora_domain_singleton(a), aurora_domain_singleton(b),
                    aurora_domain_singleton(m));
                CHECK(packet.r == aurora_domain_singleton(
                                      expected_majority(a, b, m)));
                CHECK(packet.e == aurora_e_from_state(packet.state));
                if (packet.state == AURORA_CLOSED) {
                    ++coherent;
                    CHECK(aurora_domain_is_singleton(packet.o));
                } else if (packet.state == AURORA_OPEN) {
                    ++ambiguous;
                    CHECK(aurora_domain_is_singleton(packet.o));
                    CHECK(aurora_domain_size(packet.o_candidates) > 1u);
                } else {
                    ++incoherent;
                    CHECK(packet.o == AURORA_DOMAIN_EMPTY);
                }
            }
        }
    }
    CHECK(coherent == 16);
    CHECK(ambiguous == 9);
    CHECK(incoherent == 2);
    aurora_trigate_packet homogeneous = aurora_trigate_order_packet(
        AURORA_DOMAIN_2, AURORA_DOMAIN_2, AURORA_DOMAIN_2);
    CHECK(homogeneous.o == AURORA_DOMAIN_2);
    CHECK(homogeneous.o_candidates == AURORA_DOMAIN_ALL);

    CHECK(aurora_trigate_residual(
              AURORA_DOMAIN_2, AURORA_DOMAIN_2, AURORA_DOMAIN_0,
              AURORA_DOMAIN_2, AURORA_INFER_R, AURORA_CLOSED) ==
          AURORA_DOMAIN_0);
    CHECK(aurora_trigate_residual(
              AURORA_DOMAIN_2, AURORA_DOMAIN_2, AURORA_DOMAIN_1,
              AURORA_DOMAIN_2, AURORA_INFER_R, AURORA_CLOSED) ==
          AURORA_DOMAIN_1);
    CHECK(aurora_trigate_residual(
              AURORA_DOMAIN_2, AURORA_DOMAIN_2, AURORA_DOMAIN_2,
              AURORA_DOMAIN_2, AURORA_INFER_R, AURORA_CLOSED) ==
          AURORA_DOMAIN_2);
    CHECK(aurora_trigate_residual(
              AURORA_DOMAIN_0, AURORA_DOMAIN_1, AURORA_DOMAIN_2,
              AURORA_DOMAIN_2, AURORA_INFER_R, AURORA_CLOSED) ==
          AURORA_DOMAIN_2);

    aurora_trigate_resolution inference = aurora_trigate_resolve(
        AURORA_DOMAIN_0, AURORA_DOMAIN_1, AURORA_DOMAIN_1,
        AURORA_DOMAIN_ALL, AURORA_INFER_R);
    CHECK(inference.packet.e == AURORA_DOMAIN_0);
    CHECK(inference.result == AURORA_DOMAIN_1);

    aurora_trigate_resolution deduction = aurora_trigate_resolve(
        AURORA_DOMAIN_0, AURORA_DOMAIN_ALL, AURORA_DOMAIN_1,
        AURORA_DOMAIN_0, AURORA_DEDUCE_B);
    CHECK(deduction.packet.e == AURORA_DOMAIN_0);
    CHECK(deduction.b == AURORA_DOMAIN_0);

    aurora_trigate_resolution learning = aurora_trigate_resolve(
        AURORA_DOMAIN_0, AURORA_DOMAIN_1, AURORA_DOMAIN_ALL,
        AURORA_DOMAIN_0, AURORA_LEARN_M);
    CHECK(learning.packet.e == AURORA_DOMAIN_0);
    CHECK(learning.mode == AURORA_DOMAIN_0);

    aurora_trigate_resolution unresolved = aurora_trigate_resolve(
        AURORA_DOMAIN_0, AURORA_DOMAIN_ALL, AURORA_DOMAIN_0,
        AURORA_DOMAIN_0, AURORA_DEDUCE_B);
    CHECK(unresolved.packet.e == AURORA_DOMAIN_2);
    CHECK(unresolved.b == AURORA_DOMAIN_ALL);

    aurora_trigate_resolution contradiction = aurora_trigate_resolve(
        AURORA_DOMAIN_1, AURORA_DOMAIN_1, AURORA_DOMAIN_0,
        AURORA_DOMAIN_0, AURORA_INFER_R);
    CHECK(contradiction.packet.e == AURORA_DOMAIN_1);
    CHECK(contradiction.packet.state == AURORA_CONTRADICTION);
}

static void test_directional_e_table(void) {
    size_t concrete_cases = 0u;
    for (uint8_t direction = 0u; direction < 3u; ++direction) {
        for (uint8_t a = 0u; a < 3u; ++a) {
            for (uint8_t b = 0u; b < 3u; ++b) {
                for (uint8_t m = 0u; m < 3u; ++m) {
                    for (uint8_t r = 0u; r < 3u; ++r) {
                        aurora_trigate_resolution resolution =
                            aurora_trigate_resolve(
                                aurora_domain_singleton(a),
                                aurora_domain_singleton(b),
                                aurora_domain_singleton(m),
                                aurora_domain_singleton(r),
                                (aurora_direction)direction);
                        bool satisfies = expected_majority(a, b, m) == r;
                        if (!satisfies) {
                            CHECK(resolution.packet.state ==
                                  AURORA_CONTRADICTION);
                            CHECK(resolution.packet.e == AURORA_DOMAIN_1);
                        } else if (r == 2u) {
                            CHECK(resolution.packet.state == AURORA_OPEN);
                            CHECK(resolution.packet.e ==
                                  aurora_domain_singleton(
                                      expected_r2_residual(a, b, m)));
                        } else {
                            CHECK(resolution.packet.state == AURORA_CLOSED);
                            CHECK(resolution.packet.e == AURORA_DOMAIN_0);
                        }
                        ++concrete_cases;
                    }
                }
            }
        }
    }
    CHECK(concrete_cases == 243u);

    size_t open_target_cases = 0u;
    for (uint8_t direction = 0u; direction < 3u; ++direction) {
        for (uint8_t a = 0u; a < 3u; ++a) {
            for (uint8_t b = 0u; b < 3u; ++b) {
                for (uint8_t m = 0u; m < 3u; ++m) {
                    for (uint8_t r = 0u; r < 3u; ++r) {
                        aurora_domain ad = aurora_domain_singleton(a);
                        aurora_domain bd = aurora_domain_singleton(b);
                        aurora_domain md = aurora_domain_singleton(m);
                        aurora_domain rd = aurora_domain_singleton(r);
                        if (direction == AURORA_DEDUCE_B) {
                            bd = AURORA_DOMAIN_ALL;
                        } else if (direction == AURORA_INFER_R) {
                            rd = AURORA_DOMAIN_ALL;
                        } else {
                            md = AURORA_DOMAIN_ALL;
                        }
                        aurora_trigate_resolution resolution =
                            aurora_trigate_resolve(
                                ad, bd, md, rd,
                                (aurora_direction)direction);

                        size_t solutions = 0u;
                        uint8_t solution = 3u;
                        for (uint8_t value = 0u; value < 3u; ++value) {
                            uint8_t av = a, bv = b, mv = m, rv = r;
                            if (direction == AURORA_DEDUCE_B) bv = value;
                            if (direction == AURORA_INFER_R) rv = value;
                            if (direction == AURORA_LEARN_M) mv = value;
                            if (expected_majority(av, bv, mv) == rv) {
                                ++solutions;
                                solution = value;
                            }
                        }
                        if (solutions == 0u) {
                            CHECK(resolution.packet.e == AURORA_DOMAIN_1);
                            CHECK(resolution.packet.state ==
                                  AURORA_CONTRADICTION);
                        } else if (solutions > 1u) {
                            CHECK(resolution.packet.e == AURORA_DOMAIN_2);
                            CHECK(resolution.packet.state == AURORA_OPEN);
                        } else {
                            aurora_domain target =
                                direction == AURORA_DEDUCE_B
                                    ? resolution.b
                                    : direction == AURORA_INFER_R
                                          ? resolution.result
                                          : resolution.mode;
                            CHECK(target == aurora_domain_singleton(solution));
                            if (resolution.result == AURORA_DOMAIN_2) {
                                CHECK(resolution.packet.state == AURORA_OPEN);
                            } else {
                                CHECK(resolution.packet.e == AURORA_DOMAIN_0);
                                CHECK(resolution.packet.state == AURORA_CLOSED);
                            }
                        }
                        ++open_target_cases;
                    }
                }
            }
        }
    }
    CHECK(open_target_cases == 243u);
}

static void test_do_fibonacci_walk(void) {
    aurora_do_cursor cursor;
    aurora_do_cursor_init(&cursor);
    uint32_t seen = 0u;
    for (size_t attempt = 0u; attempt < AURORA_DO_STATE_COUNT; ++attempt) {
        CHECK(aurora_do_cursor_next(&cursor));
        size_t encoded = attempt * AURORA_DO_FIBONACCI_STRIDE %
                         AURORA_DO_STATE_COUNT;
        uint32_t bit = UINT32_C(1) << encoded;
        CHECK((seen & bit) == 0u);
        seen |= bit;
        CHECK(cursor.state[0] ==
              aurora_domain_singleton((uint8_t)(encoded / 9u)));
        CHECK(cursor.state[1] == aurora_domain_singleton(
                                      (uint8_t)((encoded / 3u) % 3u)));
        CHECK(cursor.state[2] ==
              aurora_domain_singleton((uint8_t)(encoded % 3u)));
    }
    CHECK(seen == UINT32_C(0x07ffffff));
    CHECK(cursor.exhausted);
    CHECK(!aurora_do_cursor_next(&cursor));

    aurora_domain ambiguous[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                                  AURORA_DOMAIN_0};
    aurora_ordering phase_zero = aurora_order_triplet_at_phase(
        ambiguous, AURORA_DOMAIN_0);
    aurora_ordering phase_two = aurora_order_triplet_at_phase(
        ambiguous, AURORA_DOMAIN_2);
    CHECK(phase_zero.orientation == AURORA_DOMAIN_1);
    CHECK(phase_two.orientation == AURORA_DOMAIN_2);
    CHECK(phase_zero.alternatives == phase_two.alternatives);
}

static void test_truth_table(void) {
    for (uint8_t a = 0u; a < 3u; ++a) {
        for (uint8_t b = 0u; b < 3u; ++b) {
            for (uint8_t m = 0u; m < 3u; ++m) {
                CHECK(aurora_majority3(a, b, m) ==
                      expected_majority(a, b, m));
                CHECK(aurora_majority3(a, b, m) ==
                      aurora_majority3(b, a, m));
            }
        }
    }
}

static void test_forward_domains(void) {
    for (aurora_domain a = 1u; a <= AURORA_DOMAIN_ALL; ++a) {
        for (aurora_domain b = 1u; b <= AURORA_DOMAIN_ALL; ++b) {
            for (aurora_domain m = 1u; m <= AURORA_DOMAIN_ALL; ++m) {
                aurora_domain expected = AURORA_DOMAIN_EMPTY;
                for (uint8_t av = 0u; av < 3u; ++av) {
                    for (uint8_t bv = 0u; bv < 3u; ++bv) {
                        for (uint8_t mv = 0u; mv < 3u; ++mv) {
                            if (aurora_domain_contains(a, av) &&
                                aurora_domain_contains(b, bv) &&
                                aurora_domain_contains(m, mv)) {
                                expected |= aurora_domain_singleton(
                                    expected_majority(av, bv, mv));
                            }
                        }
                    }
                }
                CHECK(aurora_majority3_domain(a, b, m) == expected);
            }
        }
    }
}

static void test_inverse_and_contradiction(void) {
    aurora_domain a = AURORA_DOMAIN_ALL;
    aurora_domain b = AURORA_DOMAIN_ALL;
    aurora_domain m = AURORA_DOMAIN_1;
    aurora_domain r = AURORA_DOMAIN_0;
    CHECK(aurora_trigate_propagate(&a, &b, &m, &r) == AURORA_CLOSED);
    CHECK(a == AURORA_DOMAIN_0);
    CHECK(b == AURORA_DOMAIN_0);

    a = b = AURORA_DOMAIN_1;
    m = AURORA_DOMAIN_0;
    r = AURORA_DOMAIN_0;
    CHECK(aurora_trigate_propagate(&a, &b, &m, &r) ==
          AURORA_CONTRADICTION);
    CHECK(a == AURORA_DOMAIN_EMPTY && b == AURORA_DOMAIN_EMPTY &&
          m == AURORA_DOMAIN_EMPTY && r == AURORA_DOMAIN_EMPTY);
}

static void test_exhaustive_propagation(void) {
    int cases = 0;
    for (aurora_domain ai = 1u; ai <= AURORA_DOMAIN_ALL; ++ai) {
        for (aurora_domain bi = 1u; bi <= AURORA_DOMAIN_ALL; ++bi) {
            for (aurora_domain mi = 1u; mi <= AURORA_DOMAIN_ALL; ++mi) {
                for (aurora_domain ri = 1u; ri <= AURORA_DOMAIN_ALL; ++ri) {
                    aurora_domain a = ai, b = bi, m = mi, r = ri;
                    aurora_domain expected_a = AURORA_DOMAIN_EMPTY;
                    aurora_domain expected_b = AURORA_DOMAIN_EMPTY;
                    aurora_domain expected_m = AURORA_DOMAIN_EMPTY;
                    aurora_domain expected_r = AURORA_DOMAIN_EMPTY;
                    for (uint8_t av = 0u; av < 3u; ++av) {
                        for (uint8_t bv = 0u; bv < 3u; ++bv) {
                            for (uint8_t mv = 0u; mv < 3u; ++mv) {
                                uint8_t rv = expected_majority(av, bv, mv);
                                if (aurora_domain_contains(ai, av) &&
                                    aurora_domain_contains(bi, bv) &&
                                    aurora_domain_contains(mi, mv) &&
                                    aurora_domain_contains(ri, rv)) {
                                    expected_a |= aurora_domain_singleton(av);
                                    expected_b |= aurora_domain_singleton(bv);
                                    expected_m |= aurora_domain_singleton(mv);
                                    expected_r |= aurora_domain_singleton(rv);
                                }
                            }
                        }
                    }
                    aurora_state state =
                        aurora_trigate_propagate(&a, &b, &m, &r);
                    ++cases;

                    if (state == AURORA_CONTRADICTION) {
                        CHECK(expected_a == AURORA_DOMAIN_EMPTY);
                        CHECK(a == AURORA_DOMAIN_EMPTY);
                        continue;
                    }
                    CHECK(a == expected_a && b == expected_b &&
                          m == expected_m && r == expected_r);
                    CHECK((a & (aurora_domain)~ai) == 0u);
                    CHECK((b & (aurora_domain)~bi) == 0u);
                    CHECK((m & (aurora_domain)~mi) == 0u);
                    CHECK((r & (aurora_domain)~ri) == 0u);

                    aurora_domain a2 = a, b2 = b, m2 = m, r2 = r;
                    aurora_state state2 =
                        aurora_trigate_propagate(&a2, &b2, &m2, &r2);
                    CHECK(state2 == state);
                    CHECK(a2 == a && b2 == b && m2 == m && r2 == r);
                }
            }
        }
    }
    CHECK(cases == 2401);
}

static void test_ordering_census(void) {
    int closed = 0, open = 0, contradiction = 0;
    for (uint8_t a = 0u; a < 3u; ++a) {
        for (uint8_t b = 0u; b < 3u; ++b) {
            for (uint8_t c = 0u; c < 3u; ++c) {
                aurora_domain triplet[3] = {
                    aurora_domain_singleton(a),
                    aurora_domain_singleton(b),
                    aurora_domain_singleton(c)};
                aurora_ordering ordering = aurora_order_triplet(triplet);
                if (ordering.state == AURORA_CLOSED) {
                    ++closed;
                    CHECK(ordering.e == AURORA_DOMAIN_0);
                    CHECK(aurora_domain_is_singleton(ordering.orientation));
                } else if (ordering.state == AURORA_OPEN) {
                    ++open;
                    CHECK(ordering.e == AURORA_DOMAIN_2);
                    CHECK(aurora_domain_is_singleton(ordering.orientation));
                    CHECK(aurora_domain_size(ordering.alternatives) > 1u);
                } else {
                    ++contradiction;
                    CHECK(ordering.e == AURORA_DOMAIN_1);
                    CHECK(ordering.orientation == AURORA_DOMAIN_EMPTY);
                    CHECK((a == 0u && b == 1u && c == 2u) ||
                          (a == 1u && b == 0u && c == 2u));
                }
            }
        }
    }
    CHECK(closed == 16);
    CHECK(open == 9);
    CHECK(contradiction == 2);
}

static void test_ordering_examples(void) {
    aurora_domain example[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                                AURORA_DOMAIN_1};
    aurora_ordering ordering = aurora_order_triplet(example);
    CHECK(ordering.state == AURORA_CLOSED);
    CHECK(ordering.orientation == AURORA_DOMAIN_1); /* índice 1 */
    CHECK(ordering.es == AURORA_DOMAIN_0);
    CHECK(ordering.fn == AURORA_DOMAIN_0);
    CHECK(ordering.fo == AURORA_DOMAIN_1);

    aurora_domain ambiguous[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                                  AURORA_DOMAIN_0};
    ordering = aurora_order_triplet(ambiguous);
    CHECK(ordering.state == AURORA_OPEN);
    CHECK(ordering.orientation == AURORA_DOMAIN_1);
    CHECK(ordering.alternatives ==
          (AURORA_DOMAIN_1 | AURORA_DOMAIN_2));

    aurora_domain unknown[3] = {AURORA_DOMAIN_ALL, AURORA_DOMAIN_ALL,
                                AURORA_DOMAIN_ALL};
    ordering = aurora_order_triplet(unknown);
    CHECK(ordering.state == AURORA_OPEN);
    CHECK(ordering.orientation == AURORA_DOMAIN_0);
    CHECK(ordering.alternatives == AURORA_DOMAIN_ALL);
    CHECK(ordering.es == AURORA_DOMAIN_ALL);
    CHECK(ordering.fn == AURORA_DOMAIN_ALL);
    CHECK(ordering.fo == AURORA_DOMAIN_ALL);
}

static void build_circular_pair(aurora_network *network, bool reverse) {
    aurora_network_init(network);
    int fo_a = aurora_network_add_cell(network, AURORA_DOMAIN_0);
    int fo_b = aurora_network_add_cell(network, AURORA_DOMAIN_0);
    int fn_a = aurora_network_add_cell(network, AURORA_DOMAIN_1);
    int fn_b = aurora_network_add_cell(network, AURORA_DOMAIN_1);
    int link = aurora_network_add_cell(network, AURORA_DOMAIN_ALL);
    int out_fo = aurora_network_add_cell(network, AURORA_DOMAIN_ALL);
    int out_fn = aurora_network_add_cell(network, AURORA_DOMAIN_ALL);
    CHECK(fo_a == 0 && fo_b == 1 && fn_a == 2 && fn_b == 3 && link == 4 &&
          out_fo == 5 && out_fn == 6);
    if (!reverse) {
        CHECK(aurora_network_add_gate(network, 0u, 1u, 4u, 5u) >= 0);
        CHECK(aurora_network_add_gate(network, 2u, 3u, 6u, 4u) >= 0);
    } else {
        CHECK(aurora_network_add_gate(network, 2u, 3u, 6u, 4u) >= 0);
        CHECK(aurora_network_add_gate(network, 0u, 1u, 4u, 5u) >= 0);
    }
}

static void test_circular_network(void) {
    aurora_network forward, reverse;
    build_circular_pair(&forward, false);
    build_circular_pair(&reverse, true);
    CHECK(aurora_network_propagate(&forward) == AURORA_OPEN);
    CHECK(aurora_network_propagate(&reverse) == AURORA_OPEN);
    CHECK(forward.cell_count == reverse.cell_count);
    for (size_t index = 0u; index < forward.cell_count; ++index) {
        CHECK(forward.cells[index] == reverse.cells[index]);
    }
    CHECK(forward.cells[4] == AURORA_DOMAIN_1); /* FN.R = FO.M */
    CHECK(forward.cells[5] == AURORA_DOMAIN_0); /* forma destilada */
    CHECK(forward.cells[6] == AURORA_DOMAIN_ALL); /* función aún abierta */

    aurora_domain before[7];
    for (size_t index = 0u; index < 7u; ++index) {
        before[index] = forward.cells[index];
    }
    CHECK(aurora_network_propagate(&forward) == AURORA_OPEN);
    for (size_t index = 0u; index < 7u; ++index) {
        CHECK(before[index] == forward.cells[index]);
    }
}

static void test_minimum_operation_census(void) {
    uint8_t valid[25][3];
    size_t valid_count = 0u;
    for (uint8_t a = 0u; a < 3u; ++a) {
        for (uint8_t b = 0u; b < 3u; ++b) {
            for (uint8_t c = 0u; c < 3u; ++c) {
                aurora_domain triplet[3] = {
                    aurora_domain_singleton(a),
                    aurora_domain_singleton(b),
                    aurora_domain_singleton(c)};
                if (aurora_order_triplet(triplet).state !=
                    AURORA_CONTRADICTION) {
                    valid[valid_count][0] = a;
                    valid[valid_count][1] = b;
                    valid[valid_count][2] = c;
                    ++valid_count;
                }
            }
        }
    }
    CHECK(valid_count == 25u);

    size_t combinations = 0u;
    for (size_t i = 0u; i < valid_count; ++i) {
        for (size_t j = i + 1u; j < valid_count; ++j) {
            for (size_t k = j + 1u; k < valid_count; ++k) {
                aurora_domain triplets[3][3];
                size_t selected[3] = {i, j, k};
                for (size_t row = 0u; row < 3u; ++row) {
                    for (size_t column = 0u; column < 3u; ++column) {
                        triplets[row][column] = aurora_domain_singleton(
                            valid[selected[row]][column]);
                    }
                }
                aurora_operation operation =
                    aurora_operate_minimum(triplets[0], triplets[1],
                                           triplets[2]);
                if (operation.state != AURORA_CONTRADICTION) {
                    aurora_state expected_state = AURORA_CLOSED;
                    for (size_t dimension = 0u; dimension < 3u;
                         ++dimension) {
                        if (operation.ds[dimension] == AURORA_DOMAIN_2 ||
                            operation.de[dimension] != AURORA_DOMAIN_0) {
                            expected_state = AURORA_OPEN;
                        }
                    }
                    CHECK(operation.state == expected_state);
                    for (uint8_t position = 0u; position < 3u; ++position) {
                        if (aurora_domain_contains(operation.do_[0],
                                                   position)) {
                            CHECK(aurora_domain_intersection(
                                      operation.ds[0],
                                      operation.ordered[position].es) !=
                                  AURORA_DOMAIN_EMPTY);
                        }
                    }
                }
                ++combinations;
            }
        }
    }
    CHECK(combinations == 2300u);

    aurora_domain impossible[3][3] = {
        {AURORA_DOMAIN_0, AURORA_DOMAIN_1, AURORA_DOMAIN_2},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_1, AURORA_DOMAIN_1, AURORA_DOMAIN_0}};
    CHECK(aurora_operate_minimum(impossible[0], impossible[1], impossible[2])
              .state ==
          AURORA_CONTRADICTION);
}

static void test_minimum_operation_with_knowledge(void) {
    aurora_domain triplet[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                                AURORA_DOMAIN_1};
    aurora_domain compatible[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                                   AURORA_DOMAIN_1};
    aurora_operation operation = aurora_operate_minimum_with_knowledge(
        triplet, triplet, triplet, compatible);
    CHECK(operation.state != AURORA_CONTRADICTION);
    CHECK(operation.ds[0] == AURORA_DOMAIN_0);
    CHECK(operation.ds[1] == AURORA_DOMAIN_0);
    CHECK(operation.ds[2] == AURORA_DOMAIN_1);
    CHECK(operation.de[0] == AURORA_DOMAIN_0);
    CHECK(operation.de[2] == AURORA_DOMAIN_0);

    aurora_domain incompatible[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_ALL,
                                     AURORA_DOMAIN_0};
    CHECK(aurora_operate_minimum_with_knowledge(
              triplet, triplet, triplet, incompatible)
              .state == AURORA_CONTRADICTION);
}

static void test_operational_seed(void) {
    aurora_domain a_ds[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                             AURORA_DOMAIN_1};
    aurora_domain b_ds[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                             AURORA_DOMAIN_1};
    aurora_domain c_ds[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                             AURORA_DOMAIN_1};
    aurora_so a = aurora_so_seed(a_ds);
    aurora_so b = aurora_so_seed(b_ds);
    aurora_so c = aurora_so_seed(c_ds);
    CHECK(a.state != AURORA_CONTRADICTION);
    CHECK(b.state != AURORA_CONTRADICTION);
    CHECK(c.state != AURORA_CONTRADICTION);

    aurora_operation relation = aurora_operate_minimum(
        a.upper_ds, b.upper_ds, c.upper_ds);
    aurora_so emergent = aurora_so_relate(&a, &b, &c);
    CHECK(emergent.state != AURORA_CONTRADICTION);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(emergent.ds[dimension] == relation.ds[dimension]);
        CHECK(emergent.de[dimension] == relation.de[dimension]);
        CHECK(emergent.do_[dimension] == relation.do_[dimension]);
    }
    aurora_operation summary = aurora_operate_minimum(
        emergent.ds, emergent.de, emergent.do_);
    CHECK(summary.state != AURORA_CONTRADICTION);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(emergent.upper_ds[dimension] == summary.ds[dimension]);
    }
}

static void test_fractal_tensor(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain leaf_value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                                   AURORA_DOMAIN_1};
    size_t leaves[9];
    for (size_t index = 0u; index < 9u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, leaf_value);
        CHECK(leaf >= 0);
        leaves[index] = (size_t)leaf;
        const aurora_tensor_node *node = aurora_tensor_get(&tensor, leaves[index]);
        CHECK(node != NULL && node->is_leaf);
        CHECK(node->depth == 0u && node->provenance_depth == 0u &&
              node->leaf_count == 1u);
        CHECK(node->children[0] == AURORA_TENSOR_NO_CHILD);
        CHECK(!node->has_window_orientation);
        CHECK(node->window_orientation[0] == AURORA_DOMAIN_EMPTY);
    }

    size_t level_one[3];
    for (size_t group = 0u; group < 3u; ++group) {
        int composite = aurora_tensor_compose(
            &tensor, leaves[group * 3u], leaves[group * 3u + 1u],
            leaves[group * 3u + 2u]);
        CHECK(composite >= 0);
        level_one[group] = (size_t)composite;
        const aurora_tensor_node *node =
            aurora_tensor_get(&tensor, level_one[group]);
        CHECK(node != NULL && !node->is_leaf);
        CHECK(node->depth == 1u && node->provenance_depth == 1u &&
              node->leaf_count == 3u);
        CHECK(node->has_window_orientation);
        CHECK(node->children[0] == leaves[group * 3u]);
        CHECK(node->children[1] == leaves[group * 3u + 1u]);
        CHECK(node->children[2] == leaves[group * 3u + 2u]);
    }

    aurora_so expected = aurora_so_relate(
        &tensor.nodes[level_one[0]].superior,
        &tensor.nodes[level_one[1]].superior,
        &tensor.nodes[level_one[2]].superior);
    int root_index = aurora_tensor_compose(
        &tensor, level_one[0], level_one[1], level_one[2]);
    CHECK(root_index >= 0);
    const aurora_tensor_node *root =
        aurora_tensor_get(&tensor, (size_t)root_index);
    CHECK(root != NULL && root->depth == 2u &&
          root->provenance_depth == 2u && root->leaf_count == 9u);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(root->superior.upper_ds[dimension] ==
              expected.upper_ds[dimension]);
        CHECK(root->superior.ds[dimension] == expected.ds[dimension]);
        CHECK(root->superior.de[dimension] == expected.de[dimension]);
        CHECK(root->superior.do_[dimension] == expected.do_[dimension]);
        CHECK(root->children[dimension] == level_one[dimension]);
    }

    size_t count_before = tensor.count;
    CHECK(aurora_tensor_compose(&tensor, leaves[0], level_one[0],
                                level_one[1]) < 0);
    CHECK(tensor.count == count_before); /* niveles mezclados rechazados */

    aurora_domain impossible[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_1,
                                   AURORA_DOMAIN_2};
    CHECK(aurora_tensor_add_leaf(&tensor, impossible) < 0);
    CHECK(tensor.count == count_before);
}

static void test_window_state(void) {
    aurora_domain closed[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                               AURORA_DOMAIN_0};
    aurora_domain open[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_2,
                             AURORA_DOMAIN_0};
    aurora_domain plural[3] = {AURORA_DOMAIN_0 | AURORA_DOMAIN_2,
                               AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    aurora_domain contradiction[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_1,
                                      AURORA_DOMAIN_2};
    CHECK(aurora_window_state_from_de(closed) == AURORA_CLOSED);
    CHECK(aurora_window_state_from_de(open) == AURORA_OPEN);
    CHECK(aurora_window_state_from_de(plural) == AURORA_OPEN);
    CHECK(aurora_window_state_from_de(contradiction) ==
          AURORA_CONTRADICTION);
}

static void test_atomic_window(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    int a = aurora_tensor_add_leaf(&tensor, value);
    int b = aurora_tensor_add_leaf(&tensor, value);
    int c = aurora_tensor_add_leaf(&tensor, value);
    CHECK(a >= 0 && b >= 0 && c >= 0);
    size_t before = tensor.count;
    aurora_window_result result = aurora_window_process(
        &tensor, (size_t)a, (size_t)b, (size_t)c);
    CHECK(result.state == AURORA_OPEN);
    CHECK(result.action == AURORA_WINDOW_CARRY);
    CHECK(result.output >= 0);
    CHECK(tensor.count == before + 1u);
    const aurora_tensor_node *carry =
        aurora_tensor_get(&tensor, (size_t)result.output);
    CHECK(carry != NULL && carry->depth == 0u &&
          carry->provenance_depth == 1u && carry->leaf_count == 3u);
    CHECK(carry->has_window_orientation);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(carry->superior.upper_ds[dimension] ==
              result.relation.upper_ds[dimension]);
        CHECK(carry->superior.de[dimension] == result.relation.de[dimension]);
        CHECK(result.u_orientation[dimension] == AURORA_DOMAIN_0);
        CHECK(carry->window_orientation[dimension] ==
              result.u_orientation[dimension]);
    }

    before = tensor.count;
    result = aurora_window_process(&tensor, (size_t)result.output,
                                   (size_t)a, (size_t)b);
    CHECK(result.state == AURORA_OPEN);
    CHECK(result.action == AURORA_WINDOW_CARRY);
    CHECK(result.output >= 0 && tensor.count == before + 1u);

    aurora_domain oriented[3] = {AURORA_DOMAIN_2, AURORA_DOMAIN_1,
                                 AURORA_DOMAIN_0};
    before = tensor.count;
    result = aurora_window_process_at_do(
        &tensor, (size_t)a, (size_t)b, (size_t)c, oriented);
    CHECK(result.state != AURORA_CONTRADICTION);
    CHECK(result.output >= 0 && tensor.count == before + 1u);
    const aurora_tensor_node *oriented_u =
        aurora_tensor_get(&tensor, (size_t)result.output);
    CHECK(oriented_u != NULL && oriented_u->has_window_orientation);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(result.u_orientation[dimension] == oriented[dimension]);
        CHECK(oriented_u->window_orientation[dimension] ==
              oriented[dimension]);
    }
    aurora_window_result inherited = aurora_window_evaluate(
        &tensor, (size_t)result.output, (size_t)a, (size_t)b);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(inherited.u_orientation[dimension] == oriented[dimension]);
    }

    aurora_tensor broken;
    aurora_tensor_init(&broken);
    a = aurora_tensor_add_leaf(&broken, value);
    b = aurora_tensor_add_leaf(&broken, value);
    c = aurora_tensor_add_leaf(&broken, value);
    CHECK(a >= 0 && b >= 0 && c >= 0);
    broken.nodes[a].superior.upper_ds[0] = AURORA_DOMAIN_0;
    broken.nodes[a].superior.upper_ds[1] = AURORA_DOMAIN_1;
    broken.nodes[a].superior.upper_ds[2] = AURORA_DOMAIN_2;
    before = broken.count;
    result = aurora_window_process(&broken, (size_t)a, (size_t)b, (size_t)c);
    CHECK(result.state == AURORA_CONTRADICTION);
    CHECK(result.action == AURORA_WINDOW_SLIDE);
    CHECK(result.output < 0 && broken.count == before);
}

static void test_complete_sequence(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    size_t input[8];
    for (size_t index = 0u; index < 8u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, value);
        CHECK(leaf >= 0);
        input[index] = (size_t)leaf;
    }
    aurora_sequence_result result =
        aurora_sequence_run_level(&tensor, input, 8u);
    CHECK(result.status == AURORA_SEQUENCE_PENDING);
    CHECK(result.windows == 3u);
    CHECK(result.carries == 3u);
    CHECK(result.emergences == 0u && result.slides == 0u);
    CHECK(result.input_consumed == 7u);
    CHECK(result.pending_count == 2u);
    const aurora_tensor_node *carry =
        aurora_tensor_get(&tensor, result.pending[0]);
    CHECK(carry != NULL && carry->depth == 0u);
    CHECK(carry->provenance_depth == 3u);
    CHECK(carry->leaf_count == 7u);
    CHECK(result.pending[1] == input[7]);

    aurora_sequence_result empty =
        aurora_sequence_run_level(&tensor, NULL, 0u);
    CHECK(empty.status == AURORA_SEQUENCE_COMPLETE);
    CHECK(empty.pending_count == 0u && empty.windows == 0u);
}

static void test_sequence_sliding(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    size_t input[4];
    for (size_t index = 0u; index < 4u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, value);
        CHECK(leaf >= 0);
        input[index] = (size_t)leaf;
    }
    tensor.nodes[input[0]].superior.upper_ds[0] = AURORA_DOMAIN_0;
    tensor.nodes[input[0]].superior.upper_ds[1] = AURORA_DOMAIN_1;
    tensor.nodes[input[0]].superior.upper_ds[2] = AURORA_DOMAIN_2;
    aurora_sequence_result result =
        aurora_sequence_run_level(&tensor, input, 4u);
    CHECK(result.status == AURORA_SEQUENCE_PENDING);
    CHECK(result.windows == 2u);
    CHECK(result.slides == 1u && result.carries == 1u);
    CHECK(result.input_consumed == 4u);
    CHECK(result.pending_count == 1u);
    CHECK(aurora_tensor_get(&tensor, result.pending[0]) != NULL);

    int upper = aurora_tensor_compose(&tensor, input[1], input[2], input[3]);
    CHECK(upper >= 0);
    size_t mixed[3] = {input[1], input[2], (size_t)upper};
    size_t before = tensor.count;
    result = aurora_sequence_run_level(&tensor, mixed, 3u);
    CHECK(result.status == AURORA_SEQUENCE_INVALID);
    CHECK(tensor.count == before);
}

static void test_sequence_rejects_carry_without_losing_input(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    size_t input[6];
    for (size_t index = 0u; index < 6u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, value);
        CHECK(leaf >= 0);
        input[index] = (size_t)leaf;
    }
    tensor.nodes[input[3]].superior.upper_ds[0] = AURORA_DOMAIN_0;
    tensor.nodes[input[3]].superior.upper_ds[1] = AURORA_DOMAIN_1;
    tensor.nodes[input[3]].superior.upper_ds[2] = AURORA_DOMAIN_2;

    aurora_sequence_result result =
        aurora_sequence_run_level(&tensor, input, 6u);
    CHECK(result.status == AURORA_SEQUENCE_PENDING);
    CHECK(result.windows == 3u);
    CHECK(result.carries == 1u && result.slides == 2u);
    CHECK(result.input_consumed == 4u);
    CHECK(result.pending_count == 2u);
    CHECK(result.pending[0] == input[4]);
    CHECK(result.pending[1] == input[5]);
}

static void test_recursive_hierarchy_driver(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    size_t input[8];
    for (size_t index = 0u; index < 8u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, value);
        CHECK(leaf >= 0);
        input[index] = (size_t)leaf;
    }
    aurora_hierarchy_result hierarchy =
        aurora_sequence_run_hierarchy(&tensor, input, 8u);
    CHECK(hierarchy.status == AURORA_SEQUENCE_PENDING);
    CHECK(hierarchy.levels == 1u);
    CHECK(hierarchy.windows == 3u && hierarchy.carries == 3u);
    CHECK(hierarchy.emergences == 0u && hierarchy.root_count == 0u);
    CHECK(hierarchy.pending_count == 2u);

    hierarchy = aurora_sequence_run_hierarchy(&tensor, input, 2u);
    CHECK(hierarchy.status == AURORA_SEQUENCE_COMPLETE);
    CHECK(hierarchy.levels == 0u && hierarchy.root_count == 2u);
    CHECK(hierarchy.roots[0] == input[0] && hierarchy.roots[1] == input[1]);
}

static void test_harmonizer(void) {
    const uint8_t target[3] = {0u, 1u, 2u};
    aurora_domain exact[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_1,
                              AURORA_DOMAIN_2};
    aurora_domain open[3] = {AURORA_DOMAIN_ALL, AURORA_DOMAIN_ALL,
                             AURORA_DOMAIN_ALL};
    aurora_domain far[3] = {AURORA_DOMAIN_1, AURORA_DOMAIN_1,
                            AURORA_DOMAIN_1};
    size_t distance = SIZE_MAX;
    CHECK(aurora_harmonize_face(exact, target, 1u, &distance) ==
          AURORA_DOMAIN_0);
    CHECK(distance == 0u);
    CHECK(aurora_harmonize_face(open, target, 1u, &distance) ==
          AURORA_DOMAIN_2);
    CHECK(distance == 0u); /* contiene el objetivo, pero no está cerrada */
    CHECK(aurora_harmonize_face(far, target, 1u, &distance) ==
          AURORA_DOMAIN_1);
    CHECK(distance == 2u);
    CHECK(aurora_harmonize_face(far, target, 2u, &distance) ==
          AURORA_DOMAIN_1);
}

static void test_c4_c5_resolution(void) {
    CHECK(aurora_executive_resolve_faces(
              AURORA_OPEN, AURORA_OPEN, AURORA_DOMAIN_0) ==
          AURORA_DEDUCE_B);
    CHECK(aurora_executive_resolve_faces(
              AURORA_OPEN, AURORA_OPEN, AURORA_DOMAIN_1) ==
          AURORA_INFER_R);
    CHECK(aurora_executive_resolve_faces(
              AURORA_OPEN, AURORA_OPEN, AURORA_DOMAIN_2) ==
          AURORA_LEARN_M);
    CHECK(aurora_executive_resolve_faces(
              AURORA_OPEN, AURORA_CLOSED, AURORA_DOMAIN_2) ==
          AURORA_INFER_R);
    CHECK(aurora_executive_resolve_faces(
              AURORA_OPEN, AURORA_CONTRADICTION, AURORA_DOMAIN_0) ==
          AURORA_LEARN_M);
    CHECK(aurora_executive_resolve_faces(
              AURORA_CLOSED, AURORA_CONTRADICTION, AURORA_DOMAIN_1) ==
          AURORA_LEARN_M);
    CHECK(aurora_executive_resolve_faces(
              AURORA_CONTRADICTION, AURORA_OPEN, AURORA_DOMAIN_1) ==
          AURORA_DEDUCE_B);
}

static void test_executive_seed(void) {
    aurora_domain a_ds[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                             AURORA_DOMAIN_1};
    aurora_domain b_ds[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                             AURORA_DOMAIN_1};
    aurora_domain c_ds[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                             AURORA_DOMAIN_1};
    aurora_so input = aurora_so_seed(a_ds);
    aurora_so knowledge = aurora_so_seed(b_ds);
    aurora_so output = aurora_so_seed(c_ds);
    aurora_harmonizer harmonizer = {1u, 1u, 1u};
    aurora_executive executive = aurora_executive_decide(
        &input, &knowledge, &output, &harmonizer);

    const aurora_domain *channels[3][3] = {
        {input.ds, knowledge.ds, output.ds},
        {input.de, knowledge.de, output.de},
        {input.do_, knowledge.do_, output.do_}};
    for (size_t face = 0u; face < 3u; ++face) {
        aurora_operation expected = aurora_operate_minimum(
            channels[face][0], channels[face][1], channels[face][2]);
        CHECK(expected.state != AURORA_CONTRADICTION);
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            CHECK(executive.faces[face][dimension] ==
                  expected.ds[dimension]);
            CHECK(executive.seed.upper_ds[dimension] ==
                  executive.deltas[dimension]);
        }
    }
    CHECK(executive.r == aurora_majority3_domain(
                              executive.deltas[0], executive.deltas[1],
                              executive.deltas[2]));
    aurora_ordering order = aurora_order_triplet_at_phase(
        executive.deltas, AURORA_DOMAIN_0);
    CHECK(executive.e == aurora_e_from_state(executive.state));
    CHECK(executive.o == order.orientation);

    aurora_executive repeated = aurora_executive_decide(
        &input, &knowledge, &output, &harmonizer);
    CHECK(repeated.r == executive.r && repeated.e == executive.e &&
          repeated.o == executive.o && repeated.state == executive.state);
}

static void test_emergent_dictionary(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    size_t leaves[8];
    for (size_t index = 0u; index < 8u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, value);
        CHECK(leaf >= 0);
        leaves[index] = (size_t)leaf;
    }
    int candidate_tensor =
        aurora_tensor_carry(&tensor, leaves[0], leaves[1], leaves[2]);
    CHECK(candidate_tensor >= 0);
    int used_output = aurora_tensor_carry(
        &tensor, (size_t)candidate_tensor, leaves[3], leaves[4]);
    CHECK(used_output >= 0);

    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);
    uint32_t short_key[2] = {10u, 20u};
    int short_entry = aurora_dictionary_propose(
        &dictionary, &tensor, short_key, 2u, (size_t)candidate_tensor);
    CHECK(short_entry >= 0);
    CHECK(dictionary.entries[short_entry].state == AURORA_ENTRY_CANDIDATE);
    CHECK(aurora_dictionary_lookup(&dictionary, short_key, 2u) < 0);
    CHECK(aurora_dictionary_knowledge(&dictionary, &tensor,
                                      (size_t)short_entry) == NULL);

    CHECK(!aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)short_entry, leaves[7],
        AURORA_LEARN_CRYSTALLIZE));
    CHECK(dictionary.entries[short_entry].state == AURORA_ENTRY_CANDIDATE);

    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)short_entry, (size_t)used_output,
        AURORA_LEARN_TEMPORARY));
    CHECK(dictionary.entries[short_entry].state == AURORA_ENTRY_CANDIDATE);
    CHECK(aurora_dictionary_lookup(&dictionary, short_key, 2u) < 0);

    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)short_entry, (size_t)used_output,
        AURORA_LEARN_CRYSTALLIZE));
    CHECK(dictionary.entries[short_entry].state ==
          AURORA_ENTRY_CRYSTALLIZED);
    CHECK(aurora_dictionary_lookup(&dictionary, short_key, 2u) == short_entry);
    const aurora_so *knowledge = aurora_dictionary_knowledge(
        &dictionary, &tensor, (size_t)short_entry);
    CHECK(knowledge == &tensor.nodes[candidate_tensor].superior);

    uint32_t long_key[3] = {10u, 20u, 30u};
    int long_entry = aurora_dictionary_propose(
        &dictionary, &tensor, long_key, 3u, (size_t)used_output);
    CHECK(long_entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)long_entry, (size_t)used_output,
        AURORA_LEARN_CRYSTALLIZE));
    uint32_t query[4] = {10u, 20u, 30u, 40u};
    CHECK(aurora_dictionary_lookup(&dictionary, query, 4u) == long_entry);

    int alternative_tensor =
        aurora_tensor_carry(&tensor, leaves[5], leaves[6], leaves[7]);
    CHECK(alternative_tensor >= 0);
    int alternative = aurora_dictionary_propose(
        &dictionary, &tensor, long_key, 3u, (size_t)alternative_tensor);
    CHECK(alternative >= 0 && alternative != long_entry);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)alternative,
        (size_t)alternative_tensor, AURORA_LEARN_CRYSTALLIZE));
    CHECK(aurora_dictionary_lookup(&dictionary, query, 4u) == alternative);

    uint32_t rejected_key[1] = {99u};
    int rejected = aurora_dictionary_propose(
        &dictionary, &tensor, rejected_key, 1u, leaves[0]);
    CHECK(rejected >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)rejected, leaves[0],
        AURORA_LEARN_NONE));
    CHECK(dictionary.entries[rejected].state == AURORA_ENTRY_REJECTED);
    CHECK(aurora_dictionary_lookup(&dictionary, rejected_key, 1u) < 0);

    aurora_executive decision = aurora_dictionary_decide(
        &dictionary, &tensor, (size_t)short_entry,
        &tensor.nodes[leaves[0]].superior,
        &tensor.nodes[used_output].superior, NULL);
    aurora_executive direct = aurora_executive_decide(
        &tensor.nodes[leaves[0]].superior, knowledge,
        &tensor.nodes[used_output].superior, NULL);
    CHECK(decision.r == direct.r && decision.e == direct.e &&
          decision.o == direct.o && decision.state == direct.state);
}

static void test_associative_dictionary(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain stored_values[3][3] = {
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_1, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_2, AURORA_DOMAIN_0, AURORA_DOMAIN_1}};
    size_t stored_tensors[3];
    for (size_t index = 0u; index < 3u; ++index) {
        int tensor_index = aurora_tensor_add_leaf(&tensor,
                                                  stored_values[index]);
        CHECK(tensor_index >= 0);
        stored_tensors[index] = (size_t)tensor_index;
    }

    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);
    size_t entries[3];
    for (size_t index = 0u; index < 3u; ++index) {
        uint32_t token[1] = {(uint32_t)(100u + index)};
        int entry = aurora_dictionary_propose(
            &dictionary, &tensor, token, 1u, stored_tensors[index]);
        CHECK(entry >= 0);
        entries[index] = (size_t)entry;
        CHECK(aurora_dictionary_used_in_output(
            &dictionary, &tensor, entries[index], stored_tensors[index],
            AURORA_LEARN_CRYSTALLIZE));
    }
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, entries[1], stored_tensors[1],
        AURORA_LEARN_TEMPORARY));

    aurora_domain open_value[3] = {AURORA_DOMAIN_2, AURORA_DOMAIN_0,
                                   AURORA_DOMAIN_1};
    int open_query = aurora_tensor_add_leaf(&tensor, open_value);
    CHECK(open_query >= 0);
    aurora_dictionary_matches open = aurora_dictionary_search_tensor(
        &dictionary, &tensor, (size_t)open_query);
    CHECK(open.count == 3u);
    CHECK(open.roots_visited == 3u);
    CHECK(open.entries[0] == entries[1]);

    aurora_dictionary_matches determined = aurora_dictionary_search_tensor(
        &dictionary, &tensor, stored_tensors[0]);
    CHECK(determined.count == 1u);
    CHECK(determined.entries[0] == entries[0]);

    int matching_root = aurora_tensor_carry(
        &tensor, stored_tensors[0], stored_tensors[0], stored_tensors[0]);
    int rejected_root = aurora_tensor_carry(
        &tensor, stored_tensors[0], stored_tensors[1], stored_tensors[0]);
    CHECK(matching_root >= 0 && rejected_root >= 0);
    tensor.nodes[rejected_root].superior =
        tensor.nodes[matching_root].superior; /* misma cima, distinto descenso */
    uint32_t matching_token[1] = {200u};
    uint32_t rejected_token[1] = {201u};
    int matching_entry = aurora_dictionary_propose(
        &dictionary, &tensor, matching_token, 1u, (size_t)matching_root);
    int rejected_entry = aurora_dictionary_propose(
        &dictionary, &tensor, rejected_token, 1u, (size_t)rejected_root);
    CHECK(matching_entry >= 0 && rejected_entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)matching_entry,
        (size_t)matching_root, AURORA_LEARN_CRYSTALLIZE));
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)rejected_entry,
        (size_t)rejected_root, AURORA_LEARN_CRYSTALLIZE));
    aurora_dictionary_matches descended = aurora_dictionary_search_tensor(
        &dictionary, &tensor, (size_t)matching_root);
    CHECK(descended.count == 1u);
    CHECK(descended.entries[0] == (size_t)matching_entry);
    CHECK(descended.branches_rejected >= 1u);

    CHECK(aurora_dictionary_reindex(&dictionary, &tensor));
    aurora_dictionary_matches rebuilt = aurora_dictionary_search_tensor(
        &dictionary, &tensor, (size_t)open_query);
    CHECK(rebuilt.count == open.count);
    CHECK(rebuilt.entries[0] == open.entries[0]);
}

static void test_dictionary_window_reecution(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    size_t leaves[25];
    size_t leaf_count = 0u;
    for (uint8_t a = 0u; a < 3u; ++a) {
        for (uint8_t b = 0u; b < 3u; ++b) {
            for (uint8_t m = 0u; m < 3u; ++m) {
                aurora_domain triplet[3] = {
                    aurora_domain_singleton(a),
                    aurora_domain_singleton(b),
                    aurora_domain_singleton(m)};
                int leaf = aurora_tensor_add_leaf(&tensor, triplet);
                if (leaf >= 0) leaves[leaf_count++] = (size_t)leaf;
            }
        }
    }
    CHECK(leaf_count == 25u);

    size_t selected[3] = {0u, 0u, 0u};
    bool found = false;
    for (size_t first = 0u; first < leaf_count && !found; ++first) {
        for (size_t second = 0u; second < leaf_count && !found; ++second) {
            for (size_t third = 0u; third < leaf_count && !found; ++third) {
                aurora_do_cursor cursor;
                aurora_do_cursor_init(&cursor);
                while (aurora_do_cursor_next(&cursor)) {
                    size_t before = tensor.count;
                    aurora_window_result evaluated =
                        aurora_window_evaluate_knowledge_at_do(
                            &tensor, leaves[first], leaves[second],
                            leaves[third], cursor.state);
                    CHECK(tensor.count == before);
                    if (evaluated.state == AURORA_CLOSED) {
                        selected[0] = leaves[first];
                        selected[1] = leaves[second];
                        selected[2] = leaves[third];
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    CHECK(found);

    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);
    uint32_t key[1] = {700u};
    int entry = aurora_dictionary_propose(
        &dictionary, &tensor, key, 1u, selected[2]);
    CHECK(entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)entry, selected[2],
        AURORA_LEARN_CRYSTALLIZE));

    size_t before = tensor.count;
    aurora_dictionary_window_result resolved =
        aurora_dictionary_reexecute_window(
            &dictionary, &tensor, selected[2], selected[0], selected[1]);
    CHECK(resolved.window.state == AURORA_CLOSED);
    CHECK(resolved.window.action == AURORA_WINDOW_EMERGE);
    CHECK(resolved.window.output >= 0);
    CHECK(resolved.selected_entry == entry);
    CHECK(!resolved.needs_alternative);
    CHECK(resolved.candidates_tried > 0u &&
          resolved.candidates_tried <= AURORA_DO_STATE_COUNT);
    CHECK(tensor.count == before + 1u);
    const aurora_tensor_node *resolved_u = aurora_tensor_get(
        &tensor, (size_t)resolved.window.output);
    CHECK(resolved_u != NULL && resolved_u->has_window_orientation);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(resolved.window.u_orientation[dimension] ==
              resolved.traversal.state[dimension]);
        CHECK(resolved_u->window_orientation[dimension] ==
              resolved.window.u_orientation[dimension]);
    }

    size_t after_resolution = tensor.count;
    aurora_dictionary_window_result exhausted =
        aurora_dictionary_reexecute_window(
            &dictionary, &tensor, selected[2],
            (size_t)resolved.window.output, selected[1]);
    CHECK(exhausted.selected_entry < 0);
    CHECK(exhausted.needs_alternative);
    CHECK(exhausted.exhausted);
    CHECK(exhausted.candidates_tried == AURORA_DO_STATE_COUNT);
    CHECK(tensor.count == after_resolution);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(exhausted.window.u_orientation[dimension] ==
              exhausted.traversal.state[dimension]);
    }
}

static void test_translation_and_backoff(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain values[4][3] = {
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_2, AURORA_DOMAIN_0}};
    uint32_t simple_tokens[4] = {10u, 20u, 30u, 40u};
    size_t simple_tensors[4];
    for (size_t index = 0u; index < 4u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, values[index]);
        CHECK(leaf >= 0);
        simple_tensors[index] = (size_t)leaf;
    }

    aurora_dictionary empty;
    aurora_dictionary_init(&empty);
    uint32_t phrase[3] = {10u, 20u, 30u};
    aurora_translation raw = aurora_translate(
        &empty, &tensor, phrase, 3u, simple_tokens, simple_tensors, 4u, true);
    CHECK(raw.status == AURORA_TRANSLATION_OK);
    CHECK(raw.count == 3u && raw.simple_hits == 3u);
    CHECK(raw.dictionary_hits == 0u && raw.windows == 1u);

    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);
    int complex = aurora_tensor_carry(
        &tensor, simple_tensors[0], simple_tensors[1], simple_tensors[2]);
    CHECK(complex >= 0);
    int long_entry = aurora_dictionary_propose(
        &dictionary, &tensor, phrase, 3u, (size_t)complex);
    CHECK(long_entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)long_entry, (size_t)complex,
        AURORA_LEARN_CRYSTALLIZE));

    aurora_translation known = aurora_translate(
        &dictionary, &tensor, phrase, 3u, simple_tokens, simple_tensors, 4u,
        true);
    CHECK(known.status == AURORA_TRANSLATION_OK);
    CHECK(known.count == 1u && known.tensors[0] == (size_t)complex);
    CHECK(known.dictionary_hits == 1u && known.simple_hits == 0u);
    CHECK(known.windows == 0u);

    uint32_t short_key[2] = {10u, 20u};
    int shorter = aurora_tensor_carry(
        &tensor, simple_tensors[0], simple_tensors[1], simple_tensors[2]);
    CHECK(shorter >= 0);
    int short_entry = aurora_dictionary_propose(
        &dictionary, &tensor, short_key, 2u, (size_t)shorter);
    CHECK(short_entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)short_entry, (size_t)shorter,
        AURORA_LEARN_CRYSTALLIZE));

    tensor.nodes[complex].superior.upper_ds[0] = AURORA_DOMAIN_0;
    tensor.nodes[complex].superior.upper_ds[1] = AURORA_DOMAIN_1;
    tensor.nodes[complex].superior.upper_ds[2] = AURORA_DOMAIN_2;
    aurora_translation backed_off = aurora_translate(
        &dictionary, &tensor, phrase, 3u, simple_tokens, simple_tensors, 4u,
        true);
    CHECK(backed_off.status == AURORA_TRANSLATION_OK);
    CHECK(backed_off.backoffs == 1u);
    CHECK(backed_off.count == 2u);
    CHECK(backed_off.tensors[0] == (size_t)shorter);
    CHECK(backed_off.tensors[1] == simple_tensors[2]);
    CHECK(backed_off.dictionary_hits == 1u && backed_off.simple_hits == 1u);

    uint32_t unknown_token[1] = {999u};
    aurora_translation unknown = aurora_translate(
        &dictionary, &tensor, unknown_token, 1u, simple_tokens,
        simple_tensors, 4u, true);
    CHECK(unknown.status == AURORA_TRANSLATION_UNKNOWN);
}

static void test_bootstrap(void) {
    aurora_so empty = aurora_so_empty();
    CHECK(aurora_so_is_empty(&empty));
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    aurora_so input = aurora_so_seed(value);
    aurora_bootstrap_result boot = aurora_bootstrap_knowledge(
        &input, &empty, &input, AURORA_OPEN);
    CHECK(boot.written);
    CHECK(!aurora_so_is_empty(&boot.knowledge));
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(boot.knowledge.upper_ds[dimension] == input.upper_ds[dimension]);
    }
    aurora_bootstrap_result fixed = aurora_bootstrap_knowledge(
        &input, &boot.knowledge, &input, AURORA_OPEN);
    CHECK(!fixed.written);
    aurora_bootstrap_result closed = aurora_bootstrap_knowledge(
        &input, &empty, &input, AURORA_CLOSED);
    CHECK(!closed.written && aurora_so_is_empty(&closed.knowledge));

    aurora_so closed_two = empty;
    closed_two.upper_ds[0] = AURORA_DOMAIN_2;
    CHECK(!aurora_so_is_empty(&closed_two));
}

static void test_dictionary_eviction(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain value[3] = {AURORA_DOMAIN_0, AURORA_DOMAIN_0,
                              AURORA_DOMAIN_1};
    int leaf = aurora_tensor_add_leaf(&tensor, value);
    CHECK(leaf >= 0);
    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);
    for (uint32_t token = 0u; token < AURORA_DICTIONARY_MAX_ENTRIES; ++token) {
        uint32_t key[1] = {1000u + token};
        CHECK(aurora_dictionary_propose(&dictionary, &tensor, key, 1u,
                                        (size_t)leaf) >= 0);
    }
    CHECK(dictionary.count == AURORA_DICTIONARY_MAX_ENTRIES);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, 0u, (size_t)leaf,
        AURORA_LEARN_CRYSTALLIZE));
    uint32_t first_key[1] = {1000u};
    CHECK(aurora_dictionary_lookup(&dictionary, first_key, 1u) == 0);

    uint32_t replacement[1] = {9999u};
    int replaced = aurora_dictionary_propose(
        &dictionary, &tensor, replacement, 1u, (size_t)leaf);
    CHECK(replaced == 1); /* la entrada 0 fue refrescada; la 1 era la LRU */
    CHECK(dictionary.evictions == 1u);
    CHECK(dictionary.entries[1].tokens[0] == 9999u);
    CHECK(dictionary.entries[0].tokens[0] == 1000u);
}

static void test_integral_learning_acceleration(void) {
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain values[3][3] = {
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1}};
    uint32_t tokens[3] = {7u, 8u, 9u};
    size_t simple_tensors[3];
    for (size_t index = 0u; index < 3u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, values[index]);
        CHECK(leaf >= 0);
        simple_tensors[index] = (size_t)leaf;
    }
    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);

    aurora_translation first = aurora_translate(
        &dictionary, &tensor, tokens, 3u, tokens, simple_tensors, 3u, true);
    CHECK(first.status == AURORA_TRANSLATION_OK);
    CHECK(first.windows == 1u && first.output >= 0);
    int entry = aurora_dictionary_propose(
        &dictionary, &tensor, tokens, 3u, (size_t)first.output);
    CHECK(entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)entry, (size_t)first.output,
        AURORA_LEARN_CRYSTALLIZE));

    aurora_translation second = aurora_translate(
        &dictionary, &tensor, tokens, 3u, tokens, simple_tensors, 3u, true);
    CHECK(second.status == AURORA_TRANSLATION_OK);
    CHECK(second.dictionary_hits == 1u && second.count == 1u);
    CHECK(second.windows == 0u);
    CHECK(second.output == first.output);
}

static void test_snapshot_round_trip(void) {
    const char *path = "aurora_snapshot_test.bin";
    remove(path);
    aurora_tensor tensor;
    aurora_tensor_init(&tensor);
    aurora_domain values[3][3] = {
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1},
        {AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_1}};
    uint32_t tokens[3] = {71u, 72u, 73u};
    size_t simple_tensors[3];
    for (size_t index = 0u; index < 3u; ++index) {
        int leaf = aurora_tensor_add_leaf(&tensor, values[index]);
        CHECK(leaf >= 0);
        simple_tensors[index] = (size_t)leaf;
    }
    aurora_domain saved_orientation[3] = {
        AURORA_DOMAIN_2, AURORA_DOMAIN_1, AURORA_DOMAIN_0};
    int complex = aurora_tensor_carry_at_do(
        &tensor, simple_tensors[0], simple_tensors[1], simple_tensors[2],
        saved_orientation);
    CHECK(complex >= 0);
    aurora_dictionary dictionary;
    aurora_dictionary_init(&dictionary);
    int entry = aurora_dictionary_propose(
        &dictionary, &tensor, tokens, 3u, (size_t)complex);
    CHECK(entry >= 0);
    CHECK(aurora_dictionary_used_in_output(
        &dictionary, &tensor, (size_t)entry, (size_t)complex,
        AURORA_LEARN_CRYSTALLIZE));
    CHECK(aurora_dictionary_lookup(&dictionary, tokens, 3u) == entry);

    size_t tensor_count = tensor.count;
    size_t dictionary_count = dictionary.count;
    uint64_t clock = dictionary.clock;
    CHECK(aurora_snapshot_save(path, &dictionary, &tensor) ==
          AURORA_SNAPSHOT_OK);

    aurora_tensor restored_tensor;
    aurora_dictionary restored_dictionary;
    aurora_tensor_init(&restored_tensor);
    aurora_dictionary_init(&restored_dictionary);
    CHECK(aurora_snapshot_load(path, &restored_dictionary,
                               &restored_tensor) == AURORA_SNAPSHOT_OK);
    CHECK(restored_tensor.count == tensor_count);
    CHECK(restored_dictionary.count == dictionary_count);
    CHECK(restored_dictionary.clock == clock);
    CHECK(restored_dictionary.entries[entry].state ==
          AURORA_ENTRY_CRYSTALLIZED);
    CHECK(restored_dictionary.entries[entry].tensor == (size_t)complex);
    CHECK(restored_tensor.nodes[complex].has_window_orientation);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        CHECK(restored_tensor.nodes[complex].window_orientation[dimension] ==
              saved_orientation[dimension]);
    }
    CHECK(aurora_dictionary_knowledge(&restored_dictionary, &restored_tensor,
                                      (size_t)entry) ==
          &restored_tensor.nodes[complex].superior);
    aurora_dictionary_matches restored_matches =
        aurora_dictionary_search_tensor(&restored_dictionary,
                                        &restored_tensor, (size_t)complex);
    CHECK(restored_matches.count == 1u);
    CHECK(restored_matches.entries[0] == (size_t)entry);

    aurora_translation translated = aurora_translate(
        &restored_dictionary, &restored_tensor, tokens, 3u, tokens,
        simple_tensors, 3u, true);
    CHECK(translated.status == AURORA_TRANSLATION_OK);
    CHECK(translated.count == 1u && translated.dictionary_hits == 1u);
    CHECK(translated.windows == 0u && translated.output == complex);
    CHECK(remove(path) == 0);

    FILE *broken = fopen(path, "wb");
    CHECK(broken != NULL);
    CHECK(fwrite("bad", 1u, 3u, broken) == 3u);
    CHECK(fclose(broken) == 0);
    size_t restored_count = restored_tensor.count;
    size_t restored_entries = restored_dictionary.count;
    CHECK(aurora_snapshot_load(path, &restored_dictionary,
                               &restored_tensor) == AURORA_SNAPSHOT_INVALID);
    CHECK(restored_tensor.count == restored_count);
    CHECK(restored_dictionary.count == restored_entries);
    CHECK(remove(path) == 0);
}

static void test_global_circular_coherence(void) {
    aurora_circular_network network;
    aurora_circular_init(&network);
    int a = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    int b = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    int mode = aurora_circular_add_variable(&network, AURORA_DOMAIN_ALL);
    int result = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    CHECK(a >= 0 && b >= 0 && mode >= 0 && result >= 0);
    CHECK(aurora_circular_add_gate(&network, (size_t)a, (size_t)b,
                                   (size_t)mode, (size_t)result));
    aurora_circular_solution solution = aurora_circular_solve(&network);
    CHECK(solution.state == AURORA_OPEN && solution.model_count == 3u);
    CHECK(solution.supported[mode] == AURORA_DOMAIN_ALL);
    CHECK(solution.supported[result] == AURORA_DOMAIN_1);

    aurora_circular_init(&network);
    int a1 = aurora_circular_add_variable(&network, AURORA_DOMAIN_0);
    int b1 = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    int a2 = aurora_circular_add_variable(&network, AURORA_DOMAIN_0);
    int b2 = aurora_circular_add_variable(&network, AURORA_DOMAIN_2);
    int a3 = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    int b3 = aurora_circular_add_variable(&network, AURORA_DOMAIN_2);
    int r1 = aurora_circular_add_variable(&network, AURORA_DOMAIN_ALL);
    int r2 = aurora_circular_add_variable(&network, AURORA_DOMAIN_ALL);
    int r3 = aurora_circular_add_variable(&network, AURORA_DOMAIN_ALL);
    CHECK(a1 >= 0 && b1 >= 0 && a2 >= 0 && b2 >= 0 && a3 >= 0 &&
          b3 >= 0 && r1 >= 0 && r2 >= 0 && r3 >= 0);
    CHECK(aurora_circular_add_gate(&network, (size_t)a1, (size_t)b1,
                                   (size_t)r3, (size_t)r1));
    CHECK(aurora_circular_add_gate(&network, (size_t)a2, (size_t)b2,
                                   (size_t)r1, (size_t)r2));
    CHECK(aurora_circular_add_gate(&network, (size_t)a3, (size_t)b3,
                                   (size_t)r2, (size_t)r3));
    solution = aurora_circular_solve(&network);
    CHECK(solution.state == AURORA_CLOSED && solution.model_count == 1u);
    CHECK(solution.supported[r1] == AURORA_DOMAIN_2);
    CHECK(solution.supported[r2] == AURORA_DOMAIN_2);
    CHECK(solution.supported[r3] == AURORA_DOMAIN_2);

    aurora_circular_init(&network);
    a = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    b = aurora_circular_add_variable(&network, AURORA_DOMAIN_1);
    mode = aurora_circular_add_variable(&network, AURORA_DOMAIN_0);
    result = aurora_circular_add_variable(&network, AURORA_DOMAIN_0);
    CHECK(aurora_circular_add_gate(&network, (size_t)a, (size_t)b,
                                   (size_t)mode, (size_t)result));
    solution = aurora_circular_solve(&network);
    CHECK(solution.state == AURORA_CONTRADICTION);
    CHECK(solution.model_count == 0u);

    aurora_circular_census ring, sample;
    CHECK(aurora_circular_ring_census(&ring));
    CHECK(ring.networks == 250047u);
    CHECK(ring.global_contradictions == 196279u);
    CHECK(ring.global_unique == 52696u);
    CHECK(ring.global_open == 1072u);
    CHECK(ring.unique_missed_locally == 0u);
    CHECK(ring.contradictions_missed_locally == 0u);
    CHECK(ring.values_pruned_globally == 0u);
    CHECK(ring.unsound_local_pruning == 0u);

    CHECK(aurora_circular_sample_census(42u, 10000u, &sample));
    CHECK(sample.global_contradictions == 6130u);
    CHECK(sample.global_unique == 1102u);
    CHECK(sample.global_open == 2768u);
    CHECK(sample.unique_missed_locally == 114u);
    CHECK(sample.contradictions_missed_locally == 77u);
    CHECK(sample.values_pruned_globally == 2515u);
    CHECK(sample.unsound_local_pruning == 0u);
    CHECK(sample.false_local_closure == 54u);
}

int main(void) {
    test_domains();
    test_truth_table();
    test_trigate_order_packets();
    test_directional_e_table();
    test_do_fibonacci_walk();
    test_forward_domains();
    test_inverse_and_contradiction();
    test_exhaustive_propagation();
    test_ordering_census();
    test_ordering_examples();
    test_circular_network();
    test_global_circular_coherence();
    test_minimum_operation_census();
    test_minimum_operation_with_knowledge();
    test_operational_seed();
    test_fractal_tensor();
    test_window_state();
    test_atomic_window();
    test_complete_sequence();
    test_sequence_sliding();
    test_sequence_rejects_carry_without_losing_input();
    test_recursive_hierarchy_driver();
    test_harmonizer();
    test_c4_c5_resolution();
    test_executive_seed();
    test_emergent_dictionary();
    test_associative_dictionary();
    test_dictionary_window_reecution();
    test_translation_and_backoff();
    test_bootstrap();
    test_dictionary_eviction();
    test_integral_learning_acceleration();
    test_snapshot_round_trip();
    printf("Aurora core: %d comprobaciones superadas "
           "(ciclo completo + persistencia y recuperación).\n",
           checks);
    return EXIT_SUCCESS;
}
