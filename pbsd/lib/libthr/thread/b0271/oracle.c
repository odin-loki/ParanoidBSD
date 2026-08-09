
/*
 * Batch b0271 oracle: original HardenedBSD C sources concatenated.
 * Every function renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef PBSD_B0271_PORT_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif
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

#define SCHED_OTHER 0
#define EINVAL 22
#define ENOTSUP 95
#define _SIG_MAXSIG 64

#define THR_LOCK(t) b0271_thr_lock(t)
#define THR_THREAD_UNLOCK(c, t) b0271_thread_unlock(c, t)

typedef void (*pthread_switch_routine_t)(pthread_t, pthread_t);

extern int errno;
extern struct pthread *_get_curthread(void);
extern int _thr_find_thread(struct pthread *curthread, pthread_t pthread,
    int include_dead);
extern int _thr_setscheduler(long tid, int policy, struct sched_param *param);
extern void _thr_send_sig(pthread_t pthread, int sig);
extern void b0271_thr_lock(struct pthread *t);
extern void b0271_thread_unlock(struct pthread *c, pthread_t t);
#ifdef __cplusplus
}
#endif
#else /* PBSD_B0271_PORT_INCLUDE */

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

#define SCHED_OTHER 0
#define EINVAL 22
#define ENOTSUP 95
#define _SIG_MAXSIG 64

#define THR_LOCK(t) b0271_thr_lock(t)
#define THR_THREAD_UNLOCK(c, t) b0271_thread_unlock(c, t)

typedef void (*pthread_switch_routine_t)(pthread_t, pthread_t);

int errno;

static struct pthread *b0271_curthread;
static int b0271_find_thread_ret;
static struct pthread *b0271_find_last_cur;
static pthread_t b0271_find_last_target;
static int b0271_find_last_include_dead;

static unsigned b0271_thr_lock_count;
static struct pthread *b0271_thr_lock_last_thread;

static unsigned b0271_thread_unlock_count;
static struct pthread *b0271_thread_unlock_last_cur;
static pthread_t b0271_thread_unlock_last_target;

static int b0271_setscheduler_ret;
static int b0271_setscheduler_errno;
static long b0271_setscheduler_last_tid;
static int b0271_setscheduler_last_policy;
static int b0271_setscheduler_last_prio;

static unsigned b0271_send_sig_count;
static pthread_t b0271_send_sig_last_pthread;
static int b0271_send_sig_last_sig;

struct pthread *
_get_curthread(void)
{

	return (b0271_curthread);
}

int
_thr_find_thread(struct pthread *curthread, pthread_t pthread, int include_dead)
{

	b0271_find_last_cur = curthread;
	b0271_find_last_target = pthread;
	b0271_find_last_include_dead = include_dead;
	return (b0271_find_thread_ret);
}

int
_thr_setscheduler(long tid, int policy, struct sched_param *param)
{

	b0271_setscheduler_last_tid = tid;
	b0271_setscheduler_last_policy = policy;
	b0271_setscheduler_last_prio = param != NULL ? param->sched_priority : 0;
	if (b0271_setscheduler_ret == -1)
		errno = b0271_setscheduler_errno;
	return (b0271_setscheduler_ret);
}

void
_thr_send_sig(pthread_t pthread, int sig)
{

	b0271_send_sig_count++;
	b0271_send_sig_last_pthread = pthread;
	b0271_send_sig_last_sig = sig;
}

void
b0271_thr_lock(struct pthread *t)
{

	b0271_thr_lock_count++;
	b0271_thr_lock_last_thread = t;
}

void
b0271_thread_unlock(struct pthread *c, pthread_t t)
{

	b0271_thread_unlock_count++;
	b0271_thread_unlock_last_cur = c;
	b0271_thread_unlock_last_target = t;
}

void
b0271_reset_mocks(void)
{

	b0271_curthread = NULL;
	b0271_find_thread_ret = 0;
	b0271_find_last_cur = NULL;
	b0271_find_last_target = NULL;
	b0271_find_last_include_dead = 0;
	b0271_thr_lock_count = 0;
	b0271_thr_lock_last_thread = NULL;
	b0271_thread_unlock_count = 0;
	b0271_thread_unlock_last_cur = NULL;
	b0271_thread_unlock_last_target = NULL;
	b0271_setscheduler_ret = 0;
	b0271_setscheduler_errno = 0;
	b0271_setscheduler_last_tid = 0;
	b0271_setscheduler_last_policy = 0;
	b0271_setscheduler_last_prio = 0;
	b0271_send_sig_count = 0;
	b0271_send_sig_last_pthread = NULL;
	b0271_send_sig_last_sig = 0;
	errno = 0;
}

