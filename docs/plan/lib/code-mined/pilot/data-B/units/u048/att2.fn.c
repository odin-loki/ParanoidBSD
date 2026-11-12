static int __init dmi_checksum(const u8 *buf, u8 len)
{
	u8 sum = 0;
	int a;

	/*@ loop invariant 0 <= a <= (int) len;
	    loop assigns a, sum;
	    loop variant (int) len - a; */
	for (a = 0; a < len; a++)
		sum += buf[a];

	return sum == 0;
}
