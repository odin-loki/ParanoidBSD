/*
 * oracle.c -- reference implementation for batch b0284.
 *
 * Original C sources, concatenated, with every function renamed with a
 * ref_ prefix.  Function bodies are UNMODIFIED.  Only defines/declarations
 * that the unavailable FreeBSD/HardenedBSD headers used to supply have been
 * added.
 *
 * Sources included:
 *	hbsd/src/lib/libc/gen/siglist.c
 *	hbsd/src/lib/libc/gen/sig2str.c
 *	hbsd/src/lib/libc/gen/__xuname.c
 *
 * Sources omitted (see skipped.txt):
 *	hbsd/src/lib/libc/gen/readdir-compat11.c
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define	__ssp_real(x)	x

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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#ifndef strlcpy
static size_t
strlcpy(char *dst, const char *src, size_t len)
{
	size_t srclen;

	srclen = strlen(src);
	if (len != 0) {
		if (srclen >= len) {
			memcpy(dst, src, len - 1);
			dst[len - 1] = '\0';
		} else
			memcpy(dst, src, srclen + 1);
	}
	return (srclen);
}
#endif

/* ------------------------------------------------------------------ */
/* Harness-controlled mocks                                           */
/* ------------------------------------------------------------------ */

#define	HARNESS_ENV_MAX	256
#define	HARNESS_SYSCTL_MAX	512

static char mock_env_s[HARNESS_ENV_MAX];
static char mock_env_r[HARNESS_ENV_MAX];
static char mock_env_v[HARNESS_ENV_MAX];
static char mock_env_m[HARNESS_ENV_MAX];
static int mock_env_s_set;
static int mock_env_r_set;
static int mock_env_v_set;
static int mock_env_m_set;

static char mock_sys_ostype[HARNESS_SYSCTL_MAX];
static char mock_sys_hostname[HARNESS_SYSCTL_MAX];
static char mock_sys_osrelease[HARNESS_SYSCTL_MAX];
static char mock_sys_version[HARNESS_SYSCTL_MAX];
static char mock_sys_machine[HARNESS_SYSCTL_MAX];
static size_t mock_sys_ostype_len;
static size_t mock_sys_hostname_len;
static size_t mock_sys_osrelease_len;
static size_t mock_sys_version_len;
static size_t mock_sys_machine_len;

static int mock_sysctl_fail;
static int mock_sysctl_errno;
static int mock_sysctl_enomem;

void
harness_reset_mocks(void)
{
	mock_env_s_set = 0;
	mock_env_r_set = 0;
	mock_env_v_set = 0;
	mock_env_m_set = 0;
	mock_env_s[0] = '\0';
	mock_env_r[0] = '\0';
	mock_env_v[0] = '\0';
	mock_env_m[0] = '\0';

	mock_sys_ostype[0] = '\0';
	mock_sys_hostname[0] = '\0';
	mock_sys_osrelease[0] = '\0';
	mock_sys_version[0] = '\0';
	mock_sys_machine[0] = '\0';
	mock_sys_ostype_len = 0;
	mock_sys_hostname_len = 0;
	mock_sys_osrelease_len = 0;
	mock_sys_version_len = 0;
	mock_sys_machine_len = 0;

	mock_sysctl_fail = 0;
	mock_sysctl_errno = 0;
	mock_sysctl_enomem = 0;
}

void
harness_set_env_s(const char *v)
{
	if (v == NULL) {
		mock_env_s_set = 0;
		mock_env_s[0] = '\0';
	} else {
		mock_env_s_set = 1;
		strlcpy(mock_env_s, v, sizeof(mock_env_s));
	}
}

void
harness_set_env_r(const char *v)
{
	if (v == NULL) {
		mock_env_r_set = 0;
		mock_env_r[0] = '\0';
	} else {
		mock_env_r_set = 1;
		strlcpy(mock_env_r, v, sizeof(mock_env_r));
	}
}

void
harness_set_env_v(const char *v)
{
	if (v == NULL) {
		mock_env_v_set = 0;
		mock_env_v[0] = '\0';
	} else {
		mock_env_v_set = 1;
		strlcpy(mock_env_v, v, sizeof(mock_env_v));
	}
}

void
harness_set_env_m(const char *v)
{
	if (v == NULL) {
		mock_env_m_set = 0;
		mock_env_m[0] = '\0';
	} else {
		mock_env_m_set = 1;
		strlcpy(mock_env_m, v, sizeof(mock_env_m));
	}
}

void
harness_set_sys_ostype(const char *v)
{
	if (v == NULL) {
		mock_sys_ostype[0] = '\0';
		mock_sys_ostype_len = 0;
	} else {
		mock_sys_ostype_len = strlcpy(mock_sys_ostype, v,
		    sizeof(mock_sys_ostype));
	}
}

