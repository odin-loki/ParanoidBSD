/* Reference logic from hbsd/src/usr.bin/indent/args.c (dual-world). */
#include <stddef.h>

struct indent_opts {
	int lindent;
	int pindent;
	int continuation;
	int case_indent;
	int comment_dist;
};

static int
parse_int(const char *s, int *out)
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
	return 0;
}

int
indent_parse_li(const char *arg, struct indent_opts *opt)
{
	if (arg == NULL || opt == NULL || arg[0] != '-' || arg[1] != 'l')
		return -1;
	const char *val = arg + 2;
	if (*val == '\0')
		return -1;
	return parse_int(val, &opt->lindent);
}

int
indent_parse_ip(const char *arg, struct indent_opts *opt)
{
	if (arg == NULL || opt == NULL || arg[0] != '-' || arg[1] != 'i')
		return -1;
	const char *val = (arg[2] == 'p') ? arg + 3 : arg + 2;
	if (*val == '\0')
		return -1;
	return parse_int(val, &opt->pindent);
}
