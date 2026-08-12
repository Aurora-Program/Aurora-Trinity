#ifndef GENESIS_EDUCATION_H
#define GENESIS_EDUCATION_H

#include "genesis/random_origin.h"

typedef enum {
    GENESIS_ANSWER_UNKNOWN = 0,
    GENESIS_ANSWER_RESOLVED = 1,
    GENESIS_ANSWER_AMBIGUOUS = 2
} genesis_answer_state;

typedef enum {
    GENESIS_EVIDENCE_NONE = 0,
    GENESIS_EVIDENCE_LITERAL = 1,
    GENESIS_EVIDENCE_RULE = 2
} genesis_evidence;

typedef struct {
    genesis_answer_state state;
    genesis_evidence evidence;
    uint32_t object;
} genesis_answer;

#define GENESIS_EXPLANATION_MAX_CONDITIONS 3u

typedef struct {
    uint32_t relation;
    uint32_t object;
} genesis_condition;

typedef struct {
    genesis_answer answer;
    genesis_condition conditions[GENESIS_EXPLANATION_MAX_CONDITIONS];
    size_t condition_count;
    size_t support;
    size_t dictionary_entry;
    size_t uses_after_output;
} genesis_explanation;

typedef struct {
    aurora_tensor arena;
    aurora_dictionary dictionary;
    uint64_t public_seed;
} genesis_education;

void genesis_education_init(genesis_education *education,
                            uint64_t public_seed);
bool genesis_education_register_token(genesis_education *education,
                                      uint32_t token);
bool genesis_education_teach_fact(genesis_education *education,
                                  uint32_t subject,
                                  uint32_t relation,
                                  uint32_t object);

/* Añade la corrección y cristaliza discriminantes con soporte independiente. */
/* Las reglas pueden reunir hasta tres condiciones y gana la más específica. */
size_t genesis_education_correct(genesis_education *education,
                                 uint32_t subject,
                                 uint32_t relation,
                                 uint32_t correct_object);

/* Número de rectificaciones históricas conservadas en el diccionario. */
size_t genesis_education_revision_count(const genesis_education *education);

genesis_answer genesis_education_answer(genesis_education *education,
                                        uint32_t subject,
                                        uint32_t relation);

genesis_answer genesis_education_answer_explain(
    genesis_education *education,
    uint32_t subject,
    uint32_t relation,
    genesis_explanation *explanation);

aurora_snapshot_status genesis_education_save(const genesis_education *education,
                                              const char *path);
aurora_snapshot_status genesis_education_load(genesis_education *education,
                                              uint64_t public_seed,
                                              const char *path);

#endif
