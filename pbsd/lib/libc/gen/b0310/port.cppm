module;

#include <cstdio>
#include <cstdlib>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

export module pbsd.lib.libc.gen.b0310;

export namespace pbsd::lib_libc_gen::b0310 {

/* from libc's private spinlock.h */
struct _spinlock {
	volatile long	access_lock;
	volatile long	lock_owner;
	volatile char	*fname;
	volatile int	lineno;
};
typedef struct _spinlock spinlock_t;

/* from libc's private libc_private.h */
typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_spinlock	= 3,
	INTERPOS_spinunlock	= 4,
	INTERPOS_MAX		= 8
};

/* from libc's private gen-private.h and telldir.h */
struct _telldir;
struct freebsd11_dirent;

struct _dirdesc {
	int	dd_fd;		/* file descriptor associated with directory */
	long	dd_loc;		/* offset in current buffer */
	long	dd_size;	/* amount of data returned by getdirentries */
	char	*dd_buf;	/* data buffer */
	int	dd_len;		/* size of data buffer */
	off_t	dd_seek;	/* magic cookie returned by getdirentries */
	int	dd_flags;	/* flags for readdir */
	pthread_mutex_t	dd_lock;	/* lock */
	struct _telldir *dd_td;	/* telldir position recording */
	struct freebsd11_dirent *dd_compat_de; /* compat dirent */
};
typedef struct _dirdesc DIR;

} // namespace pbsd::lib_libc_gen::b0310

extern "C" {
using pbsd::lib_libc_gen::b0310::DIR;
using pbsd::lib_libc_gen::b0310::interpos_func_t;
using pbsd::lib_libc_gen::b0310::INTERPOS_spinlock;
using pbsd::lib_libc_gen::b0310::INTERPOS_spinunlock;
using pbsd::lib_libc_gen::b0310::spinlock_t;

extern interpos_func_t __libc_interposing[];
extern int __isthreaded;

void _reclaim_telldir(DIR *);
int _pthread_mutex_lock(pthread_mutex_t *);
int _pthread_mutex_unlock(pthread_mutex_t *);
int _pthread_mutex_destroy(pthread_mutex_t *);
int _close(int);
}

export namespace pbsd::lib_libc_gen::b0310 {

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1998 John Birrell <jb@cimlogic.com.au>.
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

long
_atomic_lock_stub(volatile long *lck __attribute__((__unused__)))
{
	return (0L);
}

long
_atomic_lock(volatile long *lck __attribute__((__unused__)))
{
	return (0L);
}

void
_spinlock(spinlock_t *lck)
{

	((void (*)(spinlock_t *lck))__libc_interposing[INTERPOS_spinlock])
	    (lck);

}

void
_spinunlock(spinlock_t *lck)
{

	((void (*)(spinlock_t *lck))__libc_interposing[INTERPOS_spinunlock])
	    (lck);

}

void
__libc_spinlock_stub(spinlock_t *lck __attribute__((__unused__)))
{
}

void
__libc_spinunlock_stub(spinlock_t *lck __attribute__((__unused__)))
{
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
 *	Regents of the University of California.  All rights reserved.
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
 * close a directory.
 */
int
fdclosedir(DIR *dirp)
{
	int fd;

	if (__isthreaded)
		_pthread_mutex_lock(&dirp->dd_lock);
	fd = dirp->dd_fd;
	dirp->dd_fd = -1;
	dirp->dd_loc = 0;
	free((void *)dirp->dd_buf);
	free(dirp->dd_compat_de);
	_reclaim_telldir(dirp);
	if (__isthreaded) {
		_pthread_mutex_unlock(&dirp->dd_lock);
		_pthread_mutex_destroy(&dirp->dd_lock);
	}
	free((void *)dirp);
	return (fd);
}

int
closedir(DIR *dirp)
{
	return (_close(fdclosedir(dirp)));
}

} // namespace pbsd::lib_libc_gen::b0310
