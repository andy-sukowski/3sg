/* See LICENSE file for copyright and license details. */

#include "tmpl.h"

void *
emalloc(size_t size)
{
	void *p = malloc(size);
	if (!p) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);
	if (!p) {
		fprintf(stderr, "calloc: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

struct var *
new_var(char *key, char *val)
{
	struct var *v = emalloc(sizeof *v);
	v->key = key;
	v->val = val;
	return v;
}

void
free_vars(struct var *v)
{
	struct var *tmp;
	while (v) {
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
	if (strchr("=\n\r", **s))
		return NULL;
	char *start = *s;

	*s = &(*s)[strcspn(*s, " \t\n\r=")];
	if (strchr("\n\r", **s))
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

/* greedy parse upto '\n', '\r' or '\0',
 * return NULL on error */
char *
parse_val(char **s)
{
	while (isblank(**s))
		++*s;
	char *start = *s;

	*s = &(*s)[strcspn(*s, "\r\n")];
	char *end = *s;
	while (isblank(*(end - 1)))
		--end;

	char *val = ecalloc(1, end - start + 1);
	memcpy(val, start, end - start);
	return val;
}

/* greedy parse upto '\n', '\r' or '\0',
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
	return new_var(key, val);
}

/* parse all variables in string,
 * return NULL on error */
struct var *
parse_vars(char **s)
{
	struct var *head = NULL;
	struct var **tail = &head;
	while (**s) {
		*tail = parse_var(s);
		if (!*tail) {
			free_vars(head);
			return NULL;
		}
		if (**s)
			++*s; /* skip '\n' or '\r' */
		tail = &(*tail)->next;
	}
	return head;
}

struct expr *
new_expr(enum expr_type type, char *arg)
{
	struct expr *x = emalloc(sizeof *x);
	x->type = type;
	x->arg = arg;
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
