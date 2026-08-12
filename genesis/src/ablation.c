#include "genesis/ablation.h"

#include <string.h>

#define MAX_FACTS 64u
#define MAX_RULES 32u

enum {
    CAT = 1, DOG, EAGLE, BIRD, CROW, BLACKBIRD,
    HAMSTER, FALCON, CONDOR, RAVEN, BAT, COW,
    LIVES_WITH = 20, HAS, LIVES_IN,
    HUMANS = 40, FEATHERS, BLACK, FARM,
    SLEEPS_IN = 60, HOUSE, NEST, TOWER
};

typedef struct { uint32_t s, r, o; } fact;
typedef struct {
    uint32_t relation, output;
    fact conditions[2];
    size_t condition_count;
} rule;
typedef struct {
    fact facts[MAX_FACTS];
    size_t fact_count;
    rule rules[MAX_RULES];
    size_t rule_count;
} symbolic_memory;

static const fact TRAINING[] = {
    {CAT, LIVES_WITH, HUMANS}, {CAT, HAS, BLACK}, {CAT, SLEEPS_IN, HOUSE},
    {DOG, LIVES_WITH, HUMANS}, {DOG, SLEEPS_IN, HOUSE},
    {EAGLE, HAS, FEATHERS}, {EAGLE, SLEEPS_IN, NEST},
    {BIRD, HAS, FEATHERS}, {BIRD, SLEEPS_IN, NEST},
    {CROW, HAS, FEATHERS}, {CROW, HAS, BLACK}, {CROW, SLEEPS_IN, TOWER},
    {BLACKBIRD, HAS, FEATHERS}, {BLACKBIRD, HAS, BLACK},
    {BLACKBIRD, SLEEPS_IN, TOWER}
};

static const fact HELDOUT[] = {
    {HAMSTER, LIVES_WITH, HUMANS}, {FALCON, HAS, FEATHERS},
    {FALCON, HAS, BLACK}, {CONDOR, HAS, FEATHERS},
    {RAVEN, HAS, FEATHERS}, {RAVEN, HAS, BLACK},
    {BAT, HAS, BLACK}, {COW, LIVES_IN, FARM}
};

static const fact EXPECTED[] = {
    {HAMSTER, SLEEPS_IN, HOUSE}, {FALCON, SLEEPS_IN, TOWER},
    {CONDOR, SLEEPS_IN, NEST}, {RAVEN, SLEEPS_IN, TOWER}
};

static bool has_fact(const symbolic_memory *memory,
                     uint32_t subject, uint32_t relation, uint32_t object) {
    for (size_t i = 0u; i < memory->fact_count; ++i) {
        const fact *item = &memory->facts[i];
        if (item->s == subject && item->r == relation && item->o == object) {
            return true;
        }
    }
    return false;
}

static bool add_fact(symbolic_memory *memory, fact value) {
    if (has_fact(memory, value.s, value.r, value.o)) return true;
    if (memory->fact_count >= MAX_FACTS) return false;
    memory->facts[memory->fact_count++] = value;
    return true;
}

static bool matches(const symbolic_memory *memory, uint32_t subject,
                    const fact *conditions, size_t count) {
    for (size_t i = 0u; i < count; ++i) {
        if (!has_fact(memory, subject, conditions[i].r, conditions[i].o))
            return false;
    }
    return true;
}

static size_t support(const symbolic_memory *memory, const fact *conditions,
                      size_t count, uint32_t output, bool *conflict) {
    size_t result = 0u;
    *conflict = false;
    for (size_t i = 0u; i < memory->fact_count; ++i) {
        const fact *target = &memory->facts[i];
        if (target->r != SLEEPS_IN ||
            !matches(memory, target->s, conditions, count)) continue;
        if (target->o == output) ++result; else *conflict = true;
    }
    return result;
}

static bool same_rule(const rule *item, const fact *conditions,
                      size_t count, uint32_t output) {
    if (item->output != output || item->condition_count != count) return false;
    for (size_t i = 0u; i < count; ++i) {
        if (item->conditions[i].r != conditions[i].r ||
            item->conditions[i].o != conditions[i].o) return false;
    }
    return true;
}

static bool add_rule(symbolic_memory *memory, const fact *conditions,
                     size_t count, uint32_t output) {
    for (size_t i = 0u; i < memory->rule_count; ++i)
        if (same_rule(&memory->rules[i], conditions, count, output)) return true;
    if (memory->rule_count >= MAX_RULES) return false;
    rule *created = &memory->rules[memory->rule_count++];
    created->relation = SLEEPS_IN;
    created->output = output;
    created->condition_count = count;
    memcpy(created->conditions, conditions, count * sizeof conditions[0]);
    return true;
}