void
b0271_set_curthread(struct pthread *t)
{

	b0271_curthread = t;
}

void
b0271_set_find_thread_ret(int ret)
{

	b0271_find_thread_ret = ret;
}

void
b0271_set_setscheduler(int ret, int err)
{

	b0271_setscheduler_ret = ret;
	b0271_setscheduler_errno = err;
}

unsigned
b0271_get_thr_lock_count(void)
{

	return (b0271_thr_lock_count);
}

struct pthread *
b0271_get_thr_lock_last_thread(void)
{

	return (b0271_thr_lock_last_thread);
}

unsigned
b0271_get_thread_unlock_count(void)
{

	return (b0271_thread_unlock_count);
}

struct pthread *
b0271_get_thread_unlock_last_cur(void)
{

	return (b0271_thread_unlock_last_cur);
}

pthread_t
b0271_get_thread_unlock_last_target(void)
{

	return (b0271_thread_unlock_last_target);
}

int
b0271_get_find_last_include_dead(void)
{

	return (b0271_find_last_include_dead);
}

struct pthread *
b0271_get_find_last_cur(void)
{

	return (b0271_find_last_cur);
}

pthread_t
b0271_get_find_last_target(void)
{

	return (b0271_find_last_target);
}

long
b0271_get_setscheduler_last_tid(void)
{

	return (b0271_setscheduler_last_tid);
}

int
b0271_get_setscheduler_last_policy(void)
{

	return (b0271_setscheduler_last_policy);
}

int
b0271_get_setscheduler_last_prio(void)
{

	return (b0271_setscheduler_last_prio);
}

unsigned
b0271_get_send_sig_count(void)
{

	return (b0271_send_sig_count);
}

pthread_t
b0271_get_send_sig_last_pthread(void)
{

	return (b0271_send_sig_last_pthread);
}

int
b0271_get_send_sig_last_sig(void)
{

	return (b0271_send_sig_last_sig);
}

/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 2003 Sergey Osokin <osa@FreeBSD.org.ru>.
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
 *      This product includes software developed by Sergey Osokin.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SERGEY OSOKIN AND CONTRIBUTORS ``AS IS'' AND
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

static int current_concurrency = 0;

int
ref__pthread_getconcurrency(void)
{
	return current_concurrency;
}

int
ref__pthread_setconcurrency(int new_level)
{
	int ret;

	if (new_level < 0) {
		ret = EINVAL;
	} else {
		current_concurrency = new_level;
		ret = 0;
	}
	return (ret);
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
ref__pthread_setprio(pthread_t pthread, int prio)
{
	struct pthread	*curthread = _get_curthread();
	struct sched_param	param;
	int	ret;

	param.sched_priority = prio;
	if (pthread == curthread)
		THR_LOCK(curthread);
	else if ((ret = _thr_find_thread(curthread, pthread, /*include dead*/0)))
		return (ret);
	if (pthread->attr.sched_policy == SCHED_OTHER ||
	    pthread->attr.prio == prio) {
		pthread->attr.prio = prio;
		ret = 0;
	} else {
		ret = _thr_setscheduler(pthread->tid,
			pthread->attr.sched_policy, &param);
		if (ret == -1)
			ret = errno;
		else
			pthread->attr.prio = prio;
	}
	THR_THREAD_UNLOCK(curthread, pthread);
	return (ret);
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
ref__pthread_switch_add_np(pthread_switch_routine_t routine __attribute__((unused)))
{
	return (ENOTSUP);
}

int
ref__pthread_switch_delete_np(pthread_switch_routine_t routine __attribute__((unused)))
{
	return (ENOTSUP);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1997 John Birrell <jb@cimlogic.com.au>.
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
ref__Tthr_kill(pthread_t pthread, int sig)
{
	struct pthread *curthread;
	int ret;

	/* Check for invalid signal numbers: */
	if (sig < 0 || sig > _SIG_MAXSIG)
		/* Invalid signal: */
		return (EINVAL);

	curthread = _get_curthread();

	/*
	 * Ensure the thread is in the list of active threads, and the
	 * signal is valid (signal 0 specifies error checking only) and
	 * not being ignored:
	 */
	if (curthread == pthread) {
		if (sig > 0)
			_thr_send_sig(pthread, sig);
		ret = 0;
	} else if ((ret = _thr_find_thread(curthread, pthread,
	    /*include dead*/0)) == 0) {
		if (sig > 0)
			_thr_send_sig(pthread, sig);
		THR_THREAD_UNLOCK(curthread, pthread);
	}

	/* Return the completion status: */
	return (ret);
}

#endif /* PBSD_B0271_PORT_INCLUDE */
