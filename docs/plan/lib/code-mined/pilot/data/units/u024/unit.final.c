#include "prelude.h"
#undef DEBUG
#define DEBUG
#undef VERSION
#define VERSION 3
#undef PORT_MAX
#define PORT_MAX UINT16_MAX
/*@ requires 0 <= c <= 255; ensures \result == ((c & 0xC0) == 0x80); */
static int
isu8cont(unsigned char c)
{
	return (c & (0x80 | 0x40)) == 0x80;
}
