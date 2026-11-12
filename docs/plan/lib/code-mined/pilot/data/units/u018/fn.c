bool iwl_he_is_sgi(u32 rate_n_flags)
{
	u32 type = rate_n_flags & RATE_MCS_HE_TYPE_MSK;
	u32 ltf_gi = rate_n_flags & RATE_MCS_HE_GI_LTF_MSK;

	if (type == RATE_MCS_HE_TYPE_SU ||
	    type == RATE_MCS_HE_TYPE_EXT_SU)
		return ltf_gi == RATE_MCS_HE_SU_4_LTF_08_GI;
	return false;
}
