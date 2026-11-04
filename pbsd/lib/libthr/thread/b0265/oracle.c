
/*
 * Batch b0265 oracle: original HardenedBSD C sources concatenated.
 * Every function renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef PBSD_B0265_PORT_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif
typedef struct pthread *pthread_t;

struct sched_param {
	int sched_priority;
};

struct thr_event_buf {
	int event;
	uintptr_t th_p;
	uintptr_t data;
};

struct pthread {
	unsigned flags;
	struct pthread *joiner;
	struct thr_event_buf event_buf;
};

struct umutex {
	int dummy;
};

#define TD_CREATE 1
#define TD_DEATH 2
#define THR_FLAGS_DETACHED 0x01000000
#define EINVAL 22

#define THR_UMUTEX_LOCK(t, l) b0265_umutex_lock(t, l)
#define THR_UMUTEX_UNLOCK(t, l) b0265_umutex_unlock(t, l)
#define THR_THREAD_UNLOCK(c, t) b0265_thread_unlock(c, t)

extern int errno;
extern struct pthread *_thread_last_event;
extern struct umutex _thr_event_lock;

extern void _pthread_suspend_all_np(void);
extern int _pthread_getschedparam(pthread_t pthread, int *policy,
    struct sched_param *param);
extern struct pthread *_get_curthread(void);
extern int _thr_find_thread(struct pthread *curthread, pthread_t pthread,
    int include_dead);
extern void _thr_try_gc(struct pthread *curthread, pthread_t pthread);
extern void b0265_umutex_lock(struct pthread *t, struct umutex *l);
extern void b0265_umutex_unlock(struct pthread *t, struct umutex *l);
extern void b0265_thread_unlock(struct pthread *c, pthread_t t);
#ifdef __cplusplus
}
#endif
#else /* PBSD_B0265_PORT_INCLUDE */

typedef struct pthread *pthread_t;

struct sched_param {
	int sched_priority;
};

struct thr_event_buf {
	int event;
	uintptr_t th_p;
	uintptr_t data;
};

struct pthread {
	unsigned flags;
	struct pthread *joiner;
	struct thr_event_buf event_buf;
};

struct umutex {
	int dummy;
};

#define TD_CREATE 1
#define TD_DEATH 2
#define THR_FLAGS_DETACHED 0x01000000
#define EINVAL 22

#define THR_UMUTEX_LOCK(t, l) b0265_umutex_lock(t, l)
#define THR_UMUTEX_UNLOCK(t, l) b0265_umutex_unlock(t, l)
#define THR_THREAD_UNLOCK(c, t) b0265_thread_unlock(c, t)

int errno;
struct pthread *_thread_last_event;
struct umutex _thr_event_lock;

static unsigned b0265_suspend_all_count;
static unsigned b0265_umutex_lock_count;
static unsigned b0265_umutex_unlock_count;
static unsigned b0265_thread_unlock_count;
static unsigned b0265_try_gc_count;
static unsigned b0265_bp_create_hook_count;
static unsigned b0265_bp_death_hook_count;
static struct pthread *b0265_last_event_during_bp;
static struct pthread *b0265_curthread;

static int b0265_getschedparam_ret;
static int b0265_getschedparam_errno;
static int b0265_getschedparam_policy_out;
static int b0265_getschedparam_prio_out;
static pthread_t b0265_getschedparam_last_pthread;
static int *b0265_getschedparam_last_policy_ptr;
static struct sched_param *b0265_getschedparam_last_param_ptr;

static int b0265_find_thread_ret;
static struct pthread *b0265_find_last_cur;
static pthread_t b0265_find_last_target;
static int b0265_find_last_include_dead;

static struct pthread *b0265_umutex_lock_last_thread;
static struct umutex *b0265_umutex_lock_last_lock;
static struct pthread *b0265_umutex_unlock_last_thread;
static struct umutex *b0265_umutex_unlock_last_lock;
static struct pthread *b0265_thread_unlock_last_cur;
static pthread_t b0265_thread_unlock_last_target;
static struct pthread *b0265_try_gc_last_cur;
static pthread_t b0265_try_gc_last_target;

void
_pthread_suspend_all_np(void)
{

	b0265_suspend_all_count++;
}

int
_pthread_getschedparam(pthread_t pthread, int *policy, struct sched_param *param)
{

	b0265_getschedparam_last_pthread = pthread;
	b0265_getschedparam_last_policy_ptr = policy;
	b0265_getschedparam_last_param_ptr = param;
	if (policy != NULL)
		*policy = b0265_getschedparam_policy_out;
	if (param != NULL)
		param->sched_priority = b0265_getschedparam_prio_out;
	if (b0265_getschedparam_ret != 0)
		errno = b0265_getschedparam_errno;
	return (b0265_getschedparam_ret);
}

