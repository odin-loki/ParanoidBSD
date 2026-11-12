static unsigned int random_ecc_bit(size_t size)
{
	unsigned int offset = get_random_u32_below(3 * BITS_PER_BYTE);

	if (size == 256) {
		/*
		 * Don't inject a bit error into the insignificant bits (16th
		 * and 17th bit) in ECC code for 256 byte data block
		 */
		while (offset == 16 || offset == 17)
			offset = get_random_u32_below(3 * BITS_PER_BYTE);
	}

	return offset;
}
