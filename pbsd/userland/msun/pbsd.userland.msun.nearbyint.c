/* Reference logic from hbsd/src/lib/msun/src/s_nearbyint.c (dual-world). */
#include <math.h>

double
pbsd_nearbyint(double x)
{
	return nearbyint(x);
}

float
pbsd_nearbyintf(float x)
{
	return nearbyintf(x);
}