struct pthread *
_get_curthread(void)
{

	return (b0265_curthread);
}

int
_thr_find_thread(struct pthread *curthread, pthread_t pthread, int include_dead)
{

	b0265_find_last_cur = curthread;
	b0265_find_last_target = pthread;
	b0265_find_last_include_dead = include_dead;
	return (b0265_find_thread_ret);
}

void
_thr_try_gc(struct pthread *curthread, pthread_t pthread)
{

	b0265_try_gc_count++;
	b0265_try_gc_last_cur = curthread;
	b0265_try_gc_last_target = pthread;
}

void
b0265_umutex_lock(struct pthread *t, struct umutex *l)
{

	b0265_umutex_lock_count++;
	b0265_umutex_lock_last_thread = t;
	b0265_umutex_lock_last_lock = l;
}

void
b0265_umutex_unlock(struct pthread *t, struct umutex *l)
{

	b0265_umutex_unlock_count++;
	b0265_umutex_unlock_last_thread = t;
	b0265_umutex_unlock_last_lock = l;
}

void
b0265_thread_unlock(struct pthread *c, pthread_t t)
{

	b0265_thread_unlock_count++;
	b0265_thread_unlock_last_cur = c;
	b0265_thread_unlock_last_target = t;
}

void
_thread_bp_create(void)
{

	b0265_bp_create_hook_count++;
	b0265_last_event_during_bp = _thread_last_event;
}

void
_thread_bp_death(void)
{

	b0265_bp_death_hook_count++;
	b0265_last_event_during_bp = _thread_last_event;
}

void
b0265_reset_mocks(void)
{

	b0265_suspend_all_count = 0;
	b0265_umutex_lock_count = 0;
	b0265_umutex_unlock_count = 0;
	b0265_thread_unlock_count = 0;
	b0265_try_gc_count = 0;
	b0265_bp_create_hook_count = 0;
	b0265_bp_death_hook_count = 0;
	b0265_last_event_during_bp = NULL;
	b0265_curthread = NULL;
	b0265_getschedparam_ret = 0;
	b0265_getschedparam_errno = 0;
	b0265_getschedparam_policy_out = 0;
	b0265_getschedparam_prio_out = 0;
	b0265_getschedparam_last_pthread = NULL;
	b0265_getschedparam_last_policy_ptr = NULL;
	b0265_getschedparam_last_param_ptr = NULL;
	b0265_find_thread_ret = 0;
	b0265_find_last_cur = NULL;
	b0265_find_last_target = NULL;
	b0265_find_last_include_dead = 0;
	b0265_umutex_lock_last_thread = NULL;
	b0265_umutex_lock_last_lock = NULL;
	b0265_umutex_unlock_last_thread = NULL;
	b0265_umutex_unlock_last_lock = NULL;
	b0265_thread_unlock_last_cur = NULL;
	b0265_thread_unlock_last_target = NULL;
	b0265_try_gc_last_cur = NULL;
	b0265_try_gc_last_target = NULL;
	_thread_last_event = NULL;
	errno = 0;
}

void
b0265_set_curthread(struct pthread *t)
{

	b0265_curthread = t;
}

void
b0265_set_getschedparam(int ret, int err, int policy, int prio)
{

	b0265_getschedparam_ret = ret;
	b0265_getschedparam_errno = err;
	b0265_getschedparam_policy_out = policy;
	b0265_getschedparam_prio_out = prio;
}

void
b0265_set_find_thread_ret(int ret)
{

	b0265_find_thread_ret = ret;
}

unsigned
b0265_get_suspend_all_count(void)
{

	return (b0265_suspend_all_count);
}

unsigned
b0265_get_umutex_lock_count(void)
{

	return (b0265_umutex_lock_count);
}

unsigned
b0265_get_umutex_unlock_count(void)
{

	return (b0265_umutex_unlock_count);
}

unsigned
b0265_get_thread_unlock_count(void)
{

	return (b0265_thread_unlock_count);
}

unsigned
b0265_get_try_gc_count(void)
{

	return (b0265_try_gc_count);
}

unsigned
b0265_get_bp_create_hook_count(void)
{

	return (b0265_bp_create_hook_count);
}

unsigned
b0265_get_bp_death_hook_count(void)
{

	return (b0265_bp_death_hook_count);
}

struct pthread *
b0265_get_last_event_during_bp(void)
{

	return (b0265_last_event_during_bp);
}

struct pthread *
b0265_get_thread_last_event(void)
{

	return (_thread_last_event);
}

