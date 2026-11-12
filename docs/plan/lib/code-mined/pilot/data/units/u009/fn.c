static int skl_get_dimm_width(u16 val)
{
	if (skl_get_dimm_size(val) == 0)
		return 0;

	switch (val & SKL_DRAM_WIDTH_MASK) {
	case SKL_DRAM_WIDTH_X8:
	case SKL_DRAM_WIDTH_X16:
	case SKL_DRAM_WIDTH_X32:
		val = (val & SKL_DRAM_WIDTH_MASK) >> SKL_DRAM_WIDTH_SHIFT;
		return 8 << val;
	default:
		MISSING_CASE(val);
		return 0;
	}
}
