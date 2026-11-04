module;

#define PBSD_B0278_PORT_INCLUDE
#include "oracle.c"
#undef PBSD_B0278_PORT_INCLUDE

export module pbsd.lib.libthr.thread.b0278;

export namespace pbsd::lib_libthr_thread::b0278 {

typedef struct pthread *pthread_t;

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
_pthread_setaffinity_np(pthread_t td, size_t cpusetsize, const cpuset_t *cpusetp)
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
_pthread_getaffinity_np(pthread_t td, size_t cpusetsize, cpuset_t *cpusetp)
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
_pthread_rwlockattr_destroy(pthread_rwlockattr_t *rwlockattr)
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
_pthread_rwlockattr_getpshared(
    const pthread_rwlockattr_t * __restrict rwlockattr,
    int * __restrict pshared)
{

	*pshared = (*rwlockattr)->pshared;
	return (0);
}

int
_pthread_rwlockattr_init(pthread_rwlockattr_t *rwlockattr)
{
	pthread_rwlockattr_t prwlockattr;

	if (rwlockattr == NULL)
		return (EINVAL);

	prwlockattr = (pthread_rwlockattr_t)malloc(sizeof(struct pthread_rwlockattr));
	if (prwlockattr == NULL)
		return (ENOMEM);

	prwlockattr->pshared = PTHREAD_PROCESS_PRIVATE;
	*rwlockattr = prwlockattr;
	return (0);
}

int
_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *rwlockattr, int pshared)
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
_pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	free(*attr);
	return (0);
}

int
_pthread_barrierattr_getpshared(const pthread_barrierattr_t * __restrict attr,
    int * __restrict pshared)
{

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	*pshared = (*attr)->pshared;
	return (0);
}

int
_pthread_barrierattr_init(pthread_barrierattr_t *attr)
{

	if (attr == NULL)
		return (EINVAL);

	if ((*attr = (pthread_barrierattr_t)malloc(sizeof(struct pthread_barrierattr))) == NULL)
		return (ENOMEM);

	(*attr)->pshared = PTHREAD_PROCESS_PRIVATE;
	return (0);
}

int
_pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
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
_pthread_setschedparam(pthread_t pthread, int policy, 
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

} /* namespace */
