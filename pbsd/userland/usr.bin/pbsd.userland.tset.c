/* Reference logic from hbsd/src/usr.bin/tset/tset.c (dual-world). */
#include <stddef.h>

struct tset_opts {
	int query;
	int quiet;
	int reset;
	const char *term;
};

int
tset_parse_args(int argc, char *const *argv, struct tset_opts *opt, int *optind)
{
	if (opt == NULL || optind == NULL)
		return -1;
	int i = 1;
	for (; i < argc && argv[i] != NULL && argv[i][0] == '-'; ++i) {
		const char *arg = argv[i];
		if (arg[1] == 'Q' && arg[2] == '\0') {
			opt->query = 1;
			continue;
		}
		if (arg[1] == 'q' && arg[2] == '\0') {
			opt->quiet = 1;
			continue;
		}
		if (arg[1] == 'r' && arg[2] == '\0') {
			opt->reset = 1;
			continue;
		}
		return -1;
	}
	*optind = i;
	if (argc - i == 1)
		opt->term = argv[i];
	else if (argc - i > 1)
		return -1;
	return 0;
}

int
tset_valid_term(const char *term)
{
	if (term == NULL || term[0] == '\0')
		return 0;
	for (const char *p = term; *p != '\0'; ++p) {
		char c = *p;
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		    (c >= '0' && c <= '9') || c == '-' || c == '.'))
			return 0;
	}
	return 1;
}
