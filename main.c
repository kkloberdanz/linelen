/*
 * MIT License
 *
 * Copyright (c) 2024 Kyle Kloberdanz
 */
#define _DEFAULT_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define bool char
#define true 1
#define false 0

enum {
	BUFFER_SIZE = 4096
};

struct opts {
	char **filenames;
	long tab_width;
	bool max_col_only;
	long alert;
};

struct line_info {
	long len;
	long lineno;
	long longest_len;
	long longest_lineno;
};

static void print_usage(void) {
	printf(
		"linelen: Get length of each line in a file.\n"
		"usage:\n"
		"    linelen [OPTIONS] FILENAME\n"
		"options:\n"
		"    -a <int>  Alert only when a line is longer then the "
		"given threshold.\n"
		"    -h        Display this help menu.\n"
		"    -m        Show only the maximum line length.\n"
		"    -t <int>  Set the number of characters to count for each "
		"tab. (Default 8)\n"
	);
}

static bool is_number(const char *s) {
	size_t i;
	for (i = 0; s[i] != '\0'; i++) {
		if (!isdigit(s[i])) {
			return false;
		}
	}
	return true;
}

static void parse_opts(
	int argc,
	char **argv,
	size_t *nel, /* out */
	struct opts *opts /* out */
) {
	int c;

	opts->tab_width = 8;
	opts->filenames = NULL;
	opts->max_col_only = false;
	opts->alert = -1;

	while ((c = getopt(argc, argv, "a:hmt:")) != -1) {
		switch (c) {
		case 'a':
			if (!is_number(optarg)) {
				print_usage();
				exit(EXIT_FAILURE);
			}
			opts->alert = atol(optarg);
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
		case 'm':
			opts->max_col_only = true;
			break;
		case 't':
			if (!is_number(optarg)) {
				print_usage();
				exit(EXIT_FAILURE);
			}
			opts->tab_width = atol(optarg);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}	
	
	if (optind >= argc) {
		print_usage();
		exit(EXIT_FAILURE);
	}
	
	opts->filenames = argv + optind;
	*nel = argc - optind;
}

static void handle_character(
	char c,
	const char *filename,
	const struct opts *opts,
	struct line_info *li /* out */
) {
	int always_print = (opts->alert < 0) && (!opts->max_col_only);
	switch (c) {
	case '\n':
		if ((opts->alert >= 0) && (li->len > opts->alert)) {
			printf("%s:%ld\t%ld\n", filename, li->lineno, li->len);
		}

		if (opts->max_col_only && (li->len > li->longest_len)) {
			li->longest_lineno = li->lineno;
			li->longest_len = li->len;
		}

		if (always_print) {
			printf("%s:%ld\t%ld\n", filename, li->lineno, li->len);
		}

		li->len = 0;
		li->lineno++;
		break;
	case '\t':
		li->len += opts->tab_width;
		break;
	case '\r':
		break;
	default:
		li->len++;
		break;
	}
}

static int run_file(const char *filename, const struct opts *opts) {
	FILE *fp = NULL;
	int rc = 1;
	size_t bytes_read = 0;
	char buf[BUFFER_SIZE + 1];
	struct line_info li;
	char c;
	bool has_data = false;

	li.lineno = 1;
	li.len = 0;
	li.longest_lineno = 1;
	li.longest_len = 0;

	fp = fopen(filename, "r");
	if (!fp) {
		perror("fopen");
		goto done;
	}

	while ((bytes_read = fread(buf, 1, BUFFER_SIZE, fp))) {
		size_t i;
		has_data = true;

		if ((bytes_read != BUFFER_SIZE) && ferror(fp)) {
			perror("fread");
			rc = 1;
			goto done;
		}
		for (i = 0; i < bytes_read; i++) {
			c = buf[i];
			handle_character(
				c,
				filename,
				opts,
				&li
			);
		}
	}

	/* Handle case if file does not end in \n. */
	if (has_data && (c != '\n')) {
		handle_character('\n', filename, opts, &li);
	}

	if (opts->max_col_only) {
		printf(
			"%s:%ld\t%ld\n",
			filename,
			li.longest_lineno,
			li.longest_len
		);
	}

	rc = 0;
done:
	if (fp) {
		fclose(fp);
	}
	return rc;
}

static int compar(const void *a, const void *b) {
	const char *const *s1 = a;
	const char *const *s2 = b;
	return strcmp(*s1, *s2);
}

int main(int argc, char **argv) {
	struct opts opts;
	int i = 0;
	int rc = 0;
	size_t nel = 0;

	parse_opts(argc, argv, &nel, &opts);

	qsort(opts.filenames, nel, sizeof(*opts.filenames), compar);

	for (i = 0; opts.filenames[i] != NULL; i++) {
		rc = run_file(opts.filenames[i], &opts);
		if (rc) {
			goto done;
		}
	}

done:
	return rc;
}
