/* Reference logic from hbsd/src/lib/libc/gen/sig2str.c (dual-world subset). */
#include <stddef.h>

int
sig2str_name(int signum, char *str, size_t len)
{
	static const char *names[] = { "HUP", "INT", "QUIT", "ILL", "TRAP" };

	if (str == NULL || len == 0)
		return -1;
	if (signum <= 0 || signum > 5)
		return -1;
	size_t i = 0;
	for (const char *p = names[signum - 1]; *p != '\0' && i + 1 < len; ++p)
		str[i++] = *p;
	str[i] = '\0';
	return 0;
}
