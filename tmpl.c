/* See LICENSE file for copyright and license details. */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "fatal.h"
#include "tmpl.h"

#define BLANKS " \t" /* isblank() */

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
	char *bgn = *s + strspn(*s, BLANKS);
	char *end = bgn + strcspn(bgn, BLANKS "=");
	*s = end + strspn(end, BLANKS);
	if (**s != '=' || bgn == end)
		return NULL;

	char *key = ecalloc(1, end - bgn + 1);
	memcpy(key, bgn, end - bgn);
	return key;
}

/* greedy parse upto '\0',
 * trim before and after */
char *
parse_val(char **s)
{
	char *bgn = *s + strspn(*s, BLANKS);
	*s += strlen(*s);
	char *end = *s;
	while (strchr(BLANKS, end[-1]))
		--end;

	char *val = ecalloc(1, end - bgn + 1);
	memcpy(val, bgn, end - bgn);
	return val;
}

/* greedy parse upto '\0',
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
 * may contain comments and empty lines,
 * prepend most recent variable,
 * return 0 on success,
 * return line number on error */
int
parse_vars(char *s, struct var **head)
{
	struct var *v;
	for (int l = 1; *s; ++l) {
		char *bgn = s + strspn(s, BLANKS);
		char *end = bgn + strcspn(bgn, "#\n");
		s = end + strcspn(end, "\n");
		if (*s)
			++s; /* skip '\n' */
		if (bgn == end)
			continue;

		char tmp = *end;
		*end = '\0';
		v = parse_var(&bgn);
		if (!v)
			return l;
		*end = tmp;

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
	enum expr_type t = 0;
	while (t < EXPR_INVALID && strcmp(start, expr_type_str[t]))
		++t;
	**s = tmp;
	return t;
}

bool
has_arg(enum expr_type t)
{
	enum expr_type have_arg[8] = {
		EXPR_VAR,
		EXPR_INCL,
		EXPR_IF,
		EXPR_IFNOT,
		EXPR_FOR,
		EXPR_REVFOR,
		EXPR_FORALL,
		EXPR_REVFORALL
	};
	for (int i = 0; i < 8; ++i)
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
