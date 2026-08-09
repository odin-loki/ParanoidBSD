/*
 * oracle.c -- reference implementation for batch b0225.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.  Prelude shims supply
 * FreeBSD libc_private entry points and headers not present on Linux.
 *
 * Sources:
 *   hbsd/src/lib/libc/compat-43/getwd.c
 *   hbsd/src/lib/libc/compat-43/sigcompat.c
 */

#define _DEFAULT_SOURCE

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#ifdef __GLIBC__
#define __bits __val
#endif

#ifndef SV_INTERRUPT
#define SV_INTERRUPT 0x00000002
#endif

#ifndef SIG_HOLD
#define SIG_HOLD ((void (*)(int))2)
#endif

struct sigvec {
	void (*sv_handler)(int);
	int sv_mask;
	int sv_flags;
};

typedef void (*__sighandler_t)(int);

static void bzero(void *s, size_t n)
{
	memset(s, 0, n);
}

/* ------------------------------------------------------------------ */
/* Mock libc_private layer (harness-controlled, non-blocking)          */
/* ------------------------------------------------------------------ */

static int mock_sigaction_ret;
static int mock_sigprocmask_ret;
static int mock_sigsuspend_ret;
static sigset_t mock_mask;
static struct sigaction mock_actions[_NSIG];
static int mock_actions_inited;

void pbsd_mock_reset(void)
{
	mock_sigaction_ret = 0;
	mock_sigprocmask_ret = 0;
	mock_sigsuspend_ret = 0;
	sigemptyset(&mock_mask);
	mock_actions_inited = 0;
}

void pbsd_mock_set_sigaction_ret(int v)
{
	mock_sigaction_ret = v;
}

void pbsd_mock_set_sigprocmask_ret(int v)
{
	mock_sigprocmask_ret = v;
}

void pbsd_mock_set_sigsuspend_ret(int v)
{
	mock_sigsuspend_ret = v;
}

void pbsd_mock_set_mask_bits(int mask)
{
	sigemptyset(&mock_mask);
	mock_mask.__bits[0] = (unsigned int)mask;
}

int pbsd_mock_get_mask_bits(void)
{
	return (int)mock_mask.__bits[0];
}

void pbsd_mock_set_handler(int sig, void (*h)(int))
{
	if (!mock_actions_inited) {
		for (int i = 0; i < _NSIG; i++) {
			sigemptyset(&mock_actions[i].sa_mask);
			mock_actions[i].sa_handler = SIG_DFL;
			mock_actions[i].sa_flags = 0;
		}
		mock_actions_inited = 1;
	}
	if (sig > 0 && sig < _NSIG)
		mock_actions[sig].sa_handler = h;
}

static void mock_init_actions(void)
{
	if (!mock_actions_inited) {
		for (int i = 0; i < _NSIG; i++) {
			sigemptyset(&mock_actions[i].sa_mask);
			mock_actions[i].sa_handler = SIG_DFL;
			mock_actions[i].sa_flags = 0;
		}
		mock_actions_inited = 1;
	}
}

int __libc_sigaction(int signo, const struct sigaction *act,
    struct sigaction *oact)
{
	mock_init_actions();
	if (mock_sigaction_ret != 0) {
		if (mock_sigaction_ret == -1)
			errno = EINVAL;
		return (mock_sigaction_ret);
	}
	if (signo <= 0 || signo >= _NSIG) {
		errno = EINVAL;
		return (-1);
	}
	if (oact != NULL)
		*oact = mock_actions[signo];
	if (act != NULL)
		mock_actions[signo] = *act;
	return (0);
}

int __libc_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	if (mock_sigprocmask_ret != 0) {
		if (mock_sigprocmask_ret == -1)
			errno = EINVAL;
		return (mock_sigprocmask_ret);
	}
	if (oset != NULL)
		*oset = mock_mask;
	if (set == NULL)
		return (0);
	switch (how) {
	case SIG_SETMASK:
		mock_mask = *set;
		break;
	case SIG_BLOCK: {
		size_t words = sizeof(mock_mask) / sizeof(mock_mask.__bits[0]);
		size_t i;

		for (i = 0; i < words; i++)
			mock_mask.__bits[i] |= set->__bits[i];
		break;
	}
	case SIG_UNBLOCK: {
		size_t words = sizeof(mock_mask) / sizeof(mock_mask.__bits[0]);
		size_t i;

		for (i = 0; i < words; i++)
			mock_mask.__bits[i] &= ~set->__bits[i];
		break;
	}
	default:
		errno = EINVAL;
		return (-1);
	}
	return (0);
}

