/* Reference logic from hbsd/src/lib/msun/src/s_scalbln.c (dual-world). */
#include <math.h>

#define NMAX 65536
#define NMIN -65536

double
scalbln_ref(double x, long n)
{
	int exp = (n > NMAX) ? NMAX : (n < NMIN) ? NMIN : (int)n;
	return scalbn(x, exp);
}

float
scalblnf_ref(float x, long n)
{
	int exp = (n > NMAX) ? NMAX : (n < NMIN) ? NMIN : (int)n;
	return scalbnf(x, exp);
}
