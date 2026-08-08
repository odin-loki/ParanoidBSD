/*
 * b0132 oracle: the original HardenedBSD C sources, concatenated, with every
 * ported function renamed with a `ref_' prefix.  The renaming is performed by
 * the preprocessor so that the function bodies below are byte-for-byte the
 * original text.  Nothing else about the bodies is changed.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/readdir.c
 *   hbsd/src/lib/libc/gen/utxdb.c
 *   hbsd/src/lib/libc/gen/disklabel.c   (gettype() only; see skipped.txt)
 */

/* -std=c11 hides the BSD/GNU extras; htobe32() and friends live behind this. */
#define _DEFAULT_SOURCE 1

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <endian.h>

/*
 * ---------------------------------------------------------------------------
 * Definitions that the original translation units obtained from FreeBSD
 * private headers (sys/dirent.h, gen-private.h, telldir.h, utmpx.h, utxdb.h,
 * sys/param.h).  These are declarations only -- no function body below is
 * altered.  The identical layouts are repeated in port.cppm so that the two
 * implementations can be handed the very same objects.
 * ---------------------------------------------------------------------------
 */

struct dirent {
	uint64_t	d_fileno;
	int64_t		d_off;
	uint16_t	d_reclen;
	uint8_t		d_type;
	uint8_t		d_pad0;
	uint16_t	d_namlen;
	uint16_t	d_pad1;
	char		d_name[256];
};

struct _telldir;

typedef struct _dirdesc {
	int		dd_fd;
	size_t		dd_loc;
	size_t		dd_size;
	char		*dd_buf;
	int		dd_len;
	off_t		dd_seek;
	int		dd_flags;
	pthread_mutex_t	dd_lock;
	struct _telldir	*dd_td;
} DIR;

struct utmpx {
	short		ut_type;
	struct timeval	ut_tv;
	char		ut_id[8];
	pid_t		ut_pid;
	char		ut_user[32];
	char		ut_line[16];
	char		ut_host[128];
	char		__ut_spare[64];
};

struct futx {
	uint8_t		fu_type;
	uint64_t	fu_tv;
	char		fu_id[4];
	uint32_t	fu_pid;
	char		fu_user[32];
	char		fu_line[16];
	char		fu_host[128];
} __attribute__((__packed__));

extern int __isthreaded;
ssize_t _getdirentries(int fd, char *buf, size_t nbytes, off_t *basep);
void _fixtelldir(DIR *dirp, off_t oldseek, size_t oldloc);
int _pthread_mutex_lock(pthread_mutex_t *m);
int _pthread_mutex_unlock(pthread_mutex_t *m);

#define	MIN(a,b)	(((a)<(b))?(a):(b))

#define	DTF_HIDEW	0x0001
#define	DTF_NODUP	0x0002
#define	DTF_REWIND	0x0004
#define	__DTF_READALL	0x0008
#define	__DTF_SKIPREAD	0x0010

#define	RDU_SKIP	0x0001
#define	RDU_SHORT	0x0002

#define	DT_WHT		14

#define	d_ino		d_fileno

#define	_GENERIC_DIRLEN(namlen)						\
	((offsetof(struct dirent, d_name) + (namlen) + 1 + 7) & ~7)
#define	_GENERIC_DIRSIZ(dp)	_GENERIC_DIRLEN((dp)->d_namlen)

#define	EMPTY		0
#define	BOOT_TIME	1
#define	OLD_TIME	2
#define	NEW_TIME	3
#define	USER_PROCESS	4
#define	INIT_PROCESS	5
#define	LOGIN_PROCESS	6
#define	DEAD_PROCESS	7
#define	SHUTDOWN_TIME	8

/* Rename every ported function; the bodies themselves stay untouched. */
#define	_readdir_unlocked	ref__readdir_unlocked
#define	readdir			ref_readdir
#define	__readdir_r		ref___readdir_r
#define	utx_to_futx		ref_utx_to_futx
#define	futx_to_utx		ref_futx_to_utx
#define	gettype			ref_gettype

