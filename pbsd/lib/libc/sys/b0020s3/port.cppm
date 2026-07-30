/*
 * port.cppm -- PBSD C++23 port of batch b0020s3.
 *
 * Faithful port of lib/libc/sys/sigwait.c.  Behaviour is preserved exactly:
 * the call is dispatched indirectly through libc's interposing table slot for
 * sigwait, so that a threading library which has filled that slot keeps its
 * cancellation-aware implementation.  Nothing is improved.
 *
 * The interposing table and the INTERPOS_SYS dispatch macro come from libc's
 * private header (lib/libc/include/libc_private.h); they are reproduced here
 * because that header is not part of this batch.  The table object itself is
 * defined elsewhere in libc, so it is only declared.
 */

module;

#define _POSIX_C_SOURCE 200809L

#include <signal.h>

export module pbsd.lib.libc.sys.b0020s3;

namespace pbsd::lib_libc_sys::b0020s3::detail {

enum {
	INTERPOS_accept,
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_close,
	INTERPOS_connect,
	INTERPOS_fcntl,
	INTERPOS_fsync,
	INTERPOS_fork,
	INTERPOS_msync,
	INTERPOS_nanosleep,
	INTERPOS_openat,
	INTERPOS_poll,
	INTERPOS_pselect,
	INTERPOS_recvfrom,
	INTERPOS_recvmsg,
	INTERPOS_select,
	INTERPOS_sendmsg,
	INTERPOS_sendto,
	INTERPOS_setcontext,
	INTERPOS_sigaction,
	INTERPOS_sigprocmask,
	INTERPOS_sigsuspend,
	INTERPOS_sigwait,
	INTERPOS_sigtimedwait,
	INTERPOS_sigwaitinfo,
	INTERPOS_swapcontext,
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_read,
	INTERPOS_readv,
	INTERPOS_wait4,
	INTERPOS_write,
	INTERPOS_writev,
	INTERPOS__pthread_mutex_init_calloc_cb,
	INTERPOS_spinlock,
	INTERPOS_wait6,
	INTERPOS_kevent,
	INTERPOS_wait,
	INTERPOS_pdfork,
	INTERPOS_MAX
};

using interpos_func_t = int (*)(void);

extern "C" interpos_func_t __libc_interposing[INTERPOS_MAX];

inline interpos_func_t *
libc_interposing_slot(int interposno)
{
	return (&__libc_interposing[interposno]);
}

} // namespace pbsd::lib_libc_sys::b0020s3::detail

#define	INTERPOS_SYS(syscall, ...)					\
	((reinterpret_cast<__typeof__(::syscall) *>(*(::pbsd::		\
	    lib_libc_sys::b0020s3::detail::libc_interposing_slot(	\
	    ::pbsd::lib_libc_sys::b0020s3::detail::INTERPOS_##syscall))))\
	    (__VA_ARGS__))

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010 davidxu@freebsd.org
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

export namespace pbsd::lib_libc_sys::b0020s3 {

/* #pragma weak sigwait */
[[gnu::weak]]
int
sigwait(const sigset_t *set, int *sig)
{
	return (INTERPOS_SYS(sigwait, set, sig));
}

} // namespace pbsd::lib_libc_sys::b0020s3
