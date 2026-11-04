/* Reference logic from hbsd/src/usr.bin/lam/lam.c (dual-world). */
#include <stddef.h>

#define MAXOFILES 20

int
lam_max_files(void)
{
	return MAXOFILES;
}

int
lam_is_format_spec(const char *arg)
{
	return arg != NULL && arg[0] == 'F' && arg[1] != '\0';
}

size_t
lam_trim_newline(char *line, size_t len)
{
	if (line == NULL || len == 0)
		return 0;
	if (line[len - 1] == '\n') {
		line[len - 1] = '\0';
		return len - 1;
	}
	return len;
}
