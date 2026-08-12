#include "genesis/curriculum.h"

static bool fact(genesis_education *education,
                 uint32_t subject,
                 uint32_t relation,
                 uint32_t object) {
    return genesis_education_teach_fact(education, subject, relation, object);
}

static bool correction(genesis_education *education,
                       uint32_t subject,
                       uint32_t relation,
                       uint32_t object) {
    (void)genesis_education_correct(education, subject, relation, object);
    genesis_answer answer =
        genesis_education_answer(education, subject, relation);
    return answer.state == GENESIS_ANSWER_RESOLVED &&
           answer.evidence == GENESIS_EVIDENCE_LITERAL &&
           answer.object == object;
}

bool genesis_curriculum_foundation(genesis_education *education) {
    if (education == NULL) {
        return false;
    }

    /* Dos ejemplos enseñan la convivencia; el hámster queda como transferencia. */
    if (!fact(education, GENESIS_GATO, GENESIS_VIVE_CON, GENESIS_HUMANOS) ||
        !correction(education, GENESIS_GATO, GENESIS_DUERME_EN,
                    GENESIS_CASA) ||
        !fact(education, GENESIS_PERRO, GENESIS_VIVE_CON, GENESIS_HUMANOS) ||
        !correction(education, GENESIS_PERRO, GENESIS_DUERME_EN,
                    GENESIS_CASA) ||
        !fact(education, GENESIS_HAMSTER, GENESIS_VIVE_CON,
              GENESIS_HUMANOS)) {
        return false;
    }

    /* Regla general y excepción compuesta, ambas nacidas de dos ejemplos. */
    if (!fact(education, GENESIS_AGUILA, GENESIS_TIENE, GENESIS_PLUMAS) ||
        !correction(education, GENESIS_AGUILA, GENESIS_DUERME_EN,
                    GENESIS_NIDO) ||
        !fact(education, GENESIS_PAJARO, GENESIS_TIENE, GENESIS_PLUMAS) ||
        !correction(education, GENESIS_PAJARO, GENESIS_DUERME_EN,
                    GENESIS_NIDO) ||
        !fact(education, GENESIS_CUERVO, GENESIS_TIENE, GENESIS_PLUMAS) ||
        !fact(education, GENESIS_CUERVO, GENESIS_TIENE, GENESIS_NEGRO) ||
        !correction(education, GENESIS_CUERVO, GENESIS_DUERME_EN,
                    GENESIS_TORRE) ||
        !fact(education, GENESIS_MIRLO, GENESIS_TIENE, GENESIS_PLUMAS) ||
        !fact(education, GENESIS_MIRLO, GENESIS_TIENE, GENESIS_NEGRO) ||
        !correction(education, GENESIS_MIRLO, GENESIS_DUERME_EN,
                    GENESIS_TORRE) ||
        !fact(education, GENESIS_HALCON, GENESIS_TIENE, GENESIS_PLUMAS) ||
        !fact(education, GENESIS_HALCON, GENESIS_TIENE, GENESIS_NEGRO)) {
        return false;
    }

    /* Segundo dominio: la misma forma educativa, sin cambiar el motor. */
    if (!fact(education, GENESIS_LEON, GENESIS_COME, GENESIS_CARNE) ||
        !correction(education, GENESIS_LEON, GENESIS_DIETA,
                    GENESIS_CARNIVORO) ||
        !fact(education, GENESIS_TIGRE, GENESIS_COME, GENESIS_CARNE) ||
        !correction(education, GENESIS_TIGRE, GENESIS_DIETA,
                    GENESIS_CARNIVORO) ||
        !fact(education, GENESIS_LOBO, GENESIS_COME, GENESIS_CARNE)) {
        return false;
    }

    /* Tercer dominio: regla acuática y excepción mamífera contextual. */
    if (!fact(education, GENESIS_PEZ, GENESIS_VIVE_EN, GENESIS_AGUA) ||
        !correction(education, GENESIS_PEZ, GENESIS_RESPIRA,
                    GENESIS_BRANQUIAS) ||
        !fact(education, GENESIS_TIBURON, GENESIS_VIVE_EN, GENESIS_AGUA) ||
        !correction(education, GENESIS_TIBURON, GENESIS_RESPIRA,
                    GENESIS_BRANQUIAS) ||
        !fact(education, GENESIS_DELFIN, GENESIS_VIVE_EN, GENESIS_AGUA) ||
        !fact(education, GENESIS_DELFIN, GENESIS_ES, GENESIS_MAMIFERO) ||
        !correction(education, GENESIS_DELFIN, GENESIS_RESPIRA,
                    GENESIS_PULMONES) ||
        !fact(education, GENESIS_BALLENA, GENESIS_VIVE_EN, GENESIS_AGUA) ||
        !fact(education, GENESIS_BALLENA, GENESIS_ES, GENESIS_MAMIFERO) ||
        !correction(education, GENESIS_BALLENA, GENESIS_RESPIRA,
                    GENESIS_PULMONES) ||
        !fact(education, GENESIS_SALMON, GENESIS_VIVE_EN, GENESIS_AGUA) ||
        !fact(education, GENESIS_FOCA, GENESIS_VIVE_EN, GENESIS_AGUA) ||
        !fact(education, GENESIS_FOCA, GENESIS_ES, GENESIS_MAMIFERO)) {
        return false;
    }

    /* Cuarta conversación: conectar dominios reutilizando sólo tokens vivos. */
    if (!fact(education, GENESIS_TIBURON, GENESIS_COME, GENESIS_CARNE) ||
        !fact(education, GENESIS_DELFIN, GENESIS_COME, GENESIS_CARNE) ||
        !fact(education, GENESIS_AGUILA, GENESIS_COME, GENESIS_CARNE) ||
        !fact(education, GENESIS_FOCA, GENESIS_COME, GENESIS_CARNE)) {
        return false;
    }

    genesis_answer hamster = genesis_education_answer(
        education, GENESIS_HAMSTER, GENESIS_DUERME_EN);
    genesis_answer halcon = genesis_education_answer(
        education, GENESIS_HALCON, GENESIS_DUERME_EN);
    genesis_answer lobo = genesis_education_answer(
        education, GENESIS_LOBO, GENESIS_DIETA);
    genesis_answer salmon = genesis_education_answer(
        education, GENESIS_SALMON, GENESIS_RESPIRA);
    genesis_answer foca = genesis_education_answer(
        education, GENESIS_FOCA, GENESIS_RESPIRA);
    genesis_answer tiburon_dieta = genesis_education_answer(
        education, GENESIS_TIBURON, GENESIS_DIETA);
    genesis_answer delfin_dieta = genesis_education_answer(
        education, GENESIS_DELFIN, GENESIS_DIETA);
    genesis_answer delfin_tipo = genesis_education_answer(
        education, GENESIS_DELFIN, GENESIS_ES);
    genesis_answer tiburon_tipo = genesis_education_answer(
        education, GENESIS_TIBURON, GENESIS_ES);
    return hamster.state == GENESIS_ANSWER_RESOLVED &&
           hamster.evidence == GENESIS_EVIDENCE_RULE &&
           hamster.object == GENESIS_CASA &&
           halcon.state == GENESIS_ANSWER_RESOLVED &&
           halcon.evidence == GENESIS_EVIDENCE_RULE &&
           halcon.object == GENESIS_TORRE &&
           lobo.state == GENESIS_ANSWER_RESOLVED &&
           lobo.evidence == GENESIS_EVIDENCE_RULE &&
           lobo.object == GENESIS_CARNIVORO &&
           salmon.state == GENESIS_ANSWER_RESOLVED &&
           salmon.evidence == GENESIS_EVIDENCE_RULE &&
           salmon.object == GENESIS_BRANQUIAS &&
           foca.state == GENESIS_ANSWER_RESOLVED &&
           foca.evidence == GENESIS_EVIDENCE_RULE &&
           foca.object == GENESIS_PULMONES &&
           tiburon_dieta.state == GENESIS_ANSWER_RESOLVED &&
           tiburon_dieta.object == GENESIS_CARNIVORO &&
           delfin_dieta.state == GENESIS_ANSWER_RESOLVED &&
           delfin_dieta.object == GENESIS_CARNIVORO &&
           delfin_tipo.state == GENESIS_ANSWER_RESOLVED &&
           delfin_tipo.object == GENESIS_MAMIFERO &&
           tiburon_tipo.state == GENESIS_ANSWER_UNKNOWN;
}
