/* See LICENSE file for copyright and license details. */

#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fatal.h"
#include "tmpl.h"

char *
read_file(char *path)
{
	FILE *f = efopen(path, "rb");

	efseek(f, 0, SEEK_END);
	long length = eftell(f);
	efseek(f, 0, SEEK_SET);

	char *content = emalloc(length);
	efread(content, 1, length, f);
	content[length] = '\0';
	efclose(f);
	return content;
}

/* check if b in same directory as a,
 * or index.html in immediate subdirectory,
 * expect simplified paths (use realpath(3)) */
bool
is_subpage(char *a, char *b)
{
	while (*a && *a == *b)
		++a, ++b;
	if (strchr(a, '/'))
		return false;
	b = strchr(b, '/');
	return !b || !strcmp(b, "/index.html");
}

/* arguments provided by user,
 * simplified paths (use realpath(3)),
 * except pages, which are relative to web_root */
struct user_args {
	char output_dir[PATH_MAX];
	char project_dir[PATH_MAX];
	char web_root[PATH_MAX];
	char **pages;
};

void
free_pages(char **pages)
{
	if (!pages)
		return;
	for (char **p = pages; *p; ++p)
		free(*p);
	free(pages);
}

/* read page paths seperated by '\n',
 * expect web_root to be simplified (use realpath(3))
 * fill pages with paths relative to web_root */
char **
read_pages(FILE *f, char *web_root)
{
	char rel[PATH_MAX];
	char abs[PATH_MAX];
	char **pages = NULL;
	int len;
	for (len = 1; fgets(rel, PATH_MAX, f) && *rel != '\n'; ++len) {
		rel[strcspn(rel, "\n")] = '\0';
		erealpath(rel, abs);

		char *a = abs, *w = web_root;
		while (*a && *a == *w)
			++a, ++w;
		if (*w) {
			free_pages(pages);
			fprintf(stderr, "Page %s not in web root %s\n", abs, web_root);
			exit(EXIT_FAILURE);
		}

		pages = erealloc(pages, (len + 1) * sizeof(char *));
		pages[len - 1] = estrdup(a);
		pages[len] = NULL;
	}
	return pages;
}

void
usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [-c config] [-o output_dir]"
	                "[-p project_dir] [-w web_root]\n", argv0);
	exit(EXIT_FAILURE);
}

/* handle command-line arguments */
int
main(int argc, char *argv[])
{
	char *config = "global.cfg";
	char *output_dir = "output";
	char *project_dir = ".";
	char *web_root = "content";

	for (int argi = 1; argi < argc; ++argi) {
		if (argi > argc - 2 || argv[argi][0] != '-' || argv[argi][2])
			usage(argv[0]);
		switch (argv[argi][1]) {
		case 'c':
			config = argv[++argi];
			break;
		case 'o':
			output_dir = argv[++argi];
			break;
		case 'p':
			project_dir = argv[++argi];
			break;
		case 'w':
			web_root = argv[++argi];
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	struct user_args a;
	erealpath(output_dir, a.output_dir);
	erealpath(project_dir, a.project_dir);
	erealpath(web_root, a.web_root);
	a.pages = read_pages(stdin, a.web_root);

	printf("config: %s\n", config);
	printf("output_dir: %s\n", a.output_dir);
	printf("project_dir: %s\n", a.project_dir);
	printf("web_root: %s\n", a.web_root);
	for (char **p = a.pages; *p; ++p)
		printf("page: %s\n", *p);

	free_pages(a.pages);
	return EXIT_SUCCESS;
}
