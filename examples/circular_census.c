#include <trinity/trinity.h>

#include <stdio.h>

int main(void) {
    aurora_circular_census c, sample;
    if (!aurora_circular_ring_census(&c) ||
        !aurora_circular_sample_census(42u, 10000u, &sample)) return 1;
    printf("networks=%zu\n", c.networks);
    printf("global contradiction=%zu unique=%zu open=%zu\n",
           c.global_contradictions, c.global_unique, c.global_open);
    printf("local  contradiction=%zu closed=%zu open=%zu\n",
           c.local_contradictions, c.local_closed, c.local_open);
    printf("missed unique=%zu contradiction=%zu extra_values=%zu unsound=%zu\n",
           c.unique_missed_locally, c.contradictions_missed_locally,
           c.values_pruned_globally, c.unsound_local_pruning);
    printf("sample networks=%zu global=(%zu,%zu,%zu) local=(%zu,%zu,%zu)\n",
           sample.networks, sample.global_contradictions,
           sample.global_unique, sample.global_open,
           sample.local_contradictions, sample.local_closed,
           sample.local_open);
    printf("sample missed_unique=%zu missed_contradiction=%zu extra_values=%zu "
           "unsound=%zu false_closed=%zu\n", sample.unique_missed_locally,
           sample.contradictions_missed_locally,
           sample.values_pruned_globally, sample.unsound_local_pruning,
           sample.false_local_closure);
    return 0;
}
