/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fatal.h"

void *
emalloc(size_t size)
{
	void *p = malloc(size);
	if (!p) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);
	if (!p) {
		fprintf(stderr, "calloc: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

FILE *
efopen(const char *path, const char *mode)
{
	FILE *f = fopen(path, mode);
	if (!f) {
		fprintf(stderr, "fopen %s with %s mode: %s\n",
			path, mode, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return f;
}

void
efseek(FILE *f, long offset, int origin)
{
	if (fseek(f, offset, origin)) {
		fprintf(stderr, "fseek: %s\n", strerror(errno));
		efclose(f);
		exit(EXIT_FAILURE);
	}
}

long
eftell(FILE *f)
{
	long offset = ftell(f);
	if (offset == -1) {
		fprintf(stderr, "ftell: %s\n", strerror(errno));
		efclose(f);
		exit(EXIT_FAILURE);
	}
	return offset;
}

size_t
efread(void *buffer, size_t size, size_t count, FILE *f)
{
	size_t n = fread(buffer, size, count, f);
	if (ferror(f)) {
		fprintf(stderr, "fread: %s\n", strerror(errno));
		efclose(f);
		exit(EXIT_FAILURE);
	}
	return n;
}

void
efclose(FILE *f)
{
	if (fclose(f) == EOF) {
		fprintf(stderr, "fclose: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void
erealpath(const char *path, char *resolved)
{
	if (!realpath(path, resolved)) {
		fprintf(stderr, "realpath of %s: %s\n", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
}
