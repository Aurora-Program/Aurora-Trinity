#ifndef GENESIS_ABLATION_H
#define GENESIS_ABLATION_H

#include "genesis/education.h"

typedef struct {
    size_t seen_correct;
    size_t transfer_correct;
    size_t abstention_correct;
    size_t false_positives;
    size_t active_facts;
    size_t active_rules;
    size_t representation_bytes;
} genesis_ablation_metrics;

typedef struct {
    genesis_ablation_metrics literal;
    genesis_ablation_metrics symbolic;
    genesis_ablation_metrics trinity;
    bool seed_invariant;
} genesis_ablation_report;

bool genesis_ablation_run(genesis_ablation_report *report);

#endif
