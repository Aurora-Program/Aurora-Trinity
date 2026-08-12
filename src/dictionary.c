#include "aurora/dictionary.h"

#include <string.h>

void aurora_dictionary_init(aurora_dictionary *dictionary) {
    if (dictionary != NULL) {
        memset(dictionary, 0, sizeof *dictionary);
        for (size_t bucket = 0u; bucket < AURORA_DICTIONARY_ROOT_BUCKETS;
             ++bucket) {
            dictionary->root_buckets[bucket] = -1;
        }
    }
}

static uint8_t project_domain(aurora_domain domain) {
    if (domain == AURORA_DOMAIN_0) return 0u;
    if (domain == AURORA_DOMAIN_1) return 1u;
    return 2u;
}

static int root_bucket(const aurora_tensor *tensor, size_t tensor_index) {
    const aurora_tensor_node *node = aurora_tensor_get(tensor, tensor_index);
    if (node == NULL) return -1;
    uint8_t a = project_domain(node->superior.upper_ds[0]);
    uint8_t b = project_domain(node->superior.upper_ds[1]);
    uint8_t c = project_domain(node->superior.upper_ds[2]);
    return (int)(a * 9u + b * 3u + c);
}

static void unindex_entry(aurora_dictionary *dictionary, size_t entry_index) {
    aurora_dictionary_entry *entry = &dictionary->entries[entry_index];
    int bucket = entry->associative_bucket;
    if (bucket < 0 || bucket >= (int)AURORA_DICTIONARY_ROOT_BUCKETS) return;
    int *link = &dictionary->root_buckets[bucket];
    while (*link >= 0) {
        if ((size_t)*link == entry_index) {
            *link = dictionary->entries[*link].associative_next;
            break;
        }
        link = &dictionary->entries[*link].associative_next;
    }
    entry->associative_next = -1;
    entry->associative_bucket = -1;
}

static bool index_entry(aurora_dictionary *dictionary,
                        const aurora_tensor *tensor,
                        size_t entry_index) {
    aurora_dictionary_entry *entry = &dictionary->entries[entry_index];
    int bucket = root_bucket(tensor, entry->tensor);
    if (bucket < 0) return false;
    if (entry->associative_bucket >= 0) unindex_entry(dictionary, entry_index);
    entry->associative_bucket = bucket;
    entry->associative_next = dictionary->root_buckets[bucket];
    dictionary->root_buckets[bucket] = (int)entry_index;
    return true;
}

static bool valid_tensor(const aurora_tensor *tensor, size_t index) {
    return aurora_tensor_get(tensor, index) != NULL;
}

static bool contains_tensor(const aurora_tensor *tensor,
                            size_t root,
                            size_t target) {
    if (root == target) {
        return true;
    }
    const aurora_tensor_node *node = aurora_tensor_get(tensor, root);
    if (node == NULL || node->is_leaf) {
        return false;
    }
    for (size_t child = 0u; child < 3u; ++child) {
        if (contains_tensor(tensor, node->children[child], target)) {
            return true;
        }
    }
    return false;
}

