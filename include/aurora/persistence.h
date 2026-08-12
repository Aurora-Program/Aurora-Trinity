#ifndef AURORA_PERSISTENCE_H
#define AURORA_PERSISTENCE_H

#include "aurora/dictionary.h"

typedef enum {
    AURORA_SNAPSHOT_OK = 0,
    AURORA_SNAPSHOT_IO_ERROR = 1,
    AURORA_SNAPSHOT_INVALID = 2,
    AURORA_SNAPSHOT_INCOMPATIBLE = 3
} aurora_snapshot_status;

/* Guarda conjuntamente diccionario, tensores, procedencia y reloj lógico. */
aurora_snapshot_status aurora_snapshot_save(
    const char *path,
    const aurora_dictionary *dictionary,
    const aurora_tensor *tensor);

/* Solo sustituye los objetos de destino cuando el snapshot pasa validación. */
aurora_snapshot_status aurora_snapshot_load(
    const char *path,
    aurora_dictionary *dictionary,
    aurora_tensor *tensor);

#endif
