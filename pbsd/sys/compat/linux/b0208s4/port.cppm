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

export module pbsd.sys.compat.linux.b0208s4;

#define __predict_false(exp) __builtin_expect(((exp) != 0), 0)

export namespace pbsd::sys_compat_linux::b0208s4 {

using size_t = __SIZE_TYPE__;
using register_t = long;
using l_ulong = unsigned long;

/* <machine/../linux/linux.h> */
inline constexpr int LINUX_PATH_MAX = 4096;

/* <sys/errno.h> */
inline constexpr int ENOMEM = 12;
inline constexpr int ERANGE = 34;

/* <sys/malloc.h> */
inline constexpr int M_TEMP = 1;
inline constexpr int M_WAITOK = 2;

/* <sys/proc.h> */
struct thread {
	register_t td_retval[2];
};

/* <machine/../linux/linux_proto.h> */
struct linux_getcwd_args {
	char *buf;
	l_ulong bufsize;
};

/*
 * Model of the kernel environment the ported function runs in: the M_TEMP
 * allocator, vn_getcwd() and copyout().  Every input is taken from the cfg_*
 * variables and every side effect is recorded in the obs_* variables so that
 * the differential harness can observe them; the same model is implemented,
 * byte for byte, by oracle.c.
 */
inline constexpr size_t KCAP = 8192;
inline constexpr size_t NOVAL = ~static_cast<size_t>(0);

int cfg_vn_error = 0;
size_t cfg_vn_off = 0;
int cfg_copyout_error = 0;
unsigned cfg_fill_seed = 0;
int cfg_fill_mode = 0;
size_t cfg_ucap = 0;

size_t obs_malloc_calls = 0;
size_t obs_malloc_size = 0;
size_t obs_vn_calls = 0;
size_t obs_vn_in_len = 0;
size_t obs_vn_off_out = 0;
size_t obs_vn_len_out = 0;
size_t obs_copyout_calls = 0;
size_t obs_copyout_src_off = 0;
size_t obs_copyout_len = 0;
size_t obs_copyout_copied = 0;
int obs_copyout_ret = 0;
size_t obs_free_calls = 0;
size_t obs_free_off = 0;
size_t obs_kbuf_len = 0;
unsigned char obs_kbuf[KCAP];

unsigned char kmem[KCAP];
size_t kmem_use = 0;
size_t kmem_hw = 0;

void
env_reset(void)
{
	__builtin_memset(kmem, 0x7f, kmem_hw);
	__builtin_memset(obs_kbuf, 0x7f, kmem_hw);
	kmem_hw = 0;
	kmem_use = 0;
	obs_malloc_calls = 0;
	obs_malloc_size = NOVAL;
	obs_vn_calls = 0;
	obs_vn_in_len = NOVAL;
	obs_vn_off_out = NOVAL;
	obs_vn_len_out = NOVAL;
	obs_copyout_calls = 0;
	obs_copyout_src_off = NOVAL;
	obs_copyout_len = NOVAL;
	obs_copyout_copied = NOVAL;
	obs_copyout_ret = 0x5a5a;
	obs_free_calls = 0;
	obs_free_off = NOVAL;
	obs_kbuf_len = NOVAL;
}

char
env_fill_byte(size_t i, size_t in_len)
{
	switch (cfg_fill_mode) {
	case 0:
		return (char)(cfg_fill_seed + (unsigned)i * 7u + (unsigned)in_len);
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

char *
malloc(size_t size, int type, int flags)
{
	(void)type;
	(void)flags;
	obs_malloc_calls++;
	obs_malloc_size = size;
	kmem_use = size < KCAP ? size : KCAP;
	if (kmem_use > KCAP)
		kmem_use = KCAP;		/* belt and braces: never index past kmem */
	if (kmem_use > kmem_hw)
		kmem_hw = kmem_use;
	if (kmem_hw > KCAP)
		kmem_hw = KCAP;
	return (char *)kmem;
}

void
free(char *addr, int type)
{
	size_t n;

	(void)type;
	obs_free_calls++;
	obs_free_off = (size_t)((unsigned char *)addr - kmem);
	n = kmem_use < KCAP ? kmem_use : KCAP;
	obs_kbuf_len = kmem_use;
	__builtin_memcpy(obs_kbuf, kmem, n);
}

int
vn_getcwd(char *buf, char **retbuf, size_t *buflen)
{
	size_t in_len, n, off, out, i;

	obs_vn_calls++;
	in_len = *buflen;
	obs_vn_in_len = in_len;
	n = in_len < kmem_use ? in_len : kmem_use;
	if (n > KCAP)
		n = KCAP;			/* belt and braces */
	for (i = 0; i < n; i++)
		buf[i] = env_fill_byte(i, in_len);
	off = cfg_vn_off < n ? cfg_vn_off : n;
	out = n - off;
	*retbuf = buf + off;
	*buflen = out;
	obs_vn_off_out = off;
	obs_vn_len_out = out;
	return (cfg_vn_error);
}

int
copyout(const void *kaddr, void *uaddr, size_t len)
{
	size_t off, avail, n;

	obs_copyout_calls++;
	off = (size_t)((const unsigned char *)kaddr - kmem);
	obs_copyout_src_off = off;
	obs_copyout_len = len;
	if (cfg_copyout_error != 0) {
		obs_copyout_copied = 0;
		obs_copyout_ret = cfg_copyout_error;
		return (cfg_copyout_error);
	}
	avail = off < KCAP ? KCAP - off : 0;
	n = len < avail ? len : avail;
	if (n > cfg_ucap)
		n = cfg_ucap;
	__builtin_memcpy(uaddr, kaddr, n);
	obs_copyout_copied = n;
	obs_copyout_ret = 0;
	return (0);
}

/*
 * Find pathname of process's current directory.
 */
int
linux_getcwd(struct thread *td, struct linux_getcwd_args *uap)
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

} /* namespace pbsd::sys_compat_linux::b0208s4 */
