#include "genesis/education.h"

#define TOKEN_TAG UINT32_C(0xfffffff0)
#define FACT_TAG  UINT32_C(0xfffffff1)
#define RULE_TAG  UINT32_C(0xfffffff2)
#define REVISION_TAG UINT32_C(0xfffffff3)
#define MAX_RULE_CONDITIONS 3u
#define MAX_SUBJECT_FEATURES 16u

typedef struct {
    uint32_t relation;
    uint32_t object;
} condition;

void genesis_education_init(genesis_education *education,
                            uint64_t public_seed) {
    if (education != NULL) {
        aurora_tensor_init(&education->arena);
        aurora_dictionary_init(&education->dictionary);
        education->public_seed = public_seed;
    }
}

static int exact_entry(const genesis_education *education,
                       const uint32_t *key,
                       size_t length) {
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *entry =
            &education->dictionary.entries[index];
        if (entry->state != AURORA_ENTRY_CRYSTALLIZED ||
            entry->token_count != length) {
            continue;
        }
        bool equal = true;
        for (size_t token = 0u; token < length; ++token) {
            if (entry->tokens[token] != key[token]) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return (int)index;
        }
    }
    return -1;
}

static int token_entry(const genesis_education *education, uint32_t token) {
    uint32_t key[2] = {TOKEN_TAG, token};
    return exact_entry(education, key, 2u);
}

static bool tensor_to_token(const genesis_education *education,
                            size_t tensor,
                            uint32_t *token) {
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *entry =
            &education->dictionary.entries[index];
        if (entry->state == AURORA_ENTRY_CRYSTALLIZED &&
            entry->token_count == 2u && entry->tokens[0] == TOKEN_TAG &&
            entry->tensor == tensor) {
            *token = entry->tokens[1];
            return true;
        }
    }
    return false;
}

static bool crystallize(genesis_education *education,
                        const uint32_t *key,
                        size_t length,
                        size_t tensor) {
    int entry = aurora_dictionary_propose(
        &education->dictionary, &education->arena, key, length, tensor);
    return entry >= 0 && aurora_dictionary_used_in_output(
                             &education->dictionary, &education->arena,
                             (size_t)entry, tensor,
                             AURORA_LEARN_CRYSTALLIZE);
}

bool genesis_education_register_token(genesis_education *education,
                                      uint32_t token) {
    if (education == NULL) {
        return false;
    }
    if (token_entry(education, token) >= 0) {
        return true;
    }
    genesis_tensor117 payload;
    size_t root;
    if (!genesis_gr_generate(education->public_seed, token, &payload) ||
        !genesis_tensor117_to_trinity(&payload, &education->arena, &root)) {
        return false;
    }
    uint32_t key[2] = {TOKEN_TAG, token};
    return crystallize(education, key, 2u, root);
}

bool genesis_education_teach_fact(genesis_education *education,
                                  uint32_t subject,
                                  uint32_t relation,
                                  uint32_t object) {
    if (education == NULL ||
        !genesis_education_register_token(education, subject) ||
        !genesis_education_register_token(education, relation) ||
        !genesis_education_register_token(education, object)) {
        return false;
    }
    uint32_t key[4] = {FACT_TAG, subject, relation, object};
    if (exact_entry(education, key, 4u) >= 0) {
        return true;
    }
    int object_entry = token_entry(education, object);
    return object_entry >= 0 && crystallize(
                                    education, key, 4u,
                                    education->dictionary
                                        .entries[object_entry].tensor);
}

static bool has_fact(const genesis_education *education,
                     uint32_t subject,
                     uint32_t relation,
                     uint32_t object) {
    uint32_t key[4] = {FACT_TAG, subject, relation, object};
    return exact_entry(education, key, 4u) >= 0;
}

static bool subject_matches(const genesis_education *education,
                            uint32_t subject,
                            const condition *conditions,
                            size_t condition_count) {
    for (size_t index = 0u; index < condition_count; ++index) {
        if (!has_fact(education, subject, conditions[index].relation,
                      conditions[index].object)) {
            return false;
        }
    }
    return true;
}

static size_t target_support(const genesis_education *education,
                             const condition *conditions,
                             size_t condition_count,
                             uint32_t target_relation,
                             uint32_t target_object,
                             bool *conflict) {
    size_t support = 0u;
    *conflict = false;
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *target =
            &education->dictionary.entries[index];
        if (target->state != AURORA_ENTRY_CRYSTALLIZED ||
            target->token_count != 4u || target->tokens[0] != FACT_TAG ||
            target->tokens[2] != target_relation ||
            !subject_matches(education, target->tokens[1], conditions,
                             condition_count)) {
            continue;
        }
        if (target->tokens[3] == target_object) {
            ++support;
        } else {
            *conflict = true;
        }
    }
    return support;
}

