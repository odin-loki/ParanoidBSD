/* Reference logic from hbsd/src/lib/msun/src/e_remainder.c (dual-world). */
#include <math.h>

double
pbsd_remainder(double x, double y)
{
	return remainder(x, y);
}

float
pbsd_remainderf(float x, float y)
{
	return remainderf(x, y);
}
