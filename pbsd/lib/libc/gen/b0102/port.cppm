/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 The FreeBSD Foundation
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
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
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1985, 1989, 1993
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

module;

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>

#define	LIO_READ		1
#define	LIO_WRITE		2
#define	LIO_NOP			4
#define	LIO_FOFFSET		0x10
#define	LIO_VECTORED		0x20
#define	LIO_WAIT		0
#define	LIO_NOWAIT		1
#define	AIO_OP2_FOFFSET		0x0001
#define	AIO_OP2_VECTORED	0x0002
#define	GETENTROPY_MAX		256

struct aiocb {
	int	aio_lio_opcode;
};

struct DIR {
	pthread_mutex_t	dd_lock;
};

extern "C" {
int lio_listio(int, struct aiocb *const *, int, void *);
int aio_error(const struct aiocb *);
void _seekdir(struct DIR *, long);
void _pthread_mutex_lock(pthread_mutex_t *);
void _pthread_mutex_unlock(pthread_mutex_t *);
extern int __isthreaded;
int __libc_sigaction(int, const struct sigaction *, struct sigaction *);
ssize_t getrandom(void *, size_t, unsigned int);
int raise(int);
}

export module pbsd.lib.libc.gen.b0102;

export namespace pbsd::lib_libc_gen::b0102 {

int
aio_write2(struct aiocb *iocb, int flags)
{
	int error;

	if ((flags & ~(AIO_OP2_FOFFSET | AIO_OP2_VECTORED)) != 0) {
		errno = EINVAL;
		return (-1);
	}
	iocb->aio_lio_opcode = LIO_WRITE;
	if ((flags & AIO_OP2_FOFFSET) != 0)
		iocb->aio_lio_opcode |= LIO_FOFFSET;
	if ((flags & AIO_OP2_VECTORED) != 0)
		iocb->aio_lio_opcode |= LIO_VECTORED;

	error = lio_listio(LIO_NOWAIT, &iocb, 1, NULL);
	if (error == -1 && errno == EIO) {
		error = aio_error(iocb);
		if (error != -1 && error != 0)
			errno = error;
		error = -1;
	}
	return (error);
}

void
seekdir(struct DIR *dirp, long loc)
{
	if (__isthreaded)
		_pthread_mutex_lock(&dirp->dd_lock);
	_seekdir(dirp, loc);
	if (__isthreaded)
		_pthread_mutex_unlock(&dirp->dd_lock);
}

sigset_t _sigintr;

sig_t
signal(int s, sig_t a)
{
	struct sigaction sa, osa;

	sa.sa_handler = a;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (!sigismember(&_sigintr, s))
		sa.sa_flags |= SA_RESTART;
	if (__libc_sigaction(s, &sa, &osa) < 0)
		return (SIG_ERR);
	return (osa.sa_handler);
}

static inline void
_getentropy_fail(void)
{
	raise(SIGKILL);
}

int
getentropy(void *buf, size_t buflen)
{
	ssize_t rd;

	if (buflen > GETENTROPY_MAX) {
		errno = EINVAL;
		return (-1);
	}

	while (buflen > 0) {
		rd = getrandom(buf, buflen, 0);
		if (rd == -1) {
			switch (errno) {
			case EINTR:
				continue;
			case EFAULT:
				return (-1);
			default:
				_getentropy_fail();
			}
		}

		/* This cannot happen. */
		if (rd == 0)
			_getentropy_fail();

		buf = (char *)buf + rd;
		buflen -= rd;
	}

	return (0);
}

} /* namespace pbsd::lib_libc_gen::b0102 */
