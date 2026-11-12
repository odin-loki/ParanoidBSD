static inline u32 omap_usec_to_32k(u32 usec)
{
	return DIV_ROUND_UP_ULL(32768ULL * (u64)usec, 1000000ULL);
}
