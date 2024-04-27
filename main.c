/*
 * MIT License
 *
 * Copyright (c) 2024 Kyle Kloberdanz
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void print_usage(void) {
	printf(
		"\nlinelen: Get length of each line in a file.\n"
		"usage:\n"
		"    linelen [OPTIONS] FILENAME\n"
		"options:\n"
		"    -a <int>  Alert only when a line is longer then the "
		"given threshold.\n"
		"    -h        Display this help menu.\n"
		"    -m        Show only the maximum line length.\n"
		"header.\n"
		"    -t <int>  Set the number of characters to count for each "
		"tab. (Default 8)\n"
		"\n"
	);
}

struct opts {
	char **filenames;
	long tab_width;
	int max_col_only;
	long alert;
};

static void parse_opts(
	int argc,
	char **argv,
	struct opts *opts /* out */
) {
	int c;

	opts->tab_width = 8;
	opts->filenames = NULL;
	opts->max_col_only = 0;
	opts->alert = -1;

	while ((c = getopt(argc, argv, "t:a:hm")) != -1) {
		switch (c) {
		case 'a':
			opts->alert = atol(optarg);
			break;
		case 't':
			opts->tab_width = atol(optarg);
			break;
		case 'm':
			opts->max_col_only = 1;
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
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
}

static void print_info(long len, long lineno, const char *filename) {
	printf("%s:%ld\t%ld\n", filename, lineno, len);
}

static int run_file(const char *filename, const struct opts *opts) {
	FILE *fp = NULL;
	int rc = 1;
	long lineno = 1;
	long len = 0;
	int c;
	long longest_lineno = 1;
	long longest_len = 0;
	int always_print = (opts->alert < 0) && (!opts->max_col_only);

	fp = fopen(filename, "r");
	if (!fp) {
		perror("fopen");
		goto done;
	}

	/* TODO: Read data into a buffer using fread. */
	while ((c = fgetc(fp))) {
		if (feof(fp)) {
			break;
		}
		switch (c) {
		case '\n':
			if ((opts->alert >= 0) && (len > opts->alert)) {
				print_info(len, lineno, filename);
			}

			if (opts->max_col_only && (len > longest_len)) {
				longest_lineno = lineno;
				longest_len = len;
			}

			if (always_print) {
				print_info(len, lineno, filename);
			}

			len = 0;
			lineno++;
			break;
		case '\t':
			len += opts->tab_width;
			break;
		case '\r':
			break;
		default:
			len += 1;
			break;
		}
	}

	if (opts->max_col_only) {
		print_info(longest_len, longest_lineno, filename);
	}
	rc = 0;
done:
	if (fp) {
		fclose(fp);
	}
	return rc;
}

int main(int argc, char **argv) {
	struct opts opts;
	int i = 0;
	int rc = 0;

	parse_opts(argc, argv, &opts);

	for (i = 0; opts.filenames[i] != NULL; i++) {
		rc = run_file(opts.filenames[i], &opts);
		if (rc) {
			goto done;
		}
	}

done:
	return rc;
}
