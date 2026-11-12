static u32 altera_bits_req(u32 n)
{
	u32 result = SHORT_BITS;

	if (n == 0)
		result = 1;
	else {
		/* Look for the highest non-zero bit position */
		while ((n & (1 << (SHORT_BITS - 1))) == 0) {
			n <<= 1;
			--result;
		}
	}

	return result;
}
