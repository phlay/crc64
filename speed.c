#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <err.h>

#define BUFLEN	(1024*1024)
#define COUNT	1024

#include "crc64.h"

int
main()
{
	struct timeval tv0, tv1;
	double delta, rate;

	uint8_t buffer[BUFLEN];
	long n;

	uint64_t sum = (uint64_t)-1;

	for (n = 0; n < BUFLEN; ++n)
		buffer[n] = n & 0xff;

	gettimeofday(&tv0, NULL);
	for (n = 0; n < COUNT; n++)
		sum = crc64(sum, buffer, BUFLEN);
	sum = ~sum;
	gettimeofday(&tv1, NULL);

	printf("crc64-ecma: 0x%016lx\n", sum);

	delta = (double)(tv1.tv_sec - tv0.tv_sec);
	delta += (double)(tv1.tv_usec - tv0.tv_usec) / 1000000.;
	rate = (double)COUNT / delta;
	printf("speed: %f MiB/s\n", rate);

	return 0;
}
