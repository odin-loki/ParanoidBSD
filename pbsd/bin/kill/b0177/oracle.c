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
 * oracle.c -- reference specification for PBSD batch b0177 (kill.c).
 *
 * kill.c is renamed with a ref_ prefix; bodies are otherwise UNMODIFIED.
 * Supporting defines, globals, and shims are added only where required so the
 * unmodified bodies compile and link.
 */

#define _DEFAULT_SOURCE

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __dead2
#define __dead2	__attribute__((__noreturn__))
#endif

#ifndef SIG2STR_MAX
#define SIG2STR_MAX	32
#endif

#ifndef NSIG
#define NSIG	65
#endif

static size_t
kill_shim_strlcpy(char *dst, const char *src, size_t siz)
{
	size_t len;

	len = strlen(src);
	if (siz != 0) {
		if (len >= siz)
			len = siz - 1;
		memcpy(dst, src, len);
		dst[len] = '\0';
	}
	return (strlen(src));
}

static long long
kill_shim_strtonum(const char *nptr, long long minval, long long maxval,
    const char **err)
{
	char *end;
	long long val;

	*err = NULL;
	errno = 0;
	val = strtoll(nptr, &end, 10);
	if (nptr[0] == '\0' || *end != '\0') {
		*err = "invalid";
		return (0);
	}
	if (errno == ERANGE || val < minval || val > maxval) {
		*err = "out of range";
		return (0);
	}
	return (val);
}

static const char *const sys_signame[NSIG] = {
	[0] =		"Signal 0",
	[1] =		"HUP",
	[2] =		"INT",
	[3] =		"QUIT",
	[4] =		"ILL",
	[5] =		"TRAP",
	[6] =		"ABRT",
	[7] =		"BUS",
	[8] =		"FPE",
	[9] =		"KILL",
	[10] =		"USR1",
	[11] =		"SEGV",
	[12] =		"USR2",
	[13] =		"PIPE",
	[14] =		"ALRM",
	[15] =		"TERM",
	[16] =		"STKFLT",
	[17] =		"CHLD",
	[18] =		"CONT",
	[19] =		"STOP",
	[20] =		"TSTP",
	[21] =		"TTIN",
	[22] =		"TTOU",
	[23] =		"URG",
	[24] =		"XCPU",
	[25] =		"XFSZ",
	[26] =		"VTALRM",
	[27] =		"PROF",
	[28] =		"WINCH",
	[29] =		"IO",
	[30] =		"INFO",
	[31] =		"SYS",
	[32] =		"",
	[33] =		"",
	[34] =		"",
	[35] =		"",
	[36] =		"",
	[37] =		"",
	[38] =		"",
	[39] =		"",
	[40] =		"",
	[41] =		"",
	[42] =		"",
	[43] =		"",
	[44] =		"",
	[45] =		"",
	[46] =		"",
	[47] =		"",
	[48] =		"",
	[49] =		"",
	[50] =		"",
	[51] =		"",
	[52] =		"",
	[53] =		"",
	[54] =		"",
	[55] =		"",
	[56] =		"",
	[57] =		"",
	[58] =		"",
	[59] =		"",
	[60] =		"",
	[61] =		"",
	[62] =		"",
	[63] =		"",
	[64] =		"",
};

const int sys_nsig = NSIG;

static const char kill_rtmin_str[] = "RTMIN";
static const char kill_rtmax_str[] = "RTMAX";

int
sig2str(int signum, char *str)
{
	if (signum <= 0 || signum > SIGRTMAX)
		return (-1);

	if (signum < sys_nsig)
		(void)kill_shim_strlcpy(str, sys_signame[signum], SIG2STR_MAX);
	else if (signum < SIGRTMIN)
		(void)snprintf(str, SIG2STR_MAX, "%d", signum);
	else if (signum == SIGRTMIN)
		(void)kill_shim_strlcpy(str, kill_rtmin_str, SIG2STR_MAX);
	else if (signum == SIGRTMAX)
		(void)kill_shim_strlcpy(str, kill_rtmax_str, SIG2STR_MAX);
	else if (signum <= (SIGRTMIN + SIGRTMAX) / 2)
		(void)snprintf(str, SIG2STR_MAX, "%s+%d",
		    kill_rtmin_str, signum - SIGRTMIN);
	else
		(void)snprintf(str, SIG2STR_MAX, "%s-%d",
		    kill_rtmax_str, SIGRTMAX - signum);

	return (0);
}

int
str2sig(const char *str, int *pnum)
{
	const char *errstr;
	long long n;
	int sig;
	int rtend = sizeof(kill_rtmin_str) - 1;

	if (strncasecmp(str, "SIG", 3) == 0)
		str += 3;

	if (strncasecmp(str, kill_rtmin_str, sizeof(kill_rtmin_str) - 1) == 0 ||
	    strncasecmp(str, kill_rtmax_str, sizeof(kill_rtmax_str) - 1) == 0) {
		sig = (toupper((unsigned char)str[4]) == 'X') ? SIGRTMAX : SIGRTMIN;
		n = 0;
		if (str[rtend] == '+' || str[rtend] == '-') {
			n = kill_shim_strtonum(str + rtend, INT_MIN, INT_MAX,
			    &errstr);
			if (n == 0 || errstr != NULL)
				return (-1);
		} else if (str[rtend] != '\0') {
			return (-1);
		}
		sig += (int)n;
		if (sig < SIGRTMIN || sig > SIGRTMAX)
			return (-1);
		*pnum = sig;
		return (0);
	}

	if (isdigit((unsigned char)str[0])) {
		n = kill_shim_strtonum(str, 1, SIGRTMAX, &errstr);
		if (errstr == NULL) {
			*pnum = (int)n;
			return (0);
		}
	}

	for (sig = 1; sig < sys_nsig; sig++) {
		if (sys_signame[sig] != NULL &&
		    strcasecmp(sys_signame[sig], str) == 0) {
			*pnum = sig;
			return (0);
		}
	}

	return (-1);
}

#define nosig		ref_nosig
#define printsignals	ref_printsignals
#define usage		ref_usage

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
		usage();

	numsig = SIGTERM;

	argc--, argv++;
	if (!strcmp(*argv, "-l")) {
		argc--, argv++;
		if (argc > 1)
			usage();
		if (argc == 1) {
			if (!isdigit((unsigned char)**argv))
				usage();
			numsig = strtol(*argv, &ep, 10);
			if (!**argv || *ep)
				errx(2, "invalid signal number: %s", *argv);
			if (numsig >= 128)
				numsig -= 128;
			if (sig2str(numsig, signame) < 0)
				nosig(*argv);
			printf("%s\n", signame);
			return (0);
		}
		printsignals(stdout);
		return (0);
	}

	if (!strcmp(*argv, "-s")) {
		argc--, argv++;
		if (argc < 1) {
			warnx("option requires an argument -- s");
			usage();
		}
		if (strcmp(*argv, "0") == 0)
			numsig = 0;
		else if (str2sig(*argv, &numsig) < 0)
			nosig(*argv);
		argc--, argv++;
	} else if (**argv == '-' && *(*argv + 1) != '-') {
		++*argv;
		if (strcmp(*argv, "0") == 0)
			numsig = 0;
		else if (str2sig(*argv, &numsig) < 0)
			nosig(*argv);
		argc--, argv++;
	}

	if (argc > 0 && strncmp(*argv, "--", 2) == 0)
		argc--, argv++;

	if (argc == 0)
		usage();

	for (errors = 0; argc; argc--, argv++) {
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
	printsignals(stderr);
	exit(2);
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
	exit(2);
}