static bool rule_contains_conditions(const aurora_dictionary_entry *rule,
                                     const condition *base,
                                     size_t base_count) {
    for (size_t base_index = 0u; base_index < base_count; ++base_index) {
        bool found = false;
        for (size_t index = 0u; index < rule->tokens[2]; ++index) {
            if (rule->tokens[3u + 2u * index] == base[base_index].relation &&
                rule->tokens[4u + 2u * index] == base[base_index].object) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

static bool conflict_has_specific_rule(const genesis_education *education,
                                       uint32_t subject,
                                       uint32_t relation,
                                       uint32_t output,
                                       const condition *base,
                                       size_t base_count) {
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *rule =
            &education->dictionary.entries[index];
        if (rule->state != AURORA_ENTRY_CRYSTALLIZED ||
            rule->tokens[0] != RULE_TAG || rule->token_count < 5u ||
            rule->tokens[1] != relation || rule->tokens[2] <= base_count ||
            rule->tokens[2] > MAX_RULE_CONDITIONS ||
            rule->token_count != 3u + 2u * rule->tokens[2] ||
            !rule_contains_conditions(rule, base, base_count)) continue;
        uint32_t candidate;
        if (!tensor_to_token(education, rule->tensor, &candidate) ||
            candidate != output) continue;
        condition specific[MAX_RULE_CONDITIONS];
        for (size_t c = 0u; c < rule->tokens[2]; ++c) {
            specific[c].relation = rule->tokens[3u + 2u * c];
            specific[c].object = rule->tokens[4u + 2u * c];
        }
        bool conflict;
        if (subject_matches(education, subject, specific, rule->tokens[2]) &&
            target_support(education, specific, rule->tokens[2], relation,
                           output, &conflict) >= 2u && !conflict) return true;
    }
    return false;
}

static bool rule_is_valid(const genesis_education *education,
                          const condition *conditions,
                          size_t condition_count,
                          uint32_t relation,
                          uint32_t output,
                          size_t *support_out) {
    size_t support = 0u;
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *target =
            &education->dictionary.entries[index];
        if (target->state != AURORA_ENTRY_CRYSTALLIZED ||
            target->token_count != 4u || target->tokens[0] != FACT_TAG ||
            target->tokens[2] != relation ||
            !subject_matches(education, target->tokens[1], conditions,
                             condition_count)) continue;
        if (target->tokens[3] == output) {
            ++support;
        } else if (!conflict_has_specific_rule(
                       education, target->tokens[1], relation,
                       target->tokens[3], conditions, condition_count)) {
            return false;
        }
    }
    *support_out = support;
    return support >= 2u;
}

static void sort_conditions(condition *conditions, size_t count) {
    for (size_t right = 1u; right < count; ++right) {
        condition value = conditions[right];
        size_t left = right;
        while (left > 0u &&
               (conditions[left - 1u].relation > value.relation ||
                (conditions[left - 1u].relation == value.relation &&
                 conditions[left - 1u].object > value.object))) {
            conditions[left] = conditions[left - 1u];
            --left;
        }
        conditions[left] = value;
    }
}

static bool learn_rule(genesis_education *education,
                       const condition *conditions,
                       size_t condition_count,
                       uint32_t relation,
                       uint32_t correct_object,
                       size_t output_tensor) {
    bool conflict;
    if (target_support(education, conditions, condition_count, relation,
                       correct_object, &conflict) < 2u || conflict) {
        return false;
    }
    uint32_t rule[3u + 2u * MAX_RULE_CONDITIONS];
    rule[0] = RULE_TAG;
    rule[1] = relation;
    rule[2] = (uint32_t)condition_count;
    for (size_t index = 0u; index < condition_count; ++index) {
        rule[3u + 2u * index] = conditions[index].relation;
        rule[4u + 2u * index] = conditions[index].object;
    }
    size_t length = 3u + 2u * condition_count;
    return exact_entry(education, rule, length) < 0 &&
           crystallize(education, rule, length, output_tensor);
}

static bool retire_conflicting_facts(genesis_education *education,
                                     uint32_t subject,
                                     uint32_t relation,
                                     uint32_t correct_object) {
    int output_entry = token_entry(education, correct_object);
    if (output_entry < 0) {
        return false;
    }
    size_t initial_count = education->dictionary.count;
    for (size_t index = 0u; index < initial_count; ++index) {
        aurora_dictionary_entry *fact = &education->dictionary.entries[index];
        if (fact->state != AURORA_ENTRY_CRYSTALLIZED ||
            fact->token_count != 4u || fact->tokens[0] != FACT_TAG ||
            fact->tokens[1] != subject || fact->tokens[2] != relation ||
            fact->tokens[3] == correct_object) {
            continue;
        }
        uint32_t revision[5] = {REVISION_TAG, subject, relation,
                                fact->tokens[3], correct_object};
        if (exact_entry(education, revision, 5u) < 0 &&
            !crystallize(education, revision, 5u,
                         education->dictionary.entries[output_entry].tensor)) {
            return false;
        }
        fact = &education->dictionary.entries[index];
        fact->state = AURORA_ENTRY_REJECTED;
    }
    return true;
}

size_t genesis_education_correct(genesis_education *education,
                                 uint32_t subject,
                                 uint32_t relation,
                                 uint32_t correct_object) {
    if (education == NULL ||
        !genesis_education_register_token(education, correct_object) ||
        !retire_conflicting_facts(education, subject, relation,
                                  correct_object) ||
        !genesis_education_teach_fact(education, subject, relation,
                                      correct_object)) {
        return 0u;
    }
    int output_entry = token_entry(education, correct_object);
    if (output_entry < 0) {
        return 0u;
    }
    condition features[MAX_SUBJECT_FEATURES];
    size_t feature_count = 0u;
    size_t initial_count = education->dictionary.count;
    for (size_t index = 0u; index < initial_count; ++index) {
        const aurora_dictionary_entry *feature =
            &education->dictionary.entries[index];
        if (feature->state != AURORA_ENTRY_CRYSTALLIZED ||
            feature->token_count != 4u || feature->tokens[0] != FACT_TAG ||
            feature->tokens[1] != subject || feature->tokens[2] == relation) {
            continue;
        }
        if (feature_count < MAX_SUBJECT_FEATURES) {
            features[feature_count].relation = feature->tokens[2];
            features[feature_count].object = feature->tokens[3];
            ++feature_count;
        }
    }
    sort_conditions(features, feature_count);

    size_t learned = 0u;
    size_t combinations = (size_t)1u << feature_count;
    for (size_t mask = 1u; mask < combinations; ++mask) {
        condition selected[MAX_RULE_CONDITIONS];
        size_t selected_count = 0u;
        size_t represented = 0u;
        for (size_t bit = 0u; bit < feature_count; ++bit) {
            if ((mask & ((size_t)1u << bit)) != 0u) {
                ++represented;
                if (selected_count < MAX_RULE_CONDITIONS) {
                    selected[selected_count++] = features[bit];
                }
            }
        }
        if (represented == selected_count &&
            learn_rule(education, selected, selected_count, relation,
                       correct_object,
                       education->dictionary.entries[output_entry].tensor)) {
            ++learned;
        }
    }
    return learned;
}

size_t genesis_education_revision_count(const genesis_education *education) {
    if (education == NULL) {
        return 0u;
    }
    size_t count = 0u;
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *entry =
            &education->dictionary.entries[index];
        if (entry->state == AURORA_ENTRY_CRYSTALLIZED &&
            entry->token_count == 5u && entry->tokens[0] == REVISION_TAG) {
            ++count;
        }
    }
    return count;
}

