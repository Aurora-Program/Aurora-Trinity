#ifndef AURORA_DOMAIN_H
#define AURORA_DOMAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Un dominio es un subconjunto de {0,1,2}, representado como una máscara. */
typedef uint8_t aurora_domain;

enum {
    AURORA_DOMAIN_EMPTY = 0u,
    AURORA_DOMAIN_0 = 1u << 0,
    AURORA_DOMAIN_1 = 1u << 1,
    AURORA_DOMAIN_2 = 1u << 2,
    AURORA_DOMAIN_ALL = AURORA_DOMAIN_0 | AURORA_DOMAIN_1 | AURORA_DOMAIN_2
};

aurora_domain aurora_domain_singleton(uint8_t value);
bool aurora_domain_is_valid(aurora_domain domain);
bool aurora_domain_contains(aurora_domain domain, uint8_t value);
bool aurora_domain_is_singleton(aurora_domain domain);
size_t aurora_domain_size(aurora_domain domain);
aurora_domain aurora_domain_intersection(aurora_domain left,
                                         aurora_domain right);

#endif
