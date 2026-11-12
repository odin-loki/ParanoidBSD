static inline u32 omap_usec_to_32k(u32 usec)
{
	/*@ assert DIV_ROUND_UP_ULL(32768ULL * (u64)usec, 1000000ULL) ==
	           (32768ULL * (u64)usec + 999999ULL) / 1000000ULL; */
	return DIV_ROUND_UP_ULL(32768ULL * (u64)usec, 1000000ULL);
}
