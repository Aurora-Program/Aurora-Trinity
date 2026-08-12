#include <trinity/trinity.h>

#include <stdio.h>

int main(void) {
    printf("Trinity %s: Majority3(0, 0, 1) = %u\n",
           TRINITY_VERSION_STRING,
           (unsigned)aurora_majority3(0u, 0u, 1u));
    return aurora_majority3(0u, 0u, 1u) == 0u ? 0 : 1;
}
