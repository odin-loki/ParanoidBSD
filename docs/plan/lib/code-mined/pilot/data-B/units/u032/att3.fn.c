static u32 altera_bits_req(u32 n)
{
	u32 result = SHORT_BITS;

	if (n == 0)
		result = 1;
	else {
		/* Look for the highest non-zero bit position */
		/*@ loop invariant 1 <= result <= 16;
		    loop invariant n == \at(n, Pre) << (16 - result);
		    loop invariant \at(n, Pre) >> result == 0;
		    loop invariant n <= 65535;
		    loop assigns n, result;
		    loop variant result; */
		while ((n & (1 << (SHORT_BITS - 1))) == 0) {
			n <<= 1;
			--result;
		}
	}

	return result;
}
