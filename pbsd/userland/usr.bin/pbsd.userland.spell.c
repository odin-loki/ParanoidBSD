/* Reference logic from hbsd/src/usr.bin/spell/spell.c (dual-world). */
#include <stddef.h>

static int
is_alpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
	    c == '\'' || c == '-';
}

int
spell_valid_word(const char *word)
{
	if (word == NULL || word[0] == '\0')
		return 0;
	for (const char *p = word; *p != '\0'; ++p) {
		if (!is_alpha(*p))
			return 0;
	}
	return 1;
}

int
spell_parse_args(int argc, char *const *argv, int *british, int *stop_list,
    int *optind)
{
	if (british == NULL || stop_list == NULL || optind == NULL)
		return -1;
	*british = 0;
	*stop_list = 0;
	int i = 1;
	for (; i < argc && argv[i] != NULL && argv[i][0] == '-'; ++i) {
		const char *arg = argv[i];
		if (arg[1] == 'b' && arg[2] == '\0') {
			*british = 1;
			continue;
		}
		if (arg[1] == 'x' && arg[2] == '\0') {
			*stop_list = 1;
			continue;
		}
		return -1;
	}
	*optind = i;
	return 0;
}
