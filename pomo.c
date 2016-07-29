#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TASK_LIMIT 32

struct task {
	char name[8192];
	int est;
	int comp;
};

static struct task tasks[TASK_LIMIT];
static int n_tasks;

static void usage(void);
static void begin(void);
static void write_tasks(void);
static char *task_filename(void);
static void rtrim(char *s);

static void usage()
{
	fprintf(stderr, "usage: pomo command\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "valid commands:\n");
	fprintf(stderr, "\tbegin\n");
	exit(2);
}

static void begin()
{
	char linebuf[8192];
	struct task *current;

	for (current = tasks;; current++) {
		if (current >= tasks + TASK_LIMIT) {
			errx(EXIT_FAILURE, "TASK_LIMIT exceeded");
		}

		fprintf(stderr, "task name: ");
		fflush(stderr);
		if (!fgets(linebuf, sizeof(linebuf), stdin)) {
			fprintf(stderr, "\n");
			if (feof(stdin)) {
				break;
			} else {
				err(EXIT_FAILURE, "error reading task");
			}
		}
		rtrim(linebuf);
		memmove(current->name, linebuf, sizeof(linebuf));

		fprintf(stderr, "estimated pomos: ");
		fflush(stderr);
		if (!fgets(linebuf, sizeof(linebuf), stdin)) {
			fprintf(stderr, "\n");
			if (feof(stdin)) {
				errx(EXIT_FAILURE, "incomplete task");
			} else {
				err(EXIT_FAILURE, "error reading task");
			}
		}
		current->est = atoi(linebuf);
		current->comp = 0;
		n_tasks++;
	}

	write_tasks();
}

static void write_tasks()
{
	struct task *current;
	char *filename, *i;
	FILE *f;

	filename = task_filename();
	f = fopen(filename, "w");
	if (!f) {
		err(EXIT_FAILURE, "unable to open .pomo file");
	}

	for (current = tasks; current < tasks + n_tasks; current++) {
		if (putc('"', f) == EOF)
			goto fail;
		for (i = current->name; *i; i++) {
			if (*i == '"') {
				if (putc('\\', f) == EOF)
					goto fail;
			}
			if (putc(*i, f) == EOF)
				goto fail;
		}
		if (putc('"', f) == EOF)
			goto fail;
		if (putc('\t', f) == EOF)
			goto fail;
		if (fprintf(f, "%d\t%d\n", current->est, current->comp) < 0)
			goto fail;
	}
	return;
fail:
	err(EXIT_FAILURE, "error writing to .pomo file");
}

static char *task_filename()
{
	static char filenamebuf[8192] = {0};
	char *home, *i;
	int sz, w;

	if (*filenamebuf) {
		return filenamebuf;
	}

	home = getenv("HOME");
	if (!home) {
		errx(EXIT_FAILURE, "$HOME not found");
	}
	strcpy(filenamebuf, home);
	i = filenamebuf + strlen(filenamebuf);
	sz = 8192 - strlen(filenamebuf);
	w = snprintf(i, sz, "/.pomo");
	if (w != strlen("/.pomo")) {
		errx(EXIT_FAILURE, "Couldn't construct .pomo filename");
	}

	return filenamebuf;
}

static void rtrim(char *s)
{
	char *i;
	for (i = s + strlen(s) - 1; isspace(*i); i--);
	i++;
	*i = '\0';
}

int main(int argc, char **argv)
{
	char *i;

	if (argc != 2) {
		usage();
	}

	for (i = argv[1]; *i; i++) {
		*i = tolower(*i);
	}

	if (strcmp(argv[1], "begin") == 0) {
		begin();
	} else {
		usage();
	}

	return EXIT_SUCCESS;
}
