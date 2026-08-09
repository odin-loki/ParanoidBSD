/*
 * oracle.c -- PBSD batch b0310 reference (specification) implementation.
 *
 * The original C sources
 *	hbsd/src/lib/libc/gen/_spinlock_stub.c
 *	hbsd/src/lib/libc/gen/closedir.c
 * concatenated, every function renamed with a "ref_" prefix.  No function
 * body has been altered.
 *
 * Both sources include libc-private headers that are not part of the batch
 * (sys/cdefs.h's __weak_reference/__unused, spinlock.h, libc_private.h,
 * gen-private.h, telldir.h).  The declarations they supply are reproduced
 * below; the ported module reproduces the identical declarations, and the
 * harness provides the definitions, so the oracle and the port run against
 * one and the same environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

/* from <sys/cdefs.h> */
#define	__weak_reference(sym, alias)					\
	__asm__(".weak " #alias);					\
	__asm__(".equ "  #alias ", " #sym)
#define	__unused	__attribute__((__unused__))

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

extern interpos_func_t __libc_interposing[];

extern int __isthreaded;

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

void _reclaim_telldir(DIR *);
int _pthread_mutex_lock(pthread_mutex_t *);
int _pthread_mutex_unlock(pthread_mutex_t *);
int _pthread_mutex_destroy(pthread_mutex_t *);
int _close(int);

/* ===================================================================== */
/* hbsd/src/lib/libc/gen/_spinlock_stub.c				 */
/* ===================================================================== */

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

#include <stdio.h>

/* #include "spinlock.h"		-- see reconstruction above */
/* #include "libc_private.h"	-- see reconstruction above */

long ref__atomic_lock_stub(volatile long *);
void ref__spinlock_stub(spinlock_t *);
void ref__spinunlock_stub(spinlock_t *);

__weak_reference(ref__atomic_lock_stub, ref__atomic_lock);

long
ref__atomic_lock_stub(volatile long *lck __unused)
{
	return (0L);
}

#pragma weak ref__spinlock
void
ref__spinlock(spinlock_t *lck)
{

	((void (*)(spinlock_t *lck))__libc_interposing[INTERPOS_spinlock])
	    (lck);

}

#pragma weak ref__spinunlock
void
ref__spinunlock(spinlock_t *lck)
{

	((void (*)(spinlock_t *lck))__libc_interposing[INTERPOS_spinunlock])
	    (lck);

}

void
ref___libc_spinlock_stub(spinlock_t *lck __unused)
{
}

void
ref___libc_spinunlock_stub(spinlock_t *lck __unused)
{
}

/* ===================================================================== */
/* hbsd/src/lib/libc/gen/closedir.c					 */
/* ===================================================================== */

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

/* #include "namespace.h"	-- see reconstruction above */
#include <sys/types.h>
/* #include <dirent.h>		-- DIR comes from the reconstruction above */
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
/* #include "un-namespace.h"	-- see reconstruction above */

/* #include "libc_private.h"	-- see reconstruction above */
/* #include "gen-private.h"	-- see reconstruction above */
/* #include "telldir.h"		-- see reconstruction above */

/*
 * close a directory.
 */
int
ref_fdclosedir(DIR *dirp)
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
ref_closedir(DIR *dirp)
{
	return (_close(ref_fdclosedir(dirp)));
}
