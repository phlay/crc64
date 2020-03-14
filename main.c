#include <sys/mman.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>

#include "map.h"
#include "crc64.h"

#define BUFLEN			4096
#define DEFAULT_START_CRC	0xffffffffffffffffUL


bool		opt_decimal = false;
bool		opt_negate = false;
bool		opt_show_filename = false;
bool		opt_show_length = false;
uint64_t	opt_startcrc = DEFAULT_START_CRC;


static void
usage()
{
	fprintf(stderr, "usage: crc64 [options] [files]\n\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -h\t\tshow this help message\n");
	fprintf(stderr, "  -d\t\tshow result in decimal instead of hexadecimal\n");
	fprintf(stderr, "  -n\t\tnegate result\n");
	fprintf(stderr, "  -f\t\tshow file names in result (always on for multiple inputs)\n");
	fprintf(stderr, "  -l\t\tshow file length in result\n");
	fprintf(stderr, "  -s <start>\tset start value for CRC (default is 0x%lx)\n", DEFAULT_START_CRC);
}

static void
print_result(uint64_t sum, size_t length, const char *fn)
{

	if (opt_decimal)
		printf("%lu", sum);
	else
		printf("%016lx", sum);

	if (opt_show_length)
		printf(" %lu", length);

	if (opt_show_filename)
		printf(" %s", fn);

	putchar('\n');
}

static bool
parse_u64(uint64_t *out, const char *str)
{
	char *endptr;
	int base = 10;

	if (str[0] == '\0')
		return false;

	if (str[0] == '0')
		base = tolower(str[1]) == 'x' ? 16 : 8;

	*out = strtoul(optarg, &endptr, base);
	if (*endptr != '\0')
		return false;

	return true;
}


int
main(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "dnfls:h")) != -1) {
		switch (opt) {
		case 'd':
			opt_decimal = true;
			break;
		case 'n':
			opt_negate = true;
			break;
		case 'f':
			opt_show_filename = true;
			break;
		case 'l':
			opt_show_length = true;
			break;
		case 's':
			if (!parse_u64(&opt_startcrc, optarg))
				errx(1, "%s: invalid start sum, unsigned integer expected", optarg);
			break;


		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(1);
		}

	}
	argc -= optind;
	argv += optind;

	if (argc > 1)
		opt_show_filename = true;


	if (argc == 0) {
		/*
		 * got no input files: use stdin as input.
		 */
		uint8_t buffer[BUFLEN];
		uint64_t sum = opt_startcrc;
		size_t length = 0;
		ssize_t n;

		while ((n = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
			sum = crc64(sum, buffer, n);
			length += n;
		}
		if (n == -1)
			err(1, "error reading from stdin");

		if (opt_negate)
			sum = ~sum;

		print_result(sum, length, "-");

	} else {
		/*
		 * loop over input files
		 */
		for (int i = 0; i < argc; i++) {
			uint8_t *data;
			size_t length;
			uint64_t sum;

			if (!map_file_ro(&data, &length, argv[i]))
				err(1, "%s: can't memory map file", argv[i]);

			sum = crc64(opt_startcrc, data, length);
			if (opt_negate)
				sum = ~sum;

			print_result(sum, length, argv[i]);

			if (!map_remove(data, length))
				err(1, "%s: can't unmap file", argv[i]);
		}
	}

	return 0;
}
