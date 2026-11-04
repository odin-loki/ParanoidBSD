/* Reference logic from hbsd/src/usr.bin/lorder/lorder.c (dual-world). */
#include <stddef.h>

static int
cmp_sym(const char *a, const char *b)
{
	while (*a != '\0' && *b != '\0' && *a == *b) {
		++a;
		++b;
	}
	if (*a == *b)
		return 0;
	return (unsigned char)*a < (unsigned char)*b ? -1 : 1;
}

int
lorder_parse_object(const char *line, char *obj, size_t objlen, char *sym,
    size_t symlen)
{
	if (line == NULL || obj == NULL || sym == NULL)
		return -1;
	size_t o = 0;
	while (*line != '\0' && *line != ' ' && *line != '\t') {
		if (o + 1 >= objlen)
			return -1;
		obj[o++] = *line++;
	}
	obj[o] = '\0';
	while (*line == ' ' || *line == '\t')
		++line;
	size_t s = 0;
	while (*line != '\0' && *line != '\n') {
		if (s + 1 >= symlen)
			return -1;
		sym[s++] = *line++;
	}
	sym[s] = '\0';
	return obj[0] == '\0' || sym[0] == '\0' ? -1 : 0;
}

int
lorder_symbol_less(const char *a, const char *b)
{
	return cmp_sym(a, b) < 0;
}
