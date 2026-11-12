static u32 __pure nexp_u32(u32 ux)
{
	static const u16 v[] = {
		/* exp(-x)*65536-1 for x = 0, 0.000256, 0.000512, ... */
		65535,
		65518, 65501, 65468, 65401, 65267, 65001, 64470, 63422,
		61378, 57484, 50423, 38795, 22965, 8047,  987,   14,
	};
	u32 msb = ux >> 8;
	u32 res;
	int i;

	/* Cut off when ux >= 2^24 (actual result is <= 222/U32_MAX). */
	if (msb > U16_MAX)
		return 0;

	/* Scale first eight bits linearly: */
	res = U32_MAX - (ux & 0xff) * (U32_MAX / 1000000);

	/* Obtain e^(x + y + ...) by computing e^x * e^y * ...: */
	for (i = 1; msb; i++, msb >>= 1) {
		u32 y = v[i & -(msb & 1)] + U32_C(1);

		res = ((u64)res * y) >> 16;
	}

	return res;
}
