/* Reference logic from hbsd/src/lib/msun/src/s_remquo.c (dual-world). */
#include <math.h>

double
remquo_ref(double x, double y, int *quo)
{
	return remquo(x, y, quo);
}

float
remquof_ref(float x, float y, int *quo)
{
	return remquof(x, y, quo);
}
