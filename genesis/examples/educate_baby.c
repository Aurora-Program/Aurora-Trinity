#include <genesis/curriculum.h>

#include <stdio.h>

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : "genesis_dictionary.bin";
    genesis_education baby;
    aurora_snapshot_status loaded =
        genesis_education_load(&baby, 42u, path);
    if (loaded != AURORA_SNAPSHOT_OK) {
        genesis_education_init(&baby, 42u);
    }

    size_t entries_before = baby.dictionary.count;
    size_t tensors_before = baby.arena.count;
    if (!genesis_curriculum_foundation(&baby)) {
        fprintf(stderr, "No se pudo completar el curriculo.\n");
        return 1;
    }
    if (genesis_education_save(&baby, path) != AURORA_SNAPSHOT_OK) {
        fprintf(stderr, "No se pudo guardar %s.\n", path);
        return 1;
    }

    genesis_explanation hamster_why;
    genesis_explanation halcon_why;
    genesis_explanation lobo_why;
    genesis_explanation salmon_why;
    genesis_explanation foca_why;
    genesis_explanation tiburon_why;
    genesis_explanation delfin_why;
    genesis_answer hamster = genesis_education_answer_explain(
        &baby, GENESIS_HAMSTER, GENESIS_DUERME_EN, &hamster_why);
    genesis_answer halcon = genesis_education_answer_explain(
        &baby, GENESIS_HALCON, GENESIS_DUERME_EN, &halcon_why);
    genesis_answer lobo = genesis_education_answer_explain(
        &baby, GENESIS_LOBO, GENESIS_DIETA, &lobo_why);
    genesis_answer salmon = genesis_education_answer_explain(
        &baby, GENESIS_SALMON, GENESIS_RESPIRA, &salmon_why);
    genesis_answer foca = genesis_education_answer_explain(
        &baby, GENESIS_FOCA, GENESIS_RESPIRA, &foca_why);
    genesis_answer tiburon = genesis_education_answer_explain(
        &baby, GENESIS_TIBURON, GENESIS_DIETA, &tiburon_why);
    genesis_answer delfin = genesis_education_answer_explain(
        &baby, GENESIS_DELFIN, GENESIS_DIETA, &delfin_why);
    if (genesis_education_save(&baby, path) != AURORA_SNAPSHOT_OK) {
        fprintf(stderr, "No se pudo guardar el uso de conocimiento.\n");
        return 1;
    }
    printf("sesion=%s entradas=%zu->%zu tensores=%zu->%zu\n",
           loaded == AURORA_SNAPSHOT_OK ? "continuada" : "inicial",
           entries_before, baby.dictionary.count,
           tensors_before, baby.arena.count);
    printf("hamster->%u halcon->%u lobo->%u salmon->%u foca->%u snapshot=%s\n",
           hamster.object, halcon.object, lobo.object, salmon.object,
           foca.object, path);
    printf("explicacion hamster: condiciones=%zu soporte=%zu usos=%zu\n",
           hamster_why.condition_count, hamster_why.support,
           hamster_why.uses_after_output);
    printf("explicacion halcon: condiciones=%zu soporte=%zu usos=%zu\n",
           halcon_why.condition_count, halcon_why.support,
           halcon_why.uses_after_output);
    printf("explicacion lobo: condiciones=%zu soporte=%zu usos=%zu\n",
           lobo_why.condition_count, lobo_why.support,
           lobo_why.uses_after_output);
    printf("explicacion salmon: condiciones=%zu soporte=%zu usos=%zu\n",
           salmon_why.condition_count, salmon_why.support,
           salmon_why.uses_after_output);
    printf("explicacion foca: condiciones=%zu soporte=%zu usos=%zu\n",
           foca_why.condition_count, foca_why.support,
           foca_why.uses_after_output);
    printf("integracion tiburon: dieta=%u condiciones=%zu soporte=%zu usos=%zu\n",
           tiburon.object, tiburon_why.condition_count, tiburon_why.support,
           tiburon_why.uses_after_output);
    printf("integracion delfin: dieta=%u condiciones=%zu soporte=%zu usos=%zu\n",
           delfin.object, delfin_why.condition_count, delfin_why.support,
           delfin_why.uses_after_output);
    return 0;
}
