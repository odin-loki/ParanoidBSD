/*
 * b0222 -- C++23 port of HardenedBSD lib/libc/aarch64/gen/getcontextx.c and
 * makecontext.c.  The original copyright notices are reproduced with each
 * function group below.
 */

module;

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" {
#include <errno.h>

typedef int64_t __register_t;
typedef uint64_t __uint64_t;
typedef uint32_t __uint32_t;
typedef size_t __size_t;

typedef struct __sigset {
	__uint32_t __bits[4];
} __sigset_t;

struct gpregs {
	__register_t	gp_x[30];
	__register_t	gp_lr;
	__register_t	gp_sp;
	__register_t	gp_elr;
	__uint64_t	gp_spsr;
};

struct fpregs {
	unsigned __int128	fp_q[32];
	__uint32_t	fp_sr;
	__uint32_t	fp_cr;
	int		fp_flags;
	int		fp_pad;
};

struct __mcontext {
	struct gpregs	mc_gpregs;
	struct fpregs	mc_fpregs;
	int		mc_flags;
	int		mc_pad;
	__uint64_t	mc_ptr;
	__uint64_t	mc_spare[7];
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

struct arm64_reg_context {
	__uint32_t	ctx_id;
	__uint32_t	ctx_size;
};

#define	ARM64_CTX_END		0xa5a5a5a5

#define	STACKALIGNBYTES	(16 - 1)
#define	STACKALIGN(p)	((uint64_t)(p) & ~STACKALIGNBYTES)

int getcontext(ucontext_t *);
void exit(int);
int setcontext(const ucontext_t *);
void abort(void);
void _ctx_start(void);
}

export module pbsd.lib.libc.aarch64.gen.b0222;

export namespace pbsd::lib_libc_aarch64_gen::b0222 {

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

int
__getcontextx_size(void)
{
	size_t size;

	size = sizeof(ucontext_t);
	size += sizeof(struct arm64_reg_context); /* Space for ARM64_CTX_END */

	return (size);
}

int
__fillcontextx2(char *ctx)
{
	struct arm64_reg_context *reg_ctx;
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	ucp->uc_mcontext.mc_ptr = (uint64_t)(ucp + 1);

	reg_ctx = (struct arm64_reg_context *)ucp->uc_mcontext.mc_ptr;
	reg_ctx->ctx_id = ARM64_CTX_END;
	reg_ctx->ctx_size = sizeof(struct arm64_reg_context);

	return (0);
}

int
__fillcontextx(char *ctx)
{
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	if (getcontext(ucp) == -1)
		return (-1);
	__fillcontextx2(ctx);
	return (0);
}

ucontext_t *
__getcontextx(void)
{
	char *ctx;
	int error;

	ctx = (char *)malloc(__getcontextx_size());
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

/*-
 * Copyright (c) 2015 The FreeBSD Foundation
 *
 * This software was developed by Andrew Turner under
 * sponsorship from the FreeBSD Foundation.
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

void
ctx_done(ucontext_t *ucp)
{

	if (ucp->uc_link == NULL) {
		exit(0);
	} else {
		setcontext((const ucontext_t *)ucp->uc_link);
		abort();
	}
}

void
__makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...)
{
	struct gpregs *gp;
	va_list ap;
	int i;

	/* A valid context is required. */
	if (ucp == NULL)
		return;

	if ((argc < 0) || (argc > 8))
		return;

	gp = &ucp->uc_mcontext.mc_gpregs;

	va_start(ap, argc);
	/* Pass up to eight arguments in x0-7. */
	for (i = 0; i < argc && i < 8; i++)
		gp->gp_x[i] = va_arg(ap, uint64_t);
	va_end(ap);

	/* Set the stack */
	gp->gp_sp = STACKALIGN(ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
	/* Arrange for return via the trampoline code. */
	gp->gp_elr = (__register_t)_ctx_start;
	gp->gp_x[19] = (__register_t)func;
	gp->gp_x[20] = (__register_t)ucp;
}

} /* namespace pbsd::lib_libc_aarch64_gen::b0222 */
