/* Reference logic from hbsd/src/lib/libc/gen/strmode.c (dual-world). */
#include <stddef.h>

static const char *type_chars = "?-pldcbs-?";

void
strmode(unsigned mode, char *p)
{
	if (p == NULL)
		return;
	unsigned type = (mode >> 12) & 017;
	*p++ = type < 8 ? type_chars[type] : '?';
	*p++ = (mode & 0400) ? 'r' : '-';
	*p++ = (mode & 0200) ? 'w' : '-';
	*p++ = (mode & 0100) ? 'x' : '-';
	*p++ = (mode & 0040) ? 'r' : '-';
	*p++ = (mode & 0020) ? 'w' : '-';
	*p++ = (mode & 0010) ? 'x' : '-';
	*p++ = (mode & 0004) ? 'r' : '-';
	*p++ = (mode & 0002) ? 'w' : '-';
	*p++ = (mode & 0001) ? 'x' : '-';
	*p = '\0';
}
