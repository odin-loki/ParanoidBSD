#include <stddef.h>
#include <stdint.h>
void *libiconv_open(const char *t, const char *f)
{
	(void)t;
	(void)f;
	return ((void *)0x200);
}
int libiconv_close(void *cd)
{
	(void)cd;
	return (0);
}
