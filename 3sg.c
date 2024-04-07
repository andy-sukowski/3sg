/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

	char *content = emalloc(length + 1);
	efread(content, 1, length, f);
	content[length] = '\0';
	efclose(f);
	return content;
}

/* path doesn't include .cfg extension */
void
read_cfg(char *path, struct var **vars)
{
	char cfg_path[PATH_MAX];
	snprintf(cfg_path, PATH_MAX, "%s.cfg", path);
	if (access(cfg_path, F_OK))
		return;
	char *cfg = read_file(cfg_path);
	int l = parse_vars(cfg, vars);
	free(cfg);
	if (l > 0) {
		fprintf(stderr, "Parse error (%s:%i)\n", cfg_path, l);
		exit(EXIT_FAILURE);
	}
}

/* mkdir_p("a/b/c", 0755) creates "a/b/" */
int
mkdir_p(char *path, mode_t mode)
{
	char *p = path;
	while ((p = strchr(p + 1, '/'))) {
		*p = '\0';
		int ret = mkdir(path, mode);
		*p = '/';
		if (ret == -1 && errno != EEXIST)
			return -1;
	}
	return 0;
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

void
gen(struct user_args *a, FILE *fout, struct var *vars, char *path)
{
	struct var *prev = vars;
	read_cfg(path, &vars);

	char *content = read_file(path);
	bool esc = false;
	int l = 1;
	for (char *s = content; *s; ++s) {
		if (!esc && *s == '\\') {
			esc = true;
			continue;
		}
		if (*s == '\n')
			++l;
		if (esc || !strchr("[]", *s)) {
			fputc(*s, fout);
		} else if (*s == ']') {
			fprintf(stderr, "Unmatched ']' (%s:%i)\n", path, l);
			exit(EXIT_FAILURE);
		} else {
			/* TODO, this is only temporary */
			s = &s[strcspn(s, "]")];
		}
		esc = false;
	}
	free(content);
	free_vars(vars, prev);
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
	char *cfg_path = "global.cfg";
	char *output_dir = "output";
	char *project_dir = ".";
	char *web_root = "content";

	for (int argi = 1; argi < argc; ++argi) {
		if (argi > argc - 2 || argv[argi][0] != '-' || argv[argi][2])
			usage(argv[0]);
		switch (argv[argi][1]) {
		case 'c':
			cfg_path = argv[++argi];
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

	struct var *vars = NULL;
	char *cfg = read_file(cfg_path);
	int l = parse_vars(cfg, &vars);
	free(cfg);
	if (l > 0) {
		fprintf(stderr, "Parse error (%s:%i)\n", cfg_path, l);
		return EXIT_FAILURE;
	}

	char abs[PATH_MAX];
	char out[PATH_MAX];
	for (char **p = a.pages; *p; ++p) {
		snprintf(abs, PATH_MAX, "%s%s", a.web_root, *p);
		snprintf(out, PATH_MAX, "%s%s", a.output_dir, *p);
		if (mkdir_p(out, 0755) == -1) {
			fprintf(stderr, "Failed to create %s: %s\n", out, strerror(errno));
			exit(EXIT_FAILURE);
		}
		FILE *fout = efopen(out, "wb");

		vars = new_var(estrdup("PATH"), estrdup(*p), vars);
		gen(&a, fout, vars, abs);
		efclose(fout);
	}
	free_vars(vars, NULL);
	free_pages(a.pages);
	return EXIT_SUCCESS;
}