pthread_t
b0265_get_getschedparam_last_pthread(void)
{

	return (b0265_getschedparam_last_pthread);
}

int *
b0265_get_getschedparam_last_policy_ptr(void)
{

	return (b0265_getschedparam_last_policy_ptr);
}

struct sched_param *
b0265_get_getschedparam_last_param_ptr(void)
{

	return (b0265_getschedparam_last_param_ptr);
}

int
b0265_get_find_last_include_dead(void)
{

	return (b0265_find_last_include_dead);
}

struct pthread *
b0265_get_find_last_cur(void)
{

	return (b0265_find_last_cur);
}

pthread_t
b0265_get_find_last_target(void)
{

	return (b0265_find_last_target);
}

struct pthread *
b0265_get_umutex_lock_last_thread(void)
{

	return (b0265_umutex_lock_last_thread);
}

struct umutex *
b0265_get_umutex_lock_last_lock(void)
{

	return (b0265_umutex_lock_last_lock);
}

struct pthread *
b0265_get_umutex_unlock_last_thread(void)
{

	return (b0265_umutex_unlock_last_thread);
}

struct umutex *
b0265_get_umutex_unlock_last_lock(void)
{

	return (b0265_umutex_unlock_last_lock);
}

struct pthread *
b0265_get_thread_unlock_last_cur(void)
{

	return (b0265_thread_unlock_last_cur);
}

pthread_t
b0265_get_thread_unlock_last_target(void)
{

	return (b0265_thread_unlock_last_target);
}

struct pthread *
b0265_get_try_gc_last_cur(void)
{

	return (b0265_try_gc_last_cur);
}

pthread_t
b0265_get_try_gc_last_target(void)
{

	return (b0265_try_gc_last_target);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1996 John Birrell <jb@cimlogic.com.au>.
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
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
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
ref__pthread_single_np(void)
{

	/* Enter single-threaded (non-POSIX) scheduling mode: */
	_pthread_suspend_all_np();
	/*
	 * XXX - Do we want to do this?
	 * __is_threaded = 0;
	 */
	return (0);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1995 John Birrell <jb@cimlogic.com.au>.
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
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JOHN BIRRELL AND CONTRIBUTORS ``AS IS'' AND
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
ref__pthread_getprio(pthread_t pthread)
{
	int policy, ret;
	struct sched_param param;

	if ((ret = _pthread_getschedparam(pthread, &policy, &param)) == 0)
		ret = param.sched_priority;
	else {
		/* Invalid thread: */
		errno = ret;
		ret = -1;
	}

	/* Return the thread priority or an error status: */
	return (ret);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 David Xu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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

void
ref__thread_bp_create(void)
{
}

void
ref__thread_bp_death(void)
{
}

void
ref__thr_report_creation(struct pthread *curthread, struct pthread *newthread)
{
	curthread->event_buf.event = TD_CREATE;
	curthread->event_buf.th_p = (uintptr_t)newthread;
	curthread->event_buf.data = 0;
	THR_UMUTEX_LOCK(curthread, &_thr_event_lock);
	_thread_last_event = curthread;
	_thread_bp_create();
	_thread_last_event = NULL;
	THR_UMUTEX_UNLOCK(curthread, &_thr_event_lock);
}

void
ref__thr_report_death(struct pthread *curthread)
{
	curthread->event_buf.event = TD_DEATH;
	curthread->event_buf.th_p = (uintptr_t)curthread;
	curthread->event_buf.data = 0;
	THR_UMUTEX_LOCK(curthread, &_thr_event_lock);
	_thread_last_event = curthread;
	_thread_bp_death();
	_thread_last_event = NULL;
	THR_UMUTEX_UNLOCK(curthread, &_thr_event_lock);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 David Xu <davidxu@freebsd.org>
 * Copyright (C) 2003 Daniel M. Eischen <deischen@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

int
ref__thr_detach(pthread_t pthread)
{
	struct pthread *curthread = _get_curthread();
	int rval;

	if (pthread == NULL)
		return (EINVAL);

	if ((rval = _thr_find_thread(curthread, pthread,
			/*include dead*/1)) != 0) {
		return (rval);
	}

	/* Check if the thread is already detached or has a joiner. */
	if ((pthread->flags & THR_FLAGS_DETACHED) != 0 ||
	    (pthread->joiner != NULL)) {
		THR_THREAD_UNLOCK(curthread, pthread);
		return (EINVAL);
	}

	/* Flag the thread as detached. */
	pthread->flags |= THR_FLAGS_DETACHED;
	_thr_try_gc(curthread, pthread); /* thread lock released */

	return (0);
}

#endif /* PBSD_B0265_PORT_INCLUDE */
