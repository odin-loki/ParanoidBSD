/* $OpenBSD: linux_getcwd.c,v 1.2 2001/05/16 12:50:21 ho Exp $ */
/* $NetBSD: vfs_getcwd.c,v 1.3.2.3 1999/07/11 10:24:09 sommerfeld Exp $ */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * Copyright (c) 2015 The FreeBSD Foundation
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Bill Sommerfeld.
 *
 * Portions of this software were developed by Edward Tomasz Napierala
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Reference oracle: the original bodies, verbatim, with ref_ prefixed names.
 * Everything above ref_linux_getcwd() is the kernel environment the body needs
 * (types, errno values, allocator, vn_getcwd(), copyout()); no libc header is
 * pulled in so that the constants cannot drift from the C++ port's.
 */

typedef __SIZE_TYPE__ size_t;
typedef unsigned long l_ulong;
typedef long register_t;

/* <machine/../linux/linux.h> */
#define LINUX_PATH_MAX	4096

/* <sys/errno.h> */
#define ENOMEM		12
#define ERANGE		34

/* <sys/malloc.h> */
#define M_TEMP		1
#define M_WAITOK	2

#define __predict_false(exp)	__builtin_expect(((exp) != 0), 0)

/* <sys/proc.h> */
struct thread {
	register_t td_retval[2];
};

/* <machine/../linux/linux_proto.h> */
struct linux_getcwd_args {
	char *buf;
	l_ulong bufsize;
};

#define REF_KCAP	8192
#define REF_NOVAL	((size_t)-1)

int ref_cfg_vn_error = 0;
size_t ref_cfg_vn_off = 0;
int ref_cfg_copyout_error = 0;
unsigned ref_cfg_fill_seed = 0;
int ref_cfg_fill_mode = 0;
size_t ref_cfg_ucap = 0;

size_t ref_obs_malloc_calls = 0;
size_t ref_obs_malloc_size = 0;
size_t ref_obs_vn_calls = 0;
size_t ref_obs_vn_in_len = 0;
size_t ref_obs_vn_off_out = 0;
size_t ref_obs_vn_len_out = 0;
size_t ref_obs_copyout_calls = 0;
size_t ref_obs_copyout_src_off = 0;
size_t ref_obs_copyout_len = 0;
size_t ref_obs_copyout_copied = 0;
int ref_obs_copyout_ret = 0;
size_t ref_obs_free_calls = 0;
size_t ref_obs_free_off = 0;
size_t ref_obs_kbuf_len = 0;
unsigned char ref_obs_kbuf[REF_KCAP];

unsigned char ref_kmem[REF_KCAP];
size_t ref_kmem_use = 0;
size_t ref_kmem_hw = 0;

void
ref_env_reset(void)
{
	__builtin_memset(ref_kmem, 0x7f, ref_kmem_hw);
	__builtin_memset(ref_obs_kbuf, 0x7f, ref_kmem_hw);
	ref_kmem_hw = 0;
	ref_kmem_use = 0;
	ref_obs_malloc_calls = 0;
	ref_obs_malloc_size = REF_NOVAL;
	ref_obs_vn_calls = 0;
	ref_obs_vn_in_len = REF_NOVAL;
	ref_obs_vn_off_out = REF_NOVAL;
	ref_obs_vn_len_out = REF_NOVAL;
	ref_obs_copyout_calls = 0;
	ref_obs_copyout_src_off = REF_NOVAL;
	ref_obs_copyout_len = REF_NOVAL;
	ref_obs_copyout_copied = REF_NOVAL;
	ref_obs_copyout_ret = 0x5a5a;
	ref_obs_free_calls = 0;
	ref_obs_free_off = REF_NOVAL;
	ref_obs_kbuf_len = REF_NOVAL;
}

static char
ref_env_fill_byte(size_t i, size_t in_len)
{
	switch (ref_cfg_fill_mode) {
	case 0:
		return (char)(ref_cfg_fill_seed + (unsigned)i * 7u +
		    (unsigned)in_len);
	case 1:
		return (char)0;
	case 2:
		return (char)((i & 1) ? 0xff : 0x00);
	case 3:
		return (char)0x80;
	default:
		return (char)(unsigned)i;
	}
}

