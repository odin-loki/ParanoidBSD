/*
 * Derived from HardenedBSD src/lib/libc/sys/fork.c, fsync.c and close.c.
 *
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
 * Derived from HardenedBSD src/lib/libc/sys/fdatasync.c.
 *
 * Copyright (c) 2016 The FreeBSD Foundation.
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

module;

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

export module pbsd.lib.libc.sys.b0025;

export namespace pbsd::lib_libc_sys::b0025 {

using interpos_func_t = int (*)(void);

/*
 * Slot numbers for the libc interposing table.  The syscall wrappers in this
 * batch do not call the kernel directly: they load a function pointer out of
 * this table and tail into it, so that libthr can substitute cancellation
 * aware entry points at run time.
 */
enum {
	INTERPOS_close,
	INTERPOS_fdatasync,
	INTERPOS_fork,
	INTERPOS_fsync,
	INTERPOS_MAX
};

interpos_func_t __libc_interposing[INTERPOS_MAX] = {
	reinterpret_cast<interpos_func_t>(&::close),
	reinterpret_cast<interpos_func_t>(&::fdatasync),
	reinterpret_cast<interpos_func_t>(&::fork),
	reinterpret_cast<interpos_func_t>(&::fsync),
};

interpos_func_t *
__libc_interposing_slot(int interposno)
{
	return (&__libc_interposing[interposno]);
}

[[gnu::weak]] pid_t
fork(void)
{
	return (reinterpret_cast<pid_t (*)(void)>(
	    *(__libc_interposing_slot(INTERPOS_fork))))();
}

int
fsync(int fd)
{
	return (reinterpret_cast<int (*)(int)>(
	    *(__libc_interposing_slot(INTERPOS_fsync))))(fd);
}

int
fdatasync(int fd)
{
	return (reinterpret_cast<int (*)(int)>(
	    *(__libc_interposing_slot(INTERPOS_fdatasync))))(fd);
}

[[gnu::weak]] int
close(int fd)
{
	return (reinterpret_cast<int (*)(int)>(
	    *(__libc_interposing_slot(INTERPOS_close))))(fd);
}

}
