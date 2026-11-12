static pt_entry_t
sysctl_kmaps_block_attrs(pt_entry_t block)
{
	return (block & (ATTR_S1_AP_MASK | ATTR_S1_XN | ATTR_S1_IDX_MASK |
	    ATTR_S1_GP));
}
