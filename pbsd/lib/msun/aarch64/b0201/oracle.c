/*-
 * Copyright (c) 2004 David Schultz <das@FreeBSD.ORG>
 * Copyright (c) 2013 Andrew Turner <andrew@FreeBSD.ORG>
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

/*
 * Reference oracle for PBSD batch b0201.
 *
 * The original hbsd/src/lib/msun/aarch64/fenv.c concatenated, with every
 * function renamed with a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * fenv.c delegates to __fe*_int from fenv.h.  Those inlines use aarch64
 * "mrs/msr fpcr/fpsr" instructions that cannot be assembled on this host,
 * so the register access macros below substitute software state that is
 * otherwise bit-for-bit faithful to fenv.h.  Only the asm macros differ;
 * every __fe*_int body matches fenv.h verbatim.
 */

#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

/* The high 32 bits contain fpcr, low 32 contain fpsr. */
typedef	uint64_t	fenv_t;
typedef	uint64_t	fexcept_t;

/* Exception flags */
#define	FE_INVALID	0x00000001
#define	FE_DIVBYZERO	0x00000002
#define	FE_OVERFLOW	0x00000004
#define	FE_UNDERFLOW	0x00000008
#define	FE_INEXACT	0x00000010
#define	FE_ALL_EXCEPT	(FE_DIVBYZERO | FE_INEXACT | \
			 FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

#define	FE_TONEAREST	0x0
#define	FE_UPWARD	0x1
#define	FE_DOWNWARD	0x2
#define	FE_TOWARDZERO	0x3
#define	_ROUND_MASK	(FE_TONEAREST | FE_DOWNWARD | \
			 FE_UPWARD | FE_TOWARDZERO)
#define	_ROUND_SHIFT	22

#define _FPUSW_SHIFT	8
#define	_ENABLE_MASK	(FE_ALL_EXCEPT << _FPUSW_SHIFT)

static uint64_t sim_fpcr;
static uint64_t sim_fpsr;

void
ref_test_set_fpcr(uint64_t v)
{
	sim_fpcr = v;
}

void
ref_test_set_fpsr(uint64_t v)
{
	sim_fpsr = v;
}

uint64_t
ref_test_get_fpcr(void)
{
	return (sim_fpcr);
}

uint64_t
ref_test_get_fpsr(void)
{
	return (sim_fpsr);
}

#define	__mrs_fpcr(__r)	((__r) = sim_fpcr)
#define	__msr_fpcr(__r)	(sim_fpcr = (__r))

#define	__mrs_fpsr(__r)	((__r) = sim_fpsr)
#define	__msr_fpsr(__r)	(sim_fpsr = (__r))

static inline int
__feclearexcept_int(int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	__r &= ~__excepts;
	__msr_fpsr(__r);
	return (0);
}

static inline int
__fegetexceptflag_int(fexcept_t *__flagp, int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	*__flagp = __r & __excepts;
	return (0);
}

static inline int
__fesetexceptflag_int(const fexcept_t *__flagp, int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	__r &= ~__excepts;
	__r |= *__flagp & __excepts;
	__msr_fpsr(__r);
	return (0);
}

static inline int
__feraiseexcept_int(int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	__r |= __excepts;
	__msr_fpsr(__r);
	return (0);
}

static inline int
__fetestexcept_int(int __excepts)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	return (__r & __excepts);
}

static inline int
__fegetround_int(void)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	return ((__r >> _ROUND_SHIFT) & _ROUND_MASK);
}

static inline int
__fesetround_int(int __round)
{
	fenv_t __r;

	if (__round & ~_ROUND_MASK)
		return (-1);
	__mrs_fpcr(__r);
	__r &= ~(_ROUND_MASK << _ROUND_SHIFT);
	__r |= __round << _ROUND_SHIFT;
	__msr_fpcr(__r);
	return (0);
}

static inline int
__fegetenv_int(fenv_t *__envp)
{
	uint64_t fpcr;
	uint64_t fpsr;

	__mrs_fpcr(fpcr);
	__mrs_fpsr(fpsr);
	*__envp = fpsr | (fpcr << 32);

	return (0);
}

static inline int
__feholdexcept_int(fenv_t *__envp)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	*__envp = __r << 32;
	__r &= ~(_ENABLE_MASK);
	__msr_fpcr(__r);

	__mrs_fpsr(__r);
	*__envp |= (uint32_t)__r;
	__r &= ~(_ENABLE_MASK);
	__msr_fpsr(__r);
	return (0);
}

static inline int
__fesetenv_int(const fenv_t *__envp)
{

	__msr_fpcr((*__envp) >> 32);
	__msr_fpsr((fenv_t)(uint32_t)*__envp);
	return (0);
}

#define	fesetenv(e)	__fesetenv_int(e)
#define	feraiseexcept(a)	__feraiseexcept_int(a)

static inline int
__feupdateenv_int(const fenv_t *__envp)
{
	fexcept_t __r;

	__mrs_fpsr(__r);
	fesetenv(__envp);
	feraiseexcept(__r & FE_ALL_EXCEPT);
	return (0);
}

#undef	fesetenv
#undef	feraiseexcept

static inline int
__feenableexcept_int(int __mask)
{
	fenv_t __old_r, __new_r;

	__mrs_fpcr(__old_r);
	__new_r = __old_r | ((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	__msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

static inline int
__fedisableexcept_int(int __mask)
{
	fenv_t __old_r, __new_r;

	__mrs_fpcr(__old_r);
	__new_r = __old_r & ~((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	__msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

static inline int
__fegetexcept_int(void)
{
	fenv_t __r;

	__mrs_fpcr(__r);
	return ((__r & _ENABLE_MASK) >> _FPUSW_SHIFT);
}

/*
 * Hopefully the system ID byte is immutable, so it's valid to use
 * this as a default environment.
 */
const fenv_t ref___fe_dfl_env = 0;

int
ref_feclearexcept(int excepts)
{
	return (__feclearexcept_int(excepts));
}

int
ref_fegetexceptflag(fexcept_t *flagp, int excepts)
{
	return (__fegetexceptflag_int(flagp, excepts));
}

int
ref_fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	return (__fesetexceptflag_int(flagp, excepts));
}

int
ref_feraiseexcept(int excepts)
{
	return (__feraiseexcept_int(excepts));
}

int
ref_fetestexcept(int excepts)
{
	return (__fetestexcept_int(excepts));
}

int
ref_fegetround(void)
{
	return (__fegetround_int());
}

int
ref_fesetround(int round)
{
	return (__fesetround_int(round));
}

int
ref_fegetenv(fenv_t *envp)
{
	return (__fegetenv_int(envp));
}

int
ref_feholdexcept(fenv_t *envp)
{
	return (__feholdexcept_int(envp));
}

int
ref_fesetenv(const fenv_t *envp)
{
	return (__fesetenv_int(envp));
}

int
ref_feupdateenv(const fenv_t *envp)
{
	return (__feupdateenv_int(envp));
}

int
ref_feenableexcept(int mask)
{
	return (__feenableexcept_int(mask));
}

int
ref_fedisableexcept(int mask)
{
	return (__fedisableexcept_int(mask));
}

int
ref_fegetexcept(void)
{
	return (__fegetexcept_int());
}
