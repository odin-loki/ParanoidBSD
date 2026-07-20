/* Reference logic from hbsd/src/lib/libc/gen/unvis.c (dual-world). */
#include <stddef.h>

int
unvis_octal(const char *cp, unsigned char *out)
{
	if (cp == NULL || out == NULL)
		return -1;
	if (cp[0] != '\\')
		return -1;
	unsigned val = 0;
	for (int i = 0; i < 3; ++i) {
		char c = cp[i + 1];
		if (c < '0' || c > '7')
			return -1;
		val = (val << 3) | (unsigned)(c - '0');
	}
	*out = (unsigned char)val;
	return 4;
}

int
unvis_simple(const char *cp, unsigned char *out)
{
	if (cp == NULL || out == NULL || cp[0] != '\\')
		return -1;
	switch (cp[1]) {
	case 'n':
		*out = '\n';
		return 2;
	case 'r':
		*out = '\r';
		return 2;
	case 't':
		*out = '\t';
		return 2;
	case '\\':
		*out = '\\';
		return 2;
	default:
		return -1;
	}
}
