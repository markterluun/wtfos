#include "endian.h"

uint16_t le16toh(uint16_t x) {
    // Assuming little-endian host
    return x;
}

uint32_t le32toh(uint32_t x) {
    return x;
}

uint16_t htole16(uint16_t x) {
    return x;
}

uint32_t htole32(uint32_t x) {
    return x;
}