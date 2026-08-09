/* ---- pmadvise.c ---- */
/*
 * The contents of this file are in the public domain.
 * Written by Garrett A. Wollman, 2000-10-07.
 *
 */

#include <sys/mman.h>
#include <errno.h>

int
ref_posix_madvise(void *address, size_t size, int how)
{
	int ret, saved_errno;

	saved_errno = errno;
	if (madvise(address, size, how) == -1) {
		ret = errno;
		errno = saved_errno;
	} else {
		ret = 0;
	}
	return (ret);
}

/* ---- timespec_getres.c ---- */
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

/* ---- lrand48.c ---- */
/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#include "rand48.h"

long
ref_lrand48(void)
{
	_DORAND48(_rand48_seed);
	return (_rand48_seed >> 17) & 0x7fffffff;
}

/* ---- drand48.c ---- */
/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#include "rand48.h"

double
ref_drand48(void)
{
	ERAND48_BEGIN;
	_DORAND48(_rand48_seed);
	ERAND48_END(_rand48_seed);
}

