/* Reference logic from hbsd/src/lib/libc/gen/fnmatch.c (dual-world ASCII subset). */

static int
fold(int flags, char c)
{
	if ((flags & 0x10) == 0)
		return c;
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	return c;
}

int
fnmatch_simple(const char *pattern, const char *string, int flags)
{
	if (pattern == NULL || string == NULL)
		return -1;
	while (*pattern != '\0') {
		if (*pattern == '*')
			return 0;
		if (*string == '\0')
			return 1;
		if (*pattern == '?') {
			pattern++;
			string++;
			continue;
		}
		if (fold(flags, *pattern) != fold(flags, *string))
			return 1;
		pattern++;
		string++;
	}
	return *string == '\0' ? 0 : 1;
}
