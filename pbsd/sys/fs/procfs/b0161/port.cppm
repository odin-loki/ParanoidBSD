// PBSD port of HardenedBSD sys/fs/procfs batch b0161.

module;

#define _KERNEL
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <pthread.h>

export module pbsd.sys.fs.procfs.b0161;

export namespace pbsd::sys_fs_procfs::b0161 {

enum uio_rw_enum {
	UIO_READ,
	UIO_WRITE
};

struct mtx {
	pthread_mutex_t lock;
};

struct thread {
	int dummy;
};

struct pfs_node {
	int dummy;
};

struct proc {
	mtx p_mtx;
	int p_osrel;
};

struct sbuf {
	char		*s_buf;
	ssize_t		s_size;
	ssize_t		s_len;
	int		s_flags;
	int		s_error;
};

struct uio {
	ssize_t		uio_resid;
	uio_rw_enum	uio_rw;
};

constexpr int EINVAL = 22;
constexpr int EOPNOTSUPP = 45;
constexpr int PRVM_BLOCK_EXEC = 0x00000001;
constexpr int PRVM_CHECK_DEBUG = 0x00000004;

} // namespace pbsd::sys_fs_procfs::b0161

namespace pbsd::sys_fs_procfs::b0161::detail {

using namespace pbsd::sys_fs_procfs::b0161;

#define SBUF_FINISHED	0x00020000
#define SBUF_INCLUDENUL	0x00000002

#define SBUF_ISFINISHED(s)	((s)->s_flags & SBUF_FINISHED)
#define SBUF_NULINCLUDED(s)	((s)->s_flags & SBUF_INCLUDENUL)
#define SBUF_SETFLAG(s, f)	((s)->s_flags |= (f))

#define PFS_FILL_ARGS \
	thread *td, proc *p, pfs_node *pn, sbuf *sb, uio *uio

#define PROC_LOCK(p)	pthread_mutex_lock(&(p)->p_mtx.lock)
#define PROC_UNLOCK(p)	pthread_mutex_unlock(&(p)->p_mtx.lock)

inline int g_p_candebug_ret = 0;
inline int g_proc_rwmem_ret = 0;
inline ssize_t g_proc_rwmem_resid = -1;
inline int g_proc_rwmem_calls = 0;

inline void stub_reset() noexcept
{
	g_p_candebug_ret = 0;
	g_proc_rwmem_ret = 0;
	g_proc_rwmem_resid = -1;
	g_proc_rwmem_calls = 0;
}

inline int
p_candebug(thread *td, proc *p)
{
	(void)td;
	(void)p;
	return (g_p_candebug_ret);
}

inline int
proc_rwmem(proc *p, uio *uio, int flags)
{
	(void)p;
	(void)flags;

	g_proc_rwmem_calls++;
	if (g_proc_rwmem_ret != 0)
		return (g_proc_rwmem_ret);
	if (g_proc_rwmem_resid >= 0)
		uio->uio_resid = g_proc_rwmem_resid;
	return (0);
}

inline int
sbuf_trim(sbuf *s)
{

	if (s->s_error != 0)
		return (-1);

	while (s->s_len > 0 && isspace(s->s_buf[s->s_len - 1]))
		--s->s_len;

	return (0);
}

inline int
sbuf_finish(sbuf *s)
{

	s->s_buf[s->s_len] = '\0';
	if (SBUF_NULINCLUDED(s))
		s->s_len++;
	SBUF_SETFLAG(s, SBUF_FINISHED);
	return (s->s_error);
}

inline int
sbuf_vprintf(sbuf *s, const char *fmt, va_list ap)
{
	int n;

	if (s->s_error != 0)
		return (-1);
	if (s->s_len >= s->s_size)
		return (-1);
	n = std::vsnprintf(s->s_buf + s->s_len,
	    static_cast<std::size_t>(s->s_size - s->s_len), fmt, ap);
	if (n < 0)
		return (-1);
	if (n >= s->s_size - s->s_len)
		n = static_cast<int>(s->s_size - s->s_len - 1);
	s->s_len += n;
	return (n);
}

inline int
sbuf_printf(sbuf *s, const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = sbuf_vprintf(s, fmt, ap);
	va_end(ap);
	return (n);
}

inline char *
sbuf_data(sbuf *s)
{

	return (s->s_buf);
}

inline ssize_t
sbuf_len(sbuf *s)
{

	if (s->s_error != 0)
		return (-1);
	if (SBUF_NULINCLUDED(s) && !SBUF_ISFINISHED(s))
		return (s->s_len + 1);
	return (s->s_len);
}

} // namespace pbsd::sys_fs_procfs::b0161::detail

export namespace pbsd::sys_fs_procfs::b0161 {

#define PFS_FILL_ARGS \
	thread *td, proc *p, pfs_node *pn, sbuf *sb, uio *uio

#define PROC_LOCK(p)	pthread_mutex_lock(&(p)->p_mtx.lock)
#define PROC_UNLOCK(p)	pthread_mutex_unlock(&(p)->p_mtx.lock)

inline void stub_reset() noexcept
{
	detail::stub_reset();
}

inline int proc_rwmem_calls() noexcept
{
	return (detail::g_proc_rwmem_calls);
}

inline void stub_p_candebug(int ret) noexcept
{
	detail::g_p_candebug_ret = ret;
}

inline void stub_proc_rwmem(int ret, ssize_t resid) noexcept
{
	detail::g_proc_rwmem_ret = ret;
	detail::g_proc_rwmem_resid = resid;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993 Jan-Simon Pendry
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
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
procfs_doprocnote(PFS_FILL_ARGS)
{
	detail::sbuf_trim(sb);
	detail::sbuf_finish(sb);
	/* send to process's notify function */
	return (EOPNOTSUPP);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009 Konstantin Belousov
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
procfs_doosrel(PFS_FILL_ARGS)
{
	const char *pp;
	int ov, osrel, i;

	if (uio != NULL)
		return (EOPNOTSUPP);
	switch (uio->uio_rw) {
	case UIO_READ:
		detail::sbuf_printf(sb, "%d\n", p->p_osrel);
		break;
	case UIO_WRITE:
		detail::sbuf_trim(sb);
		detail::sbuf_finish(sb);
		pp = detail::sbuf_data(sb);
		osrel = 0;
		i = detail::sbuf_len(sb);
		while (i--) {
			if (*pp < '0' || *pp > '9')
				return (EINVAL);
			ov = osrel * 10 + *pp++ - '0';
			if (ov < osrel)
				return (EINVAL);
			osrel = ov;
		}
		p->p_osrel = osrel;
		break;
	}
	return (0);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993 Jan-Simon Pendry
 * Copyright (c) 1993 Sean Eric Fagan
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry and Sean Eric Fagan.
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
procfs_doprocmem(PFS_FILL_ARGS)
{
	int error;

	if (uio->uio_resid == 0) {
		return (0);
	}

	PROC_LOCK(p);
	error = detail::p_candebug(td, p);
#ifdef PAX_HARDENING
	if (error == 0) {
		error = pax_procfs_harden(td);
	}
#endif
	PROC_UNLOCK(p);
	if (error == 0) {
		error = detail::proc_rwmem(p, uio,
		    PRVM_CHECK_DEBUG | PRVM_BLOCK_EXEC);
	}

	return (error);
}

} // namespace pbsd::sys_fs_procfs::b0161
