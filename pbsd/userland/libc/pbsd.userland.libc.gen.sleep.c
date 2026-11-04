/* Reference logic from hbsd/src/lib/libc/gen/sleep.c (dual-world). */
#include <limits.h>

unsigned
sleep_chunk(unsigned seconds)
{
	return seconds > (unsigned)INT_MAX ? (unsigned)INT_MAX : seconds;
}

unsigned
sleep_remain(unsigned requested, unsigned slept)
{
	return slept >= requested ? 0 : requested - slept;
}