/*-
 * ===========================================================================
 * lib/libc/gen/readdir.c
 * ===========================================================================
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
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
 * get next entry in a directory.
 */
struct dirent *
_readdir_unlocked(DIR *dirp, int flags)
{
	struct dirent *dp;
	off_t initial_seek;
	size_t initial_loc = 0;
	ssize_t ret;

	for (;;) {
		if (dirp->dd_loc >= dirp->dd_size) {
			if (dirp->dd_flags & __DTF_READALL)
				return (NULL);
			initial_loc = dirp->dd_loc;
			dirp->dd_flags &= ~__DTF_SKIPREAD;
			dirp->dd_loc = 0;
		}
		if (dirp->dd_loc == 0 &&
		    !(dirp->dd_flags & (__DTF_READALL | __DTF_SKIPREAD))) {
			dirp->dd_size = 0;
			initial_seek = dirp->dd_seek;
			ret = _getdirentries(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_len, &dirp->dd_seek);
			if (ret <= 0)
				return (NULL);
			dirp->dd_size = (size_t)ret;
			_fixtelldir(dirp, initial_seek, initial_loc);
		}
		dirp->dd_flags &= ~__DTF_SKIPREAD;
		dp = (struct dirent *)(dirp->dd_buf + dirp->dd_loc);
		if ((long)dp & 03L)	/* bogus pointer check */
			return (NULL);
		if (dp->d_reclen <= 0 ||
		    dp->d_reclen > dirp->dd_len + 1 - dirp->dd_loc)
			return (NULL);
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0 && (flags & RDU_SKIP) != 0)
			continue;
		if (dp->d_type == DT_WHT && (dirp->dd_flags & DTF_HIDEW))
			continue;
		if (dp->d_namlen >= sizeof(dp->d_name) &&
		    (flags & RDU_SHORT) != 0)
			continue;
		return (dp);
	}
}

struct dirent *
readdir(DIR *dirp)
{
	struct dirent *dp;

	if (__isthreaded)
		_pthread_mutex_lock(&dirp->dd_lock);
	dp = _readdir_unlocked(dirp, RDU_SKIP);
	if (__isthreaded)
		_pthread_mutex_unlock(&dirp->dd_lock);
	return (dp);
}

int
__readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
	struct dirent *dp;
	int saved_errno;

	saved_errno = errno;
	errno = 0;
	if (__isthreaded)
		_pthread_mutex_lock(&dirp->dd_lock);
	dp = _readdir_unlocked(dirp, RDU_SKIP | RDU_SHORT);
	if (dp != NULL)
		memcpy(entry, dp, _GENERIC_DIRSIZ(dp));
	if (__isthreaded)
		_pthread_mutex_unlock(&dirp->dd_lock);

	if (errno != 0) {
		if (dp == NULL)
			return (errno);
	} else
		errno = saved_errno;

	if (dp != NULL)
		*result = entry;
	else
		*result = NULL;

	return (0);
}

