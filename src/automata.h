#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool wolfberd(bool* neighbors, uint8_t rule) {
    int n = neighbors[-1] * 4 + neighbors[0] * 2 + neighbors[1];
    uint8_t mask = 1 << n;
    return (mask & rule) > 0;
}
