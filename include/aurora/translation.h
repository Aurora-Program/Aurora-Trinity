#ifndef AURORA_TRANSLATION_H
#define AURORA_TRANSLATION_H

#include "aurora/dictionary.h"
#include "aurora/sequence.h"

typedef enum {
    AURORA_TRANSLATION_OK = 0,
    AURORA_TRANSLATION_UNKNOWN = 1,
    AURORA_TRANSLATION_INCOHERENT = 2,
    AURORA_TRANSLATION_CAPACITY = 3
} aurora_translation_status;

typedef struct {
    size_t tensors[AURORA_TENSOR_MAX_NODES];
    size_t count;
    size_t dictionary_hits;
    size_t simple_hits;
    size_t backoffs;
    size_t windows;
    size_t max_length_used;
    int output;
    aurora_translation_status status;
} aurora_translation;

/* Traduce y valida; simple_tokens/simple_tensors constituyen la base canónica. */
aurora_translation aurora_translate(
    aurora_dictionary *dictionary,
    aurora_tensor *tensor,
    const uint32_t *tokens,
    size_t token_count,
    const uint32_t *simple_tokens,
    const size_t *simple_tensors,
    size_t simple_count,
    bool validate);

#endif
