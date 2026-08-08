/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
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
/*
 * Important: This file is used both as a standalone program /bin/kill and
 * as a builtin for /bin/sh (#define SHELL).
 */

/*
 * PBSD batch b0177 oracle: hbsd/src/bin/kill/kill.c.
 *
 * Every function of the original file appears below with a ref_ prefix and an
 * otherwise UNMODIFIED body; the file-static qualifiers were dropped so the
 * differential harness can reach them.
 *
 * Everything above the "original sources" banner is scaffolding for the
 * interfaces FreeBSD's libc provides but the host libc does not: sys_nsig,
 * sys_signame, sig2str(), str2sig(), SIG2STR_MAX and __dead2.  port.cppm
 * links against these very definitions, so both sides of the comparison see
 * bit-identical support code and only the ported logic is under test.
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifndef __dead2
#define	__dead2	__attribute__((__noreturn__))
#endif

#ifndef LONG_BIT
#define	LONG_BIT	(CHAR_BIT * (int)sizeof(long))
#endif

#ifndef SIG2STR_MAX
#define	SIG2STR_MAX	17
#endif

/* Number of sys_signame[] slots that carry a genuine FreeBSD signal name. */
#define	PBSD_NAMED_NSIG	32
/* Highest signal number sig2str()/str2sig() accept. */
#define	PBSD_MAX_SIG	255

/*
 * sys_signame[] carries the 32 real names, then padding so that a read past
 * sys_nsig (which the harness sweeps) still lands on initialised memory.
 */
const char *const sys_signame[] = {
	"0",	 "hup",	  "int",    "quit",  "ill",   "trap",  "abrt", "emt",
	"fpe",	 "kill",  "bus",    "segv",  "sys",   "pipe",  "alrm", "term",
	"urg",	 "stop",  "tstp",   "cont",  "chld",  "ttin",  "ttou", "io",
	"xcpu",	 "xfsz",  "vtalrm", "prof",  "winch", "info",  "usr1", "usr2",
	"rt1",	 "rt2",	  "rt3",    "rt4",   "rt5",   "rt6",   "rt7",  "rt8",
	"rt9",	 "rt10",  "rt11",   "rt12",  "rt13",  "rt14",  "rt15", "rt16",
	"rt17",	 "rt18",  "rt19",   "rt20",  "rt21",  "rt22",  "rt23", "rt24",
	"rt25",	 "rt26",  "rt27",   "rt28",  "rt29",  "rt30",  "rt31", "rt32",
};

/*
 * Not const: the harness sweeps it so that printsignals()'s three uses of it
 * are exercised at every interesting boundary.
 */
int sys_nsig = PBSD_NAMED_NSIG;

int sig2str(int, char *);
int str2sig(const char * restrict, int * restrict);

int
sig2str(int sig, char *str)
{
	size_t i;

	if (sig < 1 || sig > PBSD_MAX_SIG)
		return (-1);
	if (sig < PBSD_NAMED_NSIG) {
		for (i = 0; sys_signame[sig][i] != '\0'; i++)
			str[i] = (char)toupper((unsigned char)
			    sys_signame[sig][i]);
		str[i] = '\0';
	} else {
		(void)snprintf(str, SIG2STR_MAX, "RT%d",
		    sig - (PBSD_NAMED_NSIG - 1));
	}
	return (0);
}

int
str2sig(const char * restrict s, int * restrict sig)
{
	char *ep;
	long v;
	int n;

	if (s == NULL || *s == '\0')
		return (-1);
	for (n = 1; n < PBSD_NAMED_NSIG; n++) {
		if (strcasecmp(s, sys_signame[n]) == 0) {
			*sig = n;
			return (0);
		}
	}
	if ((s[0] == 'R' || s[0] == 'r') && (s[1] == 'T' || s[1] == 't') &&
	    isdigit((unsigned char)s[2])) {
		errno = 0;
		v = strtol(s + 2, &ep, 10);
		if (*ep != '\0' || errno != 0 || v < 1 ||
		    v > PBSD_MAX_SIG - (PBSD_NAMED_NSIG - 1))
			return (-1);
		*sig = (int)v + (PBSD_NAMED_NSIG - 1);
		return (0);
	}
	if (!isdigit((unsigned char)s[0]))
		return (-1);
	errno = 0;
	v = strtol(s, &ep, 10);
	if (*ep != '\0' || errno != 0 || v < 1 || v > PBSD_MAX_SIG)
		return (-1);
	*sig = (int)v;
	return (0);
}

