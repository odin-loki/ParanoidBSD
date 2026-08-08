/*
 * PBSD batch b0146s1 -- reference oracle.
 *
 * Source: hbsd/src/sys/kern/sys_getrandom.c
 *
 * Every function is renamed with a "ref_" prefix.  Function bodies are
 * otherwise UNMODIFIED.  The kernel environment (types, constants, and
 * read_random_uio(9)) is modelled below and shared identically with the
 * C++23 port under test.
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#define	__unused		__attribute__((__unused__))
#define	CTASSERT(x)		_Static_assert((x), "compile-time assertion failed")

typedef long		register_t;

enum uio_rw { UIO_READ, UIO_WRITE };
enum uio_seg { UIO_USERSPACE, UIO_SYSSPACE, UIO_NOCOPY };

struct iovec {
	void	*iov_base;
	size_t	 iov_len;
};

struct thread {
	register_t	td_retval[2];
};

struct uio {
	struct iovec	*uio_iov;
	int		 uio_iovcnt;
	long long	 uio_offset;
	long		 uio_resid;
	enum uio_seg	 uio_segflg;
	enum uio_rw	 uio_rw;
	struct thread	*uio_td;
};

#define	IOSIZE_MAX	INT_MAX

#define	GRND_NONBLOCK	0x0001
#define	GRND_RANDOM	0x0002
#define	GRND_INSECURE	0x0004

/* Harness-controlled read_random_uio(9) behaviour. */
int	oracle_random_error;
int	oracle_random_nb_fail;
unsigned long	oracle_random_cap = ULONG_MAX;

void
oracle_read_random_reset(void)
{

	oracle_random_error = 0;
	oracle_random_nb_fail = 0;
	oracle_random_cap = ULONG_MAX;
}

void
oracle_read_random_configure(int error, int block, ssize_t transfer)
{

	oracle_random_error = error;
	oracle_random_nb_fail = block;
	if (transfer < 0)
		oracle_random_cap = ULONG_MAX;
	else
		oracle_random_cap = (unsigned long)transfer;
}

static int
read_random_uio(struct uio *auio, int nonblock)
{
	unsigned char *bp;
	unsigned long n, i;

	if (oracle_random_error != 0)
		return (oracle_random_error);
	if (nonblock != 0 && oracle_random_nb_fail != 0)
		return (EWOULDBLOCK);
	if (auio->uio_resid <= 0)
		return (0);
	n = (unsigned long)auio->uio_resid;
	if (n > oracle_random_cap)
		n = oracle_random_cap;
	if (n > (unsigned long)auio->uio_iov[0].iov_len)
		n = (unsigned long)auio->uio_iov[0].iov_len;
	bp = (unsigned char *)auio->uio_iov[0].iov_base;
	for (i = 0; i < n; i++)
		bp[i] = (unsigned char)(0xa5 ^ (i & 0xff));
	auio->uio_resid -= (long)n;
	return (0);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Conrad Meyer <cem@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define GRND_VALIDFLAGS	(GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

/*
 * read_random_uio(9) returns EWOULDBLOCK if a nonblocking request would block,
 * but the Linux API name is EAGAIN.  On FreeBSD, they have the same numeric
 * value for now.
 */
CTASSERT(EWOULDBLOCK == EAGAIN);

int
ref_kern_getrandom(struct thread *td, void *user_buf, size_t buflen,
    unsigned int flags)
{
	struct uio auio;
	struct iovec aiov;
	int error;

	if ((flags & ~GRND_VALIDFLAGS) != 0)
		return (EINVAL);
	if (buflen > IOSIZE_MAX)
		return (EINVAL);

	/*
	 * Linux compatibility: We have two choices for handling Linux's
	 * GRND_INSECURE.
	 *
	 * 1. We could ignore it completely (like GRND_RANDOM).  However, this
	 * might produce the surprising result of GRND_INSECURE requests
	 * blocking, when the Linux API does not block.
	 *
	 * 2. Alternatively, we could treat GRND_INSECURE requests as requests
	 * for GRND_NONBLOCK.  Here, the surprising result for Linux programs
	 * is that invocations with unseeded random(4) will produce EAGAIN,
	 * rather than garbage.
	 *
	 * Honoring the flag in the way Linux does seems fraught.  If we
	 * actually use the output of a random(4) implementation prior to
	 * seeding, we leak some entropy about the initial seed to attackers.
	 * This seems unacceptable -- it defeats the purpose of blocking on
	 * initial seeding.
	 *
	 * Secondary to that concern, before seeding we may have arbitrarily
	 * little entropy collected; producing output from zero or a handful of
	 * entropy bits does not seem particularly useful to userspace.
	 *
	 * If userspace can accept garbage, insecure non-random bytes, they can
	 * create their own insecure garbage with srandom(time(NULL)) or
	 * similar.  Asking the kernel to produce it from the secure
	 * getrandom(2) API seems inane.
	 *
	 * We elect to emulate GRND_INSECURE as an alternative spelling of
	 * GRND_NONBLOCK (2).
	 */
	if ((flags & GRND_INSECURE) != 0)
		flags |= GRND_NONBLOCK;

	if (buflen == 0) {
		td->td_retval[0] = 0;
		return (0);
	}

	aiov.iov_base = user_buf;
	aiov.iov_len = buflen;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_resid = buflen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_td = td;

	error = read_random_uio(&auio, (flags & GRND_NONBLOCK) != 0);
	if (error == 0)
		td->td_retval[0] = buflen - auio.uio_resid;
	return (error);
}

#ifndef _SYS_SYSPROTO_H_
struct getrandom_args {
	void		*buf;
	size_t		buflen;
	unsigned int	flags;
};
#endif

int
ref_sys_getrandom(struct thread *td, struct getrandom_args *uap)
{
	return (ref_kern_getrandom(td, uap->buf, uap->buflen, uap->flags));
}