int __libc_sigsuspend(const sigset_t *set)
{
	(void)set;
	if (mock_sigsuspend_ret != 0) {
		if (mock_sigsuspend_ret == -1)
			errno = EINTR;
		return (mock_sigsuspend_ret);
	}
	return (0);
}

/* ==================== lib/libc/compat-43/getwd.c ==================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
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

char *
ref_getwd(char *buf)
{
	char *p;

	if ( (p = getcwd(buf, MAXPATHLEN)) )
		return(p);
	(void)strerror_r(errno, buf, MAXPATHLEN);
	return((char *)NULL);
}

/* ==================== lib/libc/compat-43/sigcompat.c ==================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
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
ref_sigvec(int signo, struct sigvec *sv, struct sigvec *osv)
{
	struct sigaction sa, osa;
	struct sigaction *sap, *osap;
	int ret;

	if (sv != NULL) {
		sa.sa_handler = sv->sv_handler;
		sa.sa_flags = sv->sv_flags ^ SV_INTERRUPT;
		sigemptyset(&sa.sa_mask);
		sa.sa_mask.__bits[0] = sv->sv_mask;
		sap = &sa;
	} else
		sap = NULL;
	osap = osv != NULL ? &osa : NULL;
	ret = __libc_sigaction(signo, sap, osap);
	if (ret == 0 && osv != NULL) {
		osv->sv_handler = osa.sa_handler;
		osv->sv_flags = osa.sa_flags ^ SV_INTERRUPT;
		osv->sv_mask = osa.sa_mask.__bits[0];
	}
	return (ret);
}

int
ref_sigsetmask(int mask)
{
	sigset_t set, oset;
	int n;

	sigemptyset(&set);
	set.__bits[0] = mask;
	n = __libc_sigprocmask(SIG_SETMASK, &set, &oset);
	if (n)
		return (n);
	return (oset.__bits[0]);
}

int
ref_sigblock(int mask)
{
	sigset_t set, oset;
	int n;

	sigemptyset(&set);
	set.__bits[0] = mask;
	n = __libc_sigprocmask(SIG_BLOCK, &set, &oset);
	if (n)
		return (n);
	return (oset.__bits[0]);
}

int
ref_sigpause(int mask)
{
	sigset_t set;

	sigemptyset(&set);
	set.__bits[0] = mask;
	return (__libc_sigsuspend(&set));
}

int
ref_xsi_sigpause(int sig)
{
	sigset_t set;

	if (__libc_sigprocmask(SIG_BLOCK, NULL, &set) == -1)
		return (-1);
	if (sigdelset(&set, sig) == -1)
		return (-1);
	return (__libc_sigsuspend(&set));
}

int
ref_sighold(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	if (sigaddset(&set, sig) == -1)
		return (-1);
	return (__libc_sigprocmask(SIG_BLOCK, &set, NULL));
}

int
ref_sigignore(int sig)
{
	struct sigaction sa;

	bzero(&sa, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	return (__libc_sigaction(sig, &sa, NULL));
}

int
ref_sigrelse(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	if (sigaddset(&set, sig) == -1)
		return (-1);
	return (__libc_sigprocmask(SIG_UNBLOCK, &set, NULL));
}

void
(*ref_sigset(int sig, void (*disp)(int)))(int)
{
	sigset_t set, pset;
	struct sigaction sa, psa;

	sigemptyset(&set);
	if (sigaddset(&set, sig) == -1)
		return (SIG_ERR);
	if (__libc_sigprocmask(SIG_BLOCK, NULL, &pset) == -1)
		return (SIG_ERR);
	if ((__sighandler_t *)disp == SIG_HOLD) {
		if (__libc_sigprocmask(SIG_BLOCK, &set, &pset) == -1)
			return (SIG_ERR);
		if (sigismember(&pset, sig))
			return (SIG_HOLD);
		else {
			if (__libc_sigaction(sig, NULL, &psa) == -1)
				return (SIG_ERR);
			return (psa.sa_handler);
		}
	} else {
		if (__libc_sigprocmask(SIG_UNBLOCK, &set, &pset) == -1)
			return (SIG_ERR);
	}

	bzero(&sa, sizeof(sa));
	sa.sa_handler = disp;
	if (__libc_sigaction(sig, &sa, &psa) == -1)
		return (SIG_ERR);
	if (sigismember(&pset, sig))
		return (SIG_HOLD);
	else
		return (psa.sa_handler);
}
