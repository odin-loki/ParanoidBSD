int bits_memalloc(void)
{
	u64 data[2];
	int nr = 0;
	int *bit;

	__builtin_memset(&data, 0xf0, sizeof(data)); /* 4 * 16 */
	bpf_for_each(bits, bit, &data[0], ARRAY_SIZE(data))
		nr++;
	return nr;
}
