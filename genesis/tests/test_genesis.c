#include <genesis/benchmark.h>
#include <genesis/ablation.h>
#include <genesis/self_organization.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)

int main(void) {
    genesis_tensor117 first, repeated, different;
    CHECK(genesis_gr_generate(42u, 100u, &first));
    CHECK(genesis_gr_generate(42u, 100u, &repeated));
    CHECK(genesis_gr_generate(42u, 101u, &different));
    CHECK(memcmp(&first, &repeated, sizeof first) == 0);
    CHECK(memcmp(&first, &different, sizeof first) != 0);
    CHECK(genesis_tensor117_is_valid(&first));
    CHECK((first.bytes[14] & 0xe0u) == 0u);

    aurora_tensor arena;
    aurora_tensor_init(&arena);
    size_t root;
    CHECK(genesis_tensor117_to_trinity(&first, &arena, &root));
    CHECK(arena.nodes[root].depth == 2u);
    CHECK(arena.nodes[root].leaf_count == 9u);
    genesis_tensor117 round_trip;
    CHECK(genesis_tensor117_from_trinity(&arena, root, &round_trip));
    CHECK(memcmp(&first, &round_trip, sizeof first) == 0);

    genesis_tensor117 population[256];
    for (uint64_t token = 0u; token < 256u; ++token) {
        CHECK(genesis_gr_generate(42u, token, &population[token]));
        for (uint64_t previous = 0u; previous < token; ++previous) {
            CHECK(memcmp(&population[token], &population[previous],
                         sizeof population[token]) != 0);
        }
    }
    printf("Genesis G-R: %d comprobaciones, 256 tokens, 0 colisiones.\n",
           checks);

    genesis_gr_metrics gr, shuffled, repeated_metrics;
    CHECK(genesis_gr_context_benchmark(42u, false, &gr));
    CHECK(genesis_gr_context_benchmark(42u, true, &shuffled));
    CHECK(genesis_gr_context_benchmark(42u, false, &repeated_metrics));
    CHECK(memcmp(&gr, &repeated_metrics, sizeof gr) == 0);
    CHECK(gr.raw_windows == gr.training_phrases);
    CHECK(gr.repeated_windows == 0u);
    CHECK(gr.heldout_windows == gr.heldout_phrases);
    CHECK(gr.dictionary_entries == gr.training_phrases);
    CHECK(shuffled.raw_windows == shuffled.training_phrases);
    CHECK(shuffled.repeated_windows == 0u);
    CHECK(shuffled.heldout_windows == shuffled.heldout_phrases);

    genesis_ablation_report ablation;
    CHECK(genesis_ablation_run(&ablation));
    CHECK(ablation.literal.seen_correct == 6u);
    CHECK(ablation.literal.transfer_correct == 0u);
    CHECK(ablation.symbolic.transfer_correct == 4u);
    CHECK(ablation.trinity.transfer_correct == 4u);
    CHECK(ablation.symbolic.abstention_correct == 2u);
    CHECK(ablation.trinity.abstention_correct == 2u);
    CHECK(ablation.symbolic.false_positives == 0u);
    CHECK(ablation.trinity.false_positives == 0u);
    CHECK(ablation.seed_invariant);

    genesis_self_organization_metrics self42, self99, self_permuted;
    CHECK(genesis_self_organization_run(42u, false, &self42));
    CHECK(genesis_self_organization_run(42u, true, &self_permuted));
    CHECK(genesis_self_organization_run(99u, false, &self99));
    CHECK(self42.stable && self42.rounds == 2u);
    CHECK(self42.final_within_distance == 0u);
    CHECK(self42.final_between_distance == 0u);
    CHECK(self42.final_collisions == 66u);
    CHECK(self_permuted.final_collisions == 66u);
    CHECK(self99.final_collisions == 66u);
    return 0;
}
