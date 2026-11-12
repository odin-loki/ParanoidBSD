#include "prelude.h"
#undef _ASM_BITOPS_H
#define _ASM_BITOPS_H
#undef ffs
#define ffs(x)	({ unsigned long __t = (x); fls(__t & -__t); })
#undef ffz
#define ffz(x)	__ffs(~(x))
/*@ requires true; ensures (x == 0 ==> \result == 0) && (x != 0 ==> 1 <= \result <= 32 && (x & (1 << (\result - 1))) != 0 && (x >> \result) == 0); */
static inline __attribute__ ((const)) int fls(unsigned int x)
{
	if (__builtin_constant_p(x))
	       return constant_fls(x);

	return 32 - clz(x);
}
