/* Reference logic from hbsd/src/lib/libc/gen/getbsize.c (dual-world). */

long
getbsize_parse(const char *env, long *blocksize)
{
	long n = 512;
	long mul = 1;

	if (env != NULL && *env != '\0') {
		n = 0;
		while (*env >= '0' && *env <= '9')
			n = n * 10 + (*env++ - '0');
		if (n == 0)
			n = 1;
		if (*env == 'K' || *env == 'k')
			mul = 1024L;
	}
	*blocksize = n * mul;
	if (*blocksize < 512)
		*blocksize = 512;
	return n;
}
