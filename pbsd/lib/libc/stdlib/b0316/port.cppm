/*
 * PBSD batch b0316 -- C++23 port of:
 *   hbsd/src/lib/libc/stdlib/abort.c
 *   hbsd/src/lib/libc/stdlib/set_constraint_handler_s.c
 *   hbsd/src/lib/libc/stdlib/exit.c
 *
 * Original copyright headers are reproduced above each ported unit.
 */
module;

#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

extern "C" {
void *stub_malloc(size_t);
void stub_free(void *);
ssize_t stub__write(int, const void *, size_t);
int stub___libc_sigprocmask(int, const sigset_t *, sigset_t *);
int stub___libc_sigaction(int, const struct sigaction *,
    struct sigaction *);
int stub_raise(int);
void stub__exit(int);
void stub___cxa_thread_call_dtors(void);
void stub___cxa_finalize(void *);
void stub__once(pthread_once_t *, void (*)(void));
int stub__pthread_mutex_lock(pthread_mutex_t *);
int stub__pthread_mutex_unlock(pthread_mutex_t *);
int stub__pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int stub__pthread_mutexattr_init(pthread_mutexattr_t *);
int stub__pthread_mutexattr_settype(pthread_mutexattr_t *, int);
int stub__pthread_mutexattr_destroy(pthread_mutexattr_t *);
extern void (*__cleanup)(void);
extern int __isthreaded;
extern int _thread_autoinit_dummy_decl;
}

export module pbsd.lib.libc.stdlib.b0316;

export namespace pbsd::lib_libc_stdlib::b0316 {

typedef void (*constraint_handler_t)(const char *msg, void *ptr, errno_t error);

void abort(void);
void exit(int);

/*
 * ===== hbsd/src/lib/libc/stdlib/set_constraint_handler_s.c =====
 *
 * Copyright (c) 2017 Juniper Networks.  All rights reserved.
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

static constraint_handler_t *_ch = NULL;
static pthread_mutex_t ch_lock = PTHREAD_MUTEX_INITIALIZER;

constraint_handler_t
set_constraint_handler_s(constraint_handler_t handler)
{
	constraint_handler_t *new_, *old, ret;

	new_ = (constraint_handler_t *)stub_malloc(sizeof(constraint_handler_t));
	if (new_ == NULL)
		return (NULL);
	*new_ = handler;
	if (__isthreaded)
		stub__pthread_mutex_lock(&ch_lock);
	old = _ch;
	_ch = new_;
	if (__isthreaded)
		stub__pthread_mutex_unlock(&ch_lock);
	if (old == NULL) {
		ret = NULL;
	} else {
		ret = *old;
		stub_free(old);
	}
	return (ret);
}

void
__throw_constraint_handler_s(const char * restrict msg, errno_t error)
{
	constraint_handler_t ch;

	if (__isthreaded)
		stub__pthread_mutex_lock(&ch_lock);
	ch = _ch != NULL ? *_ch : NULL;
	if (__isthreaded)
		stub__pthread_mutex_unlock(&ch_lock);
	if (ch != NULL)
		ch(msg, NULL, error);
}

void
abort_handler_s(const char * restrict msg, void * restrict ptr __attribute__((unused)),
    errno_t error __attribute__((unused)))
{
	static const char ahs[] = "abort_handler_s : ";

	(void) stub__write(STDERR_FILENO, ahs, sizeof(ahs) - 1);
	(void) stub__write(STDERR_FILENO, msg, strlen(msg));
	(void) stub__write(STDERR_FILENO, "\n", 1);
	abort();
}

void
ignore_handler_s(const char * restrict msg __attribute__((unused)),
    void * restrict ptr __attribute__((unused)),
    errno_t error __attribute__((unused)))
{
}

} /* namespace */

export namespace pbsd::lib_libc_stdlib::b0316 {

/*
 * ===== hbsd/src/lib/libc/stdlib/exit.c =====
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
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

static pthread_mutex_t exit_mutex;
static pthread_once_t exit_mutex_once = PTHREAD_ONCE_INIT;

static void
exit_mutex_init_once(void)
{
	pthread_mutexattr_t ma;

	stub__pthread_mutexattr_init(&ma);
	stub__pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
	stub__pthread_mutex_init(&exit_mutex, &ma);
	stub__pthread_mutexattr_destroy(&ma);
}

void
exit(int status)
{
	/* Ensure that the auto-initialization routine is linked in: */
	extern int _thread_autoinit_dummy_decl;

	_thread_autoinit_dummy_decl = 1;

	/* Make exit(3) thread-safe */
	if (__isthreaded) {
		stub__once(&exit_mutex_once, exit_mutex_init_once);
		stub__pthread_mutex_lock(&exit_mutex);
	}

	/*
	 * We're dealing with cleaning up thread_local destructors in the case of
	 * the process termination through main() exit.
	 * Other cases are handled elsewhere.
	 */
	stub___cxa_thread_call_dtors();
	stub___cxa_finalize(NULL);
	if (__cleanup)
		(*__cleanup)();
	stub__exit(status);
}

/*
 * ===== hbsd/src/lib/libc/stdlib/abort.c =====
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1985, 1993
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

void
abort(void)
{
	struct sigaction act;

	/*
	 * POSIX requires we flush stdio buffers on abort.
	 * XXX ISO C requires that abort() be async-signal-safe.
	 */
	if (__cleanup)
		(*__cleanup)();

	sigfillset(&act.sa_mask);
	/*
	 * Don't block SIGABRT to give any handler a chance; we ignore
	 * any errors -- ISO C doesn't allow abort to return anyway.
	 */
	sigdelset(&act.sa_mask, SIGABRT);
	(void)stub___libc_sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
	(void)stub_raise(SIGABRT);

	/*
	 * If SIGABRT was ignored, or caught and the handler returns, do
	 * it again, only harder.
	 */
	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigfillset(&act.sa_mask);
	(void)stub___libc_sigaction(SIGABRT, &act, NULL);
	sigdelset(&act.sa_mask, SIGABRT);
	(void)stub___libc_sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
	(void)stub_raise(SIGABRT);
	exit(1);
}

} /* namespace */
