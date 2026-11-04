/* Reference logic from hbsd/src/usr.bin/showmount/showmount.c (dual-world). */

#define DODUMP 0x1
#define DOEXPORTS 0x2
#define DOPARSABLEEXPORTS 0x4

int
showmount_wants_dump(int flags)
{
	return (flags & DODUMP) != 0;
}

int
showmount_parse_opt(char ch, int *flags)
{
	switch (ch) {
	case 'd':
		*flags |= DODUMP;
		return 0;
	case 'e':
		*flags |= DOEXPORTS;
		return 0;
	case 'p':
		*flags |= DOPARSABLEEXPORTS;
		return 0;
	default:
		return -1;
	}
}
