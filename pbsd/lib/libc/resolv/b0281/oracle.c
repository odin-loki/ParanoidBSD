/*
 * oracle.c -- reference implementation for batch b0281.
 *
 * Original C sources concatenated, every function renamed with a ref_ prefix.
 * Function bodies are UNMODIFIED.  Only macro definitions, type stubs, and
 * mock hooks that the isolated batch cannot link against have been added.
 */

#define _POSIX_C_SOURCE 200809L
#define DO_PTHREADS 1

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef CLOCK_MONOTONIC_FAST
#define CLOCK_MONOTONIC_FAST CLOCK_MONOTONIC
#endif

#ifndef RES_INIT
#define RES_INIT 0x00000001u
#endif

#ifndef RES_TIMEOUT
#define RES_TIMEOUT 0x00000002u
#endif

#define _PATH_RESCONF "/etc/resolv.conf"

#define nitems(x) (sizeof(x) / sizeof((x)[0]))

#define DE_CONST(x, t) ((t) = (char *)(uintptr_t)(const void *)(x))

/* ------------------------------------------------------------------ */
/* Minimal resolver / threading model for this batch.                 */
/* ------------------------------------------------------------------ */

struct __res_state_ext {
	time_t conf_stat;
	struct timespec conf_mtim;
	unsigned int reload_period;
};

struct __res_state {
	int res_h_errno;
	unsigned int options;
	struct {
		struct {
			struct __res_state_ext *ext;
		} _ext;
	} _u;
};

typedef struct __res_state *res_state;

typedef struct mtctxres_t {
	unsigned char opaque[64];
} mtctxres_t;

typedef struct {
	long opaque;
} thread_key_t;

typedef struct {
	int state;
} once_t;

#define ONCE_INITIALIZER {0}

typedef struct {
	int id;
} pbsd_b0281_pthread_key_t;

typedef struct {
	int locked;
} pbsd_b0281_pthread_mutex_t;

#define pthread_key_t pbsd_b0281_pthread_key_t
#define pthread_mutex_t pbsd_b0281_pthread_mutex_t

#define PTHREAD_MUTEX_INITIALIZER {0}

/* Mock controls (harness-visible). */
int mock_thr_main_ret = 1;
int mock_thr_once_ret = 0;
int mock_thr_keycreate_ret = 0;
int mock_thr_setspecific_ret = 0;
res_state mock_thr_getspecific_val = NULL;
int mock_thr_getspecific_calls = 0;
int mock_thr_setspecific_calls = 0;
res_state mock_thr_setspecific_last = NULL;
int mock_thr_keycreate_calls = 0;
void (*mock_thr_keycreate_destructor)(void *) = NULL;
int mock_calloc_fail = 0;
int mock_clock_gettime_ret = 0;
struct timespec mock_clock_now = {0, 0};
int mock_stat_ret = -1;
struct stat mock_stat_sb;
int mock_stat_calls = 0;
int mock_res_ndestroy_calls = 0;
int mock_free_calls = 0;

void *mock_pthread_getspecific_val = NULL;
int mock_pthread_getspecific_calls = 0;
int mock_pthread_setspecific_ret = 0;
int mock_pthread_setspecific_calls = 0;
void *mock_pthread_setspecific_last = NULL;
int mock_pthread_key_create_ret = 0;
int mock_pthread_key_create_calls = 0;
void (*mock_pthread_key_destructor)(void *) = NULL;
int mock_pthread_mutex_lock_ret = 0;
int mock_pthread_mutex_unlock_ret = 0;
int mock_malloc_fail = 0;

int mock_writev_ret = 0;
int mock_writev_calls = 0;
int mock_writev_last_fd = -1;
int mock_writev_last_count = 0;
struct iovec mock_writev_last_iov[8];

int
thr_main(void)
{
	return (mock_thr_main_ret);
}

int
thr_once(once_t *once, void (*init_routine)(void))
{
	if (once->state != 0)
		return (0);
	init_routine();
	once->state = 1;
	return (mock_thr_once_ret);
}

int
thr_keycreate(thread_key_t *key, void (*destructor)(void *))
{
	(void)key;
	mock_thr_keycreate_calls++;
	mock_thr_keycreate_destructor = destructor;
	return (mock_thr_keycreate_ret);
}

void *
thr_getspecific(thread_key_t key)
{
	(void)key;
	mock_thr_getspecific_calls++;
	return (mock_thr_getspecific_val);
}

int
thr_setspecific(thread_key_t key, void *value)
{
	(void)key;
	mock_thr_setspecific_calls++;
	mock_thr_setspecific_last = (res_state)value;
	return (mock_thr_setspecific_ret);
}

