/*
 * oracle.c -- reference implementation for batch b0279.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.  Only macro definitions
 * and external declarations that the host environment does not provide have
 * been added.
 */

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef MAXLOGNAME
#define	MAXLOGNAME	32
#endif

#define	__ssp_real(x)	x
#define	__sym_compat(a, b, c)
#define	NO_COMPAT7
#define	_WANT_SEMUN

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#if !defined(__FreeBSD__)
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
#endif

static size_t strlcpy(char *, const char *, size_t);
static size_t strlcat(char *, const char *, size_t);

/* ------------------------------------------------------------------ */
/* Harness-controlled mocks                                           */
/* ------------------------------------------------------------------ */

static int mock_issetugid;
static char mock_getenv_buf[64];
static int mock_getenv_set;
static int mock_getlogin_result;
static int mock_getlogin_errno_val;
static char mock_getlogin_name[MAXLOGNAME];
static int mock_semctl_return;
static int mock_semctl_semid;
static int mock_semctl_semnum;
static int mock_semctl_cmd;
static int mock_semctl_had_semun;
static union semun mock_semctl_semun;

void
harness_set_issetugid(int v)
{
	mock_issetugid = v;
}

void
harness_set_getenv_value(const char *v)
{
	if (v == NULL) {
		mock_getenv_set = 0;
		mock_getenv_buf[0] = '\0';
	} else {
		mock_getenv_set = 1;
		strlcpy(mock_getenv_buf, v, sizeof(mock_getenv_buf));
	}
}

void
harness_set_getlogin(int result, int errnov, const char *name)
{
	mock_getlogin_result = result;
	mock_getlogin_errno_val = errnov;
	if (name != NULL)
		strlcpy(mock_getlogin_name, name, sizeof(mock_getlogin_name));
	else
		mock_getlogin_name[0] = '\0';
}

void
harness_set_semctl_return(int rv)
{
	mock_semctl_return = rv;
}

int
harness_last_semctl_semid(void)
{
	return (mock_semctl_semid);
}

int
harness_last_semctl_semnum(void)
{
	return (mock_semctl_semnum);
}

int
harness_last_semctl_cmd(void)
{
	return (mock_semctl_cmd);
}

int
harness_last_semctl_had_semun(void)
{
	return (mock_semctl_had_semun);
}

int
harness_last_semctl_semun_val(void)
{
	return (mock_semctl_semun.val);
}

int
issetugid(void)
{
	return (mock_issetugid);
}

char *
getenv(const char *name)
{
	if (mock_getenv_set == 0)
		return (NULL);
	if (strcmp(name, "EXTERROR_VERBOSE") != 0)
		return (NULL);
	return (mock_getenv_buf);
}

int
_getlogin(char *buf, int len)
{
	if (mock_getlogin_result < 0) {
		errno = mock_getlogin_errno_val;
		return (-1);
	}
	strlcpy(buf, mock_getlogin_name, (size_t)len);
	return (0);
}

int
__semctl(int semid, int semnum, int cmd, union semun *arg)
{
	mock_semctl_semid = semid;
	mock_semctl_semnum = semnum;
	mock_semctl_cmd = cmd;
	if (arg != NULL) {
		mock_semctl_had_semun = 1;
		mock_semctl_semun = *arg;
	} else {
		mock_semctl_had_semun = 0;
	}
	return (mock_semctl_return);
}

int
freebsd7___semctl(int semid, int semnum, int cmd, union semun *arg)
{
	return (__semctl(semid, semnum, cmd, arg));
}

size_t
strlcpy(char *dst, const char *src, size_t len)
{
	size_t srclen;

	srclen = strlen(src);
	if (len != 0) {
		if (srclen >= len) {
			memcpy(dst, src, len - 1);
			dst[len - 1] = '\0';
		} else
			memcpy(dst, src, srclen + 1);
	}
	return (srclen);
}

size_t
strlcat(char *dst, const char *src, size_t len)
{
	size_t dstlen, srclen;

	dstlen = strlen(dst);
	srclen = strlen(src);
	if (dstlen == len)
		return (dstlen + srclen);
	if (srclen < len - dstlen) {
		memcpy(dst + dstlen, src, srclen + 1);
	} else {
		memcpy(dst + dstlen, src, len - dstlen - 1);
		dst[len - 1] = '\0';
	}
	return (dstlen + srclen);
}

int
b0279_c_snprintf(char *buf, size_t bufsz, const char *fmt, uintmax_t p1,
    uintmax_t p2)
{
	return snprintf(buf, bufsz, fmt, p1, p2);
}

int
b0279_c_snprintf_brief(char *lbuf, const char *file, unsigned line)
{
	return snprintf(lbuf, 128, "(src sys/%s:%u)", file, line);
}

int
b0279_c_snprintf_full(char *lbuf, int error, unsigned cat, const char *file,
    unsigned line, uintmax_t p1, uintmax_t p2)
{
	return snprintf(lbuf, 128,
	    "errno %d category %u (src sys/%s:%u) p1 %#jx p2 %#jx",
	    error, cat, file, line, p1, p2);
}

