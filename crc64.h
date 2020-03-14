#ifndef CRC64_H
#define CRC64_H

#include <stdint.h>
#include <stddef.h>

uint64_t crc64(uint64_t start, const uint8_t *buffer, size_t length);

#endif
