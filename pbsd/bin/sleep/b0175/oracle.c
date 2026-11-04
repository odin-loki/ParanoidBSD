/*
 * b0175 oracle: the original HardenedBSD C sources for this batch, concatenated
 * and renamed with a `ref_' prefix.  Function bodies are unmodified; this file
 * is the specification the C++23 port is differentially tested against.
 *
 * Sources:
 *	hbsd/src/bin/sleep/sleep.c
 *
 * Mechanical adjustments only (no function body is touched):
 *	- functions renamed with a `ref_' prefix;
 *	- `static' dropped from the renamed functions and from the file-scope
 *	  variable `report_requested' so the harness can link against them;
 *	- <capsicum_helpers.h> not included and main() not carried over (see
 *	  skipped.txt);
 *	- __unused / __dead2 defined when the host <sys/cdefs.h> lacks them.
 */

/*-
 * Copyright (c) 1988, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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

#include <sys/cdefs.h>

#ifndef __unused
#define	__unused	__attribute__((__unused__))
#endif

#ifndef __dead2
#define	__dead2		__attribute__((__noreturn__))
#endif

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t report_requested;

void
ref_report_request(int signo __unused)
{
	report_requested = 1;
}

void __dead2
ref_usage(void)
{
	fprintf(stderr, "usage: sleep number[unit] [...]\n"
	    "Unit can be 's' (seconds, the default), "
	    "m (minutes), h (hours), or d (days).\n");
	exit(1);
}

double
ref_parse_interval(const char *arg)
{
	double num;
	char unit, extra;

	switch (sscanf(arg, "%lf%c%c", &num, &unit, &extra)) {
	case 2:
		switch (unit) {
		case 'd':
			num *= 24;
			/* FALLTHROUGH */
		case 'h':
			num *= 60;
			/* FALLTHROUGH */
		case 'm':
			num *= 60;
			/* FALLTHROUGH */
		case 's':
			if (!isnan(num))
				return (num);
		}
		break;
	case 1:
		if (!isnan(num))
			return (num);
	}
	warnx("invalid time interval: %s", arg);
	return (INFINITY);
}
