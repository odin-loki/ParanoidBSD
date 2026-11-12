static int
isu8cont(unsigned char c)
{
	return (c & (0x80 | 0x40)) == 0x80;
}
