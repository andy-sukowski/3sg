/* See LICENSE file for copyright and license details. */

#ifndef TMPL_H
#define TMPL_H

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct var {
	char *key, *val;
	struct var *next;
};

enum expr_type {
	EXPR_INVALID,
	EXPR_VAR,
	EXPR_CONTENT,
	EXPR_INCL,
	EXPR_IF,
	EXPR_ENDIF,
	EXPR_FOR,
	EXPR_ENDFOR,
	EXPR_REVFOR,
	EXPR_ENDREVFOR,
	EXPR_FORALL,
	EXPR_ENDFORALL,
	EXPR_REVFORALL,
	EXPR_ENDREVFORALL,
};

/* expression inside '[' and ']' */
struct expr {
	enum expr_type type;
	char *arg; /* NULL if unused */
	struct expr *next;
};

struct var *new_var(char *key, char *val);

void free_vars(struct var *v);

/* greedy parse upto '\n', '\r' or '\0',
 * return NULL on error */
struct var *parse_var(char **s);

/* parse all variables in string,
 * return NULL on error */
struct var *parse_vars(char **s);

struct expr *new_expr(enum expr_type type, char *arg);

void free_exprs(struct expr *x);

/* greedy parse expression in '[', ']',
 * expect '[' as first character,
 * return NULL on error */
struct expr *parse_expr(char **s);

#endif /* TMPL_H */
