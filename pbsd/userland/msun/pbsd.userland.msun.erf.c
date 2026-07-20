/* Reference logic from hbsd/src/lib/msun/src/s_erf.c (dual-world). */
#include <math.h>

double
erf_ref(double x)
{
	return erf(x);
}

float
erff_ref(float x)
{
	return erff(x);
}
