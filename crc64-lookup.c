#include "crc64.h"
#include "lookup.h"

uint64_t
crc64(uint64_t sum, const uint8_t *buffer, size_t length)
{
	const uint8_t *endp = buffer + length;

	while (buffer < endp)
		sum = (sum << 8) ^ crc64_lookup[(sum >> 56) ^ *buffer++];

	return sum;
}
