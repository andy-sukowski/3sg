/* See LICENSE file for copyright and license details. */

#include <stdlib.h>
#include <stdio.h>

#include "tmpl.h"

/* just for testing */
int
main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <var> <expr>\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct var *v = parse_var(&argv[1]);
	if (!v) {
		fprintf(stderr, "Failed to parse variable\n");
		return EXIT_FAILURE;
	}
	printf("key:  \"%s\"\nval:  \"%s\"\n", v->key, v->val);
	free_vars(v);

	struct expr *x = parse_expr(&argv[2]);
	if (!x) {
		fprintf(stderr, "Failed to parse expression\n");
		return EXIT_FAILURE;
	}
	printf("type: \"%d\"\narg:  \"%s\"\n", x->type, x->arg);
	free_exprs(x);
	return EXIT_SUCCESS;
}

