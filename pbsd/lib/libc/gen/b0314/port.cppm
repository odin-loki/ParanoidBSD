module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

export module pbsd.lib.libc.gen.b0314;

export namespace pbsd::lib_libc_gen::b0314 {

struct statfs {
	char	_pad[64];
};

struct freebsd11_statfs {
	char	_pad[48];
};

#define MNT_NOWAIT	1

#define CPU_SETSIZE	64
typedef unsigned long	__cpu_mask;
#define _NCPUBITS	(sizeof(__cpu_mask) * CHAR_BIT)

typedef struct cpuset {
	__cpu_mask	__bits[(CPU_SETSIZE + _NCPUBITS - 1) / _NCPUBITS];
} cpuset_t;

#define CPU_ISSET(cpu, set)						\
	(((set)->__bits[(cpu) / _NCPUBITS] &				\
	 (1UL << ((cpu) & (_NCPUBITS - 1))) ? 1 : 0))

#define CPU_SET(cpu, set)						\
	((set)->__bits[(cpu) / _NCPUBITS] |=				\
	 (1UL << ((cpu) & (_NCPUBITS - 1))))

#define CPU_CLR(cpu, set)						\
	((set)->__bits[(cpu) / _NCPUBITS] &=				\
	 ~(1UL << ((cpu) & (_NCPUBITS - 1))))

static inline int
_cpuset_find_first(const cpuset_t *set)
{
	int cpu;

	for (cpu = 0; cpu < CPU_SETSIZE; cpu++)
		if (CPU_ISSET(cpu, set))
			return cpu;
	return CPU_SETSIZE;
}

static inline int
_cpuset_find_next(const cpuset_t *set, int cpu)
{
	for (; cpu < CPU_SETSIZE; cpu++)
		if (CPU_ISSET(cpu, set))
			return cpu;
	return CPU_SETSIZE;
}

#define CPU_FLS(set)							\
	({								\
		int _idx;							\
		for (_idx = CPU_SETSIZE - 1; _idx >= 0; _idx--)		\
			if (CPU_ISSET(_idx, (set)))				\
				break;						\
		_idx + 1;							\
	})

#define CPU_FOREACH_ISSET(cpu, set)					\
	for ((cpu) = _cpuset_find_first((set));				\
	    (cpu) < CPU_SETSIZE;						\
	    (cpu) = _cpuset_find_next((set), (cpu) + 1))			\
		if (CPU_ISSET((cpu), (set)))

typedef enum {
	CPU_LEVEL_WHICH = 3,
} cpulevel_t;

typedef enum {
	CPU_WHICH_TID = 1,
	CPU_WHICH_PID = 2,
	CPU_WHICH_TIDPID = 3,
} cpuwhich_t;

#define P_OSREL_TIDPID	1400093
#define _PID_MAX	99999

} /* namespace pbsd::lib_libc_gen::b0314 */

extern "C" {
extern int getfsstat(pbsd::lib_libc_gen::b0314::statfs *, long, int);
extern int freebsd11_getfsstat(pbsd::lib_libc_gen::b0314::freebsd11_statfs *,
    long, int);
extern void *reallocf(void *, size_t);
extern int __getosreldate(void);
extern int sysctlbyname(const char *, void *, size_t *, void *, size_t);
extern int cpuset_setaffinity(pbsd::lib_libc_gen::b0314::cpulevel_t,
    pbsd::lib_libc_gen::b0314::cpuwhich_t, id_t, size_t,
    pbsd::lib_libc_gen::b0314::cpuset_t *);
}

export namespace pbsd::lib_libc_gen::b0314 {

#define	MAX_TRIES	3
#define	SCALING_FACTOR	2

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
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

/*
 * Return information about mounted filesystems.
 */
int
getmntinfo(struct statfs **mntbufp, int mode)
{
	static struct statfs *mntbuf;
	static int mntsize;
	static long bufsize;
	unsigned tries = 0;

	if (mntsize <= 0 && (mntsize = getfsstat(0, 0, MNT_NOWAIT)) < 0)
		return (0);
	if (bufsize > 0 && (mntsize = getfsstat(mntbuf, bufsize, mode)) < 0)
		return (0);
	while (tries++ < MAX_TRIES && bufsize <= mntsize * sizeof(*mntbuf)) {
		bufsize = (mntsize * SCALING_FACTOR) * sizeof(*mntbuf);
		if ((mntbuf = (struct statfs *)reallocf(mntbuf, bufsize)) == NULL)
			return (0);
		if ((mntsize = getfsstat(mntbuf, bufsize, mode)) < 0)
			return (0);
	}
	*mntbufp = mntbuf;
	if (mntsize > (bufsize / sizeof(*mntbuf)))
		return (bufsize / sizeof(*mntbuf));
	return (mntsize);
}

/*
 * Copyright (c) 1989, 1993
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

/*
 * Return information about mounted filesystems.
 */
int
freebsd11_getmntinfo(struct freebsd11_statfs **mntbufp, int flags)
{
	static struct freebsd11_statfs *mntbuf;
	static int mntsize;
	static long bufsize;

	if (mntsize <= 0 &&
	    (mntsize = freebsd11_getfsstat(0, 0, MNT_NOWAIT)) < 0)
		return (0);
	if (bufsize > 0 &&
	    (mntsize = freebsd11_getfsstat(mntbuf, bufsize, flags)) < 0)
		return (0);
	while (bufsize <= mntsize * sizeof(struct freebsd11_statfs)) {
		if (mntbuf)
			free(mntbuf);
		bufsize = (mntsize + 1) * sizeof(struct freebsd11_statfs);
		if ((mntbuf = (struct freebsd11_statfs *)malloc(bufsize)) == 0)
			return (0);
		if ((mntsize = freebsd11_getfsstat(mntbuf, bufsize, flags)) < 0)
			return (0);
	}
	*mntbufp = mntbuf;
	return (mntsize);
}

/*-
 * Copyright (c) 2021 The FreeBSD Foundation
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
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
sched_setaffinity(pid_t pid, size_t cpusetsz, const cpuset_t *cpuset)
{
	static int mp_maxid;
	cpuwhich_t which;
	cpuset_t c;
	int error, lbs, cpu;
	size_t len, sz;

	if (__getosreldate() < P_OSREL_TIDPID) {
		if (pid == 0 || pid > _PID_MAX)
			which = CPU_WHICH_TID;
		else
			which = CPU_WHICH_PID;
	} else
		which = CPU_WHICH_TIDPID;

	sz = cpusetsz > sizeof(cpuset_t) ? sizeof(cpuset_t) : cpusetsz;
	memset(&c, 0, sizeof(c));
	memcpy(&c, cpuset, sz);

	/* Linux ignores high bits */
	if (mp_maxid == 0) {
		len = sizeof(mp_maxid);
		error = sysctlbyname("kern.smp.maxid", &mp_maxid, &len,
		    NULL, 0);
		if (error == -1)
			return (error);
	}
	lbs = CPU_FLS(&c) - 1;
	if (lbs > mp_maxid) {
		CPU_FOREACH_ISSET(cpu, &c)
			if (cpu > mp_maxid)
				CPU_CLR(cpu, &c);
	}
	error = cpuset_setaffinity(CPU_LEVEL_WHICH, which,
	    pid == 0 ? -1 : pid, sizeof(cpuset_t), &c);
	if (error == -1 && errno == EDEADLK)
		errno = EINVAL;

	return (error);
}

} /* namespace pbsd::lib_libc_gen::b0314 */
