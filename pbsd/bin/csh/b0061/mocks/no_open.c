#include <stddef.h>
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
