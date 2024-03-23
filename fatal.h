/* See LICENSE file for copyright and license details. */

#ifndef FATAL_H
#define FATAL_H

#include <stdio.h>

void *emalloc(size_t size);
void *ecalloc(size_t nmemb, size_t size);

FILE *efopen(const char *path, const char *mode);
void efseek(FILE *f, long offset, int origin);
long eftell(FILE *f);
size_t efread(void *buffer, size_t size, size_t count, FILE *f);
void efclose(FILE *f);

void erealpath(const char *path, char *resolved);

#endif /* FATAL_H */
