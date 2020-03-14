#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "map.h"

/*
 * map_file_ro - map a file into memory
 */
bool
map_file_ro(uint8_t **data, size_t *length, const char *fn)
{
	struct stat status;
	int fd;

	*data = NULL;
	*length = 0;

	/* get and check file length */
	if (stat(fn, &status) == -1)
		return false;
	if (status.st_size == 0)
		return true;
	if (status.st_size > SIZE_MAX) {
		errno = EFBIG;	/* file too large */
		return false;
	}

	/* open file */
	fd = open(fn, O_RDONLY);
	if (fd == -1)
		return false;

	/* map file */
	*data = mmap(NULL, (size_t)status.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	*length = (size_t)status.st_size;
	close(fd);
	return *data != MAP_FAILED;
}

bool
map_remove(uint8_t *data, size_t length)
{
	if (length == 0)
		return true;

	return munmap(data, length) == 0;
}
