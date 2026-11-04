/*-
 * Copyright (c) 2023 Dag-Erling Smørgrav
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <time.h>

int
ref_timespec_getres(struct timespec *ts, int base)
{

	switch (base) {
	case TIME_UTC:
		if (clock_getres(CLOCK_REALTIME, ts) == 0)
			return (base);
		break;
	case TIME_MONOTONIC:
		if (clock_getres(CLOCK_MONOTONIC, ts) == 0)
			return (base);
		break;
	}
	return (0);
}
