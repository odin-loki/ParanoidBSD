
/*
 * Batch b0254 oracle: original HardenedBSD C sources concatenated.
 * Every function renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef PBSD_B0254_PORT_INCLUDE
#ifdef __cplusplus
extern "C" {
#endif
typedef struct pthread *pthread_t;
typedef int clockid_t;
typedef long pid_t;
struct pthread {
	long tid;
};
struct tcb {
	struct pthread *tcb_thread;
};
#define TID(th) ((th)->tid)
#define CPUCLOCK_WHICH_TID 1
#define TLS_TCB_SIZE 16
#define TLS_TCB_ALIGN 16
#define EINVAL 22
extern int errno;
extern int sched_yield(void);
extern int clock_getcpuclockid2(pid_t id, int which, clockid_t *clock_id);
extern void _thr_check_init(void);
extern struct pthread *_get_curthread(void);
extern struct tcb *_tcb_get(void);
extern struct tcb *_rtld_allocate_tls(void *old, size_t size, size_t align);
extern void _rtld_free_tls(struct tcb *tcb, size_t size, size_t align);
#ifdef __cplusplus
}
#endif
#else /* PBSD_B0254_PORT_INCLUDE */

typedef struct pthread *pthread_t;
typedef int clockid_t;
typedef long pid_t;

struct pthread {
	long tid;
};

struct tcb {
	struct pthread *tcb_thread;
};

#define TID(th) ((th)->tid)
#define CPUCLOCK_WHICH_TID 1
#define TLS_TCB_SIZE 16
#define TLS_TCB_ALIGN 16
#define EINVAL 22

int errno;

static unsigned b0254_sched_yield_count;
static int b0254_clock_ret;
static int b0254_clock_errno_set;
static clockid_t b0254_clock_out;
static pid_t b0254_last_tid;
static int b0254_last_which;
static unsigned b0254_check_init_count;
static struct pthread *b0254_curthread;

static struct tcb b0254_tcb_get_slot;
static int b0254_tcb_get_result;
static unsigned b0254_alloc_count;
static unsigned b0254_free_count;
static struct tcb *b0254_last_freed;
static size_t b0254_last_free_size;
static size_t b0254_last_free_align;
static struct tcb b0254_alloc_pool[8];
static unsigned b0254_alloc_pool_used;

int
sched_yield(void)
{

	b0254_sched_yield_count++;
	return (0);
}

int
clock_getcpuclockid2(pid_t id, int which, clockid_t *clock_id)
{

	b0254_last_tid = id;
	b0254_last_which = which;
	if (clock_id != NULL && b0254_clock_ret == 0)
		*clock_id = b0254_clock_out;
	if (b0254_clock_ret != 0)
		errno = b0254_clock_errno_set;
	return (b0254_clock_ret);
}

void
_thr_check_init(void)
{

	b0254_check_init_count++;
}

struct pthread *
_get_curthread(void)
{

	return (b0254_curthread);
}

struct tcb *
_tcb_get(void)
{

	if (b0254_tcb_get_result == 0)
		return (NULL);
	return (&b0254_tcb_get_slot);
}

struct tcb *
_rtld_allocate_tls(void *old, size_t size, size_t align)
{
	struct tcb *tcb;

	(void)old;
	(void)size;
	(void)align;
	if (b0254_alloc_pool_used >= sizeof(b0254_alloc_pool) /
	    sizeof(b0254_alloc_pool[0]))
		return (NULL);
	tcb = &b0254_alloc_pool[b0254_alloc_pool_used++];
	b0254_alloc_count++;
	return (tcb);
}

void
_rtld_free_tls(struct tcb *tcb, size_t size, size_t align)
{

	b0254_free_count++;
	b0254_last_freed = tcb;
	b0254_last_free_size = size;
	b0254_last_free_align = align;
}

void
b0254_reset_mocks(void)
{

	b0254_sched_yield_count = 0;
	b0254_clock_ret = 0;
	b0254_clock_errno_set = 0;
	b0254_clock_out = 0;
	b0254_last_tid = 0;
	b0254_last_which = -1;
	b0254_check_init_count = 0;
	b0254_curthread = NULL;
	b0254_tcb_get_result = 1;
	b0254_tcb_get_slot.tcb_thread = (struct pthread *)0xdeadbeef;
	b0254_alloc_count = 0;
	b0254_free_count = 0;
	b0254_last_freed = NULL;
	b0254_last_free_size = 0;
	b0254_last_free_align = 0;
	b0254_alloc_pool_used = 0;
	errno = 0;
}

void
b0254_set_curthread(struct pthread *t)
{

	b0254_curthread = t;
}

void
b0254_set_clock_getcpuclockid2(int ret, int err, clockid_t out)
{

	b0254_clock_ret = ret;
	b0254_clock_errno_set = err;
	b0254_clock_out = out;
}

void
b0254_set_tcb_get_result(int ok)
{

	b0254_tcb_get_result = ok;
}

unsigned
b0254_get_sched_yield_count(void)
{

	return (b0254_sched_yield_count);
}

unsigned
b0254_get_check_init_count(void)
{

	return (b0254_check_init_count);
}

pid_t
b0254_get_last_tid(void)
{

	return (b0254_last_tid);
}

int
b0254_get_last_which(void)
{

	return (b0254_last_which);
}

unsigned
b0254_get_alloc_count(void)
{

	return (b0254_alloc_count);
}

unsigned
b0254_get_free_count(void)
{

	return (b0254_free_count);
}

struct tcb *
b0254_get_last_freed(void)
{

	return (b0254_last_freed);
}

size_t
b0254_get_last_free_size(void)
{

	return (b0254_last_free_size);
}

size_t
b0254_get_last_free_align(void)
{

	return (b0254_last_free_align);
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

void
ref__pthread_yield(void)
{

	sched_yield();
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 David Xu <davidxu@freebsd.org>
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
ref__pthread_getcpuclockid(pthread_t pthread, clockid_t *clock_id)
{

	if (pthread == NULL)
		return (EINVAL);

	if (clock_getcpuclockid2(TID(pthread), CPUCLOCK_WHICH_TID, clock_id))
		return (errno);
	return (0);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011 Jung-uk Kim <jkim@FreeBSD.org>
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
ref__thr_getthreadid_np(void)
{
	struct pthread *curthread;

	_thr_check_init();
	curthread = _get_curthread();
	return (TID(curthread));
}

/*-
 * Copyright (C) 2003 Jake Burkholder <jake@freebsd.org>
 * Copyright (C) 2003 David Xu <davidxu@freebsd.org>
 * Copyright (c) 2001,2003 Daniel Eischen <deischen@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of its contributors
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

struct tcb *
ref__tcb_ctor(struct pthread *thread, int initial)
{
	struct tcb *tcb;

	if (initial)
		tcb = _tcb_get();
	else
		tcb = _rtld_allocate_tls(NULL, TLS_TCB_SIZE, TLS_TCB_ALIGN);
	if (tcb)
		tcb->tcb_thread = thread;
	return (tcb);
}

void
ref__tcb_dtor(struct tcb *tcb)
{

	_rtld_free_tls(tcb, TLS_TCB_SIZE, TLS_TCB_ALIGN);
}

#endif /* PBSD_B0254_PORT_INCLUDE */
