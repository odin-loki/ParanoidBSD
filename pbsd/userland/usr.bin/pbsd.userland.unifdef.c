/* Reference logic from hbsd/src/contrib/unifdef/unifdef.c (dual-world). */
#include <stddef.h>

enum linetype {
	LT_IF,
	LT_ELIF,
	LT_ELSE,
	LT_ENDIF,
	LT_PLAIN,
	LT_ERROR
};

static int
prefix_eq(const char *s, const char *pfx, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (s[i] != pfx[i])
			return 0;
	}
	return 1;
}

enum linetype
unifdef_classify(const char *line)
{
	if (line == NULL)
		return LT_PLAIN;
	while (*line == ' ' || *line == '\t')
		line++;
	if (line[0] != '#' || line[1] != ' ')
		return LT_PLAIN;
	line += 2;
	while (*line == ' ' || *line == '\t')
		line++;
	if (prefix_eq(line, "if", 2))
		return LT_IF;
	if (prefix_eq(line, "elif", 4))
		return LT_ELIF;
	if (prefix_eq(line, "else", 4))
		return LT_ELSE;
	if (prefix_eq(line, "endif", 5))
		return LT_ENDIF;
	if (prefix_eq(line, "error", 5))
		return LT_ERROR;
	return LT_PLAIN;
}
