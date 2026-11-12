addcarryx_u26(u8 /*bool*/ c, u32 a, u32 b, u32 *low)
{
	/* This function extracts 26 bits of result and 1 bit of carry
	 * (27 total), so a 32-bit intermediate is sufficient.
	 */
	u32 x = a + b + c;
	/*@ assert x == a + b + c; */
	/*@ assert x <= 134217727; */
	*low = x & ((1 << 26) - 1);
	/*@ assert *low == x % 67108864; */
	/*@ assert (x >> 26) == x / 67108864; */
	/*@ assert (x >> 26) <= 1; */
	/*@ assert ((x >> 26) & 1) == (x >> 26); */
	return (x >> 26) & 1;
}
