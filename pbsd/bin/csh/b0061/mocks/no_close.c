#include <stddef.h>
#include <stdint.h>
void *libiconv_open(const char *t, const char *f)
{
	(void)t;
	(void)f;
	return ((void *)0x300);
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
