
/*
 * Batch b0278 oracle: original HardenedBSD C sources concatenated.
 * Every function renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef PBSD_B0278_PORT_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif

extern void *malloc(size_t);
extern void free(void *);

typedef long lwpid_t;

typedef struct {
	unsigned char bits[128];
} cpuset_t;

typedef struct pthread *pthread_t;

struct sched_param {
	int sched_priority;
};

struct pthread_attr_part {
	int sched_policy;
	int prio;
};

struct pthread {
	struct pthread_attr_part attr;
	long tid;
};

struct pthread_rwlockattr {
	int pshared;
};

typedef struct pthread_rwlockattr *pthread_rwlockattr_t;

struct pthread_barrierattr {
	int pshared;
};

typedef struct pthread_barrierattr *pthread_barrierattr_t;

#define CPU_LEVEL_WHICH 1
#define CPU_WHICH_TID 2
#define TID(td) ((td)->tid)
#define SCHED_OTHER 0
#define EINVAL 22
#define ENOMEM 12
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

#define THR_LOCK(t) b0278_thr_lock(t)
#define THR_THREAD_UNLOCK(c, t) b0278_thread_unlock(c, t)

extern int errno;
extern struct pthread *_get_curthread(void);
extern int _thr_find_thread(struct pthread *curthread, pthread_t pthread,
    int include_dead);
extern int _thr_setscheduler(long tid, int policy,
    const struct sched_param *param);
extern int cpuset_setaffinity(int level, int which, long id, size_t size,
    const cpuset_t *cpuset);
extern int cpuset_getaffinity(int level, int which, long id, size_t size,
    cpuset_t *cpuset);
extern void b0278_thr_lock(struct pthread *t);
extern void b0278_thread_unlock(struct pthread *c, pthread_t t);
#ifdef __cplusplus
}
#endif
#else /* PBSD_B0278_PORT_INCLUDE */

#include <stdlib.h>
#include <string.h>

typedef long lwpid_t;

typedef struct {
	unsigned char bits[128];
} cpuset_t;

typedef struct pthread *pthread_t;

struct sched_param {
	int sched_priority;
};

struct pthread_attr_part {
	int sched_policy;
	int prio;
};

struct pthread {
	struct pthread_attr_part attr;
	long tid;
};

struct pthread_rwlockattr {
	int pshared;
};

typedef struct pthread_rwlockattr *pthread_rwlockattr_t;

struct pthread_barrierattr {
	int pshared;
};

typedef struct pthread_barrierattr *pthread_barrierattr_t;

#define CPU_LEVEL_WHICH 1
#define CPU_WHICH_TID 2
#define TID(td) ((td)->tid)
#define SCHED_OTHER 0
#define EINVAL 22
#define ENOMEM 12
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

#define THR_LOCK(t) b0278_thr_lock(t)
#define THR_THREAD_UNLOCK(c, t) b0278_thread_unlock(c, t)

int errno;

static struct pthread *b0278_curthread;
static int b0278_find_thread_ret;
static struct pthread *b0278_find_last_cur;
static pthread_t b0278_find_last_target;
static int b0278_find_last_include_dead;

static unsigned b0278_thr_lock_count;
static struct pthread *b0278_thr_lock_last_thread;

static unsigned b0278_thread_unlock_count;
static struct pthread *b0278_thread_unlock_last_cur;
static pthread_t b0278_thread_unlock_last_target;

static int b0278_setscheduler_ret;
static int b0278_setscheduler_errno;
static long b0278_setscheduler_last_tid;
static int b0278_setscheduler_last_policy;
static int b0278_setscheduler_last_prio;

static int b0278_setaffinity_ret;
static int b0278_setaffinity_errno;
static int b0278_setaffinity_last_level;
static int b0278_setaffinity_last_which;
static long b0278_setaffinity_last_id;
static size_t b0278_setaffinity_last_size;
static const cpuset_t *b0278_setaffinity_last_cpuset;

