#ifndef KERNEL_ENDIAN_H
#define KERNEL_ENDIAN_H

#include <stdint.h>

// Little-endian to host (assuming host is little-endian for x86)
uint16_t le16toh(uint16_t x);
uint32_t le32toh(uint32_t x);

// Host to little-endian
uint16_t htole16(uint16_t x);
uint32_t htole32(uint32_t x);

#endif