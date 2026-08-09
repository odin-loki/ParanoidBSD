/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004-2005 David Schultz <das@FreeBSD.ORG>
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
 * Reference oracle for PBSD batch b0250.
 *
 * The original hbsd/src/lib/msun/amd64/fenv.c concatenated, with every
 * function renamed with a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * fenv.c delegates to __fe*_int from fenv.h and uses x87/SSE instructions
 * that are modelled here with software state faithful to fenv.h.  Only the
 * instruction macros differ; every __fe*_int body matches fenv.h verbatim.
 */

#include <stdint.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

typedef	uint16_t	fexcept_t;

/* Exception flags */
#define	FE_INVALID	0x01
#define	FE_DENORMAL	0x02
#define	FE_DIVBYZERO	0x04
#define	FE_OVERFLOW	0x08
#define	FE_UNDERFLOW	0x10
#define	FE_INEXACT	0x20
#define	FE_ALL_EXCEPT	(FE_DIVBYZERO | FE_DENORMAL | FE_INEXACT | \
			 FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW)

/* Rounding modes */
#define	FE_TONEAREST	0x0000
#define	FE_DOWNWARD	0x0400
#define	FE_UPWARD	0x0800
#define	FE_TOWARDZERO	0x0c00
#define	_ROUND_MASK	(FE_TONEAREST | FE_DOWNWARD | \
			 FE_UPWARD | FE_TOWARDZERO)

#define	_SSE_ROUND_SHIFT	3
#define	_SSE_EMASK_SHIFT	7

#define	__INITIAL_FPUCW__	0x037F
#define	__INITIAL_MXCSR__	0x1F80

typedef struct {
	struct {
		uint32_t	__control;
		uint32_t	__status;
		uint32_t	__tag;
		char		__other[16];
	} __x87;
	uint32_t		__mxcsr;
} fenv_t;

static uint32_t sim_x87_control;
static uint32_t sim_x87_status;
static uint32_t sim_x87_tag;
static char sim_x87_other[16];
static uint32_t sim_mxcsr;

void
ref_test_set_x87_control(uint32_t v)
{
	sim_x87_control = v;
}

void
ref_test_set_x87_status(uint32_t v)
{
	sim_x87_status = v;
}

void
ref_test_set_x87_tag(uint32_t v)
{
	sim_x87_tag = v;
}

void
ref_test_set_x87_other(const char *v)
{
	memcpy(sim_x87_other, v, 16);
}

void
ref_test_set_mxcsr(uint32_t v)
{
	sim_mxcsr = v;
}

uint32_t
ref_test_get_x87_control(void)
{
	return (sim_x87_control);
}

uint32_t
ref_test_get_x87_status(void)
{
	return (sim_x87_status);
}

uint32_t
ref_test_get_mxcsr(void)
{
	return (sim_mxcsr);
}

void
ref_test_reset(void)
{
	sim_x87_control = __INITIAL_FPUCW__;
	sim_x87_status = 0;
	sim_x87_tag = 0;
	memset(sim_x87_other, 0, 16);
	sim_mxcsr = __INITIAL_MXCSR__;
}

#define	__fnstenv(addr)							\
do {									\
	(addr)->__control = sim_x87_control;				\
	(addr)->__status = sim_x87_status;				\
	(addr)->__tag = sim_x87_tag;					\
	memcpy((addr)->__other, sim_x87_other, 16);			\
	sim_x87_control |= FE_ALL_EXCEPT;				\
} while (0)

#define	__fldenv(addr)							\
do {									\
	sim_x87_control = (addr)->__control;				\
	sim_x87_status = (addr)->__status;				\
	sim_x87_tag = (addr)->__tag;					\
	memcpy(sim_x87_other, (addr)->__other, 16);			\
} while (0)

#define	__fldenvx(env)							\
do {									\
	sim_x87_control = (env).__control;				\
	sim_x87_status = (env).__status;				\
	sim_x87_tag = (env).__tag;					\
	memcpy(sim_x87_other, (env).__other, 16);			\
} while (0)

#define	__fnstcw(addr)	((*(addr)) = (uint16_t)sim_x87_control)

#define	__fldcw(addr)	(sim_x87_control = (*(addr)))

#define	__fnstsw(addr)	((*(addr)) = (uint16_t)sim_x87_status)

#define	__stmxcsr(addr)	((*(addr)) = sim_mxcsr)

#define	__ldmxcsr(addr)	(sim_mxcsr = (*(addr)))

#define	__fnclex()	(sim_x87_status &= ~FE_ALL_EXCEPT)

#define	__fwait()	((void)0)

static inline int
__feclearexcept_int(int __excepts)
{
	fenv_t __env;

	if (__excepts == FE_ALL_EXCEPT) {
		__fnclex();
	} else {
		__fnstenv(&__env.__x87);
		__env.__x87.__status &= ~__excepts;
		__fldenv(&__env.__x87);
	}
	__stmxcsr(&__env.__mxcsr);
	__env.__mxcsr &= ~__excepts;
	__ldmxcsr(&__env.__mxcsr);
	return (0);
}

