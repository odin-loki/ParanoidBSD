module;

#include <climits>
#include <cstddef>
#include <cstdint>

export module pbsd.sys.kern.b0146s1;

namespace pbsd::sys_kern::b0146s1::detail {

#define CTASSERT(x) typedef char __ctassert[(x) ? 1 : -1] __attribute__((__unused__))

#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM   0x0002
#define GRND_INSECURE 0x0004
#define GRND_VALIDFLAGS (GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

#define IOSIZE_MAX INT_MAX

#define UIO_USERSPACE 1
#define UIO_READ      1

#define EINVAL 22
#define EWOULDBLOCK 35
#define EAGAIN 35

struct thread {
	long td_retval[2];
};

struct iovec {
	void *iov_base;
	std::size_t iov_len;
};

struct uio {
	iovec *uio_iov;
	int uio_iovcnt;
	long long uio_offset;
	long uio_resid;
	int uio_segflg;
	int uio_rw;
	thread *uio_td;
};

struct getrandom_args {
	void *buf;
	std::size_t buflen;
	unsigned int flags;
};

inline int g_read_random_error;
inline int g_read_random_block;
inline long g_read_random_transfer = -1;

inline void read_random_reset() noexcept
{
	g_read_random_error = 0;
	g_read_random_block = 0;
	g_read_random_transfer = -1;
}

inline void read_random_configure(int error, int block, ssize_t transfer) noexcept
{
	g_read_random_error = error;
	g_read_random_block = block;
	g_read_random_transfer = transfer;
}

inline int
read_random_uio(uio *auio, int nonblock)
{
	unsigned char *bp;
	unsigned long n, i;

	if (g_read_random_error != 0)
		return (g_read_random_error);
	if (nonblock != 0 && g_read_random_block != 0)
		return (EWOULDBLOCK);
	if (auio->uio_resid <= 0)
		return (0);
	n = (unsigned long)auio->uio_resid;
	if (g_read_random_transfer >= 0 &&
	    n > (unsigned long)g_read_random_transfer)
		n = (unsigned long)g_read_random_transfer;
	if (n > (unsigned long)auio->uio_iov[0].iov_len)
		n = (unsigned long)auio->uio_iov[0].iov_len;
	bp = static_cast<unsigned char *>(auio->uio_iov[0].iov_base);
	for (i = 0; i < n; i++)
		bp[i] = static_cast<unsigned char>(0xa5 ^ (i & 0xff));
	auio->uio_resid -= (long)n;
	return (0);
}

} // namespace pbsd::sys_kern::b0146s1::detail

export namespace pbsd::sys_kern::b0146s1 {

using detail::getrandom_args;
using detail::thread;

#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM   0x0002
#define GRND_INSECURE 0x0004
#define GRND_VALIDFLAGS (GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)
#define IOSIZE_MAX INT_MAX
#define CTASSERT(x) typedef char __ctassert[(x) ? 1 : -1] __attribute__((__unused__))

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

CTASSERT(EWOULDBLOCK == EAGAIN);

static int
kern_getrandom(thread *td, void *user_buf, std::size_t buflen,
    unsigned int flags)
{
	detail::uio auio;
	detail::iovec aiov;
	int error;

	if ((flags & ~GRND_VALIDFLAGS) != 0)
		return (EINVAL);
	if (buflen > IOSIZE_MAX)
		return (EINVAL);

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

	error = detail::read_random_uio(&auio, (flags & GRND_NONBLOCK) != 0);
	if (error == 0)
		td->td_retval[0] = buflen - auio.uio_resid;
	return (error);
}

int
sys_getrandom(thread *td, getrandom_args *uap)
{
	return (kern_getrandom(td, uap->buf, uap->buflen, uap->flags));
}

inline void read_random_reset() noexcept
{
	detail::read_random_reset();
}

inline void read_random_configure(int error, int block, ssize_t transfer) noexcept
{
	detail::read_random_configure(error, block, transfer);
}

} // namespace pbsd::sys_kern::b0146s1
