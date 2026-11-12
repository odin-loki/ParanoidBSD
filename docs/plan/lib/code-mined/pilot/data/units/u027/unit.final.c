#include "prelude.h"
#undef SIGN_BIT
#define	SIGN_BIT	(0x80)		
#undef QUANT_MASK
#define	QUANT_MASK	(0xf)		
#undef NSEGS
#define	NSEGS		(8)		
#undef SEG_SHIFT
#define	SEG_SHIFT	(4)		
#undef SEG_MASK
#define	SEG_MASK	(0x70)		
#undef BIAS
#define	BIAS		(0x84)		
/*@ requires 0 <= val <= 0x7FFF; ensures 0 <= \result <= 7; */
static inline int val_seg(int val)
{
	int r = 0;
	val >>= 7;
	if (val & 0xf0) {
		val >>= 4;
		r += 4;
	}
	if (val & 0x0c) {
		val >>= 2;
		r += 2;
	}
	if (val & 0x02)
		r += 1;
	return r;
}
