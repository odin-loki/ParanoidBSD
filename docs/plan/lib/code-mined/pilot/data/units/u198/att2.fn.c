static bool
vmx_ctl_allows_zero_setting(uint64_t msr_val, int bitpos)
{
	assert(((msr_val >> bitpos) & 1) == 0 ==> (msr_val & (1UL << bitpos)) == 0);
	assert((msr_val & (1UL << bitpos)) == 0 ==> ((msr_val >> bitpos) & 1) == 0);
	return ((msr_val & (1UL << bitpos)) == 0);
}
