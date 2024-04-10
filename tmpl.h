/* See LICENSE file for copyright and license details. */

#ifndef TMPL_H
#define TMPL_H

struct var {
	char *key, *val;
	struct var *next;
};

enum expr_type {
	EXPR_PATH,
	EXPR_CONTENT,
	EXPR_INCL,
	EXPR_IF,
	EXPR_ENDIF,
	EXPR_IFNOT,
	EXPR_ENDIFNOT,
	EXPR_FOR,
	EXPR_ENDFOR,
	EXPR_REVFOR,
	EXPR_ENDREVFOR,
	EXPR_FORALL,
	EXPR_ENDFORALL,
	EXPR_REVFORALL,
	EXPR_ENDREVFORALL,
	/* not in expr_type_str */
	EXPR_INVALID,
	EXPR_VAR
};

/* expression inside '[' and ']' */
struct expr {
	enum expr_type type;
	char *arg; /* NULL if unused */
	struct expr *next;
};

struct var *new_var(char *key, char *val, struct var *next);

char *get_val(struct var *vars, const char *key);

void free_vars(struct var *v, struct var *until);

/* greedy parse upto '\n' or '\0',
 * return NULL on error */
struct var *parse_var(char **s);

/* parse all variables in string,
 * prepend most recent variable,
 * return 0 on success,
 * return line number on error */
int parse_vars(char *s, struct var **head);

struct expr *new_expr(enum expr_type type, char *arg, struct expr *next);

void free_exprs(struct expr *x);

/* greedy parse expression in '[', ']',
 * expect '[' as first character,
 * return NULL on error */
struct expr *parse_expr(char **s);

#endif /* TMPL_H */
