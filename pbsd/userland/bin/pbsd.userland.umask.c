/* Reference logic from hbsd/src/bin/umask/umask.c (dual-world). */
#include <stddef.h>

static int
parse_octal_digit(char c)
{
	if (c >= '0' && c <= '7')
		return c - '0';
	return -1;
}

int
umask_parse_mode(const char *s, unsigned *mode)
{
	if (s == NULL || mode == NULL)
		return -1;
	if (s[0] == '0' && (s[1] == 'o' || s[1] == 'O'))
		s += 2;
	unsigned val = 0;
	for (; *s != '\0'; ++s) {
		int d = parse_octal_digit(*s);
		if (d < 0)
			return -1;
		val = (val << 3) | (unsigned)d;
		if (val > 0777)
			return -1;
	}
	*mode = val;
	return 0;
}

int
umask_symbolic(const char *s)
{
	(void)s;
	return -1; /* symbolic mode deferred */
}

int
umask_parse_args(int argc, char *const *argv, int *symbolic, int *optind)
{
	if (symbolic == NULL || optind == NULL)
		return -1;
	*symbolic = 0;
	int i = 1;
	for (; i < argc && argv[i] != NULL && argv[i][0] == '-'; ++i) {
		if (argv[i][1] == 'S' && argv[i][2] == '\0') {
			*symbolic = 1;
			continue;
		}
		return -1;
	}
	*optind = i;
	if (argc - i > 1)
		return -1;
	return 0;
}