int aurora_dictionary_propose(aurora_dictionary *dictionary,
                              const aurora_tensor *tensor,
                              const uint32_t *tokens,
                              size_t token_count,
                              size_t tensor_index) {
    if (dictionary == NULL || tokens == NULL || token_count == 0u ||
        token_count > AURORA_DICTIONARY_MAX_TOKENS ||
        !valid_tensor(tensor, tensor_index)) {
        return -1;
    }
    for (size_t index = 0u; index < dictionary->count; ++index) {
        aurora_dictionary_entry *entry = &dictionary->entries[index];
        if (entry->tensor == tensor_index && entry->token_count == token_count &&
            memcmp(entry->tokens, tokens,
                   token_count * sizeof(tokens[0])) == 0) {
            return (int)index;
        }
    }
    size_t index;
    if (dictionary->count < AURORA_DICTIONARY_MAX_ENTRIES) {
        index = dictionary->count++;
    } else {
        index = 0u;
        uint64_t oldest = dictionary->entries[0].last_used != 0u
                              ? dictionary->entries[0].last_used
                              : dictionary->entries[0].created_at;
        for (size_t candidate = 1u; candidate < dictionary->count;
             ++candidate) {
            uint64_t age = dictionary->entries[candidate].last_used != 0u
                               ? dictionary->entries[candidate].last_used
                               : dictionary->entries[candidate].created_at;
            if (age < oldest) {
                oldest = age;
                index = candidate;
            }
        }
        unindex_entry(dictionary, index);
        ++dictionary->evictions;
    }
    aurora_dictionary_entry *entry = &dictionary->entries[index];
    memset(entry, 0, sizeof *entry);
    memcpy(entry->tokens, tokens, token_count * sizeof(tokens[0]));
    entry->token_count = token_count;
    entry->tensor = tensor_index;
    entry->created_at = ++dictionary->clock;
    entry->last_used = 0u;
    entry->uses = 0u;
    entry->state = AURORA_ENTRY_CANDIDATE;
    entry->associative_next = -1;
    entry->associative_bucket = -1;
    return (int)index;
}

bool aurora_dictionary_used_in_output(
    aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t entry_index,
    size_t output_tensor,
    aurora_learning_direction direction) {
    if (dictionary == NULL || entry_index >= dictionary->count ||
        !valid_tensor(tensor, output_tensor)) {
        return false;
    }
    aurora_dictionary_entry *entry = &dictionary->entries[entry_index];
    if (!contains_tensor(tensor, output_tensor, entry->tensor)) {
        return false;
    }
    entry->last_used = ++dictionary->clock;
    ++entry->uses;
    if (direction == AURORA_LEARN_CRYSTALLIZE) {
        entry->state = AURORA_ENTRY_CRYSTALLIZED;
        if (!index_entry(dictionary, tensor, entry_index)) return false;
    } else if (direction == AURORA_LEARN_NONE) {
        unindex_entry(dictionary, entry_index);
        entry->state = AURORA_ENTRY_REJECTED;
    }
    return true;
}

static bool prefix_matches(const aurora_dictionary_entry *entry,
                           const uint32_t *tokens,
                           size_t token_count) {
    return entry->state == AURORA_ENTRY_CRYSTALLIZED &&
           entry->token_count <= token_count &&
           memcmp(entry->tokens, tokens,
                  entry->token_count * sizeof(tokens[0])) == 0;
}

int aurora_dictionary_lookup(aurora_dictionary *dictionary,
                             const uint32_t *tokens,
                             size_t token_count) {
    return aurora_dictionary_lookup_limit(dictionary, tokens, token_count,
                                          SIZE_MAX);
}

int aurora_dictionary_lookup_limit(aurora_dictionary *dictionary,
                                   const uint32_t *tokens,
                                   size_t token_count,
                                   size_t max_length) {
    if (dictionary == NULL || tokens == NULL) {
        return -1;
    }
    int best = -1;
    for (size_t index = 0u; index < dictionary->count; ++index) {
        aurora_dictionary_entry *entry = &dictionary->entries[index];
        if (!prefix_matches(entry, tokens, token_count) ||
            entry->token_count > max_length) {
            continue;
        }
        if (best < 0 || entry->token_count >
                            dictionary->entries[best].token_count ||
            (entry->token_count == dictionary->entries[best].token_count &&
             entry->last_used > dictionary->entries[best].last_used)) {
            best = (int)index;
        }
    }
    if (best >= 0) {
        aurora_dictionary_entry *entry = &dictionary->entries[best];
        entry->last_used = ++dictionary->clock;
        ++entry->uses;
    }
    return best;
}

const aurora_so *aurora_dictionary_knowledge(
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t entry_index) {
    if (dictionary == NULL || entry_index >= dictionary->count ||
        dictionary->entries[entry_index].state !=
            AURORA_ENTRY_CRYSTALLIZED) {
        return NULL;
    }
    const aurora_tensor_node *node =
        aurora_tensor_get(tensor, dictionary->entries[entry_index].tensor);
    return node == NULL ? NULL : &node->superior;
}

