#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>

bool	map_file_ro(uint8_t **data, size_t *length, const char *fn);
bool	map_remove(uint8_t *data, size_t length);

#endif