/*-
 * ===========================================================================
 * lib/libc/gen/utxdb.c
 * ===========================================================================
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010 Ed Schouten <ed@FreeBSD.org>
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

#define	UTOF_STRING(ut, fu, field) do { \
	strncpy((fu)->fu_ ## field, (ut)->ut_ ## field,		\
	    MIN(sizeof (fu)->fu_ ## field, sizeof (ut)->ut_ ## field));	\
} while (0)
#define	UTOF_ID(ut, fu) do { \
	memcpy((fu)->fu_id, (ut)->ut_id,				\
	    MIN(sizeof (fu)->fu_id, sizeof (ut)->ut_id));		\
} while (0)
#define	UTOF_PID(ut, fu) do { \
	(fu)->fu_pid = htobe32((ut)->ut_pid);				\
} while (0)
#define	UTOF_TYPE(ut, fu) do { \
	(fu)->fu_type = (ut)->ut_type;					\
} while (0)
#define	UTOF_TV(fu) do { \
	struct timeval tv;						\
	gettimeofday(&tv, NULL);					\
	(fu)->fu_tv = htobe64((uint64_t)tv.tv_sec * 1000000 +		\
	    (uint64_t)tv.tv_usec);					\
} while (0)

void
utx_to_futx(const struct utmpx *ut, struct futx *fu)
{

	memset(fu, 0, sizeof *fu);

	switch (ut->ut_type) {
	case BOOT_TIME:
	case OLD_TIME:
	case NEW_TIME:
	/* Extension: shutdown time. */
	case SHUTDOWN_TIME:
		break;
	case USER_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_STRING(ut, fu, user);
		UTOF_STRING(ut, fu, line);
		/* Extension: host name. */
		UTOF_STRING(ut, fu, host);
		UTOF_PID(ut, fu);
		break;
	case INIT_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_PID(ut, fu);
		break;
	case LOGIN_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_STRING(ut, fu, user);
		UTOF_STRING(ut, fu, line);
		UTOF_PID(ut, fu);
		break;
	case DEAD_PROCESS:
		UTOF_ID(ut, fu);
		UTOF_PID(ut, fu);
		break;
	default:
		fu->fu_type = EMPTY;
		return;
	}

	UTOF_TYPE(ut, fu);
	UTOF_TV(fu);
}

#define	FTOU_STRING(fu, ut, field) do { \
	strncpy((ut)->ut_ ## field, (fu)->fu_ ## field,		\
	    MIN(sizeof (ut)->ut_ ## field - 1, sizeof (fu)->fu_ ## field)); \
} while (0)
#define	FTOU_ID(fu, ut) do { \
	memcpy((ut)->ut_id, (fu)->fu_id,				\
	    MIN(sizeof (ut)->ut_id, sizeof (fu)->fu_id));		\
} while (0)
#define	FTOU_PID(fu, ut) do { \
	(ut)->ut_pid = be32toh((fu)->fu_pid);				\
} while (0)
#define	FTOU_TYPE(fu, ut) do { \
	(ut)->ut_type = (fu)->fu_type;					\
} while (0)
#define	FTOU_TV(fu, ut) do { \
	uint64_t t;							\
	t = be64toh((fu)->fu_tv);					\
	(ut)->ut_tv.tv_sec = t / 1000000;				\
	(ut)->ut_tv.tv_usec = t % 1000000;				\
} while (0)

struct utmpx *
futx_to_utx(const struct futx *fu)
{
	static _Thread_local struct utmpx *ut;

	if (ut == NULL) {
		ut = calloc(1, sizeof *ut);
		if (ut == NULL)
			return (NULL);
	} else
		memset(ut, 0, sizeof *ut);

	switch (fu->fu_type) {
	case BOOT_TIME:
	case OLD_TIME:
	case NEW_TIME:
	/* Extension: shutdown time. */
	case SHUTDOWN_TIME:
		break;
	case USER_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_STRING(fu, ut, user);
		FTOU_STRING(fu, ut, line);
		/* Extension: host name. */
		FTOU_STRING(fu, ut, host);
		FTOU_PID(fu, ut);
		break;
	case INIT_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_PID(fu, ut);
		break;
	case LOGIN_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_STRING(fu, ut, user);
		FTOU_STRING(fu, ut, line);
		FTOU_PID(fu, ut);
		break;
	case DEAD_PROCESS:
		FTOU_ID(fu, ut);
		FTOU_PID(fu, ut);
		break;
	default:
		ut->ut_type = EMPTY;
		return (ut);
	}

	FTOU_TYPE(fu, ut);
	FTOU_TV(fu, ut);
	return (ut);
}

/*-
 * ===========================================================================
 * lib/libc/gen/disklabel.c  --  gettype() only
 * ===========================================================================
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1987, 1993
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

/* `static' dropped so the differential harness can reach it. */
int
gettype(char *t, const char **names)
{
	const char **nm;

	for (nm = names; *nm; nm++)
		if (strcasecmp(t, *nm) == 0)
			return (nm - names);
	if (isdigit((unsigned char)*t))
		return (atoi(t));
	return (0);
}
