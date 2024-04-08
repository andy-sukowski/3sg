/* See LICENSE file for copyright and license details. */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "fatal.h"
#include "tmpl.h"

struct var *
new_var(char *key, char *val, struct var *next)
{
	struct var *v = emalloc(sizeof *v);
	v->key = key;
	v->val = val;
	v->next = next;
	return v;
}

char *
get_val(struct var *vars, const char *key)
{
	for (struct var *v = vars; v; v = v->next)
		if (!strcmp(v->key, key))
			return v->val;
	return NULL;
}

void
free_vars(struct var *v, struct var *until)
{
	struct var *tmp;
	while (v && v != until) {
		tmp = v;
		v = v->next;
		free(tmp->key);
		free(tmp->val);
		free(tmp);
	}
}

/* greedy parse upto '=',
 * return NULL on error */
char *
parse_key(char **s)
{
	while (isblank(**s))
		++*s;
	if (strchr("=\n", **s))
		return NULL;
	char *start = *s;

	*s = &(*s)[strcspn(*s, " \t\n=")];
	if (strchr("\n", **s))
		return NULL;
	char *end = *s;

	while (isblank(**s))
		++*s;
	if (**s != '=')
		return NULL;

	char *key = ecalloc(1, end - start + 1);
	memcpy(key, start, end - start);
	return key;
}

/* greedy parse upto '\n' or '\0' */
char *
parse_val(char **s)
{
	while (isblank(**s))
		++*s;
	char *start = *s;

	*s = &(*s)[strcspn(*s, "\n")];
	char *end = *s;
	while (isblank(*(end - 1)))
		--end;

	char *val = ecalloc(1, end - start + 1);
	memcpy(val, start, end - start);
	return val;
}

/* greedy parse upto '\n' or '\0',
 * return NULL on error */
struct var *
parse_var(char **s)
{
	char *key = parse_key(s);
	if (!key)
		return NULL;
	++*s; /* skip '=' */
	char *val = parse_val(s);
	if (!val) {
		free(key);
		return NULL;
	}
	return new_var(key, val, NULL);
}

/* parse all variables in string,
 * prepend most recent variable,
 * return 0 on success,
 * return line number on error */
int
parse_vars(char *s, struct var **head)
{
	struct var *v;
	for (int l = 1; *s; ++l) {
		v = parse_var(&s);
		if (!v)
			return l;
		if (*s)
			++s; /* skip '\n' */
		v->next = *head;
		*head = v;
	}
	return 0;
}

struct expr *
new_expr(enum expr_type type, char *arg, struct expr *next)
{
	struct expr *x = emalloc(sizeof *x);
	x->type = type;
	x->arg = arg;
	x->next = next;
	return x;
}

void
free_exprs(struct expr *x)
{
	struct expr *tmp;
	while (x) {
		tmp = x;
		x = x->next;
		free(tmp->arg);
		free(tmp);
	}
}

/* greedy parse expression type,
 * don't trim before and after,
 * return EXPR_INVALID on error */
enum expr_type
parse_expr_type(char **s)
{
	if (islower(**s))
		return EXPR_VAR;

	char *start = *s;
	*s = &(*s)[strcspn(*s, " \t[]\n")];
	if (strchr("[\n", **s))
		return EXPR_INVALID;

	char tmp = **s;
	**s = '\0';
	enum expr_type t;
	if (!strcmp(start, "PATH")) {
		t = EXPR_PATH;
	} else if (!strcmp(start, "CONTENT")) {
		t = EXPR_CONTENT;
	} else if (!strcmp(start, "INCL")) {
		t = EXPR_INCL;
	} else if (!strcmp(start, "IF")) {
		t = EXPR_IF;
	} else if (!strcmp(start, "ENDIF")) {
		t = EXPR_ENDIF;
	} else if (!strcmp(start, "FOR")) {
		t = EXPR_FOR;
	} else if (!strcmp(start, "ENDFOR")) {
		t = EXPR_ENDFOR;
	} else if (!strcmp(start, "REVFOR")) {
		t = EXPR_REVFOR;
	} else if (!strcmp(start, "ENDREVFOR")) {
		t = EXPR_ENDREVFOR;
	} else if (!strcmp(start, "FORALL")) {
		t = EXPR_FORALL;
	} else if (!strcmp(start, "ENDFORALL")) {
		t = EXPR_ENDFORALL;
	} else if (!strcmp(start, "REVFORALL")) {
		t = EXPR_REVFORALL;
	} else if (!strcmp(start, "ENDREVFORALL")) {
		t = EXPR_ENDREVFORALL;
	} else {
		t = EXPR_INVALID;
	}
	**s = tmp;
	return t;
}

bool
has_arg(enum expr_type t)
{
	enum expr_type have_arg[7] = {
		EXPR_VAR,
		EXPR_INCL,
		EXPR_IF,
		EXPR_FOR,
		EXPR_REVFOR,
		EXPR_FORALL,
		EXPR_REVFORALL
	};
	for (int i = 0; i < 7; ++i)
		if (t == have_arg[i])
			return true;
	return false;
}

/* greedy parse expression argument,
 * don't trim before and after,
 * return NULL on error */
char *
parse_arg(char **s)
{
	char *start = *s;
	*s = &(*s)[strcspn(*s, " \t[]\n")];
	if (strchr("[\n", **s))
		return NULL;

	char *arg = ecalloc(1, *s - start + 1);
	memcpy(arg, start, *s - start);
	return arg;
}

/* greedy parse expression in '[', ']',
 * expect '[' as first character,
 * return NULL on error */
struct expr *
parse_expr(char **s)
{
	if (*(*s)++ != '[')
		return NULL;

	while (isblank(**s))
		++*s;
	if (strchr("[]\n", **s))
		return NULL;

	enum expr_type t = parse_expr_type(s);
	if (t == EXPR_INVALID)
		return NULL;

	char *arg = NULL;
	if (has_arg(t)) {
		while (isblank(**s))
			++*s;
		if (strchr("[]\n", **s))
			return NULL;
		arg = parse_arg(s);
	}

	while (isblank(**s))
		++*s;
	if (**s != ']') {
		free(arg);
		return NULL;
	}
	return new_expr(t, arg, NULL);
}