static bool query_accepts(aurora_domain query, aurora_domain stored) {
    if (!aurora_domain_is_valid(query) || !aurora_domain_is_valid(stored)) {
        return false;
    }
    if (aurora_domain_contains(query, 2u)) return true;
    if (!aurora_domain_is_singleton(stored)) return false;
    uint8_t stored_value = project_domain(stored);
    return stored_value < 2u && aurora_domain_contains(query, stored_value);
}

static bool compatible_so(const aurora_so *query, const aurora_so *stored) {
    const aurora_domain *q[4] = {query->upper_ds, query->ds, query->de,
                                 query->do_};
    const aurora_domain *s[4] = {stored->upper_ds, stored->ds, stored->de,
                                 stored->do_};
    for (size_t channel = 0u; channel < 4u; ++channel) {
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            if (!query_accepts(q[channel][dimension],
                               s[channel][dimension])) {
                return false;
            }
        }
    }
    return true;
}

static bool compatible_tensor(const aurora_tensor *tensor,
                              size_t query_index,
                              size_t stored_index,
                              size_t *visited) {
    const aurora_tensor_node *query = aurora_tensor_get(tensor, query_index);
    const aurora_tensor_node *stored = aurora_tensor_get(tensor, stored_index);
    if (query == NULL || stored == NULL || query->is_leaf != stored->is_leaf) {
        return false;
    }
    ++*visited;
    if (!compatible_so(&query->superior, &stored->superior)) return false;
    if (query->is_leaf) return true;
    for (size_t child = 0u; child < 3u; ++child) {
        if (!compatible_tensor(tensor, query->children[child],
                               stored->children[child], visited)) {
            return false;
        }
    }
    return true;
}

static bool query_bucket_accepts(const aurora_tensor_node *query,
                                 size_t bucket) {
    uint8_t stored[3] = {(uint8_t)(bucket / 9u),
                         (uint8_t)((bucket / 3u) % 3u),
                         (uint8_t)(bucket % 3u)};
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (!query_accepts(query->superior.upper_ds[dimension],
                           aurora_domain_singleton(stored[dimension]))) {
            return false;
        }
    }
    return true;
}

static bool higher_priority(const aurora_dictionary_entry *left,
                            const aurora_dictionary_entry *right) {
    if (left->uses != right->uses) return left->uses > right->uses;
    if (left->last_used != right->last_used) {
        return left->last_used > right->last_used;
    }
    return left->created_at > right->created_at;
}

aurora_dictionary_matches aurora_dictionary_search_tensor(
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t query_tensor) {
    aurora_dictionary_matches matches = {0};
    const aurora_tensor_node *query = aurora_tensor_get(tensor, query_tensor);
    if (dictionary == NULL || query == NULL) return matches;

    for (size_t bucket = 0u; bucket < AURORA_DICTIONARY_ROOT_BUCKETS;
         ++bucket) {
        if (!query_bucket_accepts(query, bucket)) continue;
        int entry_index = dictionary->root_buckets[bucket];
        while (entry_index >= 0) {
            if ((size_t)entry_index >= dictionary->count) return matches;
            const aurora_dictionary_entry *entry =
                &dictionary->entries[entry_index];
            ++matches.roots_visited;
            size_t visited = 0u;
            if (entry->state == AURORA_ENTRY_CRYSTALLIZED &&
                compatible_tensor(tensor, query_tensor, entry->tensor,
                                  &visited)) {
                size_t position = matches.count;
                while (position > 0u &&
                       higher_priority(entry,
                                       &dictionary->entries[
                                           matches.entries[position - 1u]])) {
                    matches.entries[position] = matches.entries[position - 1u];
                    --position;
                }
                matches.entries[position] = (size_t)entry_index;
                ++matches.count;
            } else {
                ++matches.branches_rejected;
            }
            entry_index = entry->associative_next;
        }
    }
    return matches;
}

