static int
int2bcd(int i)
{
	int retval = 0;
	int base = 0;

	/*
	 * Both ends.  A negative i makes `i % 10' negative and shifting
	 * a negative value left is undefined; worse, |i| can need more
	 * than eight digits, so base passes 31 and the shift distance
	 * exceeds the width as well.  -1 is what this already answers
	 * for a value it cannot represent.
	 */
	if (i < 0 || i >= 10000)
		return -1;
    
	while (i) {
		retval |= (i % 10) << base;
		i /= 10;
		base += 4;
	}
	return retval;
}