genesis_answer genesis_education_answer_explain(
    genesis_education *education,
    uint32_t subject,
    uint32_t relation,
    genesis_explanation *explanation) {
    genesis_answer answer = {GENESIS_ANSWER_UNKNOWN, GENESIS_EVIDENCE_NONE, 0u};
    if (explanation != NULL) {
        *explanation = (genesis_explanation){0};
        explanation->dictionary_entry = SIZE_MAX;
    }
    if (education == NULL) {
        return answer;
    }
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        aurora_dictionary_entry *fact = &education->dictionary.entries[index];
        if (fact->state == AURORA_ENTRY_CRYSTALLIZED &&
            fact->token_count == 4u && fact->tokens[0] == FACT_TAG &&
            fact->tokens[1] == subject && fact->tokens[2] == relation) {
            answer.state = GENESIS_ANSWER_RESOLVED;
            answer.evidence = GENESIS_EVIDENCE_LITERAL;
            answer.object = fact->tokens[3];
            (void)aurora_dictionary_used_in_output(
                &education->dictionary, &education->arena, index,
                fact->tensor, AURORA_LEARN_CRYSTALLIZE);
            if (explanation != NULL) {
                explanation->answer = answer;
                explanation->dictionary_entry = index;
                explanation->support = 1u;
                explanation->uses_after_output =
                    education->dictionary.entries[index].uses;
            }
            return answer;
        }
    }

    size_t best_specificity = 0u;
    size_t best_entry = SIZE_MAX;
    size_t best_support = 0u;
    for (size_t index = 0u; index < education->dictionary.count; ++index) {
        const aurora_dictionary_entry *rule =
            &education->dictionary.entries[index];
        if (rule->state != AURORA_ENTRY_CRYSTALLIZED ||
            rule->token_count < 5u || rule->tokens[0] != RULE_TAG ||
            rule->tokens[1] != relation) {
            continue;
        }
        size_t specificity = rule->tokens[2];
        if (specificity == 0u || specificity > MAX_RULE_CONDITIONS ||
            rule->token_count != 3u + 2u * specificity) {
            continue;
        }
        condition conditions[MAX_RULE_CONDITIONS];
        bool matches = true;
        for (size_t condition_index = 0u;
             condition_index < specificity; ++condition_index) {
            conditions[condition_index].relation =
                rule->tokens[3u + 2u * condition_index];
            conditions[condition_index].object =
                rule->tokens[4u + 2u * condition_index];
            if (!has_fact(education, subject,
                          conditions[condition_index].relation,
                          conditions[condition_index].object)) {
                matches = false;
            }
        }
        if (!matches || specificity < best_specificity) {
            continue;
        }
        uint32_t candidate;
        if (!tensor_to_token(education, rule->tensor, &candidate)) {
            continue;
        }
        size_t support;
        if (!rule_is_valid(education, conditions, specificity, relation,
                           candidate, &support)) {
            continue;
        }
        if (specificity > best_specificity) {
            answer.state = GENESIS_ANSWER_RESOLVED;
            answer.evidence = GENESIS_EVIDENCE_RULE;
            answer.object = candidate;
            best_specificity = specificity;
            best_entry = index;
            best_support = support;
            if (explanation != NULL) {
                explanation->condition_count = specificity;
                for (size_t condition_index = 0u;
                     condition_index < specificity; ++condition_index) {
                    explanation->conditions[condition_index].relation =
                        conditions[condition_index].relation;
                    explanation->conditions[condition_index].object =
                        conditions[condition_index].object;
                }
            }
        } else if (answer.object != candidate) {
            answer.state = GENESIS_ANSWER_AMBIGUOUS;
            answer.evidence = GENESIS_EVIDENCE_RULE;
            best_entry = SIZE_MAX;
        }
    }
    if (answer.state == GENESIS_ANSWER_RESOLVED && best_entry != SIZE_MAX) {
        aurora_dictionary_entry *entry =
            &education->dictionary.entries[best_entry];
        (void)aurora_dictionary_used_in_output(
            &education->dictionary, &education->arena, best_entry,
            entry->tensor, AURORA_LEARN_CRYSTALLIZE);
    }
    if (explanation != NULL) {
        explanation->answer = answer;
        explanation->dictionary_entry = best_entry;
        explanation->support = best_support;
        if (best_entry != SIZE_MAX) {
            explanation->uses_after_output =
                education->dictionary.entries[best_entry].uses;
        }
    }
    return answer;
}

genesis_answer genesis_education_answer(genesis_education *education,
                                        uint32_t subject,
                                        uint32_t relation) {
    return genesis_education_answer_explain(education, subject, relation, NULL);
}

aurora_snapshot_status genesis_education_save(const genesis_education *education,
                                              const char *path) {
    return education == NULL
               ? AURORA_SNAPSHOT_INVALID
               : aurora_snapshot_save(path, &education->dictionary,
                                      &education->arena);
}

aurora_snapshot_status genesis_education_load(genesis_education *education,
                                              uint64_t public_seed,
                                              const char *path) {
    if (education == NULL) {
        return AURORA_SNAPSHOT_INVALID;
    }
    genesis_education loaded;
    genesis_education_init(&loaded, public_seed);
    aurora_snapshot_status status = aurora_snapshot_load(
        path, &loaded.dictionary, &loaded.arena);
    if (status == AURORA_SNAPSHOT_OK) {
        *education = loaded;
    }
    return status;
}
