/*
 * PBSD batch b0281 -- C++23 port of
 *     lib/libc/resolv/h_errno.c
 *     lib/libc/resolv/mtctxres.c
 *     lib/libc/resolv/res_state.c
 *     lib/libc/resolv/herror.c
 *
 * Faithful translation: behaviour, signedness, evaluation order and pointer
 * arithmetic are preserved exactly.
 */

module;

#define DO_PTHREADS 1

typedef struct {
	int id;
} pbsd_b0281_pthread_key_t;

typedef struct {
	int locked;
} pbsd_b0281_pthread_mutex_t;

#define pthread_key_t pbsd_b0281_pthread_key_t
#define pthread_mutex_t pbsd_b0281_pthread_mutex_t

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>

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

export module pbsd.lib.libc.resolv.b0281;

export struct __res_state_ext {
	time_t conf_stat;
	struct timespec conf_mtim;
	unsigned int reload_period;
};

export struct __res_state_layout {
	int res_h_errno;
	unsigned int options;
	struct {
		struct {
			struct __res_state_ext *ext;
		} _ext;
	} _u;
};

export using res_state = __res_state_layout *;

export struct mtctxres_t {
	unsigned char opaque[64];
};

using thread_key_t = struct { long opaque; };
using once_t = struct { int state; };

#define ONCE_INITIALIZER {0}
#define PTHREAD_MUTEX_INITIALIZER {0}

extern "C" {
extern int mock_thr_main_ret;
extern int mock_thr_once_ret;
extern int mock_thr_keycreate_ret;
extern int mock_thr_setspecific_ret;
extern res_state mock_thr_getspecific_val;
extern int mock_calloc_fail;
extern int mock_clock_gettime_ret;
extern struct timespec mock_clock_now;
extern int mock_stat_ret;
extern struct stat mock_stat_sb;
extern void *mock_pthread_getspecific_val;
extern int mock_pthread_setspecific_ret;
extern int mock_pthread_key_create_ret;
extern int mock_malloc_fail;
extern int mock_writev_ret;
extern int mock_writev_calls;
extern int mock_writev_last_fd;
extern int mock_writev_last_count;
extern struct iovec mock_writev_last_iov[8];

extern struct __res_state_layout _res;
extern int h_errno;
extern const char *h_errlist[];
extern const int h_nerr;

int thr_main(void);
int thr_once(once_t *, void (*)(void));
int thr_keycreate(thread_key_t *, void (*)(void *));
void *thr_getspecific(thread_key_t);
int thr_setspecific(thread_key_t, void *);
void res_ndestroy(res_state);
int pthread_key_create(pthread_key_t *, void (*)(void *));
void *pthread_getspecific(pthread_key_t);
int pthread_setspecific(pthread_key_t, const void *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int mock_writev(int, const struct iovec *, int);
}

#define _writev mock_writev

export namespace pbsd::lib_libc_resolv::b0281 {

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

int *__h_errno(void);
void __h_errno_set(res_state res, int err);
res_state __res_state(void);

int *
__h_errno(void)
{
	return (&__res_state()->res_h_errno);
}

void
__h_errno_set(res_state res, int err)
{
	h_errno = res->res_h_errno = err;
}

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
__res_enable_mt(void) {
	return (-1);
}

int
__res_disable_mt(void) {
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

	if ((mt = (mtctxres_t *)malloc(sizeof(mtctxres_t))) == NULL) {
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
___mtctxres(void) {
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
		if (((mt = (mtctxres_t *)pthread_getspecific(key)) != NULL) ||
		    (__res_init_ctx() == 0 &&
		     (mt = (mtctxres_t *)pthread_getspecific(key)) != NULL)) {
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

static thread_key_t res_key;
static once_t res_init_once = ONCE_INITIALIZER;
static int res_thr_keycreated = 0;

static void
free_res(void *ptr)
{
	res_state statp = (res_state)ptr;

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
__res_state(void)
{
	res_state statp;

	if (thr_main() != 0)
		return res_check_reload(&_res);

	if (thr_once(&res_init_once, res_keycreate) != 0 ||
	    !res_thr_keycreated)
		return (&_res);

	statp = (res_state)thr_getspecific(res_key);
	if (statp != NULL)
		return res_check_reload(statp);
	statp = (res_state)calloc(1, sizeof(*statp));
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

const char *
hstrerror(int err) {
	if (err < 0)
		return ("Resolver internal error");
	else if (err < h_nerr)
		return (h_errlist[err]);
	return ("Unknown resolver error");
}

void
herror(const char *s) {
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
	v->iov_len = strlen((const char *)v->iov_base);
	v++;
	DE_CONST("\n", t);
	v->iov_base = t;
	v->iov_len = 1;
	_writev(STDERR_FILENO, iov, (v - iov) + 1);
}

} /* namespace */