/* ------------------------------------------------------------------ */
/* fpclassify.c                                                       */
/* ------------------------------------------------------------------ */

union IEEEf2bits {
	float	f;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
#else
		unsigned int	sign	:1;
		unsigned int	exp	:8;
		unsigned int	man	:23;
#endif
	} bits;
};

union IEEEd2bits {
	double	d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#else
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
};

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 Mike Barcroft <mike@FreeBSD.org>
 * Copyright (c) 2002, 2003 David Schultz <das@FreeBSD.ORG>
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
ref___fpclassifyf(float f)
{
	union IEEEf2bits u;

	u.f = f;
	if (u.bits.exp == 0) {
		if (u.bits.man == 0)
			return (FP_ZERO);
		return (FP_SUBNORMAL);
	}
	if (u.bits.exp == 255) {
		if (u.bits.man == 0)
			return (FP_INFINITE);
		return (FP_NAN);
	}
	return (FP_NORMAL);
}

int
ref___fpclassifyd(double d)
{
	union IEEEd2bits u;

	u.d = d;
	if (u.bits.exp == 0) {
		if ((u.bits.manl | u.bits.manh) == 0)
			return (FP_ZERO);
		return (FP_SUBNORMAL);
	}
	if (u.bits.exp == 2047) {
		if ((u.bits.manl | u.bits.manh) == 0)
			return (FP_INFINITE);
		return (FP_NAN);
	}
	return (FP_NORMAL);
}

int
ref___fpclassifyl(long double e)
{
	union IEEEl2bits u;

	u.e = e;
	if (u.bits.exp == 0) {
		if ((u.bits.manl | u.bits.manh) == 0)
			return (FP_ZERO);
		return (FP_SUBNORMAL);
	}
	mask_nbit_l(u);		/* Mask normalization bit if applicable. */
#if LDBL_MANT_DIG == 53
	if (u.bits.exp == 2047) {
		if ((u.bits.manl | u.bits.manh) == 0)
			return (FP_INFINITE);
		return (FP_NAN);
	}
#else
	if (u.bits.exp == 32767) {
		if ((u.bits.manl | u.bits.manh) == 0)
			return (FP_INFINITE);
		return (FP_NAN);
	}
#endif
	return (FP_NORMAL);
}

/* ------------------------------------------------------------------ */
/* getlogin.c                                                         */
/* ------------------------------------------------------------------ */

#define	getlogin_r	ref_getlogin_r

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993
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

char *
ref_getlogin(void)
{
	static char logname[MAXLOGNAME];

	if (_getlogin(logname, sizeof(logname)) < 0)
		return (NULL);
	return (logname[0] != '\0' ? logname : NULL);
}

int
ref_getlogin_r(char *logname, size_t namelen)
{
	char tmpname[MAXLOGNAME];
	int	len;

	if (namelen < 1)
		return (ERANGE);
	logname[0] = '\0';

	if (_getlogin(tmpname, sizeof(tmpname)) < 0)
		return (errno);
	len = strlen(tmpname) + 1;
	if (len > namelen)
		return (ERANGE);
	strlcpy(logname, tmpname, len);
	return (0);
}

/* FreeBSD 12 and earlier compat. */
int
ref___getlogin_r_fbsd12(char *logname, int namelen)
{
	if (namelen < 1)
		return (ERANGE);
	return (getlogin_r(logname, namelen));
}

#undef getlogin_r

/* ------------------------------------------------------------------ */
/* semctl.c                                                           */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Doug Rabson
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
 *
 */

int
ref_semctl(int semid, int semnum, int cmd, ...)
{
	va_list ap;
	union semun semun;
	union semun *semun_ptr;

	va_start(ap, cmd);
	if (cmd == IPC_SET || cmd == IPC_STAT || cmd == GETALL
	    || cmd == SETVAL || cmd == SETALL) {
		semun = va_arg(ap, union semun);
		semun_ptr = &semun;
	} else {
		semun_ptr = NULL;
	}
	va_end(ap);

	return (__semctl(semid, semnum, cmd, semun_ptr));
}

/* ------------------------------------------------------------------ */
/* uexterr_format.c                                                   */
/* ------------------------------------------------------------------ */

#define	EXTERR_CAT_MMAP		1
#define	EXTERR_CAT_FILEDESC	2
#define	EXTERR_KTRACE		3
#define	EXTERR_CAT_FUSE_VNOPS	4
#define	EXTERR_CAT_INOTIFY	5
#define	EXTERR_CAT_GENIO	6
#define	EXTERR_CAT_BRIDGE	7
#define	EXTERR_CAT_SWAP		8
#define	EXTERR_CAT_VFSSYSCALL	9
#define	EXTERR_CAT_VFSBIO	10
#define	EXTERR_CAT_GEOMVFS	11
#define	EXTERR_CAT_GEOM		12
#define	EXTERR_CAT_FUSE_VFS	13
#define	EXTERR_CAT_FUSE_DEVICE	14
#define	EXTERR_CAT_FORK		15
#define	EXTERR_CAT_PROCEXIT	16