static bool induce(symbolic_memory *memory) {
    for (size_t target = 0u; target < memory->fact_count; ++target) {
        fact output = memory->facts[target];
        if (output.r != SLEEPS_IN) continue;
        fact features[8];
        size_t count = 0u;
        for (size_t i = 0u; i < memory->fact_count; ++i) {
            fact item = memory->facts[i];
            if (item.s == output.s && item.r != SLEEPS_IN && count < 8u)
                features[count++] = item;
        }
        for (size_t first = 0u; first < count; ++first) {
            bool conflict;
            if (support(memory, &features[first], 1u, output.o, &conflict) >= 2u &&
                !add_rule(memory, &features[first], 1u, output.o))
                return false;
            for (size_t second = first + 1u; second < count; ++second) {
                fact pair[2] = {features[first], features[second]};
                if (support(memory, pair, 2u, output.o, &conflict) >= 2u &&
                    !add_rule(memory, pair, 2u, output.o))
                    return false;
            }
        }
    }
    return true;
}

static bool contains_conditions(const rule *specific, const rule *general) {
    for (size_t i = 0u; i < general->condition_count; ++i) {
        bool found = false;
        for (size_t j = 0u; j < specific->condition_count; ++j) {
            if (specific->conditions[j].r == general->conditions[i].r &&
                specific->conditions[j].o == general->conditions[i].o)
                found = true;
        }
        if (!found) return false;
    }
    return true;
}

static bool raw_valid(const symbolic_memory *memory, const rule *item) {
    bool conflict;
    return support(memory, item->conditions, item->condition_count,
                   item->output, &conflict) >= 2u && !conflict;
}

static bool symbolic_rule_valid(const symbolic_memory *memory,
                                const rule *item) {
    size_t same = 0u;
    for (size_t i = 0u; i < memory->fact_count; ++i) {
        const fact *target = &memory->facts[i];
        if (target->r != item->relation ||
            !matches(memory, target->s, item->conditions,
                     item->condition_count)) continue;
        if (target->o == item->output) {
            ++same;
            continue;
        }
        bool explained = false;
        for (size_t r = 0u; r < memory->rule_count; ++r) {
            const rule *specific = &memory->rules[r];
            if (specific->relation == item->relation &&
                specific->output == target->o &&
                specific->condition_count > item->condition_count &&
                contains_conditions(specific, item) &&
                matches(memory, target->s, specific->conditions,
                        specific->condition_count) && raw_valid(memory, specific)) {
                explained = true;
                break;
            }
        }
        if (!explained) return false;
    }
    return same >= 2u;
}

static bool symbolic_answer(const symbolic_memory *memory, uint32_t subject,
                            bool rules_enabled, uint32_t *output) {
    for (size_t i = 0u; i < memory->fact_count; ++i) {
        if (memory->facts[i].s == subject && memory->facts[i].r == SLEEPS_IN) {
            *output = memory->facts[i].o;
            return true;
        }
    }
    if (!rules_enabled) return false;
    size_t best = 0u;
    bool found = false, ambiguous = false;
    for (size_t i = 0u; i < memory->rule_count; ++i) {
        const rule *item = &memory->rules[i];
        if (!matches(memory, subject, item->conditions, item->condition_count) ||
            item->condition_count < best ||
            !symbolic_rule_valid(memory, item)) continue;
        if (item->condition_count > best) {
            best = item->condition_count; *output = item->output;
            found = true; ambiguous = false;
        } else if (*output != item->output) ambiguous = true;
    }
    return found && !ambiguous;
}

static void score(bool resolved, uint32_t output, size_t expected_index,
                  genesis_ablation_metrics *metrics) {
    if (expected_index < 4u) {
        if (resolved && output == EXPECTED[expected_index].o)
            ++metrics->transfer_correct;
        else if (resolved) ++metrics->false_positives;
    } else if (!resolved) ++metrics->abstention_correct;
    else ++metrics->false_positives;
}

static bool train_genesis(genesis_education *education, uint64_t seed) {
    genesis_education_init(education, seed);
    for (size_t i = 0u; i < sizeof TRAINING / sizeof TRAINING[0]; ++i) {
        fact item = TRAINING[i];
        bool ok = item.r == SLEEPS_IN
            ? (genesis_education_correct(education, item.s, item.r, item.o),
               genesis_education_answer(education, item.s, item.r).object == item.o)
            : genesis_education_teach_fact(education, item.s, item.r, item.o);
        if (!ok) return false;
    }
    for (size_t i = 0u; i < sizeof HELDOUT / sizeof HELDOUT[0]; ++i)
        if (!genesis_education_teach_fact(education, HELDOUT[i].s,
                                          HELDOUT[i].r, HELDOUT[i].o)) return false;
    return true;
}

