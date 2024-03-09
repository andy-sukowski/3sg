/* See LICENSE file for copyright and license details. */

#include "vars.h"

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
newvar(char *key, char *val)
{
	struct var *v = emalloc(sizeof *v);
	v->key = key;
	v->val = val;
	return v;
}

void
freevars(struct var *v)
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
parsekey(char **s)
{
	while (isblank(**s))
		++*s;
	if (strchr("=\n\r", **s))
		return NULL;
	char *start = *s;

	while (!isblank(**s))
		++*s;
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
parseval(char **s)
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
parsevar(char **s)
{
	char *key = parsekey(s);
	if (!key)
		return NULL;
	++*s; /* skip '=' */
	char *val = parseval(s);
	if (!val) {
		free(key);
		return NULL;
	}
	return newvar(key, val);
}

/* parse all variables in string,
 * return NULL on error */
struct var *
parsevars(char *s)
{
	struct var *head = NULL;
	struct var **tail = &head;
	while (*s) {
		*tail = parsevar(&s);
		if (!*tail) {
			freevars(head);
			return NULL;
		}
		if (*s)
			++s; /* skip '\n' or '\r' */
		tail = &(*tail)->next;
	}
	return head;
}
