static bool
vmx_ctl_allows_zero_setting(uint64_t msr_val, int bitpos)
{
	assert((msr_val & (1UL << bitpos)) == 0 || (msr_val & (1UL << bitpos)) == (1UL << bitpos));
	assert(((msr_val >> bitpos) & 1UL) == 0 || ((msr_val >> bitpos) & 1UL) == 1);
	assert((msr_val & (1UL << bitpos)) == (((msr_val >> bitpos) & 1UL) << bitpos));
	assert((1UL << bitpos) != 0);

	return ((msr_val & (1UL << bitpos)) == 0);
}
