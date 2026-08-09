module;

#define _GNU_SOURCE

#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>

#ifndef UNVIS_END
#define UNVIS_END 0x1000
#endif

#ifndef KQUEUE_CLOEXEC
#define KQUEUE_CLOEXEC 0x00000001
#endif

export module pbsd.lib.libc.gen.b0256;

export namespace pbsd::lib_libc_gen::b0256 {

extern "C" {
int unvis(char *, int, int *, int);
int kqueuex(unsigned flags);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2012 SRI International
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
 * ("CTSRD"), as part of the DARPA CRASH research programme.
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

#define	_UNVIS_END	1

int
__unvis_44bsd(char *cp, int c, int *astate, int flag)
{

	if (flag & _UNVIS_END)
		flag = (flag & ~_UNVIS_END) ^ UNVIS_END;
	return unvis(cp, c, astate, flag);
}

/*-
 * Copyright (c) 2023 The FreeBSD Foundation
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

/*
 * Provide some NetBSD compatibility.  They support a set of O_*
 * flags, we only carry O_CLOEXEC, and accept but ignore O_NONBLOCK.
 */
int
kqueue1(int openflags)
{
	u_int flags;

	if ((openflags & ~(O_CLOEXEC | O_NONBLOCK)) != 0) {
		errno = EINVAL;
		return (-1);
	}

	flags = 0;
	if ((openflags & O_CLOEXEC) != 0)
		flags |= KQUEUE_CLOEXEC;
	return (kqueuex(flags));
}

} // namespace pbsd::lib_libc_gen::b0256
