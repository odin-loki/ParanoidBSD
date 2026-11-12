#include "prelude.h"
/*@ requires 0 <= bitpos && bitpos < 32; ensures \result == ((msr_val >> bitpos) & 1) == 0; */
static bool
vmx_ctl_allows_zero_setting(uint64_t msr_val, int bitpos)
{

	return ((msr_val & (1UL << bitpos)) == 0);
}