void
res_ndestroy(res_state statp)
{
	(void)statp;
	mock_res_ndestroy_calls++;
}

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	mock_pthread_key_create_calls++;
	mock_pthread_key_destructor = destructor;
	if (mock_pthread_key_create_ret != 0)
		return (mock_pthread_key_create_ret);
	key->id = 1;
	return (0);
}

void *
pthread_getspecific(pthread_key_t key)
{
	(void)key;
	mock_pthread_getspecific_calls++;
	return (mock_pthread_getspecific_val);
}

int
pthread_setspecific(pthread_key_t key, const void *value)
{
	(void)key;
	mock_pthread_setspecific_calls++;
	mock_pthread_setspecific_last = value;
	if (mock_pthread_setspecific_ret != 0) {
		errno = mock_pthread_setspecific_ret;
		return (mock_pthread_setspecific_ret);
	}
	return (0);
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
	mutex->locked = 1;
	return (mock_pthread_mutex_lock_ret);
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	mutex->locked = 0;
	return (mock_pthread_mutex_unlock_ret);
}

int
mock_writev(int fd, const struct iovec *iov, int iovcnt)
{
	int i;

	mock_writev_calls++;
	mock_writev_last_fd = fd;
	mock_writev_last_count = iovcnt;
	for (i = 0; i < 8; i++) {
		mock_writev_last_iov[i].iov_base = NULL;
		mock_writev_last_iov[i].iov_len = 0;
	}
	for (i = 0; i < iovcnt && i < 8; i++)
		mock_writev_last_iov[i] = iov[i];
	return (mock_writev_ret);
}

#define _writev mock_writev

res_state ref___res_state(void);
int *ref___h_errno(void);
const char *ref_hstrerror(int err);

#define __res_state ref___res_state

/* ------------------------------------------------------------------ */
/* h_errno.c                                                          */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2006 The FreeBSD Project. All rights reserved.
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

#undef	h_errno
extern int h_errno;

int *ref___h_errno(void);
void ref___h_errno_set(res_state res, int err);

int *
ref___h_errno(void)
{
	return (&__res_state()->res_h_errno);
}

void
ref___h_errno_set(res_state res, int err)
{
	h_errno = res->res_h_errno = err;
}

#undef __res_state

/* ------------------------------------------------------------------ */
/* mtctxres.c                                                         */
/* ------------------------------------------------------------------ */

static pthread_key_t	key;
static int		mt_key_initialized = 0;

static int		__res_init_ctx(void);
static void		__res_destroy_ctx(void *);

static mtctxres_t	sharedctx;

#ifdef DO_PTHREADS
static void
_mtctxres_init(void) {
	int pthread_keycreate_ret;

	pthread_keycreate_ret = pthread_key_create(&key, __res_destroy_ctx);
	if (pthread_keycreate_ret == 0)
		mt_key_initialized = 1;
}
#endif

#ifndef _LIBC
int
ref___res_enable_mt(void) {
	return (-1);
}

int
ref___res_disable_mt(void) {
	return (0);
}
#endif

