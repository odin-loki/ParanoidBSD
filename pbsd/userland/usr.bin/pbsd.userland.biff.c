/* Reference logic from hbsd/src/usr.bin/biff/biff.c (dual-world). */
#include <stddef.h>

#define S_IXUSR 0000100
#define S_IXGRP 0000020

const char *
biff_status(unsigned mode)
{
	if (mode & S_IXUSR)
		return "y";
	if (mode & S_IXGRP)
		return "b";
	return "n";
}

int
biff_parse_mode(const char *arg)
{
	if (arg == NULL || arg[0] == '\0' || arg[1] != '\0')
		return -1;
	switch (arg[0]) {
	case 'n':
	case 'y':
	case 'b':
		return arg[0];
	default:
		return -1;
	}
}

unsigned
biff_apply(unsigned mode, char ch)
{
	mode &= ~(S_IXUSR | S_IXGRP);
	switch (ch) {
	case 'y':
		return mode | S_IXUSR;
	case 'b':
		return mode | S_IXGRP;
	default:
		return mode;
	}
}
