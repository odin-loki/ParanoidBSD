module;

#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef __unused
#define	__unused	__attribute__((__unused__))
#endif

#ifndef __dead2
#define	__dead2		__attribute__((__noreturn__))
#endif

export module pbsd.bin.dd.b0215;

export namespace pbsd::bin_dd::b0215 {

typedef unsigned char u_char;

struct STAT {
	uintmax_t	in_full;
	uintmax_t	in_part;
	uintmax_t	out_full;
	uintmax_t	out_part;
	uintmax_t	trunc;
	uintmax_t	swab;
	uintmax_t	bytes;
	struct timespec	start;
};

#define	C_NOXFER	0x0000000010000000ULL
#define	C_NOINFO	0x0000000020000000ULL
#define	C_PROGRESS	0x0000000040000000ULL

#define	HN_DECIMAL		0x01
#define	HN_NOSPACE		0x02
#define	HN_B			0x04
#define	HN_DIVISOR_1000		0x08
#define	HN_IEC_PREFIXES		0x10
#define	HN_GETSCALE		0x10
#define	HN_AUTOSCALE		0x20

static const int maxscale = 6;

static int
humanize_number(char *buf, size_t len, int64_t quotient,
    const char *suffix, int scale, int flags)
{
	const char *prefixes, *sep;
	int	i, r, remainder, s1, s2, sign;
	int	divisordeccut;
	int64_t	divisor, max;
	size_t	baselen;

	if (len > 0)
		buf[0] = '\0';

	if (buf == NULL || suffix == NULL)
		return (-1);
	if (scale < 0)
		return (-1);
	else if (scale > maxscale &&
	    ((scale & ~(HN_AUTOSCALE|HN_GETSCALE)) != 0))
		return (-1);
	if ((flags & HN_DIVISOR_1000) && (flags & HN_IEC_PREFIXES))
		return (-1);

	remainder = 0;

	if (flags & HN_IEC_PREFIXES) {
		baselen = 2;
		divisor = 1024;
		divisordeccut = 973;
		if (flags & HN_B)
			prefixes = "B\0\0Ki\0Mi\0Gi\0Ti\0Pi\0Ei";
		else
			prefixes = "\0\0\0Ki\0Mi\0Gi\0Ti\0Pi\0Ei";
	} else {
		baselen = 1;
		if (flags & HN_DIVISOR_1000) {
			divisor = 1000;
			divisordeccut = 950;
			if (flags & HN_B)
				prefixes = "B\0\0k\0\0M\0\0G\0\0T\0\0P\0\0E";
			else
				prefixes = "\0\0\0k\0\0M\0\0G\0\0T\0\0P\0\0E";
		} else {
			divisor = 1024;
			divisordeccut = 973;
			if (flags & HN_B)
				prefixes = "B\0\0K\0\0M\0\0G\0\0T\0\0P\0\0E";
			else
				prefixes = "\0\0\0K\0\0M\0\0G\0\0T\0\0P\0\0E";
		}
	}

#define	SCALE2PREFIX(scale)	(&prefixes[(scale) * 3])

	if (quotient < 0) {
		sign = -1;
		quotient = -quotient;
		baselen += 2;
	} else {
		sign = 1;
		baselen += 1;
	}
	if (flags & HN_NOSPACE)
		sep = "";
	else {
		sep = " ";
		baselen++;
	}
	baselen += strlen(suffix);

	if (len < baselen + 1)
		return (-1);

	if (scale & (HN_AUTOSCALE | HN_GETSCALE)) {
		for (max = 1, i = len - baselen; i-- > 0;)
			max *= 10;

		for (i = 0;
		    (quotient >= max || (quotient == max - 1 &&
		    (remainder >= divisordeccut || remainder >=
		    divisor / 2))) && i < maxscale; i++) {
			remainder = quotient % divisor;
			quotient /= divisor;
		}

		if (scale & HN_GETSCALE)
			return (i);
	} else {
		for (i = 0; i < scale && i < maxscale; i++) {
			remainder = quotient % divisor;
			quotient /= divisor;
		}
	}

	if (((quotient == 9 && remainder < divisordeccut) || quotient < 9) &&
	    i > 0 && flags & HN_DECIMAL) {
		s1 = (int)quotient + ((remainder * 10 + divisor / 2) /
		    divisor / 10);
		s2 = ((remainder * 10 + divisor / 2) / divisor) % 10;
		r = snprintf(buf, len, "%d%s%d%s%s%s",
		    sign * s1, localeconv()->decimal_point, s2,
		    sep, SCALE2PREFIX(i), suffix);
	} else
		r = snprintf(buf, len, "%" PRId64 "%s%s%s",
		    sign * (quotient + (remainder + divisor / 2) / divisor),
		    sep, SCALE2PREFIX(i), suffix);

	return (r);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

STAT st;
uint64_t ddflags;
volatile sig_atomic_t need_summary;
volatile sig_atomic_t need_progress;
volatile sig_atomic_t kill_signal;
sig_atomic_t in_io;
sig_atomic_t sigint_seen;

double
secs_elapsed(void)
{
	struct timespec end, ts_res;
	double secs, res;

	if (clock_gettime(CLOCK_MONOTONIC, &end))
		err(1, "clock_gettime");
	if (clock_getres(CLOCK_MONOTONIC, &ts_res))
		err(1, "clock_getres");
	secs = (end.tv_sec - st.start.tv_sec) + \
	       (end.tv_nsec - st.start.tv_nsec) * 1e-9;
	res = ts_res.tv_sec + ts_res.tv_nsec * 1e-9;
	if (secs < res)
		secs = res;

	return (secs);
}

void
summary(void)
{
	double secs;

	if (ddflags & C_NOINFO)
		return;

	if (ddflags & C_PROGRESS)
		fprintf(stderr, "\n");

	secs = secs_elapsed();

	(void)fprintf(stderr,
	    "%ju+%ju records in\n%ju+%ju records out\n",
	    st.in_full, st.in_part, st.out_full, st.out_part);
	if (st.swab)
		(void)fprintf(stderr, "%ju odd length swab %s\n",
		     st.swab, (st.swab == 1) ? "block" : "blocks");
	if (st.trunc)
		(void)fprintf(stderr, "%ju truncated %s\n",
		     st.trunc, (st.trunc == 1) ? "block" : "blocks");
	if (!(ddflags & C_NOXFER)) {
		(void)fprintf(stderr,
		    "%ju bytes transferred in %.6f secs (%.0f bytes/sec)\n",
		    st.bytes, secs, st.bytes / secs);
	}
	need_summary = 0;
}

void
progress(void)
{
	static int outlen;
	char si[4 + 1 + 2 + 1];
	char iec[4 + 1 + 3 + 1];
	char persec[4 + 1 + 2 + 1];
	char *buf;
	double secs;

	secs = secs_elapsed();
	humanize_number(si, sizeof(si), (int64_t)st.bytes, "B", HN_AUTOSCALE,
	    HN_DECIMAL | HN_DIVISOR_1000);
	humanize_number(iec, sizeof(iec), (int64_t)st.bytes, "B", HN_AUTOSCALE,
	    HN_DECIMAL | HN_IEC_PREFIXES);
	humanize_number(persec, sizeof(persec), (int64_t)(st.bytes / secs), "B",
	    HN_AUTOSCALE, HN_DECIMAL | HN_DIVISOR_1000);
	asprintf(&buf, "  %'ju bytes (%s, %s) transferred %.3fs, %s/s",
	    (uintmax_t)st.bytes, si, iec, secs, persec);
	outlen = fprintf(stderr, "%-*s\r", outlen, buf) - 1;
	fflush(stderr);
	free(buf);
	need_progress = 0;
}

/* ARGSUSED */
void
siginfo_handler(int signo __unused)
{

	need_summary = 1;
}

/* ARGSUSED */
void
sigalarm_handler(int signo __unused)
{

	need_progress = 1;
}

void terminate(int signo) __dead2;
void
terminate(int signo)
{
	kill_signal = signo;
	summary();
	(void)fflush(stderr);
	raise(kill_signal);
	/* NOT REACHED */
	_exit(1);
}

void
sigint_handler(int signo __unused)
{
	atomic_signal_fence(memory_order_acquire);
	if (in_io)
		terminate(SIGINT);
	sigint_seen = 1;
}

void
prepare_io(void)
{
	struct sigaction sa;
	int error;

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NODEFER | SA_RESETHAND;
	sa.sa_handler = sigint_handler;
	error = sigaction(SIGINT, &sa, 0);
	if (error != 0)
		err(1, "sigaction");
}

void
before_io(void)
{
	in_io = 1;
	atomic_signal_fence(memory_order_seq_cst);
	if (sigint_seen)
		terminate(SIGINT);
}

void
after_io(void)
{
	in_io = 0;
	atomic_signal_fence(memory_order_seq_cst);
	if (sigint_seen)
		terminate(SIGINT);
}

} // namespace pbsd::bin_dd::b0215