void
harness_set_sys_hostname(const char *v)
{
	if (v == NULL) {
		mock_sys_hostname[0] = '\0';
		mock_sys_hostname_len = 0;
	} else {
		mock_sys_hostname_len = strlcpy(mock_sys_hostname, v,
		    sizeof(mock_sys_hostname));
	}
}

void
harness_set_sys_osrelease(const char *v)
{
	if (v == NULL) {
		mock_sys_osrelease[0] = '\0';
		mock_sys_osrelease_len = 0;
	} else {
		mock_sys_osrelease_len = strlcpy(mock_sys_osrelease, v,
		    sizeof(mock_sys_osrelease));
	}
}

void
harness_set_sys_version(const char *v)
{
	if (v == NULL) {
		mock_sys_version[0] = '\0';
		mock_sys_version_len = 0;
	} else {
		mock_sys_version_len = strlcpy(mock_sys_version, v,
		    sizeof(mock_sys_version));
	}
}

void
harness_set_sys_machine(const char *v)
{
	if (v == NULL) {
		mock_sys_machine[0] = '\0';
		mock_sys_machine_len = 0;
	} else {
		mock_sys_machine_len = strlcpy(mock_sys_machine, v,
		    sizeof(mock_sys_machine));
	}
}

void
harness_set_sysctl_fail(int fail, int err)
{
	mock_sysctl_fail = fail;
	mock_sysctl_errno = err;
}

void
harness_set_sysctl_enomem(int enomem)
{
	mock_sysctl_enomem = enomem;
}

char *
getenv(const char *name)
{
	if (strcmp(name, "UNAME_s") == 0) {
		if (mock_env_s_set == 0)
			return (NULL);
		return (mock_env_s);
	}
	if (strcmp(name, "UNAME_r") == 0) {
		if (mock_env_r_set == 0)
			return (NULL);
		return (mock_env_r);
	}
	if (strcmp(name, "UNAME_v") == 0) {
		if (mock_env_v_set == 0)
			return (NULL);
		return (mock_env_v);
	}
	if (strcmp(name, "UNAME_m") == 0) {
		if (mock_env_m_set == 0)
			return (NULL);
		return (mock_env_m);
	}
	return (NULL);
}

static size_t
mock_copy_out(const char *src, size_t srclen, void *oldp, size_t *oldlenp)
{
	size_t want = *oldlenp;
	size_t outlen = srclen + 1;

	if (want < outlen) {
		errno = ENOMEM;
		if (want > 0)
			memcpy(oldp, src, want);
		*oldlenp = outlen;
		return (0);
	}
	memcpy(oldp, src, outlen);
	*oldlenp = outlen;
	return (outlen);
}

int
sysctl(int *name, unsigned int namelen, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen)
{
	const char *src;
	size_t srclen;

	(void)newp;
	(void)newlen;

	if (mock_sysctl_fail) {
		errno = mock_sysctl_errno;
		return (-1);
	}
	if (namelen != 2)
		return (-1);

	if (mock_sysctl_enomem) {
		errno = ENOMEM;
		return (-1);
	}

	switch (name[0]) {
	case CTL_KERN:
		switch (name[1]) {
		case KERN_OSTYPE:
			src = mock_sys_ostype;
			srclen = mock_sys_ostype_len;
			break;
		case KERN_HOSTNAME:
			src = mock_sys_hostname;
			srclen = mock_sys_hostname_len;
			break;
		case KERN_OSRELEASE:
			src = mock_sys_osrelease;
			srclen = mock_sys_osrelease_len;
			break;
		case KERN_VERSION:
			src = mock_sys_version;
			srclen = mock_sys_version_len;
			break;
		default:
			errno = ENOENT;
			return (-1);
		}
		break;
	case CTL_HW:
		if (name[1] != HW_MACHINE) {
			errno = ENOENT;
			return (-1);
		}
		src = mock_sys_machine;
		srclen = mock_sys_machine_len;
		break;
	default:
		errno = ENOENT;
		return (-1);
	}

	if (oldp == NULL || oldlenp == NULL)
		return (0);
	(void)mock_copy_out(src, srclen, oldp, oldlenp);
	return (0);
}

/* ------------------------------------------------------------------ */
/* strtonum (dependency of str2sig, not part of batch)                */
/* ------------------------------------------------------------------ */

long long
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
		ll = strtoll(numstr, &ep, 10);
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

/* ------------------------------------------------------------------ */
/* siglist.c                                                          */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */
/* sig2str.c                                                          */
/* ------------------------------------------------------------------ */

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
ref_sig2str(int signum, char *str)
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
ref_str2sig(const char * restrict str, int * restrict pnum)
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

/* ------------------------------------------------------------------ */
/* __xuname.c                                                         */
/* ------------------------------------------------------------------ */

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
ref___xuname(int namesize, void *namebuf)
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
