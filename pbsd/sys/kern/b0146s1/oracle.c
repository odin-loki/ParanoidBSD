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

/*
 * PBSD batch b0146s1 oracle -- sys/kern/sys_getrandom.c.
 *
 * Every function of the original translation unit appears below with a ref_
 * prefix; the function bodies are verbatim.  `kern_getrandom' loses its
 * `static' storage class so the differential harness can reach it (a
 * declaration change only; the body is untouched).
 *
 * The kernel environment that the original obtains from <sys/param.h>,
 * <sys/errno.h>, <sys/limits.h>, <sys/proc.h>, <sys/random.h>,
 * <sys/sysproto.h>, <sys/systm.h> and <sys/uio.h> is reproduced here so the
 * code compiles and runs in userspace.  read_random_uio(9) is a deterministic
 * stand-in driven by the rr_* globals; the port links against this very same
 * function, so it is a shared part of the environment rather than part of
 * either implementation.
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

/* <sys/systm.h> */
#define	CTASSERT(x)	_Static_assert(x, #x)

/* <sys/errno.h>: EWOULDBLOCK is a synonym of EAGAIN on FreeBSD too. */
#ifndef EWOULDBLOCK
#define	EWOULDBLOCK	EAGAIN
#endif

/* <sys/limits.h> */
#define	IOSIZE_MAX	INT_MAX

/* <sys/random.h> */
#define	GRND_NONBLOCK	0x0001
#define	GRND_RANDOM	0x0002
#define	GRND_INSECURE	0x0004

/* <sys/_iovec.h>, <sys/uio.h> */
enum uio_rw { UIO_READ, UIO_WRITE };
enum uio_seg { UIO_USERSPACE, UIO_SYSSPACE, UIO_NOCOPY };

typedef long register_t;

struct iovec {
	void	*iov_base;
	size_t	 iov_len;
};

/* <sys/proc.h> */
struct thread {
	register_t	td_retval[2];
};

struct uio {
	struct iovec	*uio_iov;
	int		 uio_iovcnt;
	off_t		 uio_offset;
	ssize_t		 uio_resid;
	enum uio_seg	 uio_segflg;
	enum uio_rw	 uio_rw;
	struct thread	*uio_td;
};

/*
 * Deterministic stand-in for read_random_uio(9) plus the observation record
 * the harness uses to check *how* it was called.  Shared by oracle and port.
 */
struct rr_observation {
	int		 calls;
	int		 nonblock;
	const void	*iov_base;
	size_t		 iov_len;
	int		 iovcnt;
	long long	 offset;
	long long	 resid_in;
	long long	 resid_out;
	int		 segflg;
	int		 rw;
	const void	*td;
};

int rr_error;
size_t rr_consume;
int rr_block;
struct rr_observation rr_obs;

int
read_random_uio(struct uio *uio, bool nonblock)
{
	unsigned char *p;
	size_t avail, n, i;
	unsigned int mix;

	rr_obs.calls++;
	rr_obs.nonblock = nonblock ? 1 : 0;
	rr_obs.iov_base = uio->uio_iov[0].iov_base;
	rr_obs.iov_len = uio->uio_iov[0].iov_len;
	rr_obs.iovcnt = uio->uio_iovcnt;
	rr_obs.offset = (long long)uio->uio_offset;
	rr_obs.resid_in = (long long)uio->uio_resid;
	rr_obs.segflg = (int)uio->uio_segflg;
	rr_obs.rw = (int)uio->uio_rw;
	rr_obs.td = uio->uio_td;

	/* A nonblocking request refuses to wait for initial seeding. */
	if (rr_block && nonblock) {
		rr_obs.resid_out = (long long)uio->uio_resid;
		return (EWOULDBLOCK);
	}

	avail = (uio->uio_resid > 0) ? (size_t)uio->uio_resid : 0;
	n = (rr_consume < avail) ? rr_consume : avail;
	mix = (unsigned int)(nonblock ? 0x5bu : 0x11u) +
	    (unsigned int)uio->uio_iovcnt * 9u +
	    (unsigned int)uio->uio_offset * 13u +
	    (unsigned int)uio->uio_iov[0].iov_len * 17u +
	    (unsigned int)uio->uio_segflg * 3u +
	    (unsigned int)uio->uio_rw * 5u;
	p = (unsigned char *)uio->uio_iov[0].iov_base;
	if (p != NULL) {
		for (i = 0; i < n; i++)
			p[i] = (unsigned char)(mix + (unsigned int)i * 7u);
	}
	uio->uio_resid -= (ssize_t)n;
	rr_obs.resid_out = (long long)uio->uio_resid;
	return (rr_error);
}

/* ---- original translation unit below, bodies verbatim ---------------- */

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
