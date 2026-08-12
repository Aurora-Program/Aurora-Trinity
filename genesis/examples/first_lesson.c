#include <genesis/education.h>

#include <stdio.h>

enum {
    GATO = 1, PERRO = 2, HAMSTER = 3,
    VIVE_CON = 20, HUMANOS = 40, DUERME_EN = 60, CASA = 61
};

int main(void) {
    genesis_education baby;
    genesis_education_init(&baby, 42u);
    genesis_education_teach_fact(&baby, GATO, VIVE_CON, HUMANOS);
    genesis_education_correct(&baby, GATO, DUERME_EN, CASA);
    genesis_education_teach_fact(&baby, PERRO, VIVE_CON, HUMANOS);
    genesis_education_correct(&baby, PERRO, DUERME_EN, CASA);
    genesis_education_teach_fact(&baby, HAMSTER, VIVE_CON, HUMANOS);
    genesis_answer answer =
        genesis_education_answer(&baby, HAMSTER, DUERME_EN);
    printf("hamster duerme_en %u (estado=%u, evidencia=%u)\n",
           answer.object, (unsigned)answer.state, (unsigned)answer.evidence);
    return answer.state == GENESIS_ANSWER_RESOLVED && answer.object == CASA
               ? 0
               : 1;
}
