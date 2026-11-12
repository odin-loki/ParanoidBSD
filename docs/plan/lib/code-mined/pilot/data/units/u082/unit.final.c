#include "prelude.h"
#undef _FUTEX_H
#define _FUTEX_H
#undef FLAGS_SIZE_8
#define FLAGS_SIZE_8		0x0000
#undef FLAGS_SIZE_16
#define FLAGS_SIZE_16		0x0001
#undef FLAGS_SIZE_32
#define FLAGS_SIZE_32		0x0002
#undef FLAGS_SIZE_64
#define FLAGS_SIZE_64		0x0003
#undef FLAGS_SIZE_MASK
#define FLAGS_SIZE_MASK		0x0003
#undef FLAGS_CLOCKRT
#define FLAGS_CLOCKRT		0x0020
#undef FLAGS_HAS_TIMEOUT
#define FLAGS_HAS_TIMEOUT	0x0040
#undef FLAGS_NUMA
#define FLAGS_NUMA		0x0080
#undef FLAGS_STRICT
#define FLAGS_STRICT		0x0100
#undef FLAGS_MPOL
#define FLAGS_MPOL		0x0200
#undef FUTEX2_VALID_MASK
#define FUTEX2_VALID_MASK (FUTEX2_SIZE_MASK | FUTEX2_NUMA | FUTEX2_MPOL | FUTEX2_PRIVATE)
/*@ requires true; ensures \result == false; */
static inline bool should_fail_futex(bool fshared)
{
	return false;
}
