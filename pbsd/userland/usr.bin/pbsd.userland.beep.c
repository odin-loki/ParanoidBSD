/* Reference logic from hbsd/src/usr.bin/beep/beep.c (dual-world). */

#define DURATION_MIN 50
#define DURATION_MAX 2000
#define GAIN_MIN 0
#define GAIN_MAX 100

int
beep_clamp(int val, int lo, int hi)
{
	if (val < lo)
		return lo;
	if (val > hi)
		return hi;
	return val;
}

int
beep_valid_duration(int ms)
{
	return ms >= DURATION_MIN && ms <= DURATION_MAX;
}

int
beep_valid_gain(int gain)
{
	return gain >= GAIN_MIN && gain <= GAIN_MAX;
}
