/* See LICENSE file for copyright and license details. */

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "fatal.h"
#include "tmpl.h"

#define BLANKS " \t"         /* isblank() */
#define WHITES " \t\n\v\f\r" /* isspace() */

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
 * return EXPR_INVALID on error */
enum expr_type
parse_expr_type(char **s)
{
	char *bgn = *s + strspn(*s, WHITES);
	if (*bgn == '_' || (isalnum(*bgn) && islower(*bgn)))
		return EXPR_VAR;
	char *end = bgn + strcspn(bgn, WHITES);
	*s = end + strspn(end, WHITES);

	char tmp = *end;
	*end = '\0';
	enum expr_type t = 0;
	while (t < EXPR_INVALID && strcmp(bgn, expr_type_str[t]))
		++t;
	*end = tmp;
	return t;
}

bool
has_arg(enum expr_type t)
{
	return t == EXPR_VAR
	    || t == EXPR_INCL
	    || t == EXPR_IF
	    || t == EXPR_IFNOT
	    || t == EXPR_FOR
	    || t == EXPR_REVFOR
	    || t == EXPR_FORALL
	    || t == EXPR_REVFORALL;
}

/* greedy parse expression argument,
 * return NULL on error */
char *
parse_arg(char **s)
{
	char *bgn = *s + strspn(*s, WHITES);
	char *end = bgn + strcspn(bgn, WHITES);
	*s = end + strspn(end, WHITES);
	if (**s || bgn == end)
		return NULL;

	char *arg = ecalloc(1, end - bgn + 1);
	memcpy(arg, bgn, end - bgn);
	return arg;
}

/* greedy parse expression between expr_open and expr_close,
 * return NULL on error */
struct expr *
parse_expr(char **s)
{
	int expr_open_len = strlen(expr_open);
	if (strncmp(*s, expr_open, expr_open_len))
		return NULL;
	char *bgn = *s += expr_open_len;

	char *end = bgn;
	int expr_close_len = strlen(expr_close);
	while (*end && strncmp(end, expr_close, expr_close_len))
		++end;
	if (!*end)
		return NULL;
	*s = end + expr_close_len;

	*end = '\0';
	enum expr_type t = parse_expr_type(&bgn);
	char *arg = has_arg(t) ? parse_arg(&bgn) : NULL;
	*end = expr_close[0];
	if (t == EXPR_INVALID || (has_arg(t) && !arg) || bgn != end) {
		free(arg);
		return NULL;
	}
	return new_expr(t, arg, NULL);
}
