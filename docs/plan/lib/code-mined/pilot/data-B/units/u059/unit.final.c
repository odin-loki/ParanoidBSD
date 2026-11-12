#include "prelude.h"
#undef APMDEV
#define APMDEV	"/dev/apm"
#undef APM_UNKNOWN
#define APM_UNKNOWN	255
#undef xh
#define xh(a)	(((a) & 0xff00) >> 8)
#undef xl
#define xl(a)	((a) & 0xff)
#undef APMERR
#define APMERR(a) xh(a)
/*@ requires -2147483648 <= i <= 2147483647; ensures (0 <= i < 10000) ==> (\result >= 0 && \result <= 0x9999 && ((\result & 0xf) == i % 10)); ensures (i < 0 || i >= 10000) ==> \result == -1; */
static int
int2bcd(int i)
{
	int retval = 0;
	int base = 0;

	/*
	 * Both ends.  A negative i makes `i % 10' negative and shifting
	 * a negative value left is undefined; worse, |i| can need more
	 * than eight digits, so base passes 31 and the shift distance
	 * exceeds the width as well.  -1 is what this already answers
	 * for a value it cannot represent.
	 */
	if (i < 0 || i >= 10000)
		return -1;
    
	while (i) {
		retval |= (i % 10) << base;
		i /= 10;
		base += 4;
	}
	return retval;
}
