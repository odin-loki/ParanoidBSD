module;

#include <cerrno>
#include <ctime>
#include <unistd.h>

export module pbsd.lib.libc.gen.b0261;

#define	LIO_READ		0x2
#define	LIO_FOFFSET		0x40
#define	LIO_VECTORED		0x4
#define	LIO_NOWAIT		0x0
#define	AIO_OP2_FOFFSET		0x00000001
#define	AIO_OP2_VECTORED	0x00000002

export struct aiocb {
	int	aio_lio_opcode;
};

extern "C" {
int nanosleep(const struct timespec *, struct timespec *);
int lio_listio(int, struct aiocb *const *, int, void *);
int aio_error(const struct aiocb *);
}

export namespace pbsd::lib_libc_gen::b0261 {

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
__usleep(useconds_t useconds)
{
	struct timespec time_to_sleep;

	time_to_sleep.tv_nsec = (useconds % 1000000) * 1000;
	time_to_sleep.tv_sec = useconds / 1000000;
	return (nanosleep(&time_to_sleep, NULL));
}

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

int
aio_read2(struct aiocb *iocb, int flags)
{
	int error;

	if ((flags & ~(AIO_OP2_FOFFSET | AIO_OP2_VECTORED)) != 0) {
		errno = EINVAL;
		return (-1);
	}
	iocb->aio_lio_opcode = LIO_READ;
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

} /* namespace pbsd::lib_libc_gen::b0261 */
