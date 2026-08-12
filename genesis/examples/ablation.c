#include <genesis/ablation.h>

#include <stdio.h>

static void print_row(const char *name, const genesis_ablation_metrics *m) {
    printf("%-9s seen=%zu/6 transfer=%zu/4 abstain=%zu/2 false=%zu "
           "facts=%zu rules=%zu bytes=%zu\n", name, m->seen_correct,
           m->transfer_correct, m->abstention_correct, m->false_positives,
           m->active_facts, m->active_rules, m->representation_bytes);
}

int main(void) {
    genesis_ablation_report report;
    if (!genesis_ablation_run(&report)) return 1;
    print_row("literal", &report.literal);
    print_row("symbolic", &report.symbolic);
    print_row("trinity", &report.trinity);
    printf("seed_invariant=%s\n", report.seed_invariant ? "yes" : "no");
    return 0;
}
