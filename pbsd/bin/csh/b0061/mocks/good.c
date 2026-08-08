#include <stddef.h>
#include <stdint.h>
void *libiconv_open(const char *t, const char *f)
{
	uintptr_t h = 5381;
	const unsigned char *p;
	if (t != NULL)
		for (p = (const unsigned char *)t; *p; ++p)
			h = ((h << 5) + h) + *p;
	if (f != NULL)
		for (p = (const unsigned char *)f; *p; ++p)
			h = ((h << 5) + h) + *p;
	return ((void *)(h | 0x100));
}
size_t libiconv(void *cd, char **inbuf, size_t *inleft, char **outbuf,
    size_t *outleft)
{
	(void)cd;
	(void)inbuf;
	(void)inleft;
	(void)outbuf;
	(void)outleft;
	return (0);
}
int libiconv_close(void *cd)
{
	(void)cd;
	return (0);
}
