/* Reference logic from hbsd/src/games/fortune/fortune/fortune.c (dual-world). */
#include <stddef.h>

struct fortune_opts {
	int show_all;
	int long_only;
	int offend;
};

static int
parse_count(const char *s, int *out)
{
	if (s == NULL || *s == '\0')
		return -1;
	int val = 0;
	for (; *s != '\0'; ++s) {
		if (*s < '0' || *s > '9')
			return -1;
		val = val * 10 + (*s - '0');
	}
	*out = val;
	return val > 0 ? 0 : -1;
}

int
fortune_parse_args(int argc, char *const *argv, struct fortune_opts *opt, int *optind)
{
	if (opt == NULL || optind == NULL)
		return -1;
	int i = 1;
	for (; i < argc && argv[i] != NULL && argv[i][0] == '-'; ++i) {
		const char *arg = argv[i];
		for (int j = 1; arg[j] != '\0'; ++j) {
			switch (arg[j]) {
			case 'a':
				opt->show_all = 1;
				break;
			case 'l':
				opt->long_only = 1;
				break;
			case 'o':
				opt->offend = 1;
				break;
			default:
				return -1;
			}
		}
	}
	*optind = i;
	if (argc - i == 1)
		return parse_count(argv[i], &opt->show_all);
	if (argc - i > 1)
		return -1;
	return 0;
}

size_t
fortune_pick_index(size_t count, unsigned seed)
{
	return count == 0 ? 0 : seed % count;
}
