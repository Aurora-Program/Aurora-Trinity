#ifndef AURORA_DICTIONARY_H
#define AURORA_DICTIONARY_H

#include "aurora/executive.h"
#include "aurora/tensor.h"
#include "aurora/window.h"

#define AURORA_DICTIONARY_MAX_ENTRIES 256u
#define AURORA_DICTIONARY_MAX_TOKENS 64u
#define AURORA_DICTIONARY_ROOT_BUCKETS 27u

typedef enum {
    AURORA_ENTRY_CANDIDATE = 0,
    AURORA_ENTRY_CRYSTALLIZED = 1,
    AURORA_ENTRY_REJECTED = 2
} aurora_entry_state;

typedef struct {
    uint32_t tokens[AURORA_DICTIONARY_MAX_TOKENS];
    size_t token_count;
    size_t tensor;
    uint64_t created_at;
    uint64_t last_used;
    size_t uses;
    aurora_entry_state state;
    int associative_next;
    int associative_bucket;
} aurora_dictionary_entry;

typedef struct {
    aurora_dictionary_entry entries[AURORA_DICTIONARY_MAX_ENTRIES];
    size_t count;
    uint64_t clock;
    size_t evictions;
    int root_buckets[AURORA_DICTIONARY_ROOT_BUCKETS];
} aurora_dictionary;

typedef struct {
    size_t entries[AURORA_DICTIONARY_MAX_ENTRIES];
    size_t count;
    size_t roots_visited;
    size_t branches_rejected;
} aurora_dictionary_matches;

typedef struct {
    aurora_dictionary_matches matches;
    aurora_window_result window;
    aurora_do_cursor traversal;
    size_t candidates_tried;
    int selected_entry;
    bool needs_alternative;
    bool exhausted;
} aurora_dictionary_window_result;

void aurora_dictionary_init(aurora_dictionary *dictionary);

/* Registra una resolución nacida de las entradas, todavía no utilizable. */
int aurora_dictionary_propose(aurora_dictionary *dictionary,
                              const aurora_tensor *tensor,
                              const uint32_t *tokens,
                              size_t token_count,
                              size_t tensor_index);

/*
 * Aplica la dirección ejecutiva solo si la candidata forma parte real de la
 * procedencia de output_tensor. Cristalizar la habilita para búsquedas.
 */
bool aurora_dictionary_used_in_output(
    aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t entry_index,
    size_t output_tensor,
    aurora_learning_direction direction);

/* Coincidencia de prefijo: mayor longitud y, entre iguales, mayor recencia. */
int aurora_dictionary_lookup(aurora_dictionary *dictionary,
                             const uint32_t *tokens,
                             size_t token_count);

int aurora_dictionary_lookup_limit(aurora_dictionary *dictionary,
                                   const uint32_t *tokens,
                                   size_t token_count,
                                   size_t max_length);

const aurora_so *aurora_dictionary_knowledge(
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t entry_index);

/*
 * Navegación asociativa descendente. La geometría superior selecciona los
 * buckets ternarios; cada candidata debe cerrar después en sus tres destinos.
 * El resultado conserva todas las alternativas y las ordena por experiencia.
 */
aurora_dictionary_matches aurora_dictionary_search_tensor(
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t query_tensor);

/* Reconstruye el índice derivado, por ejemplo después de cargar un snapshot. */
bool aurora_dictionary_reindex(aurora_dictionary *dictionary,
                               const aurora_tensor *tensor);

/*
 * Recupera candidatas por geometría, las reejecuta sin efectos y materializa
 * únicamente la primera ventana que alcanza cierre. Cada intento consume un
 * estado distinto del recorrido DO.
 */
aurora_dictionary_window_result aurora_dictionary_reexecute_window(
    aurora_dictionary *dictionary,
    aurora_tensor *tensor,
    size_t query_tensor,
    size_t first,
    size_t second);

aurora_executive aurora_dictionary_decide(
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor,
    size_t entry_index,
    const aurora_so *input,
    const aurora_so *output,
    const aurora_harmonizer *harmonizer);

#endif
