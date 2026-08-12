#include <genesis/education.h>
#include <genesis/curriculum.h>

#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)

enum {
    GATO = 1, PERRO, VACA, HAMSTER, MURCIELAGO, AGUILA, HALCON, PAJARO,
    CUERVO, MIRLO, ZORRO, CONDOR,
    VIVE_CON = 20, VIVE_EN, VUELA, TIENE,
    HUMANOS = 40, GRANJA, PLUMAS, NEGRO,
    DUERME_EN = 60, CASA, ESTABLO, CUEVA, NIDO, TORRE
};

int main(void) {
    const char *snapshot = "genesis_education_test.bin";
    remove(snapshot);
    genesis_education baby;
    genesis_education_init(&baby, 42u);

    CHECK(genesis_education_teach_fact(&baby, GATO, VIVE_CON, HUMANOS));
    CHECK(genesis_education_correct(&baby, GATO, DUERME_EN, CASA) == 0u);
    CHECK(genesis_education_teach_fact(&baby, HAMSTER, VIVE_CON, HUMANOS));
    CHECK(genesis_education_answer(&baby, HAMSTER, DUERME_EN).state ==
          GENESIS_ANSWER_UNKNOWN);

    CHECK(genesis_education_teach_fact(&baby, PERRO, VIVE_CON, HUMANOS));
    CHECK(genesis_education_correct(&baby, PERRO, DUERME_EN, CASA) == 1u);
    CHECK(genesis_education_teach_fact(&baby, VACA, VIVE_EN, GRANJA));
    CHECK(genesis_education_correct(&baby, VACA, DUERME_EN, ESTABLO) == 0u);
    genesis_answer hamster =
        genesis_education_answer(&baby, HAMSTER, DUERME_EN);
    CHECK(hamster.state == GENESIS_ANSWER_RESOLVED);
    CHECK(hamster.evidence == GENESIS_EVIDENCE_RULE && hamster.object == CASA);

    CHECK(genesis_education_teach_fact(&baby, MURCIELAGO, VUELA, VUELA));
    CHECK(genesis_education_correct(&baby, MURCIELAGO, DUERME_EN, CUEVA) == 0u);
    CHECK(genesis_education_teach_fact(&baby, AGUILA, VUELA, VUELA));
    CHECK(genesis_education_correct(&baby, AGUILA, DUERME_EN, NIDO) == 0u);
    CHECK(genesis_education_teach_fact(&baby, HALCON, VUELA, VUELA));
    CHECK(genesis_education_answer(&baby, HALCON, DUERME_EN).state ==
          GENESIS_ANSWER_UNKNOWN);

    CHECK(genesis_education_teach_fact(&baby, AGUILA, TIENE, PLUMAS));
    CHECK(genesis_education_teach_fact(&baby, PAJARO, TIENE, PLUMAS));
    CHECK(genesis_education_correct(&baby, PAJARO, DUERME_EN, NIDO) == 1u);
    CHECK(genesis_education_teach_fact(&baby, HALCON, TIENE, PLUMAS));
    genesis_answer halcon = genesis_education_answer(&baby, HALCON, DUERME_EN);
    CHECK(halcon.state == GENESIS_ANSWER_RESOLVED);
    CHECK(halcon.evidence == GENESIS_EVIDENCE_RULE && halcon.object == NIDO);

    /* Al rectificar un soporte, la antigua regla deja de ser operativa. */
    CHECK(genesis_education_correct(&baby, PAJARO, DUERME_EN, TORRE) == 0u);
    CHECK(genesis_education_teach_fact(&baby, CONDOR, TIENE, PLUMAS));
    CHECK(genesis_education_answer(&baby, CONDOR, DUERME_EN).state ==
          GENESIS_ANSWER_UNKNOWN);

    /* NEGRO solo contradice; PLUMAS+NEGRO identifica la excepción. */
    CHECK(genesis_education_teach_fact(&baby, GATO, TIENE, NEGRO));
    CHECK(genesis_education_teach_fact(&baby, CUERVO, TIENE, PLUMAS));
    CHECK(genesis_education_teach_fact(&baby, CUERVO, TIENE, NEGRO));
    CHECK(genesis_education_correct(&baby, CUERVO, DUERME_EN, TORRE) == 0u);
    CHECK(genesis_education_teach_fact(&baby, MIRLO, TIENE, PLUMAS));
    CHECK(genesis_education_teach_fact(&baby, MIRLO, TIENE, NEGRO));
    CHECK(genesis_education_correct(&baby, MIRLO, DUERME_EN, TORRE) == 1u);
    CHECK(genesis_education_teach_fact(&baby, HALCON, TIENE, NEGRO));
    halcon = genesis_education_answer(&baby, HALCON, DUERME_EN);
    CHECK(halcon.state == GENESIS_ANSWER_RESOLVED && halcon.object == TORRE);

    CHECK(genesis_education_teach_fact(&baby, ZORRO, DUERME_EN, CUEVA));
    CHECK(genesis_education_answer(&baby, ZORRO, DUERME_EN).object == CUEVA);
    (void)genesis_education_correct(&baby, ZORRO, DUERME_EN, CASA);
    genesis_answer zorro =
        genesis_education_answer(&baby, ZORRO, DUERME_EN);
    CHECK(zorro.state == GENESIS_ANSWER_RESOLVED);
    CHECK(zorro.evidence == GENESIS_EVIDENCE_LITERAL && zorro.object == CASA);
    CHECK(genesis_education_revision_count(&baby) == 2u);

    genesis_explanation why_halcon;
    halcon = genesis_education_answer_explain(
        &baby, HALCON, DUERME_EN, &why_halcon);
    CHECK(halcon.object == TORRE);
    CHECK(why_halcon.condition_count == 2u);
    CHECK(why_halcon.support == 2u);
    CHECK(why_halcon.dictionary_entry != SIZE_MAX);
    CHECK(why_halcon.uses_after_output > 0u);
    size_t explained_entry = why_halcon.dictionary_entry;
    size_t explained_uses = why_halcon.uses_after_output;

    CHECK(genesis_education_save(&baby, snapshot) == AURORA_SNAPSHOT_OK);
    genesis_education restored;
    genesis_education_init(&restored, 999u);
    CHECK(genesis_education_load(&restored, 42u, snapshot) ==
          AURORA_SNAPSHOT_OK);
    CHECK(restored.dictionary.entries[explained_entry].uses == explained_uses);
    hamster = genesis_education_answer(&restored, HAMSTER, DUERME_EN);
    halcon = genesis_education_answer(&restored, HALCON, DUERME_EN);
    zorro = genesis_education_answer(&restored, ZORRO, DUERME_EN);
    CHECK(hamster.state == GENESIS_ANSWER_RESOLVED && hamster.object == CASA);
    CHECK(halcon.state == GENESIS_ANSWER_RESOLVED && halcon.object == TORRE);
    CHECK(zorro.state == GENESIS_ANSWER_RESOLVED && zorro.object == CASA);
    CHECK(genesis_education_revision_count(&restored) == 2u);
    CHECK(remove(snapshot) == 0);

    genesis_education student;
    genesis_education_init(&student, 42u);
    CHECK(genesis_curriculum_foundation(&student));
    size_t curriculum_entries = student.dictionary.count;
    size_t curriculum_tensors = student.arena.count;
    CHECK(genesis_curriculum_foundation(&student));
    CHECK(student.dictionary.count == curriculum_entries);
    CHECK(student.arena.count == curriculum_tensors);

    printf("Genesis Education: explanation, usage, snapshot OK.\n");
    return 0;
}
