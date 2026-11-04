/* Reference logic from hbsd/src/usr.bin/clear/clear.c (dual-world). */
#include <stddef.h>

static const char clear_seq[] = "\033[H\033[2J";

const char *
clear_term_sequence(void)
{
	return clear_seq;
}

size_t
clear_term_length(void)
{
	size_t n = 0;
	while (clear_seq[n] != '\0')
		++n;
	return n;
}

int
clear_parse_args(int argc, char *const *argv)
{
	(void)argv;
	return argc == 1 ? 0 : -1;
}
