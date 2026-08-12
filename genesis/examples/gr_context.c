#include <genesis/benchmark.h>

#include <stdio.h>

static void print_metrics(const char *name, const genesis_gr_metrics *m) {
    printf("%s: raw=%zu repeat=%zu heldout=%zu closed=%zu open=%zu "
           "contradictions=%zu root_collisions=%zu so_collisions=%zu "
           "dictionary=%zu\n",
           name, m->raw_windows, m->repeated_windows, m->heldout_windows,
           m->closed, m->open, m->contradictions,
           m->heldout_root_collisions, m->heldout_so_collisions,
           m->dictionary_entries);
}

int main(void) {
    genesis_gr_metrics gr, shuffled;
    if (!genesis_gr_context_benchmark(42u, false, &gr) ||
        !genesis_gr_context_benchmark(42u, true, &shuffled)) {
        return 1;
    }
    print_metrics("G-R", &gr);
    print_metrics("shuffled", &shuffled);
    return 0;
}
