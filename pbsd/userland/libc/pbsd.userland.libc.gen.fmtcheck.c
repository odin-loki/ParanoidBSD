/* Reference logic from hbsd/src/lib/libc/gen/fmtcheck.c (dual-world). */
#include <stddef.h>

int
fmtcheck_scan_pct(const char *fmt, size_t *idx)
{
	if (fmt == NULL || idx == NULL)
		return -1;
	if (fmt[*idx] != '%')
		return 0;
	(*idx)++;
	if (fmt[*idx] == '*')
		(*idx)++;
	while (fmt[*idx] >= '0' && fmt[*idx] <= '9')
		(*idx)++;
	if (fmt[*idx] == '.') {
		(*idx)++;
		while (fmt[*idx] >= '0' && fmt[*idx] <= '9')
			(*idx)++;
	}
	if (fmt[*idx] != '\0')
		(*idx)++;
	return 0;
}

int
fmtcheck_same_prefix(const char *a, const char *b)
{
	if (a == NULL || b == NULL)
		return -1;
	while (*a != '\0' && *b != '\0') {
		if (*a != *b)
			return 0;
		a++;
		b++;
	}
	return *a == *b;
}
