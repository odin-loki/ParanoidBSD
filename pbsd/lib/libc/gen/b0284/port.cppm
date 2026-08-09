module;

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <strings.h>

export module pbsd.lib.libc.gen.b0284;

#ifndef restrict
#define	restrict	__restrict__
#endif

export namespace pbsd::lib_libc_gen::b0284 {

#ifndef SIG2STR_MAX
#define	SIG2STR_MAX	32
#endif

#ifndef NSIG
#define	NSIG		32
#endif

#define	SIGHUP		1
#define	SIGINT		2
#define	SIGQUIT		3
#define	SIGILL		4
#define	SIGTRAP		5
#define	SIGABRT		6
#define	SIGEMT		7
#define	SIGFPE		8
#define	SIGKILL		9
#define	SIGBUS		10
#define	SIGSEGV		11
#define	SIGSYS		12
#define	SIGPIPE		13
#define	SIGALRM		14
#define	SIGTERM		15
#define	SIGURG		16
#define	SIGSTOP		17
#define	SIGTSTP		18
#define	SIGCONT		19
#define	SIGCHLD		20
#define	SIGTTIN		21
#define	SIGTTOU		22
#define	SIGIO		23
#define	SIGXCPU		24
#define	SIGXFSZ		25
#define	SIGVTALRM	26
#define	SIGPROF		27
#define	SIGWINCH	28
#define	SIGINFO		29
#define	SIGUSR1		30
#define	SIGUSR2		31

#define	SIGRTMIN	65
#define	SIGRTMAX	126

#define	CTL_KERN	1
#define	KERN_OSTYPE	1
#define	KERN_HOSTNAME	2
#define	KERN_OSRELEASE	3
#define	KERN_VERSION	4
#define	CTL_HW		6
#define	HW_MACHINE	1

static size_t
strlcpy(char *dst, const char *src, size_t len)
{
	size_t srclen;

	srclen = std::strlen(src);
	if (len != 0) {
		if (srclen >= len) {
			std::memcpy(dst, src, len - 1);
			dst[len - 1] = '\0';
		} else
			std::memcpy(dst, src, srclen + 1);
	}
	return (srclen);
}

static long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp)
{
	long long ll = 0;
	int error = 0;
	char *ep;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval) {
		error = 1;
	} else {
		ll = std::strtoll(numstr, &ep, 10);
		if (errno == EINVAL || numstr == ep || *ep != '\0')
			error = 1;
		else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
			error = 2;
		else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
			error = 3;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}

