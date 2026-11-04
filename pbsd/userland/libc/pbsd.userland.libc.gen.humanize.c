/* Reference logic from hbsd/src/lib/libc/gen/humanize_number.c (dual-world). */
#include <stddef.h>

static const char suffixes[] = "KMGTPE";

int
humanize_scale(long long value, int scale, long long *result, char *suffix)
{
	if (result == NULL)
		return -1;
	long long v = value;
	int idx = 0;
	while (scale > 0 && v >= 1024 && idx < 6) {
		v /= 1024;
		--scale;
		++idx;
	}
	*result = v;
	if (suffix != NULL)
		*suffix = idx == 0 ? '\0' : suffixes[idx - 1];
	return 0;
}
