module;

#include <cerrno>
#include <cfloat>
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

export module pbsd.lib.libc.gen.b0279;

export namespace pbsd::lib_libc_gen::b0279 {

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef MAXLOGNAME
#define	MAXLOGNAME	32
#endif

#define	UEXTERROR_MAXLEN	256

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

struct uexterror {
	std::uint32_t ver;
	std::uint32_t error;
	std::uint32_t cat;
	std::uint32_t src_line;
	std::uint32_t flags;
	std::uint32_t rsrv0;
	std::uint64_t p1;
	std::uint64_t p2;
	std::uint64_t rsrv1[4];
	char msg[128];
};

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

static size_t
strlcpy(char *dst, const char *src, size_t len)
{
	size_t srclen;

	srclen = std::strlen(src);
	if (len != 0) {
		if (srclen >= len) {
			std::memcpy(dst, src, len - 1);
			dst[len - 1] = '\0';
		} else
			std::memcpy(dst, src, srclen + 1);
	}
	return (srclen);
}

static size_t
strlcat(char *dst, const char *src, size_t len)
{
	size_t dstlen, srclen;

	dstlen = std::strlen(dst);
	srclen = std::strlen(src);
	if (dstlen == len)
		return (dstlen + srclen);
	if (srclen < len - dstlen) {
		std::memcpy(dst + dstlen, src, srclen + 1);
	} else {
		std::memcpy(dst + dstlen, src, len - dstlen - 1);
		dst[len - 1] = '\0';
	}
	return (dstlen + srclen);
}

extern "C" {
int issetugid(void);
char *getenv(const char *);
int _getlogin(char *, int);
int __semctl(int, int, int, union semun *);
}

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
__fpclassifyf(float f)
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
__fpclassifyd(double d)
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
__fpclassifyl(long double e)
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
getlogin(void)
{
	static char logname[MAXLOGNAME];

	if (_getlogin(logname, sizeof(logname)) < 0)
		return (NULL);
	return (logname[0] != '\0' ? logname : NULL);
}

int
getlogin_r(char *logname, size_t namelen)
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
__getlogin_r_fbsd12(char *logname, int namelen)
{
	if (namelen < 1)
		return (ERANGE);
	return (getlogin_r(logname, namelen));
}

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
semctl(int semid, int semnum, int cmd, ...)
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

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 The FreeBSD Foundation
 * All rights reserved.
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

static const char * const cat_to_filenames[] = {
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
cat_to_filename(int category)
{
	if (category < 0 || category >= nitems(cat_to_filenames) ||
	    cat_to_filenames[category] == NULL)
		return ("unknown");
	return (cat_to_filenames[category]);
}

static const char exterror_verbose_name[] = "EXTERROR_VERBOSE";
enum exterr_verbose_state {
	EXTERR_VERBOSE_UNKNOWN = 100,
	EXTERR_VERBOSE_DEFAULT,
	EXTERR_VERBOSE_ALLOW_BRIEF,
	EXTERR_VERBOSE_ALLOW_FULL,
};
static enum exterr_verbose_state exterror_verbose = EXTERR_VERBOSE_UNKNOWN;

static void
exterr_verbose_init(void)
{
	const char *v;

	/*
	 * No need to care about thread-safety, the result is
	 * idempotent.
	 */
	if (exterror_verbose != EXTERR_VERBOSE_UNKNOWN)
		return;
	if (issetugid()) {
		exterror_verbose = EXTERR_VERBOSE_DEFAULT;
	} else if ((v = getenv(exterror_verbose_name)) != NULL) {
		exterror_verbose = strcmp(v, "brief") == 0 ?
		    EXTERR_VERBOSE_ALLOW_BRIEF : EXTERR_VERBOSE_ALLOW_FULL;
	} else {
		exterror_verbose = EXTERR_VERBOSE_DEFAULT;
	}
}

int
__uexterr_format(const struct uexterror *ue, char *buf, size_t bufsz)
{
	bool has_msg;

	if (bufsz > UEXTERROR_MAXLEN)
		bufsz = UEXTERROR_MAXLEN;
	if (ue->error == 0) {
		strlcpy(buf, "", bufsz);
		return (0);
	}
	exterr_verbose_init();
	has_msg = ue->msg[0] != '\0';

	if (has_msg) {
		snprintf(buf, bufsz, ue->msg, (uintmax_t)ue->p1,
		    (uintmax_t)ue->p2);
	} else {
		strlcpy(buf, "", bufsz);
	}

	if (exterror_verbose > EXTERR_VERBOSE_DEFAULT || !has_msg) {
		char lbuf[128];

#define	SRC_FMT "(src sys/%s:%u)"
		if (exterror_verbose == EXTERR_VERBOSE_ALLOW_BRIEF) {
			snprintf(lbuf, sizeof(lbuf), SRC_FMT,
                            cat_to_filename(ue->cat), ue->src_line);
		} else if (!has_msg ||
		    exterror_verbose == EXTERR_VERBOSE_ALLOW_FULL) {
			snprintf(lbuf, sizeof(lbuf),
			    "errno %d category %u " SRC_FMT " p1 %#jx p2 %#jx",
			    ue->error, ue->cat, cat_to_filename(ue->cat),
			    ue->src_line, (uintmax_t)ue->p1, (uintmax_t)ue->p2);
		}
#undef SRC_FMT
		if (has_msg)
			strlcat(buf, " ", bufsz);
		strlcat(buf, lbuf, bufsz);
	}
	return (0);
}

} /* namespace */
