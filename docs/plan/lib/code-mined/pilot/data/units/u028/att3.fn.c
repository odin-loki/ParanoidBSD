static inline int get_mmcr1_rsq(u64 mmcr1, int pmc)
{
	/*@ assert MMCR1_RSQ == 0x200000000000ULL; */
	return mmcr1 & MMCR1_RSQ;
}
