#include "aurora/persistence.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define AURORA_SNAPSHOT_VERSION 3u

typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t node_size;
    uint32_t entry_size;
    uint32_t reserved;
    uint64_t tensor_count;
    uint64_t dictionary_count;
    uint64_t clock;
    uint64_t evictions;
    uint64_t checksum;
} snapshot_header;

static uint64_t hash_bytes(uint64_t hash, const void *data, size_t size) {
    const unsigned char *bytes = data;
    for (size_t index = 0u; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint64_t payload_hash(const aurora_dictionary *dictionary,
                             const aurora_tensor *tensor) {
    uint64_t hash = UINT64_C(14695981039346656037);
    hash = hash_bytes(hash, tensor->nodes,
                      tensor->count * sizeof tensor->nodes[0]);
    return hash_bytes(hash, dictionary->entries,
                      dictionary->count * sizeof dictionary->entries[0]);
}

static bool valid_so(const aurora_so *so) {
    if (so->state > AURORA_OPEN) {
        return false;
    }
    const aurora_domain *groups[4] = {so->upper_ds, so->ds, so->de, so->do_};
    for (size_t group = 0u; group < 4u; ++group) {
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            if (!aurora_domain_is_valid(groups[group][dimension])) {
                return false;
            }
        }
    }
    return true;
}

static bool valid_payload(const aurora_dictionary *dictionary,
                          const aurora_tensor *tensor) {
    for (size_t index = 0u; index < tensor->count; ++index) {
        const aurora_tensor_node *node = &tensor->nodes[index];
        if (!valid_so(&node->superior) || node->state > AURORA_OPEN) {
            return false;
        }
        for (size_t dimension = 0u; dimension < 3u; ++dimension) {
            if (node->has_window_orientation) {
                if (!aurora_domain_is_valid(
                        node->window_orientation[dimension])) {
                    return false;
                }
            } else if (node->window_orientation[dimension] !=
                       AURORA_DOMAIN_EMPTY) {
                return false;
            }
        }
        if (node->is_leaf == node->has_window_orientation) {
            /* Las hojas no nacen de U; toda salida materializada sí. */
            return false;
        }
        for (size_t child = 0u; child < 3u; ++child) {
            if (node->is_leaf) {
                if (node->children[child] != AURORA_TENSOR_NO_CHILD) {
                    return false;
                }
            } else if (node->children[child] >= index) {
                /* La procedencia siempre apunta hacia nodos anteriores. */
                return false;
            }
        }
    }
    for (size_t index = 0u; index < dictionary->count; ++index) {
        const aurora_dictionary_entry *entry = &dictionary->entries[index];
        if (entry->token_count == 0u ||
            entry->token_count > AURORA_DICTIONARY_MAX_TOKENS ||
            entry->tensor >= tensor->count ||
            entry->state > AURORA_ENTRY_REJECTED) {
            return false;
        }
    }
    return true;
}

static bool replace_file(const char *temporary, const char *path) {
#ifdef _WIN32
    return MoveFileExA(temporary, path,
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(temporary, path) == 0;
#endif
}

aurora_snapshot_status aurora_snapshot_save(
    const char *path,
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor) {
    if (path == NULL || dictionary == NULL || tensor == NULL ||
        !valid_payload(dictionary, tensor)) {
        return AURORA_SNAPSHOT_INVALID;
    }
    char temporary[1024];
    int length = snprintf(temporary, sizeof temporary, "%s.tmp", path);
    if (length < 0 || (size_t)length >= sizeof temporary) {
        return AURORA_SNAPSHOT_INVALID;
    }
    FILE *file = fopen(temporary, "wb");
    if (file == NULL) {
        return AURORA_SNAPSHOT_IO_ERROR;
    }
    snapshot_header header = {0};
    memcpy(header.magic, "AURORA\0\1", 8u);
    header.version = AURORA_SNAPSHOT_VERSION;
    header.node_size = (uint32_t)sizeof tensor->nodes[0];
    header.entry_size = (uint32_t)sizeof dictionary->entries[0];
    header.tensor_count = tensor->count;
    header.dictionary_count = dictionary->count;
    header.clock = dictionary->clock;
    header.evictions = dictionary->evictions;
    header.checksum = payload_hash(dictionary, tensor);
    bool written = fwrite(&header, sizeof header, 1u, file) == 1u &&
                   fwrite(tensor->nodes, sizeof tensor->nodes[0], tensor->count,
                          file) == tensor->count &&
                   fwrite(dictionary->entries, sizeof dictionary->entries[0],
                          dictionary->count, file) == dictionary->count &&
                   fflush(file) == 0;
    bool closed = fclose(file) == 0;
    if (!written || !closed || !replace_file(temporary, path)) {
        remove(temporary);
        return AURORA_SNAPSHOT_IO_ERROR;
    }
    return AURORA_SNAPSHOT_OK;
}

aurora_snapshot_status aurora_snapshot_load(
    const char *path,
    aurora_dictionary *dictionary,
    aurora_tensor *tensor) {
    if (path == NULL || dictionary == NULL || tensor == NULL) {
        return AURORA_SNAPSHOT_INVALID;
    }
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return AURORA_SNAPSHOT_IO_ERROR;
    }
    snapshot_header header = {0};
    if (fread(&header, sizeof header, 1u, file) != 1u) {
        fclose(file);
        return AURORA_SNAPSHOT_INVALID;
    }
    if (memcmp(header.magic, "AURORA\0\1", 8u) != 0 ||
        header.version != AURORA_SNAPSHOT_VERSION ||
        header.node_size != sizeof tensor->nodes[0] ||
        header.entry_size != sizeof dictionary->entries[0]) {
        fclose(file);
        return AURORA_SNAPSHOT_INCOMPATIBLE;
    }
    if (header.tensor_count > AURORA_TENSOR_MAX_NODES ||
        header.dictionary_count > AURORA_DICTIONARY_MAX_ENTRIES) {
        fclose(file);
        return AURORA_SNAPSHOT_INVALID;
    }
    aurora_tensor loaded_tensor;
    aurora_dictionary loaded_dictionary;
    aurora_tensor_init(&loaded_tensor);
    aurora_dictionary_init(&loaded_dictionary);
    loaded_tensor.count = (size_t)header.tensor_count;
    loaded_dictionary.count = (size_t)header.dictionary_count;
    loaded_dictionary.clock = header.clock;
    loaded_dictionary.evictions = (size_t)header.evictions;
    bool read = fread(loaded_tensor.nodes, sizeof loaded_tensor.nodes[0],
                      loaded_tensor.count, file) == loaded_tensor.count &&
                fread(loaded_dictionary.entries,
                      sizeof loaded_dictionary.entries[0],
                      loaded_dictionary.count, file) == loaded_dictionary.count;
    int trailing = fgetc(file);
    fclose(file);
    if (!read || trailing != EOF || !valid_payload(&loaded_dictionary,
                                                    &loaded_tensor) ||
        payload_hash(&loaded_dictionary, &loaded_tensor) != header.checksum) {
        return AURORA_SNAPSHOT_INVALID;
    }
    if (!aurora_dictionary_reindex(&loaded_dictionary, &loaded_tensor)) {
        return AURORA_SNAPSHOT_INVALID;
    }
    *tensor = loaded_tensor;
    *dictionary = loaded_dictionary;
    return AURORA_SNAPSHOT_OK;
}
