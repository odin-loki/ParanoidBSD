/* Reference logic from hbsd/src/usr.bin/random/random.c (dual-world). */
#include <stddef.h>

double
random_parse_denom(const char *arg)
{
	double val = 0.0;
	int seen_dot = 0;

	if (arg == NULL || *arg == '\0')
		return -1.0;
	for (; *arg != '\0'; ++arg) {
		if (*arg == '.') {
			if (seen_dot)
				return -1.0;
			seen_dot = 1;
			continue;
		}
		if (*arg < '0' || *arg > '9')
			return -1.0;
		val = val * 10.0 + (*arg - '0');
	}
	if (val <= 0.0)
		return -1.0;
	return 1.0 / val;
}

int
random_select(double threshold, double sample)
{
	return sample < threshold;
}
