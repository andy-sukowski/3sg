/* See LICENSE file for copyright and license details. */

#include <stdlib.h>
#include <stdio.h>

#include "tmpl.h"

/* just for testing */
int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <vars>\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct var *v = NULL;
	int l = parse_vars(&argv[1], &v);
	if (l != 0) {
		fprintf(stderr, "Failed to parse variable in line %i\n", l);
		free_vars(v, NULL);
		return EXIT_FAILURE;
	}
	for (struct var *p = v; p; p = p->next)
		printf("%s=%s\n", p->key, p->val);
	free_vars(v, NULL);
	return EXIT_SUCCESS;
}

