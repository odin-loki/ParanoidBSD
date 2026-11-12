#include "prelude.h"
#undef DRAM_TYPE_STR
#define DRAM_TYPE_STR(type) [INTEL_DRAM_ ## type] = #type
#undef SKL_DRAM_WIDTH_MASK
#define SKL_DRAM_WIDTH_MASK			(0x3 << 8)
#undef SKL_DRAM_WIDTH_SHIFT
#define SKL_DRAM_WIDTH_SHIFT			8
#undef SKL_DRAM_WIDTH_X8
#define SKL_DRAM_WIDTH_X8			(0x0 << 8)
#undef SKL_DRAM_WIDTH_X16
#define SKL_DRAM_WIDTH_X16			(0x1 << 8)
#undef SKL_DRAM_WIDTH_X32
#define SKL_DRAM_WIDTH_X32			(0x2 << 8)
struct dram_dimm_info { 	u16 size; 	u8 width, ranks; };
struct dram_channel_info { 	struct dram_dimm_info dimm_l, dimm_s; 	u8 ranks; 	bool is_16gb_dimm; };
/*@ requires val >= 0; ensures \result == 0 || \result == 8 << ((val & SKL_DRAM_WIDTH_MASK) >> SKL_DRAM_WIDTH_SHIFT); */
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
