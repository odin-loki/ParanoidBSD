/*
 * Copyright (c) 2014 The FreeBSD Foundation.
 *
 * Portions of this software were developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * PBSD batch b0027.  C++23 port of:
 *
 *	hbsd/src/lib/libc/sys/read.c
 *	hbsd/src/lib/libc/sys/msync.c
 *	hbsd/src/lib/libc/sys/write.c
 *	hbsd/src/lib/libc/sys/wait4.c
 *
 * All four are libc's #pragma weak interposition wrappers: the whole of each
 * function is one indirect call through the process' interposing table,
 * forwarding its arguments in order and returning the callee's result
 * verbatim.  Argument types, signedness and order are those of the original;
 * nothing is validated, nothing is clamped and errno is not touched, exactly
 * as in the C.  The table, its slot numbers and the INTERPOS_SYS() dispatch
 * macro come from libc's private libc_private.h, which is not part of this
 * batch, so they are restated here unchanged in meaning.
 */

module;

#define	_GNU_SOURCE	1

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <cstddef>

export module pbsd.lib.libc.sys.b0027;

namespace pbsd::lib_libc_sys::b0027 {

using interpos_func_t = int (*)(void);

enum {
	INTERPOS_read,
	INTERPOS_write,
	INTERPOS_msync,
	INTERPOS_wait4,
	INTERPOS_MAX
};

interpos_func_t libc_interposing[INTERPOS_MAX];

using interpos_sig_read = ssize_t (*)(int, void *, size_t);
using interpos_sig_write = ssize_t (*)(int, const void *, size_t);
using interpos_sig_msync = int (*)(void *, size_t, int);
using interpos_sig_wait4 = pid_t (*)(pid_t, int *, int, ::rusage *);

} /* namespace pbsd::lib_libc_sys::b0027 */

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&pbsd::lib_libc_sys::b0027::libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((pbsd::lib_libc_sys::b0027::interpos_sig_ ## syscall_name)	\
	    __libc_interposing_slot(INTERPOS_ ## syscall_name))		\
	    (__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0027 {

/* hbsd/src/lib/libc/sys/read.c */
[[gnu::weak]] ssize_t
read(int fd, void *buf, size_t nbytes)
{
	return (INTERPOS_SYS(read, fd, buf, nbytes));
}

/* hbsd/src/lib/libc/sys/msync.c */
[[gnu::weak]] int
msync(void *addr, size_t len, int flags)
{
	return (INTERPOS_SYS(msync, addr, len, flags));
}

/* hbsd/src/lib/libc/sys/write.c */
[[gnu::weak]] ssize_t
write(int fd, const void *buf, size_t nbytes)
{
	return (INTERPOS_SYS(write, fd, buf, nbytes));
}

/* hbsd/src/lib/libc/sys/wait4.c */
[[gnu::weak]] pid_t
wait4(pid_t pid, int *status, int options, ::rusage *ru)
{
	return (INTERPOS_SYS(wait4, pid, status, options, ru));
}

/*
 * Test-only access to this module's interposing table.  Not part of any
 * ported function; libc fills the real table from _libc_init().
 */
void
set_interpos(int slot, interpos_func_t func)
{
	libc_interposing[slot] = func;
}

interpos_func_t
get_interpos(int slot)
{
	return (libc_interposing[slot]);
}

} /* namespace pbsd::lib_libc_sys::b0027 */
