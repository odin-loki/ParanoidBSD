static inline uint32_t
nm_prev(uint32_t i, uint32_t lim)
{
	return unlikely (i == 0) ? lim : i - 1;
}