/* ------------------------- original sources ------------------------- */

void ref_nosig(const char *);
void ref_printsignals(FILE *);
void ref_usage(void) __dead2;

int
ref_main(int argc, char *argv[])
{
	char signame[SIG2STR_MAX];
	long pidl;
	pid_t pid;
	int errors, numsig, ret;
	char *ep;

	if (argc < 2)
		ref_usage();

	numsig = SIGTERM;

	argc--, argv++;
	if (!strcmp(*argv, "-l")) {
		argc--, argv++;
		if (argc > 1)
			ref_usage();
		if (argc == 1) {
			if (!isdigit(**argv))
				ref_usage();
			numsig = strtol(*argv, &ep, 10);
			if (!**argv || *ep)
				errx(2, "invalid signal number: %s", *argv);
			if (numsig >= 128)
				numsig -= 128;
			if (sig2str(numsig, signame) < 0)
				ref_nosig(*argv);
			printf("%s\n", signame);
			return (0);
		}
		ref_printsignals(stdout);
		return (0);
	}

	if (!strcmp(*argv, "-s")) {
		argc--, argv++;
		if (argc < 1) {
			warnx("option requires an argument -- s");
			ref_usage();
		}
		if (strcmp(*argv, "0") == 0)
			numsig = 0;
		else if (str2sig(*argv, &numsig) < 0)
			ref_nosig(*argv);
		argc--, argv++;
	} else if (**argv == '-' && *(*argv + 1) != '-') {
		++*argv;
		if (strcmp(*argv, "0") == 0)
			numsig = 0;
		else if (str2sig(*argv, &numsig) < 0)
			ref_nosig(*argv);
		argc--, argv++;
	}

	if (argc > 0 && strncmp(*argv, "--", 2) == 0)
		argc--, argv++;

	if (argc == 0)
		ref_usage();

	for (errors = 0; argc; argc--, argv++) {
#ifdef SHELL
		if (**argv == '%')
			ret = killjob(*argv, numsig);
		else
#endif
		{
			pidl = strtol(*argv, &ep, 10);
			/* Check for overflow of pid_t. */
			pid = (pid_t)pidl;
			if (!**argv || *ep || pid != pidl)
				errx(2, "illegal process id: %s", *argv);
			ret = kill(pid, numsig);
		}
		if (ret == -1) {
			warn("%s", *argv);
			errors = 1;
		}
	}

	return (errors);
}

void
ref_nosig(const char *name)
{

	warnx("unknown signal %s; valid signals:", name);
	ref_printsignals(stderr);
#ifdef SHELL
	error(NULL);
#else
	exit(2);
#endif
}

void
ref_printsignals(FILE *fp)
{
	int n;

	for (n = 1; n < sys_nsig; n++) {
		(void)fprintf(fp, "%s", sys_signame[n]);
		if (n == (sys_nsig / 2) || n == (sys_nsig - 1))
			(void)fprintf(fp, "\n");
		else
			(void)fprintf(fp, " ");
	}
}

void
ref_usage(void)
{

	(void)fprintf(stderr, "%s\n%s\n%s\n%s\n",
		"usage: kill [-s signal_name] pid ...",
		"       kill -l [exit_status]",
		"       kill -signal_name pid ...",
		"       kill -signal_number pid ...");
#ifdef SHELL
	error(NULL);
#else
	exit(2);
#endif
}