static int b0278_getaffinity_ret;
static int b0278_getaffinity_errno;
static int b0278_getaffinity_last_level;
static int b0278_getaffinity_last_which;
static long b0278_getaffinity_last_id;
static size_t b0278_getaffinity_last_size;
static unsigned char b0278_getaffinity_fill_byte;

struct pthread *
_get_curthread(void)
{

	return (b0278_curthread);
}

int
_thr_find_thread(struct pthread *curthread, pthread_t pthread, int include_dead)
{

	b0278_find_last_cur = curthread;
	b0278_find_last_target = pthread;
	b0278_find_last_include_dead = include_dead;
	return (b0278_find_thread_ret);
}

int
_thr_setscheduler(long tid, int policy, const struct sched_param *param)
{

	b0278_setscheduler_last_tid = tid;
	b0278_setscheduler_last_policy = policy;
	b0278_setscheduler_last_prio = param != NULL ? param->sched_priority : 0;
	if (b0278_setscheduler_ret == -1)
		errno = b0278_setscheduler_errno;
	return (b0278_setscheduler_ret);
}

void
b0278_thr_lock(struct pthread *t)
{

	b0278_thr_lock_count++;
	b0278_thr_lock_last_thread = t;
}

void
b0278_thread_unlock(struct pthread *c, pthread_t t)
{

	b0278_thread_unlock_count++;
	b0278_thread_unlock_last_cur = c;
	b0278_thread_unlock_last_target = t;
}

int
cpuset_setaffinity(int level, int which, long id, size_t size,
    const cpuset_t *cpuset)
{

	b0278_setaffinity_last_level = level;
	b0278_setaffinity_last_which = which;
	b0278_setaffinity_last_id = id;
	b0278_setaffinity_last_size = size;
	b0278_setaffinity_last_cpuset = cpuset;
	if (b0278_setaffinity_ret == -1)
		errno = b0278_setaffinity_errno;
	return (b0278_setaffinity_ret);
}

int
cpuset_getaffinity(int level, int which, long id, size_t size, cpuset_t *cpuset)
{

	b0278_getaffinity_last_level = level;
	b0278_getaffinity_last_which = which;
	b0278_getaffinity_last_id = id;
	b0278_getaffinity_last_size = size;
	if (cpuset != NULL && size > 0)
		memset(cpuset, b0278_getaffinity_fill_byte, size);
	if (b0278_getaffinity_ret == -1)
		errno = b0278_getaffinity_errno;
	return (b0278_getaffinity_ret);
}

void
b0278_reset_mocks(void)
{

	b0278_curthread = NULL;
	b0278_find_thread_ret = 0;
	b0278_find_last_cur = NULL;
	b0278_find_last_target = NULL;
	b0278_find_last_include_dead = 0;
	b0278_thr_lock_count = 0;
	b0278_thr_lock_last_thread = NULL;
	b0278_thread_unlock_count = 0;
	b0278_thread_unlock_last_cur = NULL;
	b0278_thread_unlock_last_target = NULL;
	b0278_setscheduler_ret = 0;
	b0278_setscheduler_errno = 0;
	b0278_setscheduler_last_tid = 0;
	b0278_setscheduler_last_policy = 0;
	b0278_setscheduler_last_prio = 0;
	b0278_setaffinity_ret = 0;
	b0278_setaffinity_errno = 0;
	b0278_setaffinity_last_level = 0;
	b0278_setaffinity_last_which = 0;
	b0278_setaffinity_last_id = 0;
	b0278_setaffinity_last_size = 0;
	b0278_setaffinity_last_cpuset = NULL;
	b0278_getaffinity_ret = 0;
	b0278_getaffinity_errno = 0;
	b0278_getaffinity_last_level = 0;
	b0278_getaffinity_last_which = 0;
	b0278_getaffinity_last_id = 0;
	b0278_getaffinity_last_size = 0;
	b0278_getaffinity_fill_byte = 0;
	errno = 0;
}

