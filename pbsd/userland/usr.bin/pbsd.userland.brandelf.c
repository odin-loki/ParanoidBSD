/* Reference logic from hbsd/src/usr.bin/brandelf/brandelf.c (dual-world). */
#include <stddef.h>

#define ELFMAG0 0x7f
#define ELFOSABI_FREEBSD 9
#define ELFOSABI_LINUX 3

static const struct {
	const char *str;
	int value;
} elftypes[] = {
	{ "FreeBSD", ELFOSABI_FREEBSD },
	{ "Linux", ELFOSABI_LINUX },
};

int
brandelf_is_elf(const unsigned char *ident)
{
	return ident != NULL && ident[0] == ELFMAG0 && ident[1] == 'E' &&
	    ident[2] == 'L' && ident[3] == 'F';
}

const char *
brandelf_abi_name(int etype)
{
	for (size_t i = 0; i < sizeof(elftypes) / sizeof(elftypes[0]); i++) {
		if (etype == elftypes[i].value)
			return elftypes[i].str;
	}
	return NULL;
}
