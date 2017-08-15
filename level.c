#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "level.h"

#define LEVEL_SIZE 576

int load_level_set(const char *path)
{
	FILE *fp;
	long size;
	int n;

	fp = fopen(path, "rb");
	if (fp == NULL) {
		fprintf(stderr, "could not open level file %s\n", path);
		return -1;
	}

	// get file size
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	n = size/LEVEL_SIZE;
	if (n < 0) {
		return -1;
	}

	free(levels);
	levels = malloc(n * LEVEL_SIZE);
	fread(levels, LEVEL_SIZE, n, fp);

	fclose(fp);

	num_levels = n;

	return 0;
}