void
b0278_set_curthread(struct pthread *t)
{

	b0278_curthread = t;
}

void
b0278_set_find_thread_ret(int ret)
{

	b0278_find_thread_ret = ret;
}

void
b0278_set_setscheduler(int ret, int err)
{

	b0278_setscheduler_ret = ret;
	b0278_setscheduler_errno = err;
}

void
b0278_set_setaffinity(int ret, int err)
{

	b0278_setaffinity_ret = ret;
	b0278_setaffinity_errno = err;
}

void
b0278_set_getaffinity(int ret, int err, unsigned char fill)
{

	b0278_getaffinity_ret = ret;
	b0278_getaffinity_errno = err;
	b0278_getaffinity_fill_byte = fill;
}

unsigned
b0278_get_thr_lock_count(void)
{

	return (b0278_thr_lock_count);
}

struct pthread *
b0278_get_thr_lock_last_thread(void)
{

	return (b0278_thr_lock_last_thread);
}

unsigned
b0278_get_thread_unlock_count(void)
{

	return (b0278_thread_unlock_count);
}

struct pthread *
b0278_get_thread_unlock_last_cur(void)
{

	return (b0278_thread_unlock_last_cur);
}

pthread_t
b0278_get_thread_unlock_last_target(void)
{

	return (b0278_thread_unlock_last_target);
}

int
b0278_get_find_last_include_dead(void)
{

	return (b0278_find_last_include_dead);
}

struct pthread *
b0278_get_find_last_cur(void)
{

	return (b0278_find_last_cur);
}

pthread_t
b0278_get_find_last_target(void)
{

	return (b0278_find_last_target);
}

long
b0278_get_setscheduler_last_tid(void)
{

	return (b0278_setscheduler_last_tid);
}

int
b0278_get_setscheduler_last_policy(void)
{

	return (b0278_setscheduler_last_policy);
}

int
b0278_get_setscheduler_last_prio(void)
{

	return (b0278_setscheduler_last_prio);
}

int
b0278_get_setaffinity_last_level(void)
{

	return (b0278_setaffinity_last_level);
}

int
b0278_get_setaffinity_last_which(void)
{

	return (b0278_setaffinity_last_which);
}

long
b0278_get_setaffinity_last_id(void)
{

	return (b0278_setaffinity_last_id);
}

size_t
b0278_get_setaffinity_last_size(void)
{

	return (b0278_setaffinity_last_size);
}

const cpuset_t *
b0278_get_setaffinity_last_cpuset(void)
{

	return (b0278_setaffinity_last_cpuset);
}

int
b0278_get_getaffinity_last_level(void)
{

	return (b0278_getaffinity_last_level);
}

int
b0278_get_getaffinity_last_which(void)
{

	return (b0278_getaffinity_last_which);
}

long
b0278_get_getaffinity_last_id(void)
{

	return (b0278_getaffinity_last_id);
}