static inline int
__fegetexceptflag_int(fexcept_t *__flagp, int __excepts)
{
	uint32_t __mxcsr;
	uint16_t __status;

	__stmxcsr(&__mxcsr);
	__fnstsw(&__status);
	*__flagp = (__mxcsr | __status) & __excepts;
	return (0);
}

static inline int
__fetestexcept_int(int __excepts)
{
	uint32_t __mxcsr;
	uint16_t __status;

	__stmxcsr(&__mxcsr);
	__fnstsw(&__status);
	return ((__status | __mxcsr) & __excepts);
}

static inline int
__fegetround_int(void)
{
	uint16_t __control;

	__fnstcw(&__control);
	return (__control & _ROUND_MASK);
}

static inline int
__fesetround_int(int __round)
{
	uint32_t __mxcsr;
	uint16_t __control;

	if (__round & ~_ROUND_MASK)
		return (-1);

	__fnstcw(&__control);
	__control &= ~_ROUND_MASK;
	__control |= __round;
	__fldcw(&__control);

	__stmxcsr(&__mxcsr);
	__mxcsr &= ~(_ROUND_MASK << _SSE_ROUND_SHIFT);
	__mxcsr |= __round << _SSE_ROUND_SHIFT;
	__ldmxcsr(&__mxcsr);

	return (0);
}

static inline int
__fesetenv_int(const fenv_t *__envp)
{

	__fldenvx(__envp->__x87);
	__ldmxcsr(&__envp->__mxcsr);
	return (0);
}

#define	fesetexceptflag	ref_fesetexceptflag
#define	fesetenv(e)	__fesetenv_int(e)
#define	feraiseexcept(e)	ref_feraiseexcept(e)

const fenv_t ref___fe_dfl_env = {
	{ 0xffff0000 | __INITIAL_FPUCW__,
	  0xffff0000,
	  0xffffffff,
	  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff }
	},
	__INITIAL_MXCSR__
};

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
	fenv_t env;

	__fnstenv(&env.__x87);
	env.__x87.__status &= ~excepts;
	env.__x87.__status |= *flagp & excepts;
	__fldenv(&env.__x87);

	__stmxcsr(&env.__mxcsr);
	env.__mxcsr &= ~excepts;
	env.__mxcsr |= *flagp & excepts;
	__ldmxcsr(&env.__mxcsr);

	return (0);
}

int
ref_feraiseexcept(int excepts)
{
	fexcept_t ex = excepts;

	fesetexceptflag(&ex, excepts);
	__fwait();
	return (0);
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

	__fnstenv(&envp->__x87);
	__stmxcsr(&envp->__mxcsr);
	/*
	 * fnstenv masks all exceptions, so we need to restore the
	 * control word to avoid this side effect.
	 */
	__fldcw(&envp->__x87.__control);
	return (0);
}

int
ref_feholdexcept(fenv_t *envp)
{
	uint32_t mxcsr;

	__stmxcsr(&mxcsr);
	__fnstenv(&envp->__x87);
	__fnclex();
	envp->__mxcsr = mxcsr;
	mxcsr &= ~FE_ALL_EXCEPT;
	mxcsr |= FE_ALL_EXCEPT << _SSE_EMASK_SHIFT;
	__ldmxcsr(&mxcsr);
	return (0);
}

int
ref_fesetenv(const fenv_t *envp)
{
	return (__fesetenv_int(envp));
}

int
ref_feupdateenv(const fenv_t *envp)
{
	uint32_t mxcsr;
	uint16_t status;

	__fnstsw(&status);
	__stmxcsr(&mxcsr);
	fesetenv(envp);
	feraiseexcept((mxcsr | status) & FE_ALL_EXCEPT);
	return (0);
}

#undef	fesetexceptflag
#undef	fesetenv
#undef	feraiseexcept

int
ref___feenableexcept(int mask)
{
	uint32_t mxcsr, omask;
	uint16_t control;

	mask &= FE_ALL_EXCEPT;
	__fnstcw(&control);
	__stmxcsr(&mxcsr);
	omask = ~(control | mxcsr >> _SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control &= ~mask;
	__fldcw(&control);
	mxcsr &= ~(mask << _SSE_EMASK_SHIFT);
	__ldmxcsr(&mxcsr);
	return (omask);
}

int
ref___fedisableexcept(int mask)
{
	uint32_t mxcsr, omask;
	uint16_t control;

	mask &= FE_ALL_EXCEPT;
	__fnstcw(&control);
	__stmxcsr(&mxcsr);
	omask = ~(control | mxcsr >> _SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control |= mask;
	__fldcw(&control);
	mxcsr |= mask << _SSE_EMASK_SHIFT;
	__ldmxcsr(&mxcsr);
	return (omask);
}

int
ref_feenableexcept(int mask)
{
	return (ref___feenableexcept(mask));
}

int
ref_fedisableexcept(int mask)
{
	return (ref___fedisableexcept(mask));
}
