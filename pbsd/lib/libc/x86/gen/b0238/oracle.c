/*
 * Reference oracle for batch b0238: getcontextx.c with ref_ prefixes.
 * FreeBSD amd64 ucontext types and ifunc shims are reproduced here so the
 * oracle builds on Linux; getcontext, cpuid_count and sysarch are supplied
 * by the harness as deterministic test doubles.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

typedef int64_t __register_t;
typedef uint32_t __uint32_t;
typedef uint16_t __uint16_t;
typedef unsigned int u_int;

typedef struct {
	__uint32_t __bits[4];
} __sigset_t;

struct __mcontext {
	__register_t	mc_onstack;
	__register_t	mc_rdi;
	__register_t	mc_rsi;
	__register_t	mc_rdx;
	__register_t	mc_rcx;
	__register_t	mc_r8;
	__register_t	mc_r9;
	__register_t	mc_rax;
	__register_t	mc_rbx;
	__register_t	mc_rbp;
	__register_t	mc_r10;
	__register_t	mc_r11;
	__register_t	mc_r12;
	__register_t	mc_r13;
	__register_t	mc_r14;
	__register_t	mc_r15;
	__uint32_t	mc_trapno;
	__uint16_t	mc_fs;
	__uint16_t	mc_gs;
	__register_t	mc_addr;
	__uint32_t	mc_flags;
	__uint16_t	mc_es;
	__uint16_t	mc_ds;
	__register_t	mc_err;
	__register_t	mc_rip;
	__register_t	mc_cs;
	__register_t	mc_rflags;
	__register_t	mc_rsp;
	__register_t	mc_ss;
	long	mc_len;
	long	mc_fpformat;
	long	mc_ownedfp;
	long	mc_fpstate[64] __attribute__((__aligned__(16)));
	__register_t	mc_fsbase;
	__register_t	mc_gsbase;
	__register_t	mc_xfpustate;
	__register_t	mc_xfpustate_len;
	__register_t	mc_tlsbase;
	long	mc_spare[3];
};

typedef struct __mcontext mcontext_t;

struct __stack_t {
	void		*ss_sp;
	size_t		ss_size;
	int		ss_flags;
};

typedef struct __ucontext {
	__sigset_t		uc_sigmask;
	mcontext_t		uc_mcontext;
	struct __ucontext	*uc_link;
	struct __stack_t	uc_stack;
	int			uc_flags;
	int			__spare__[4];
} ucontext_t;

struct fpacc87 {
	uint8_t	fp_bytes[10];
};

struct __envxmm64 {
	uint16_t	en_cw;
	uint16_t	en_sw;
	uint8_t		en_tw;
	uint8_t		en_zero;
	uint16_t	en_opcode;
	uint64_t	en_rip;
	uint64_t	en_rdp;
	uint32_t	en_mxcsr;
	uint32_t	en_mxcsr_mask;
};

struct xmmacc {
	uint8_t	xmm_bytes[16];
};

struct savefpu {
	struct __envxmm64	sv_env;
	struct {
		struct fpacc87	fp_acc;
		uint8_t		fp_pad[6];
	} sv_fp[8];
	struct xmmacc		sv_xmm[16];
	uint8_t			sv_pad[96];
} __attribute__((__aligned__(16)));

typedef struct savefpu savex86_t;

struct amd64_get_xfpustate {
	void	*addr;
	int	len;
};

typedef struct amd64_get_xfpustate x86_get_xfpustate_t;

#define	X86_GET_XFPUSTATE	132
#define	CPUID2_OSXSAVE	0x08000000
#define	_MC_HASFPXSTATE	0x4
#define	__weak_reference(sym, alias)

u_int cpu_feature2;

void cpuid_count(unsigned int, unsigned int, unsigned int[4]);
int sysarch(int, void *);
int getcontext(ucontext_t *);

#define	__getcontextx_size_xfpu	ref___getcontextx_size_xfpu
#define	__fillcontextx2_xfpu	ref___fillcontextx2_xfpu
#define	__fillcontextx2_noxfpu	ref___fillcontextx2_noxfpu
#define	__getcontextx_size	ref___getcontextx_size
#define	__fillcontextx2		ref___fillcontextx2
#define	__fillcontextx		ref___fillcontextx

static int xstate_sz = 0;

void
ref_b0238_reset_xstate_sz(void)
{

	xstate_sz = 0;
}

/*
 * Copyright (c) 2011 Konstantin Belousov <kib@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
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

static int
ref___getcontextx_size_xfpu(void)
{

	return (sizeof(ucontext_t) + xstate_sz);
}

int
ref___getcontextx_size(void)
{
	u_int p[4];

	if ((cpu_feature2 & CPUID2_OSXSAVE) != 0) {
		cpuid_count(0xd, 0x0, p);
		xstate_sz = p[1] - sizeof(savex86_t);
	}
	return (__getcontextx_size_xfpu());
}

static int
ref___fillcontextx2_xfpu(char *ctx)
{
	x86_get_xfpustate_t xfpu;
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	xfpu.addr = (char *)(ucp + 1);
	xfpu.len = xstate_sz;
	if (sysarch(X86_GET_XFPUSTATE, &xfpu) == -1)
		return (-1);
	ucp->uc_mcontext.mc_xfpustate = (__register_t)xfpu.addr;
	ucp->uc_mcontext.mc_xfpustate_len = xstate_sz;
	ucp->uc_mcontext.mc_flags |= _MC_HASFPXSTATE;
	return (0);
}

static int
ref___fillcontextx2_noxfpu(char *ctx)
{
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	ucp->uc_mcontext.mc_xfpustate = 0;
	ucp->uc_mcontext.mc_xfpustate_len = 0;
	return (0);
}

int
ref___fillcontextx2(char *ctx)
{

	return ((cpu_feature2 & CPUID2_OSXSAVE) != 0 ? __fillcontextx2_xfpu : 
	    __fillcontextx2_noxfpu)(ctx);
}

int
ref___fillcontextx(char *ctx)
{
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	if (getcontext(ucp) == -1)
		return (-1);
	__fillcontextx2(ctx);
	return (0);
}

__weak_reference(__getcontextx, getcontextx);

ucontext_t *
ref___getcontextx(void)
{
	char *ctx;
	int error;

	ctx = malloc(__getcontextx_size());
	if (ctx == NULL)
		return (NULL);
	if (__fillcontextx(ctx) == -1) {
		error = errno;
		free(ctx);
		errno = error;
		return (NULL);
	}
	return ((ucontext_t *)ctx);
}
