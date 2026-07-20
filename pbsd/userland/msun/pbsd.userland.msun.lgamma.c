/* Reference logic from hbsd/src/lib/msun/src/e_lgamma.c (dual-world). */
#include <math.h>

double
lgamma_ref(double x)
{
	return lgamma(x);
}

float
lgammaf_ref(float x)
{
	return lgammaf(x);
}
