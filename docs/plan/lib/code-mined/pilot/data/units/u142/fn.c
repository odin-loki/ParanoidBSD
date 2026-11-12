static inline int inat_group_id(insn_attr_t attr)
{
	return (attr & INAT_GRP_MASK) >> INAT_GRP_OFFS;
}
