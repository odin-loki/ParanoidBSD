#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

/*
 * Reference oracle for batch b0304.
 *
 * Original HardenedBSD sources concatenated below.  Every function is renamed
 * with a "ref_" prefix; bodies are otherwise unmodified.  Support declarations
 * replace headers unavailable in this isolated build.
 */

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef __hidden
#define __hidden
#endif

#define __DTF_READALL	0x0008
#define __DTF_SKIPREAD	0x0010

struct pthread_mutex;
struct _telldir;

struct _dirdesc {
	int		dd_fd;
	size_t		dd_loc;
	size_t		dd_size;
	char		*dd_buf;
	int		dd_len;
	off_t		dd_seek;
	int		dd_flags;
	struct pthread_mutex *dd_lock;
	struct _telldir *dd_td;
	void		*dd_compat_de;
};

typedef struct _dirdesc DIR;

extern int __isthreaded;
extern int __libc_sigaction(int, const struct sigaction *, struct sigaction *);
extern void _pthread_mutex_lock(struct pthread_mutex **);
extern void _pthread_mutex_unlock(struct pthread_mutex **);
extern bool _filldir(DIR *, bool);
extern void _reclaim_telldir(DIR *);
extern off_t test_lseek(int, off_t, int);

#define lseek test_lseek

sigset_t _sigintr;

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
 * Set signal state to prevent restart of system calls
 * after an instance of the indicated signal.
 */
int
ref_siginterrupt(int sig, int flag)
{
	extern sigset_t _sigintr __hidden;
	struct sigaction sa;
	int ret;

	if ((ret = __libc_sigaction(sig, (struct sigaction *)0, &sa)) < 0)
		return (ret);
	if (flag) {
		sigaddset(&_sigintr, sig);
		sa.sa_flags &= ~SA_RESTART;
	} else {
		sigdelset(&_sigintr, sig);
		sa.sa_flags |= SA_RESTART;
	}
	return (__libc_sigaction(sig, &sa, (struct sigaction *)0));
}

/*-
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

void
ref_rewinddir(DIR *dirp)
{
	if (__isthreaded)
		_pthread_mutex_lock(&dirp->dd_lock);
	dirp->dd_flags &= ~__DTF_SKIPREAD; /* current contents are invalid */
	if (dirp->dd_flags & __DTF_READALL)
		_filldir(dirp, false);
	else {
		(void) lseek(dirp->dd_fd, 0, SEEK_SET);
		dirp->dd_seek = 0;
	}
	dirp->dd_loc = 0;
	_reclaim_telldir(dirp);
	if (__isthreaded)
		_pthread_mutex_unlock(&dirp->dd_lock);
}
