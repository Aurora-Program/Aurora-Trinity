#include <genesis/self_organization.h>

#include <stdio.h>

static void print_metrics(const char *name,
                          const genesis_self_organization_metrics *m) {
    printf("%s rounds=%zu stable=%s nodes=%zu collisions=%zu\n", name,
           m->rounds, m->stable ? "yes" : "no", m->tensor_nodes,
           m->final_collisions);
    printf("  initial within=%zu/%zu between=%zu/%zu\n",
           m->initial_within_distance, m->within_pairs,
           m->initial_between_distance, m->between_pairs);
    printf("  final   within=%zu/%zu between=%zu/%zu\n",
           m->final_within_distance, m->within_pairs,
           m->final_between_distance, m->between_pairs);
}

int main(void) {
    genesis_self_organization_metrics normal, permuted, other_seed;
    if (!genesis_self_organization_run(42u, false, &normal) ||
        !genesis_self_organization_run(42u, true, &permuted) ||
        !genesis_self_organization_run(99u, false, &other_seed)) return 1;
    print_metrics("normal", &normal);
    print_metrics("permuted", &permuted);
    print_metrics("seed99", &other_seed);
    return 0;
}
