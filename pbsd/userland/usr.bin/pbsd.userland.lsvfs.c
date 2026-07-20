/* Reference logic from hbsd/src/usr.bin/lsvfs/lsvfs.c (dual-world). */
#include <stddef.h>

static const struct {
	int flag;
	const char *str;
} fl[] = {
	{ 0x0001, "static" },
	{ 0x0002, "network" },
	{ 0x0004, "read-only" },
	{ 0x0008, "synthetic" },
	{ 0x0010, "loopback" },
	{ 0x0020, "unicode" },
	{ 0x0040, "jail" },
	{ 0x0080, "delegated-administration" },
};

size_t
lsvfs_format_flags(int flags, char *buf, size_t len)
{
	size_t pos = 0;

	if (buf == NULL || len == 0)
		return 0;
	for (size_t i = 0; i < sizeof(fl) / sizeof(fl[0]); i++) {
		if ((flags & fl[i].flag) == 0)
			continue;
		if (pos != 0) {
			if (pos + 2 >= len)
				break;
			buf[pos++] = ',';
			buf[pos++] = ' ';
		}
		const char *p = fl[i].str;
		while (*p != '\0' && pos + 1 < len)
			buf[pos++] = *p++;
	}
	if (pos < len)
		buf[pos] = '\0';
	return pos;
}
