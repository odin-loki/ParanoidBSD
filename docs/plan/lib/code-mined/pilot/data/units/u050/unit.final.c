#include "prelude.h"
typedef struct fe { u32 v[10]; } fe;
typedef struct fe_loose { u32 v[10]; } fe_loose;
/*@ requires c <= 1 && a < 67108864 && b < 67108864; ensures \result == (a + b + c) / 67108864 && *low == (a + b + c) % 67108864; */
addcarryx_u26(u8 /*bool*/ c, u32 a, u32 b, u32 *low)
{
	/* This function extracts 26 bits of result and 1 bit of carry
	 * (27 total), so a 32-bit intermediate is sufficient.
	 */
	u32 x = a + b + c;
	*low = x & ((1 << 26) - 1);
	return (x >> 26) & 1;
}
