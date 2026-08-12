#include "aurora/domain.h"

aurora_domain aurora_domain_singleton(uint8_t value) {
    return value < 3u ? (aurora_domain)(1u << value) : AURORA_DOMAIN_EMPTY;
}

bool aurora_domain_is_valid(aurora_domain domain) {
    return domain != AURORA_DOMAIN_EMPTY &&
           (domain & (aurora_domain)~AURORA_DOMAIN_ALL) == 0u;
}

bool aurora_domain_contains(aurora_domain domain, uint8_t value) {
    return value < 3u && (domain & aurora_domain_singleton(value)) != 0u;
}

bool aurora_domain_is_singleton(aurora_domain domain) {
    return aurora_domain_is_valid(domain) && (domain & (domain - 1u)) == 0u;
}

size_t aurora_domain_size(aurora_domain domain) {
    size_t count = 0u;
    for (uint8_t value = 0u; value < 3u; ++value) {
        if (aurora_domain_contains(domain, value)) {
            ++count;
        }
    }
    return count;
}

aurora_domain aurora_domain_intersection(aurora_domain left,
                                         aurora_domain right) {
    return (aurora_domain)(left & right & AURORA_DOMAIN_ALL);
}
