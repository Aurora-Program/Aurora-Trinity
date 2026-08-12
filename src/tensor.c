#include "aurora/tensor.h"

#include <string.h>

void aurora_tensor_init(aurora_tensor *tensor) {
    if (tensor != NULL) {
        memset(tensor, 0, sizeof *tensor);
    }
}

const aurora_tensor_node *aurora_tensor_get(const aurora_tensor *tensor,
                                            size_t index) {
    return tensor != NULL && index < tensor->count ? &tensor->nodes[index]
                                                   : NULL;
}

int aurora_tensor_add_leaf(aurora_tensor *tensor,
                           const aurora_domain triplet[3]) {
    if (tensor == NULL || triplet == NULL ||
        tensor->count >= AURORA_TENSOR_MAX_NODES) {
        return -1;
    }
    aurora_ordering ordering = aurora_order_triplet(triplet);
    if (ordering.state == AURORA_CONTRADICTION) {
        return -1;
    }

    size_t index = tensor->count++;
    aurora_tensor_node *node = &tensor->nodes[index];
    node->superior = aurora_so_seed(triplet);
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        node->children[dimension] = AURORA_TENSOR_NO_CHILD;
        node->window_orientation[dimension] = AURORA_DOMAIN_EMPTY;
    }
    node->state = node->superior.state;
    node->has_window_orientation = false;
    node->depth = 0u;
    node->provenance_depth = 0u;
    node->leaf_count = 1u;
    node->is_leaf = true;
    return (int)index;
}

int aurora_tensor_compose(aurora_tensor *tensor,
                          size_t first,
                          size_t second,
                          size_t third) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_tensor_compose_at_do(tensor, first, second, third, origin);
}

int aurora_tensor_compose_at_do(aurora_tensor *tensor,
                                size_t first,
                                size_t second,
                                size_t third,
                                const aurora_domain do_state[3]) {
    int index = aurora_tensor_carry_at_do(
        tensor, first, second, third, do_state);
    if (index >= 0) {
        tensor->nodes[index].depth += 1u;
    }
    return index;
}

int aurora_tensor_carry(aurora_tensor *tensor,
                        size_t first,
                        size_t second,
                        size_t third) {
    static const aurora_domain origin[3] = {
        AURORA_DOMAIN_0, AURORA_DOMAIN_0, AURORA_DOMAIN_0};
    return aurora_tensor_carry_at_do(tensor, first, second, third, origin);
}

int aurora_tensor_carry_at_do(aurora_tensor *tensor,
                              size_t first,
                              size_t second,
                              size_t third,
                              const aurora_domain do_state[3]) {
    if (tensor == NULL || first >= tensor->count || second >= tensor->count ||
        third >= tensor->count || tensor->count >= AURORA_TENSOR_MAX_NODES ||
        do_state == NULL) {
        return -1;
    }
    const aurora_tensor_node *a = &tensor->nodes[first];
    const aurora_tensor_node *b = &tensor->nodes[second];
    const aurora_tensor_node *c = &tensor->nodes[third];
    if (a->depth != b->depth || a->depth != c->depth) {
        return -1;
    }

    aurora_so superior = aurora_so_relate_at_do(
        &a->superior, &b->superior, &c->superior, do_state);
    if (superior.state == AURORA_CONTRADICTION) {
        return -1;
    }

    return aurora_tensor_materialize(
        tensor, first, second, third, &superior, do_state, false);
}

int aurora_tensor_materialize(aurora_tensor *tensor,
                              size_t first,
                              size_t second,
                              size_t third,
                              const aurora_so *superior,
                              const aurora_domain u_orientation[3],
                              bool emerge) {
    if (tensor == NULL || superior == NULL || u_orientation == NULL ||
        superior->state == AURORA_CONTRADICTION || first >= tensor->count ||
        second >= tensor->count || third >= tensor->count ||
        tensor->count >= AURORA_TENSOR_MAX_NODES) {
        return -1;
    }
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        if (!aurora_domain_is_valid(u_orientation[dimension])) return -1;
    }
    const aurora_tensor_node *a = &tensor->nodes[first];
    const aurora_tensor_node *b = &tensor->nodes[second];
    const aurora_tensor_node *c = &tensor->nodes[third];
    if (a->depth != b->depth || a->depth != c->depth) return -1;

    size_t child_depth = a->depth;
    size_t provenance_depth = a->provenance_depth;
    if (b->provenance_depth > provenance_depth) {
        provenance_depth = b->provenance_depth;
    }
    if (c->provenance_depth > provenance_depth) {
        provenance_depth = c->provenance_depth;
    }
    size_t leaf_count = a->leaf_count + b->leaf_count + c->leaf_count;
    size_t index = tensor->count++;
    aurora_tensor_node *node = &tensor->nodes[index];
    node->superior = *superior;
    for (size_t dimension = 0u; dimension < 3u; ++dimension) {
        node->window_orientation[dimension] = u_orientation[dimension];
    }
    node->children[0] = first;
    node->children[1] = second;
    node->children[2] = third;
    node->depth = child_depth + (emerge ? 1u : 0u);
    node->provenance_depth = provenance_depth + 1u;
    node->leaf_count = leaf_count;
    node->state = superior->state;
    node->has_window_orientation = true;
    node->is_leaf = false;
    return (int)index;
}