static void *
ref_env_malloc(size_t size, int type, int flags)
{
	(void)type;
	(void)flags;
	ref_obs_malloc_calls++;
	ref_obs_malloc_size = size;
	ref_kmem_use = size < REF_KCAP ? size : REF_KCAP;
	if (ref_kmem_use > REF_KCAP)
		ref_kmem_use = REF_KCAP;
	if (ref_kmem_use > ref_kmem_hw)
		ref_kmem_hw = ref_kmem_use;
	if (ref_kmem_hw > REF_KCAP)
		ref_kmem_hw = REF_KCAP;
	return (void *)ref_kmem;
}

static void
ref_env_free(void *addr, int type)
{
	size_t n;

	(void)type;
	ref_obs_free_calls++;
	ref_obs_free_off = (size_t)((unsigned char *)addr - ref_kmem);
	n = ref_kmem_use < REF_KCAP ? ref_kmem_use : REF_KCAP;
	ref_obs_kbuf_len = ref_kmem_use;
	__builtin_memcpy(ref_obs_kbuf, ref_kmem, n);
}

static int
ref_vn_getcwd(char *buf, char **retbuf, size_t *buflen)
{
	size_t in_len, n, off, out, i;

	ref_obs_vn_calls++;
	in_len = *buflen;
	ref_obs_vn_in_len = in_len;
	n = in_len < ref_kmem_use ? in_len : ref_kmem_use;
	if (n > REF_KCAP)
		n = REF_KCAP;
	for (i = 0; i < n; i++)
		buf[i] = ref_env_fill_byte(i, in_len);
	off = ref_cfg_vn_off < n ? ref_cfg_vn_off : n;
	out = n - off;
	*retbuf = buf + off;
	*buflen = out;
	ref_obs_vn_off_out = off;
	ref_obs_vn_len_out = out;
	return (ref_cfg_vn_error);
}

static int
ref_copyout(const void *kaddr, void *uaddr, size_t len)
{
	size_t off, avail, n;

	ref_obs_copyout_calls++;
	off = (size_t)((const unsigned char *)kaddr - ref_kmem);
	ref_obs_copyout_src_off = off;
	ref_obs_copyout_len = len;
	if (ref_cfg_copyout_error != 0) {
		ref_obs_copyout_copied = 0;
		ref_obs_copyout_ret = ref_cfg_copyout_error;
		return (ref_cfg_copyout_error);
	}
	avail = off < REF_KCAP ? REF_KCAP - off : 0;
	n = len < avail ? len : avail;
	if (n > ref_cfg_ucap)
		n = ref_cfg_ucap;
	__builtin_memcpy(uaddr, kaddr, n);
	ref_obs_copyout_copied = n;
	ref_obs_copyout_ret = 0;
	return (0);
}

#define malloc(sz, type, flags)		ref_env_malloc((sz), (type), (flags))
#define free(addr, type)		ref_env_free((addr), (type))
#define vn_getcwd(buf, retbuf, buflen)	ref_vn_getcwd((buf), (retbuf), (buflen))
#define copyout(kaddr, uaddr, len)	ref_copyout((kaddr), (uaddr), (len))

/*
 * Find pathname of process's current directory.
 */
int
ref_linux_getcwd(struct thread *td, struct linux_getcwd_args *uap)
{
	char *buf, *retbuf;
	size_t buflen;
	int error;

	buflen = uap->bufsize;
	if (__predict_false(buflen < 2))
		return (ERANGE);
	if (buflen > LINUX_PATH_MAX)
		buflen = LINUX_PATH_MAX;

	buf = malloc(buflen, M_TEMP, M_WAITOK);
	error = vn_getcwd(buf, &retbuf, &buflen);
	if (error == ENOMEM)
		error = ERANGE;
	if (error == 0) {
		error = copyout(retbuf, uap->buf, buflen);
		if (error == 0)
			td->td_retval[0] = buflen;
	}
	free(buf, M_TEMP);
	return (error);
}
