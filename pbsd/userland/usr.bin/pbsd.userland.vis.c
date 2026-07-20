/* Reference logic from hbsd/src/usr.bin/vis/vis.c (dual-world). */
#include <stddef.h>

#define VIS_CSTYLE 0x01
#define VIS_OCTAL  0x02
#define VIS_NOSLASH 0x04

int
vis_encode_flags(int argc, char *const *argv, int *flags, int *optind)
{
	if (flags == NULL || optind == NULL)
		return -1;
	*flags = 0;
	int i = 1;
	for (; i < argc && argv[i] != NULL && argv[i][0] == '-'; ++i) {
		const char *arg = argv[i];
		for (int j = 1; arg[j] != '\0'; ++j) {
			switch (arg[j]) {
			case 'c':
				*flags |= VIS_CSTYLE;
				break;
			case 'o':
				*flags |= VIS_OCTAL;
				break;
			case 'F':
				*flags |= VIS_NOSLASH;
				break;
			default:
				return -1;
			}
		}
	}
	*optind = i;
	return 0;
}

int
vis_need_escape(unsigned char c, int flags)
{
	if (c >= 0x20 && c < 0x7f && c != '\\')
		return 0;
	if ((flags & VIS_CSTYLE) && (c == '\n' || c == '\r' || c == '\t'))
		return 1;
	return 1;
}
