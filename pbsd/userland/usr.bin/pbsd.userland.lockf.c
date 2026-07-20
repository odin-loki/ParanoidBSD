/* Reference logic from hbsd/src/usr.bin/lockf/lockf.c (dual-world). */
#include <stddef.h>

#define FDLOCK_PREFIX "/dev/fd/"

static int
prefix_match(const char *s, const char *pfx)
{
	while (*pfx != '\0') {
		if (*s++ != *pfx++)
			return 0;
	}
	return *s != '\0';
}

int
lockf_is_fd_path(const char *name)
{
	return name != NULL && prefix_match(name, FDLOCK_PREFIX);
}

long
lockf_parse_fd(const char *name)
{
	const char *p;
	long fd;

	if (!lockf_is_fd_path(name))
		return -1;
	p = name + sizeof(FDLOCK_PREFIX) - 1;
	if (*p == '\0')
		return -1;
	fd = 0;
	for (; *p != '\0'; ++p) {
		if (*p < '0' || *p > '9')
		 return -1;
		fd = fd * 10 + (*p - '0');
	}
	return fd;
}