size_t
b0278_get_getaffinity_last_size(void)
{

	return (b0278_getaffinity_last_size);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008, David Xu <davidxu@freebsd.org>
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
ref__pthread_setaffinity_np(pthread_t td, size_t cpusetsize, const cpuset_t *cpusetp)
{
	struct pthread	*curthread = _get_curthread();
	lwpid_t		tid;
	int		error;

	if (td == curthread) {
		error = cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID,
			-1, cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
	} else if ((error = _thr_find_thread(curthread, td, 0)) == 0) {
		tid = TID(td);
		error = cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, tid,
			cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
		THR_THREAD_UNLOCK(curthread, td);
	}
	return (error);
}

int
ref__pthread_getaffinity_np(pthread_t td, size_t cpusetsize, cpuset_t *cpusetp)
{
	struct pthread	*curthread = _get_curthread();
	lwpid_t tid;
	int error;

	if (td == curthread) {
		error = cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID,
			-1, cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
	} else if ((error = _thr_find_thread(curthread, td, 0)) == 0) {
		tid = TID(td);
		error = cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, tid,
			    cpusetsize, cpusetp);
		if (error == -1)
			error = errno;
		THR_THREAD_UNLOCK(curthread, td);
	}
	return (error);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1998 Alex Nash
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

int
ref__pthread_rwlockattr_destroy(pthread_rwlockattr_t *rwlockattr)
{
	pthread_rwlockattr_t prwlockattr;

	if (rwlockattr == NULL)
		return (EINVAL);
	prwlockattr = *rwlockattr;
	if (prwlockattr == NULL)
		return (EINVAL);
	free(prwlockattr);
	return (0);
}

int
ref__pthread_rwlockattr_getpshared(
    const pthread_rwlockattr_t * __restrict rwlockattr,
    int * __restrict pshared)
{

	*pshared = (*rwlockattr)->pshared;
	return (0);
}

int
ref__pthread_rwlockattr_init(pthread_rwlockattr_t *rwlockattr)
{
	pthread_rwlockattr_t prwlockattr;

	if (rwlockattr == NULL)
		return (EINVAL);

	prwlockattr = malloc(sizeof(struct pthread_rwlockattr));
	if (prwlockattr == NULL)
		return (ENOMEM);

	prwlockattr->pshared = PTHREAD_PROCESS_PRIVATE;
	*rwlockattr = prwlockattr;
	return (0);
}

int
ref__pthread_rwlockattr_setpshared(pthread_rwlockattr_t *rwlockattr, int pshared)
{

	if (pshared != PTHREAD_PROCESS_PRIVATE &&
	    pshared != PTHREAD_PROCESS_SHARED)
		return (EINVAL);
	(*rwlockattr)->pshared = pshared;
	return (0);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 David Xu <davidxu@freebsd.org>.
 * All rights reserved.
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
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

int
ref__pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	free(*attr);
	return (0);
}

int
ref__pthread_barrierattr_getpshared(const pthread_barrierattr_t * __restrict attr,
    int * __restrict pshared)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	*pshared = (*attr)->pshared;
	return (0);
}

int
ref__pthread_barrierattr_init(pthread_barrierattr_t *attr)
{

	if (attr == NULL)
		return (EINVAL);

	if ((*attr = malloc(sizeof(struct pthread_barrierattr))) == NULL)
		return (ENOMEM);

	(*attr)->pshared = PTHREAD_PROCESS_PRIVATE;
	return (0);
}

int
ref__pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{

	if (attr == NULL || *attr == NULL ||
	    (pshared != PTHREAD_PROCESS_PRIVATE &&
	    pshared != PTHREAD_PROCESS_SHARED))
		return (EINVAL);

	(*attr)->pshared = pshared;
	return (0);
}

/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1998 Daniel Eischen <eischen@vigrid.com>.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Daniel Eischen.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DANIEL EISCHEN AND CONTRIBUTORS ``AS IS'' AND
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
ref__pthread_setschedparam(pthread_t pthread, int policy, 
	const struct sched_param *param)
{
	struct pthread	*curthread = _get_curthread();
	int	ret;

	if (pthread == curthread)
		THR_LOCK(curthread);
	else if ((ret = _thr_find_thread(curthread, pthread,
		 /*include dead*/0)) != 0)
		return (ret);
	if (pthread->attr.sched_policy == policy &&
	    (policy == SCHED_OTHER ||
	     pthread->attr.prio == param->sched_priority)) {
		pthread->attr.prio = param->sched_priority;
		THR_THREAD_UNLOCK(curthread, pthread);
		return (0);
	}
	ret = _thr_setscheduler(pthread->tid, policy, param);
	if (ret == -1)
		ret = errno;
	else {
		pthread->attr.sched_policy = policy;
		pthread->attr.prio = param->sched_priority;
	}
	THR_THREAD_UNLOCK(curthread, pthread);
	return (ret);
}

#endif /* PBSD_B0278_PORT_INCLUDE */
