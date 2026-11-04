/* Reference logic from hbsd/src/lib/msun/src/s_expm1.c (dual-world). */
#include <math.h>

double
pbsd_expm1(double x)
{
	return expm1(x);
}

float
pbsd_expm1f(float x)
{
	return expm1f(x);
}
