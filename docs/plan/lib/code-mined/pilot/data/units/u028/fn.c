static inline int get_mmcr1_rsq(u64 mmcr1, int pmc)
{
	return mmcr1 & MMCR1_RSQ;
}
