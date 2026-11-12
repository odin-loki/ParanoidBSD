static uint32_t raw_checksum(uint8_t *buf, int len, uint32_t sum)
{
	int i;

	/*@ loop invariant 0 <= i <= len;
	    loop invariant i % 2 == 0;
	    loop invariant sum <= 0xffff;
	    loop assigns i, sum;
	    loop variant len - i;
	 */
	for (i = 0; i < (len & ~1U); i += 2) {
		sum += (u_int16_t)ntohs(*((u_int16_t *)(buf + i)));
		if (sum > 0xffff)
			sum -= 0xffff;
	}

	/*@ assert i % 2 == 0; */
	if (i < len) {
		sum += buf[i] << 8;
		if (sum > 0xffff)
			sum -= 0xffff;
	}

	return sum;
}
