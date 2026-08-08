/*-
 * Copyright (c) 2014 Baptiste Daroussin <bapt@FreeBSD.org>
 * Copyright (c) 2014 Vsevolod Stakhov <vsevolod@FreeBSD.org>
 * Copyright (c) 2025 Aaron LI <aly@aaronly.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * PBSD batch b0197 -- port of hbsd/src/bin/timeout/timeout.c
 */

module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef __printflike
#define __printflike(fmtarg, firstvararg) \
	__attribute__((__format__(__printf__, fmtarg, firstvararg)))
#endif

/* FreeBSD signal that this platform's <signal.h> does not provide. */
#ifndef SIGEMT
#define SIGEMT 32
#endif

#define EXIT_TIMEOUT	124
#define EXIT_INVALID	125
#define EXIT_CMD_ERROR	126
#define EXIT_CMD_NOENT	127

export module pbsd.bin.timeout.b0197;

export namespace pbsd::bin_timeout::b0197 {

volatile ::sig_atomic_t sig_chld = 0;
volatile ::sig_atomic_t sig_alrm = 0;
volatile ::sig_atomic_t sig_term = 0; /* signal to terminate children */
volatile ::sig_atomic_t sig_other = 0; /* signal to propagate */
int killsig = SIGTERM; /* signal to kill children */
bool verbose = false;

void __printflike(1, 2)
logv(const char *fmt, ...)
{
	va_list ap;

	if (!verbose)
		return;

	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
}

double
parse_duration(const char *duration)
{
	double ret;
	char *suffix;

	ret = strtod(duration, &suffix);
	if (suffix == duration)
		errx(EXIT_INVALID, "duration is not a number");

	if (*suffix == '\0')
		return (ret);

	if (suffix[1] != '\0')
		errx(EXIT_INVALID, "duration unit suffix too long");

	switch (*suffix) {
	case 's':
		break;
	case 'm':
		ret *= 60;
		break;
	case 'h':
		ret *= 60 * 60;
		break;
	case 'd':
		ret *= 60 * 60 * 24;
		break;
	default:
		errx(EXIT_INVALID, "duration unit suffix invalid");
	}

	if (ret < 0 || ret >= 100000000UL)
		errx(EXIT_INVALID, "duration out of range");

	return (ret);
}

void
sig_handler(int signo)
{
	if (signo == killsig) {
		sig_term = signo;
		return;
	}

	switch (signo) {
	case SIGCHLD:
		sig_chld = 1;
		break;
	case SIGALRM:
		sig_alrm = 1;
		break;
	case SIGHUP:
	case SIGINT:
	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGABRT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
	case SIGPIPE:
	case SIGTERM:
	case SIGXCPU:
	case SIGXFSZ:
	case SIGVTALRM:
	case SIGPROF:
	case SIGUSR1:
	case SIGUSR2:
		/*
		 * Signals with default action to terminate the process.
		 * See the sigaction(2) man page.
		 */
		sig_term = signo;
		break;
	default:
		sig_other = signo;
		break;
	}
}

void
set_interval(double iv)
{
	struct itimerval tim;

	memset(&tim, 0, sizeof(tim));
	if (iv > 0) {
		tim.it_value.tv_sec = (time_t)iv;
		iv -= (double)(time_t)iv;
		tim.it_value.tv_usec = (suseconds_t)(iv * 1000000UL);
	}

	if (setitimer(ITIMER_REAL, &tim, NULL) < 0)
		err(EXIT_FAILURE, "setitimer()");
}

void
log_termination(const char *name, const siginfo_t *si)
{
	if (si->si_code == CLD_EXITED) {
		logv("%s: pid=%d, exit=%d", name, si->si_pid, si->si_status);
	} else if (si->si_code == CLD_DUMPED || si->si_code == CLD_KILLED) {
		logv("%s: pid=%d, sig=%d", name, si->si_pid, si->si_status);
	} else {
		logv("%s: pid=%d, reason=%d, status=%d", name, si->si_pid,
		    si->si_code, si->si_status);
	}
}

} // namespace pbsd::bin_timeout::b0197
