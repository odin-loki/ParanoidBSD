#include "prelude.h"
#undef POWER11
#define POWER11 0x82
#undef POWER10
#define POWER10 0x80
#undef POWER9
#define POWER9  0x4e
#undef PERF_POWER9_MASK
#define PERF_POWER9_MASK        0x7f8ffffffffffff
#undef PERF_POWER10_MASK
#define PERF_POWER10_MASK       0x7ffffffffffffff
#undef PERF_POWER11_MASK
#define PERF_POWER11_MASK       PERF_POWER10_MASK
#undef MMCR0_FC56
#define MMCR0_FC56      0x00000010UL 
#undef MMCR0_PMCCEXT
#define MMCR0_PMCCEXT   0x00000200UL 
#undef MMCR1_RSQ
#define MMCR1_RSQ       0x200000000000ULL 
#undef BHRB_DISABLE
#define BHRB_DISABLE    0x2000000000ULL 
#undef EV_CODE_EXTRACT
#define EV_CODE_EXTRACT(x, y)    	((x >> ev_shift_##y) & ev_mask_##y)
/*@ requires true; ensures \result == (mmcr1 & 0x200000000000ULL); */
static inline int get_mmcr1_rsq(u64 mmcr1, int pmc)
{
	return mmcr1 & MMCR1_RSQ;
}
