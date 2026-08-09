module;

#include <cstring>
#include <sys/types.h>

export module pbsd.lib.libc.gen.b0268;

export namespace pbsd::lib_libc_gen::b0268 {

/*-
 * Copyright (c) 2015-2016 Nuxi, https://nuxi.nl/
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

char *
dirname(char *path)
{
	char *end;

	/*
	 * If path is a null pointer or points to an empty string,
	 * dirname() shall return a pointer to the string ".".
	 */
	if (path == NULL || *path == '\0')
		return (reinterpret_cast<char *>(const_cast<char *>(".")));

	/* Find end of last pathname component. */
	end = path + strlen(path);
	while (end > path + 1 && *(end - 1) == '/')
		--end;

	/* Strip off the last pathname component. */
	while (end > path && *(end - 1) != '/')
		--end;

	/*
	 * If path does not contain a '/', then dirname() shall return a
	 * pointer to the string ".".
	 */
	if (end == path) {
		path[0] = '.';
		path[1] = '\0';
		return (path);
	}

	/*
	 * Remove trailing slashes from the resulting directory name. Ensure
	 * that at least one character remains.
	 */
	while (end > path + 1 && *(end - 1) == '/')
		--end;

	/* Null terminate directory name and return it. */
	*end = '\0';
	return (path);
}

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
 *    notice(s), this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
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

typedef void (*interpos_func_t)(void);

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
	INTERPOS_spinunlock,
	INTERPOS_kevent,
	INTERPOS_wait6,
	INTERPOS_ppoll,
	INTERPOS_map_stacks_exec,
	INTERPOS_fdatasync,
	INTERPOS_clock_nanosleep,
	INTERPOS__reserved0,
	INTERPOS_pdfork,
	INTERPOS_uexterr_gettext,
	INTERPOS_pdwait,
	INTERPOS_MAX
};

struct _spinlock;

extern "C" {
void __libc_system(void);
int __libc_tcdrain(int);
int _pthread_mutex_init_calloc_cb_stub(void *, void *);
void __libc_spinlock_stub(struct _spinlock *);
void __libc_spinunlock_stub(struct _spinlock *);
void __libc_map_stacks_exec(void);
int __libc_uexterr_gettext(char *, size_t);
interpos_func_t *__libsys_interposing_slot(int interposno);
}

#define	SLOT(a, b) \
	[INTERPOS_##a] = (interpos_func_t)b
interpos_func_t __libc_interposing[INTERPOS_MAX];
#undef SLOT

struct __libc_interposing_init {
	__libc_interposing_init()
	{
#define	SLOT(a, b) \
		__libc_interposing[INTERPOS_##a] = (interpos_func_t)(b)
		SLOT(system, __libc_system);
		SLOT(tcdrain, __libc_tcdrain);
		SLOT(_pthread_mutex_init_calloc_cb, _pthread_mutex_init_calloc_cb_stub);
		SLOT(spinlock, __libc_spinlock_stub);
		SLOT(spinunlock, __libc_spinunlock_stub);
		SLOT(map_stacks_exec, __libc_map_stacks_exec);
		SLOT(uexterr_gettext, __libc_uexterr_gettext);
#undef SLOT
	}
};

static __libc_interposing_init __libc_interposing_initializer;

interpos_func_t *
__libc_interposing_slot(int interposno)
{
	if (__libc_interposing[interposno] == NULL)
		return (__libsys_interposing_slot(interposno));
	return (&__libc_interposing[interposno]);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Google LLC
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

#include <stdbool.h>
#include <syslog.h>

void
__arc4random_stir_fbsd11(void)
{
	static bool warned = false;

	if (!warned)
		syslog(LOG_DEBUG, "Deprecated function arc4random_stir() called");
	warned = true;
}

void
__arc4random_addrandom_fbsd11(u_char * dummy1 __attribute__((__unused__)),
    int dummy2 __attribute__((__unused__)))
{
	static bool warned = false;

	if (!warned)
		syslog(LOG_DEBUG, "Deprecated function arc4random_addrandom() called");
	warned = true;
}

} /* namespace pbsd::lib_libc_gen::b0268 */
