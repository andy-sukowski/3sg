/* See LICENSE file for copyright and license details. */

#ifndef TMPL_H
#define TMPL_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct var {
	char *key, *val;
	struct var *next;
};

struct var *newvar(char *key, char *val);

void freevars(struct var *v);

/* greedy parse upto '\n', '\r' or '\0',
 * return NULL on error */
struct var *parsevar(char **s);

/* parse all variables in string,
 * return NULL on error */
struct var *parsevars(char *s);

#endif /* TMPL_H */