extern "C" {
char *getenv(const char *);
int sysctl(int *name, unsigned int namelen, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
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

const char *const sys_signame[NSIG] = {
	[0] =		"Signal 0",
	[SIGHUP] =	"HUP",
	[SIGINT] =	"INT",
	[SIGQUIT] =	"QUIT",
	[SIGILL] =	"ILL",
	[SIGTRAP] =	"TRAP",
	[SIGABRT] =	"ABRT",
	[SIGEMT] =	"EMT",
	[SIGFPE] =	"FPE",
	[SIGKILL] =	"KILL",
	[SIGBUS] =	"BUS",
	[SIGSEGV] =	"SEGV",
	[SIGSYS] =	"SYS",
	[SIGPIPE] =	"PIPE",
	[SIGALRM] =	"ALRM",
	[SIGTERM] =	"TERM",
	[SIGURG] =	"URG",
	[SIGSTOP] =	"STOP",
	[SIGTSTP] =	"TSTP",
	[SIGCONT] =	"CONT",
	[SIGCHLD] =	"CHLD",
	[SIGTTIN] =	"TTIN",
	[SIGTTOU] =	"TTOU",
	[SIGIO] =	"IO",
	[SIGXCPU] =	"XCPU",
	[SIGXFSZ] =	"XFSZ",
	[SIGVTALRM] =	"VTALRM",
	[SIGPROF] =	"PROF",
	[SIGWINCH] =	"WINCH",
	[SIGINFO] =	"INFO",
	[SIGUSR1] =	"USR1",
	[SIGUSR2] =	"USR2",
};

const char *const sys_siglist[NSIG] = {
	[0] =		"Signal 0",
	[SIGHUP] =	"Hangup",
	[SIGINT] =	"Interrupt",
	[SIGQUIT] =	"Quit",
	[SIGILL] =	"Illegal instruction",
	[SIGTRAP] =	"Trace/BPT trap",
	[SIGABRT] =	"Abort trap",
	[SIGEMT] =	"EMT trap",
	[SIGFPE] =	"Floating point exception",
	[SIGKILL] =	"Killed",
	[SIGBUS] =	"Bus error",
	[SIGSEGV] =	"Segmentation fault",
	[SIGSYS] =	"Bad system call",
	[SIGPIPE] =	"Broken pipe",
	[SIGALRM] =	"Alarm clock",
	[SIGTERM] =	"Terminated",
	[SIGURG] =	"Urgent I/O condition",
	[SIGSTOP] =	"Suspended (signal)",
	[SIGTSTP] =	"Suspended",
	[SIGCONT] =	"Continued",
	[SIGCHLD] =	"Child exited",
	[SIGTTIN] =	"Stopped (tty input)",
	[SIGTTOU] =	"Stopped (tty output)",
	[SIGIO] =	"I/O possible",
	[SIGXCPU] =	"Cputime limit exceeded",
	[SIGXFSZ] =	"Filesize limit exceeded",
	[SIGVTALRM] =	"Virtual timer expired",
	[SIGPROF] =	"Profiling timer expired",
	[SIGWINCH] =	"Window size changes",
	[SIGINFO] =	"Information request",
	[SIGUSR1] =	"User defined signal 1",
	[SIGUSR2] =	"User defined signal 2",
};
const int sys_nsig = sizeof(sys_siglist) / sizeof(sys_siglist[0]);

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Ricardo Branco <rbranco@suse.de>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static const char rtmin_str[] = "RTMIN";
static const char rtmax_str[] = "RTMAX";

int
sig2str(int signum, char *str)
{
	if (signum <= 0 || signum > SIGRTMAX)
		return (-1);

	if (signum < sys_nsig)
		(void)strlcpy(str, sys_signame[signum], SIG2STR_MAX);
	else if (signum < SIGRTMIN)
		(void)snprintf(str, SIG2STR_MAX, "%d", signum);
	else if (signum == SIGRTMIN)
		(void)strlcpy(str, rtmin_str, SIG2STR_MAX);
	else if (signum == SIGRTMAX)
		(void)strlcpy(str, rtmax_str, SIG2STR_MAX);
	else if (signum <= (SIGRTMIN + SIGRTMAX) / 2)
		(void)snprintf(str, SIG2STR_MAX, "%s+%d",
		    rtmin_str, signum - SIGRTMIN);
	else
		(void)snprintf(str, SIG2STR_MAX, "%s-%d",
		    rtmax_str, SIGRTMAX - signum);

	return (0);
}

int
str2sig(const char * restrict str, int * restrict pnum)
{
	const char *errstr;
	long long n;
	int sig;
	int rtend = sizeof(rtmin_str) - 1;

	if (strncasecmp(str, "SIG", 3) == 0)
		str += 3;

	if (strncasecmp(str, rtmin_str, sizeof(rtmin_str) - 1) == 0 ||
	    strncasecmp(str, rtmax_str, sizeof(rtmin_str) - 1) == 0) {
		sig = (toupper(str[4]) == 'X') ? SIGRTMAX : SIGRTMIN;
		n = 0;
		if (str[rtend] == '+' || str[rtend] == '-') {
			n = strtonum(str + rtend, INT_MIN, INT_MAX, &errstr);
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
		n = strtonum(str, 1, SIGRTMAX, &errstr);
		if (errstr == NULL) {
			*pnum = (int)n;
			return (0);
		}
	}

	for (sig = 1; sig < sys_nsig; sig++) {
		if (strcasecmp(sys_signame[sig], str) == 0) {
			*pnum = sig;
			return (0);
		}
	}

	return (-1);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1994
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

int
__xuname(int namesize, void *namebuf)
{
	int mib[2], rval;
	size_t len;
	char *p, *q;
	int oerrno;

	rval = 0;
	q = (char *)namebuf;

	mib[0] = CTL_KERN;

	if ((p = getenv("UNAME_s")))
		strlcpy(q, p, namesize);
	else {
		mib[1] = KERN_OSTYPE;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
	}
	q += namesize;

	mib[1] = KERN_HOSTNAME;
	len = namesize;
	oerrno = errno;
	if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
		if (errno == ENOMEM)
			errno = oerrno;
		else
			rval = -1;
	}
	q[namesize - 1] = '\0';
	q += namesize;

	if ((p = getenv("UNAME_r")))
		strlcpy(q, p, namesize);
	else {
		mib[1] = KERN_OSRELEASE;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
	}
	q += namesize;

	if ((p = getenv("UNAME_v")))
		strlcpy(q, p, namesize);
	else {

		/*
		 * The version may have newlines in it, turn them into
		 * spaces.
		 */
		mib[1] = KERN_VERSION;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
		for (p = q; len--; ++p) {
			if (*p == '\n' || *p == '\t') {
				if (len > 1)
					*p = ' ';
				else
					*p = '\0';
			}
		}
	}
	q += namesize;

	if ((p = getenv("UNAME_m")))
		strlcpy(q, p, namesize);
	else {
		mib[0] = CTL_HW;
		mib[1] = HW_MACHINE;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
	}

	return (rval);
}

} /* namespace */
