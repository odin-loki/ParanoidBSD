#include "prelude.h"
#undef IWL_DECLARE_RATE_INFO
#define IWL_DECLARE_RATE_INFO(r)  	[IWL_RATE_##r##M_INDEX] = IWL_RATE_##r##M_PLCP
#undef RATE_MCS_HE_GI_LTF_MSK
#define RATE_MCS_HE_GI_LTF_MSK		(0x7 << RATE_MCS_HE_GI_LTF_POS)
#undef RATE_MCS_HE_TYPE_MSK
#define RATE_MCS_HE_TYPE_MSK		(3 << RATE_MCS_HE_TYPE_POS)
#undef RATE_MCS_HE_GI_LTF_POS
#define RATE_MCS_HE_GI_LTF_POS		20
#undef RATE_MCS_HE_TYPE_POS
#define RATE_MCS_HE_TYPE_POS		23
#undef RATE_MCS_HE_SU_4_LTF_08_GI
#define RATE_MCS_HE_SU_4_LTF_08_GI	4
#undef RATE_MCS_HE_TYPE_SU
#define RATE_MCS_HE_TYPE_SU		(0 << RATE_MCS_HE_TYPE_POS)
#undef RATE_MCS_HE_TYPE_EXT_SU
#define RATE_MCS_HE_TYPE_EXT_SU		(1 << RATE_MCS_HE_TYPE_POS)
/*@ requires true; ensures \result == ((rate_n_flags & RATE_MCS_HE_TYPE_MSK) == RATE_MCS_HE_TYPE_SU || (rate_n_flags & RATE_MCS_HE_TYPE_MSK) == RATE_MCS_HE_TYPE_EXT_SU) && ((rate_n_flags & RATE_MCS_HE_GI_LTF_MSK) == RATE_MCS_HE_SU_4_LTF_08_GI); */
bool iwl_he_is_sgi(u32 rate_n_flags)
{
	u32 type = rate_n_flags & RATE_MCS_HE_TYPE_MSK;
	u32 ltf_gi = rate_n_flags & RATE_MCS_HE_GI_LTF_MSK;

	if (type == RATE_MCS_HE_TYPE_SU ||
	    type == RATE_MCS_HE_TYPE_EXT_SU)
		return ltf_gi == RATE_MCS_HE_SU_4_LTF_08_GI;
	return false;
}