bool aurora_dictionary_reindex(aurora_dictionary *dictionary,
                               const aurora_tensor *tensor) {
    if (dictionary == NULL || tensor == NULL) return false;
    for (size_t bucket = 0u; bucket < AURORA_DICTIONARY_ROOT_BUCKETS;
         ++bucket) {
        dictionary->root_buckets[bucket] = -1;
    }
    for (size_t index = 0u; index < dictionary->count; ++index) {
        aurora_dictionary_entry *entry = &dictionary->entries[index];
        entry->associative_next = -1;
        entry->associative_bucket = -1;
        if (entry->state == AURORA_ENTRY_CRYSTALLIZED &&
            !index_entry(dictionary, tensor, index)) {
            return false;
        }
    }
    return true;
}

aurora_dictionary_window_result aurora_dictionary_reexecute_window(
    aurora_dictionary *dictionary,
    aurora_tensor *tensor,
    size_t query_tensor,
    size_t first,
    size_t second) {
    aurora_dictionary_window_result result = {0};
    result.selected_entry = -1;
    result.window.state = AURORA_OPEN;
    result.window.action = AURORA_WINDOW_CARRY;
    result.window.output = -1;
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        result.window.u_orientation[dimension] = AURORA_DOMAIN_0;
    }
    aurora_do_cursor_init(&result.traversal);
    if (dictionary == NULL || tensor == NULL ||
        aurora_tensor_get(tensor, query_tensor) == NULL ||
        aurora_tensor_get(tensor, first) == NULL ||
        aurora_tensor_get(tensor, second) == NULL) {
        result.window.state = AURORA_CONTRADICTION;
        result.window.action = AURORA_WINDOW_SLIDE;
        return result;
    }
    const aurora_tensor_node *first_node = aurora_tensor_get(tensor, first);
    if (first_node->has_window_orientation) {
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            result.window.u_orientation[dimension] =
                first_node->window_orientation[dimension];
        }
    }

    result.matches = aurora_dictionary_search_tensor(
        dictionary, tensor, query_tensor);
    bool saw_open = false;
    while (result.matches.count > 0u &&
           aurora_do_cursor_next(&result.traversal)) {
        size_t candidate =
            (result.traversal.attempts - 1u) % result.matches.count;
        size_t entry_index = result.matches.entries[candidate];
        size_t candidate_tensor = dictionary->entries[entry_index].tensor;
        aurora_window_result evaluated =
            aurora_window_evaluate_knowledge_at_do(
            tensor, first, second, candidate_tensor,
            result.traversal.state);
        result.window = evaluated;
        ++result.candidates_tried;
        if (evaluated.state == AURORA_OPEN) saw_open = true;
        if (evaluated.state != AURORA_CLOSED) continue;

        aurora_window_result materialized =
            aurora_window_process_knowledge_at_do(
            tensor, first, second, candidate_tensor,
            result.traversal.state);
        if (materialized.state != AURORA_CLOSED ||
            materialized.output < 0) {
            continue;
        }
        if (!aurora_dictionary_used_in_output(
                dictionary, tensor, entry_index,
                (size_t)materialized.output, AURORA_LEARN_CRYSTALLIZE)) {
            result.window.state = AURORA_CONTRADICTION;
            result.window.action = AURORA_WINDOW_SLIDE;
            return result;
        }
        result.window = materialized;
        result.selected_entry = (int)entry_index;
        return result;
    }

    result.exhausted = result.matches.count > 0u &&
                       result.traversal.exhausted;
    result.needs_alternative = true;
    result.window.state = saw_open || result.matches.count == 0u
                              ? AURORA_OPEN
                              : AURORA_CONTRADICTION;
    result.window.action = result.window.state == AURORA_OPEN
                               ? AURORA_WINDOW_CARRY
                               : AURORA_WINDOW_SLIDE;
    return result;
}

aurora_executive aurora_dictionary_decide(
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t entry_index,
    const aurora_so *input,
    const aurora_so *output,
    const aurora_harmonizer *harmonizer) {
    const aurora_so *knowledge = aurora_dictionary_knowledge(
        dictionary, tensor, entry_index);
    return aurora_executive_decide(input, knowledge, output, harmonizer);
}