#define	UEXTERROR_MAXLEN	256

struct uexterror {
	uint32_t ver;
	uint32_t error;
	uint32_t cat;
	uint32_t src_line;
	uint32_t flags;
	uint32_t rsrv0;
	uint64_t p1;
	uint64_t p2;
	uint64_t rsrv1[4];
	char msg[128];
};

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 The FreeBSD Foundation
 * All rights reserved.
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

static const char * const ref_cat_to_filenames[] = {
	[EXTERR_CAT_FUSE_DEVICE] = "fs/fuse/fuse_device.c",
	[EXTERR_CAT_FUSE_VFS] = "fs/fuse/fuse_vfsops.c",
	[EXTERR_CAT_FUSE_VNOPS] = "fs/fuse/fuse_vnops.c",
	[EXTERR_CAT_GEOM] = "geom/geom_subr.c",
	[EXTERR_CAT_GEOMVFS] = "geom/geom_vfs.c",
	[EXTERR_CAT_FILEDESC] = "kern/kern_descrip.c",
	[EXTERR_CAT_PROCEXIT] = "kern/kern_exit.c",
	[EXTERR_CAT_FORK] = "kern/kern_fork.c",
	[EXTERR_CAT_GENIO] = "kern/sys_generic.c",
	[EXTERR_CAT_VFSBIO] = "kern/vfs_bio.c",
	[EXTERR_CAT_INOTIFY] = "kern/vfs_inotify.c",
	[EXTERR_CAT_VFSSYSCALL] = "kern/vfs_syscalls.c",
	[EXTERR_CAT_BRIDGE] = "net/if_bridge.c",
	[EXTERR_CAT_SWAP] = "vm/swap_pager.c",
	[EXTERR_CAT_MMAP] = "vm/vm_mmap.c",
};

static const char *
ref_cat_to_filename(int category)
{
	if (category < 0 || category >= nitems(ref_cat_to_filenames) ||
	    ref_cat_to_filenames[category] == NULL)
		return ("unknown");
	return (ref_cat_to_filenames[category]);
}

static const char ref_exterror_verbose_name[] = "EXTERROR_VERBOSE";
enum ref_exterr_verbose_state {
	EXTERR_VERBOSE_UNKNOWN = 100,
	EXTERR_VERBOSE_DEFAULT,
	EXTERR_VERBOSE_ALLOW_BRIEF,
	EXTERR_VERBOSE_ALLOW_FULL,
};
static enum ref_exterr_verbose_state ref_exterror_verbose = EXTERR_VERBOSE_UNKNOWN;

static void
ref_exterr_verbose_init(void)
{
	const char *v;

	/*
	 * No need to care about thread-safety, the result is
	 * idempotent.
	 */
	if (ref_exterror_verbose != EXTERR_VERBOSE_UNKNOWN)
		return;
	if (issetugid()) {
		ref_exterror_verbose = EXTERR_VERBOSE_DEFAULT;
	} else if ((v = getenv(ref_exterror_verbose_name)) != NULL) {
		ref_exterror_verbose = strcmp(v, "brief") == 0 ?
		    EXTERR_VERBOSE_ALLOW_BRIEF : EXTERR_VERBOSE_ALLOW_FULL;
	} else {
		ref_exterror_verbose = EXTERR_VERBOSE_DEFAULT;
	}
}

int
ref___uexterr_format(const struct uexterror *ue, char *buf, size_t bufsz)
{
	bool has_msg;

	if (bufsz > UEXTERROR_MAXLEN)
		bufsz = UEXTERROR_MAXLEN;
	if (ue->error == 0) {
		strlcpy(buf, "", bufsz);
		return (0);
	}
	ref_exterr_verbose_init();
	has_msg = ue->msg[0] != '\0';

	if (has_msg) {
		snprintf(buf, bufsz, ue->msg, (uintmax_t)ue->p1,
		    (uintmax_t)ue->p2);
	} else {
		strlcpy(buf, "", bufsz);
	}

	if (ref_exterror_verbose > EXTERR_VERBOSE_DEFAULT || !has_msg) {
		char lbuf[128];

#define	SRC_FMT "(src sys/%s:%u)"
		if (ref_exterror_verbose == EXTERR_VERBOSE_ALLOW_BRIEF) {
			snprintf(lbuf, sizeof(lbuf), SRC_FMT,
                            ref_cat_to_filename(ue->cat), ue->src_line);
		} else if (!has_msg ||
		    ref_exterror_verbose == EXTERR_VERBOSE_ALLOW_FULL) {
			snprintf(lbuf, sizeof(lbuf),
			    "errno %d category %u " SRC_FMT " p1 %#jx p2 %#jx",
			    ue->error, ue->cat, ref_cat_to_filename(ue->cat),
			    ue->src_line, (uintmax_t)ue->p1, (uintmax_t)ue->p2);
		}
#undef SRC_FMT
		if (has_msg)
			strlcat(buf, " ", bufsz);
		strlcat(buf, lbuf, bufsz);
	}
	return (0);
}