#ifdef DO_PTHREADS
static int
__res_init_ctx(void) {

	mtctxres_t	*mt;
	int		ret;


	if (pthread_getspecific(key) != 0) {
		/* Already exists */
		return (0);
	}

	if ((mt = malloc(sizeof(mtctxres_t))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	memset(mt, 0, sizeof (mtctxres_t));

	if ((ret = pthread_setspecific(key, mt)) != 0) {
		free(mt);
		errno = ret;
		return (-1);
	}

	return (0);
}

static void
__res_destroy_ctx(void *value) {

	free(value);
}
#endif

mtctxres_t *
ref___mtctxres(void) {
#ifdef DO_PTHREADS
	mtctxres_t	*mt;

	/*
	 * This if clause should only be executed if we are linking
	 * statically.  When linked dynamically _mtctxres_init() should
	 * be called at binding time due the #pragma above.
	 */
	if (!mt_key_initialized) {
		static pthread_mutex_t keylock = PTHREAD_MUTEX_INITIALIZER;
                if (pthread_mutex_lock(&keylock) == 0) {
			_mtctxres_init();
			(void) pthread_mutex_unlock(&keylock);
		}
	}

	/*
	 * If we have already been called in this thread return the existing
	 * context.  Otherwise recreat a new context and return it.  If
	 * that fails return a global context.
	 */
	if (mt_key_initialized) {
		if (((mt = pthread_getspecific(key)) != NULL) ||
		    (__res_init_ctx() == 0 &&
		     (mt = pthread_getspecific(key)) != NULL)) {
			return (mt);
		}
	}
#endif
	return (&sharedctx);
}

/* ------------------------------------------------------------------ */
/* res_state.c                                                        */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2006 The FreeBSD Project. All rights reserved.
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

#undef _res

struct __res_state _res;

static thread_key_t res_key;
static once_t res_init_once = ONCE_INITIALIZER;
static int res_thr_keycreated = 0;

static void
free_res(void *ptr)
{
	res_state statp = ptr;

	if (statp->_u._ext.ext != NULL)
		res_ndestroy(statp);
	free(statp);
}

static void
res_keycreate(void)
{
	res_thr_keycreated = thr_keycreate(&res_key, free_res) == 0;
}

static res_state
res_check_reload(res_state statp)
{
	struct timespec now;
	struct stat sb;
	struct __res_state_ext *ext;

	if ((statp->options & RES_INIT) == 0) {
		return (statp);
	}

	ext = statp->_u._ext.ext;
	if (ext == NULL || ext->reload_period == 0) {
		return (statp);
	}

	if (clock_gettime(CLOCK_MONOTONIC_FAST, &now) != 0 ||
	    (now.tv_sec - ext->conf_stat) < ext->reload_period) {
		return (statp);
	}

	ext->conf_stat = now.tv_sec;
	if (stat(_PATH_RESCONF, &sb) == 0 &&
	    (sb.st_mtim.tv_sec  != ext->conf_mtim.tv_sec ||
	     sb.st_mtim.tv_nsec != ext->conf_mtim.tv_nsec)) {
		statp->options &= ~RES_INIT;
	}

	return (statp);
}

res_state
ref___res_state(void)
{
	res_state statp;

	if (thr_main() != 0)
		return res_check_reload(&_res);

	if (thr_once(&res_init_once, res_keycreate) != 0 ||
	    !res_thr_keycreated)
		return (&_res);

	statp = thr_getspecific(res_key);
	if (statp != NULL)
		return res_check_reload(statp);
	statp = calloc(1, sizeof(*statp));
	if (statp == NULL)
		return (&_res);
#ifdef __BIND_RES_TEXT
	statp->options = RES_TIMEOUT;			/* Motorola, et al. */
#endif
	if (thr_setspecific(res_key, statp) == 0)
		return (statp);
	free(statp);
	return (&_res);
}

/* ------------------------------------------------------------------ */
/* herror.c                                                           */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: (BSD-3-Clause AND ISC)
 *
 * Copyright (c) 1987, 1993
 *    The Regents of the University of California.  All rights reserved.
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

/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

const char *h_errlist[] = {
	"Resolver Error 0 (no error)",
	"Unknown host",				/*%< 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/*%< 2 TRY_AGAIN */
	"Unknown server error",			/*%< 3 NO_RECOVERY */
	"No address associated with name",	/*%< 4 NO_ADDRESS */
};
const int h_nerr = { nitems(h_errlist) };

#undef	h_errno
int	h_errno;

#define __h_errno ref___h_errno
#define hstrerror ref_hstrerror

void
ref_herror(const char *s) {
	struct iovec iov[4], *v = iov;
	char *t;

	if (s != NULL && *s != '\0') {
		DE_CONST(s, t);
		v->iov_base = t;
		v->iov_len = strlen(t);
		v++;
		DE_CONST(": ", t);
		v->iov_base = t;
		v->iov_len = 2;
		v++;
	}
	DE_CONST(hstrerror(*__h_errno()), t);
	v->iov_base = t;
	v->iov_len = strlen(v->iov_base);
	v++;
	DE_CONST("\n", t);
	v->iov_base = t;
	v->iov_len = 1;
	_writev(STDERR_FILENO, iov, (v - iov) + 1);
}

const char *
ref_hstrerror(int err) {
	if (err < 0)
		return ("Resolver internal error");
	else if (err < h_nerr)
		return (h_errlist[err]);
	return ("Unknown resolver error");
}

/* Linker --wrap targets for allocation tracking. */
extern void *__real_malloc(size_t);
extern void *__real_calloc(size_t, size_t);
extern void __real_free(void *);

void *
__wrap_malloc(size_t size)
{
	if (mock_malloc_fail)
		return (NULL);
	return (__real_malloc(size));
}

void *
__wrap_calloc(size_t nmemb, size_t size)
{
	if (mock_calloc_fail)
		return (NULL);
	return (__real_calloc(nmemb, size));
}

void
__wrap_free(void *ptr)
{
	mock_free_calls++;
	__real_free(ptr);
}

extern int __wrap_clock_gettime(clockid_t, struct timespec *);

int
__wrap_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	(void)clk_id;
	if (mock_clock_gettime_ret != 0) {
		errno = EINVAL;
		return (-1);
	}
	*tp = mock_clock_now;
	return (0);
}

extern int __wrap_stat(const char *, struct stat *);

int
__wrap_stat(const char *path, struct stat *sb)
{
	(void)path;
	mock_stat_calls++;
	if (mock_stat_ret != 0) {
		errno = ENOENT;
		return (-1);
	}
	*sb = mock_stat_sb;
	return (0);
}