static void score_seen_symbolic(const symbolic_memory *memory,
                                genesis_ablation_metrics *metrics) {
    const uint32_t subjects[] = {CAT, DOG, EAGLE, BIRD, CROW, BLACKBIRD};
    for (size_t i = 0u; i < 6u; ++i) {
        uint32_t output;
        if (symbolic_answer(memory, subjects[i], false, &output))
            ++metrics->seen_correct;
    }
}

static bool run_genesis(uint64_t seed, genesis_ablation_metrics *metrics,
                        uint32_t outputs[6], bool resolved[6]) {
    genesis_education education;
    if (!train_genesis(&education, seed)) return false;
    memset(metrics, 0, sizeof *metrics);
    const uint32_t seen[] = {CAT, DOG, EAGLE, BIRD, CROW, BLACKBIRD};
    for (size_t i = 0u; i < 6u; ++i)
        if (genesis_education_answer(&education, seen[i], SLEEPS_IN).state ==
            GENESIS_ANSWER_RESOLVED) ++metrics->seen_correct;
    const uint32_t heldout[] = {HAMSTER, FALCON, CONDOR, RAVEN, BAT, COW};
    for (size_t i = 0u; i < 6u; ++i) {
        genesis_answer answer =
            genesis_education_answer(&education, heldout[i], SLEEPS_IN);
        resolved[i] = answer.state == GENESIS_ANSWER_RESOLVED;
        outputs[i] = answer.object;
        score(resolved[i], outputs[i], i, metrics);
    }
    metrics->active_facts = 0u;
    metrics->active_rules = 0u;
    for (size_t i = 0u; i < education.dictionary.count; ++i) {
        const aurora_dictionary_entry *entry = &education.dictionary.entries[i];
        if (entry->state != AURORA_ENTRY_CRYSTALLIZED) continue;
        if (entry->token_count == 4u) ++metrics->active_facts;
        else if (entry->token_count >= 5u) ++metrics->active_rules;
    }
    metrics->representation_bytes =
        education.arena.count * sizeof education.arena.nodes[0] +
        education.dictionary.count * sizeof education.dictionary.entries[0];
    return true;
}

bool genesis_ablation_run(genesis_ablation_report *report) {
    if (report == NULL) return false;
    memset(report, 0, sizeof *report);
    symbolic_memory memory = {0};
    for (size_t i = 0u; i < sizeof TRAINING / sizeof TRAINING[0]; ++i)
        if (!add_fact(&memory, TRAINING[i])) return false;
    if (!induce(&memory)) return false;
    for (size_t i = 0u; i < sizeof HELDOUT / sizeof HELDOUT[0]; ++i)
        if (!add_fact(&memory, HELDOUT[i])) return false;

    score_seen_symbolic(&memory, &report->literal);
    score_seen_symbolic(&memory, &report->symbolic);
    const uint32_t subjects[] = {HAMSTER, FALCON, CONDOR, RAVEN, BAT, COW};
    for (size_t i = 0u; i < 6u; ++i) {
        uint32_t output = 0u;
        bool resolved = symbolic_answer(
            &memory, subjects[i], false, &output);
        score(resolved, output, i, &report->literal);
        output = 0u;
        resolved = symbolic_answer(&memory, subjects[i], true, &output);
        score(resolved, output, i, &report->symbolic);
    }
    report->literal.active_facts = memory.fact_count;
    report->literal.representation_bytes = memory.fact_count * sizeof(fact);
    report->symbolic.active_facts = memory.fact_count;
    report->symbolic.active_rules = memory.rule_count;
    report->symbolic.representation_bytes = memory.fact_count * sizeof(fact) +
                                             memory.rule_count * sizeof(rule);

    uint32_t first_outputs[6], second_outputs[6];
    bool first_resolved[6], second_resolved[6];
    genesis_ablation_metrics second_metrics;
    if (!run_genesis(42u, &report->trinity, first_outputs, first_resolved) ||
        !run_genesis(99u, &second_metrics, second_outputs, second_resolved))
        return false;
    report->seed_invariant =
        memcmp(first_outputs, second_outputs, sizeof first_outputs) == 0 &&
        memcmp(first_resolved, second_resolved, sizeof first_resolved) == 0;
    return true;
}
