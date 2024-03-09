/* See LICENSE file for copyright and license details. */

#include <stdlib.h>
#include <stdio.h>

#include "vars.h"

/* parse provided variable */
int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <var>\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct var *v = parsevar(&argv[1]);
	if (!v) {
		fprintf(stderr, "Failed to parse variable\n");
		return EXIT_FAILURE;
	}
	printf("key: \"%s\"\nval: \"%s\"\n", v->key, v->val);

	freevars(v);
	return EXIT_SUCCESS;
}

