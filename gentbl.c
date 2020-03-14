/*
 * gentbl.c - generates lookup table for crc64-ecma code
 */

#include <stdint.h>
#include <stdio.h>
#include <endian.h>

//#define USE_BIG_ENDIAN


/* according to ECMA-182 we use
 *
 * p = x^64 + x^62 + x^57 + x^55 + x^54 + x^53 + x^52 + x^47 + x^46
 *   + x^45 + x^40 + x^39 + x^38 + x^37 + x^35 + x^33 + x^32 + x^31
 *   + x^29 + x^27 + x^24 + x^23 + x^22 + x^21 + x^19 + x^17 + x^13
 *   + x^12 + x^10 + x^9  + x^7  + x^4  + x    + 1
 *
 * Now CRC64_POL are the binary coefficiencts of f := x^64 mod p with
 * degree(f) < 64, where the LSB of CRC64_POL corresponds to x^0 and the
 * MSB corresponds to x^63.
 *
 * Or, alternatively,
 * 	CRC64_POL := p(2) - 2^64.
 */
#define CRC64_POL	0x42F0E1EBA9EA3693UL


uint64_t
crc(uint64_t input)
{
	input <<= 56;

	for (int i = 8; i > 0; i--) {
		if (input & 0x8000000000000000UL) {
			input = (input << 1) ^ CRC64_POL;
		} else
			input <<= 1;
	}

	return input;
}



int main()
{
	printf("/*\n * this file is auto-generated, please look at gentbl.c\n */\n\n");
	printf("#include <stdint.h>\n\n");
	printf("const uint64_t\ncrc64_lookup[] = {\n");

	for (int i = 0; i < 256; i++) {
#ifdef USE_BIG_ENDIAN
		printf("\t0x%016lX,\t\t/* %3i */\n", htobe64(crc(i)), i);
#else
		printf("\t0x%016lX,\t\t/* %3i */\n", crc(i), i);
#endif
	}
	printf("};\n");

	return 0;
}
