/* Reference logic from hbsd/src/lib/msun/src/s_cbrt.c (dual-world). */
#include <math.h>

double
pbsd_cbrt(double x)
{
	return cbrt(x);
}

float
pbsd_cbrtf(float x)
{
	return cbrtf(x);
}
