#ifndef GENESIS_CURRICULUM_H
#define GENESIS_CURRICULUM_H

#include "genesis/education.h"

/* Identificadores públicos de la primera conversación educativa. */
typedef enum {
    GENESIS_GATO = 1,
    GENESIS_PERRO,
    GENESIS_HAMSTER,
    GENESIS_HALCON,
    GENESIS_AGUILA,
    GENESIS_PAJARO,
    GENESIS_CUERVO,
    GENESIS_MIRLO,
    GENESIS_LEON,
    GENESIS_TIGRE,
    GENESIS_LOBO,
    GENESIS_PEZ,
    GENESIS_TIBURON,
    GENESIS_SALMON,
    GENESIS_DELFIN,
    GENESIS_BALLENA,
    GENESIS_FOCA,
    GENESIS_VIVE_CON = 20,
    GENESIS_TIENE,
    GENESIS_COME,
    GENESIS_ES,
    GENESIS_VIVE_EN,
    GENESIS_RESPIRA,
    GENESIS_DIETA,
    GENESIS_HUMANOS = 40,
    GENESIS_PLUMAS,
    GENESIS_NEGRO,
    GENESIS_CARNE,
    GENESIS_CARNIVORO,
    GENESIS_AGUA,
    GENESIS_BRANQUIAS,
    GENESIS_MAMIFERO,
    GENESIS_PULMONES,
    GENESIS_DUERME_EN = 60,
    GENESIS_CASA,
    GENESIS_NIDO,
    GENESIS_TORRE
} genesis_foundation_token;

/* Puede repetirse: los hechos y reglas ya cristalizados no se duplican. */
bool genesis_curriculum_foundation(genesis_education *education);

#endif
