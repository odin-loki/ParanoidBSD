/* Reference logic from hbsd/src/usr.bin/morse/morse.c (dual-world). */
#include <stddef.h>

static const struct {
	char inchar;
	const char *morse;
} mtab[] = {
	{'a', ".-"}, {'b', "-..."}, {'c', "-.-."}, {'d', "-.."}, {'e', "."},
	{'f', "..-."}, {'g', "--."}, {'h', "...."}, {'i', ".."}, {'j', ".---"},
	{'k', "-.-"}, {'l', ".-.."}, {'m', "--"}, {'n', "-."}, {'o', "---"},
	{'p', ".--."}, {'q', "--.-"}, {'r', ".-."}, {'s', "..."}, {'t', "-"},
	{'u', "..-"}, {'v', "...-"}, {'w', ".--"}, {'x', "-..-"}, {'y', "-.--"},
	{'z', "--.."},
	{'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
	{'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
	{'\0', ""}
};

const char *
morse_lookup(unsigned char ch)
{
	char lower = (char)(ch >= 'A' && ch <= 'Z' ? ch + ('a' - 'A') : ch);
	for (size_t i = 0; mtab[i].inchar != '\0'; ++i) {
		if (mtab[i].inchar == lower)
			return mtab[i].morse;
	}
	return NULL;
}
