static int hisi_ptt_trace_valid_type(u32 val)
{
	/* Different types can be set simultaneously */
	static const u32 hisi_ptt_trace_available_type[] = {
		1,	/* posted_request */
		2,	/* non-posted_request */
		4,	/* completion */
	};
	int i;

	if (!val)
		return -EINVAL;

	/*
	 * Walk the available list and clear the valid bits of
	 * the config. If there is any resident bit after the
	 * walk then the config is invalid.
	 */
	for (i = 0; i < ARRAY_SIZE(hisi_ptt_trace_available_type); i++)
		val &= ~hisi_ptt_trace_available_type[i];

	if (val)
		return -EINVAL;

	return 0;
}
