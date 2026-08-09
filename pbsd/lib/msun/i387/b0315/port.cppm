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

module;

#include <cstdint>
#include <cstring>

export module pbsd.lib.msun.i387.b0315;

namespace pbsd::lib_msun_i387::b0315::detail {

using fexcept_t = std::uint16_t;

constexpr int FE_INVALID = 0x01;
constexpr int FE_DENORMAL = 0x02;
constexpr int FE_DIVBYZERO = 0x04;
constexpr int FE_OVERFLOW = 0x08;
constexpr int FE_UNDERFLOW = 0x10;
constexpr int FE_INEXACT = 0x20;
constexpr int FE_ALL_EXCEPT = (FE_DIVBYZERO | FE_DENORMAL | FE_INEXACT |
    FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);

constexpr int FE_TONEAREST = 0x0000;
constexpr int FE_DOWNWARD = 0x0400;
constexpr int FE_UPWARD = 0x0800;
constexpr int FE_TOWARDZERO = 0x0c00;
constexpr int _ROUND_MASK = (FE_TONEAREST | FE_DOWNWARD |
    FE_UPWARD | FE_TOWARDZERO);

constexpr int _SSE_ROUND_SHIFT = 3;
constexpr int _SSE_EMASK_SHIFT = 7;

constexpr std::uint32_t __INITIAL_NPXCW__ = 0x127F;
constexpr std::uint32_t __INITIAL_MXCSR__ = 0x1F80;

struct fenv_t {
	std::uint16_t __control;
	std::uint16_t __mxcsr_hi;
	std::uint16_t __status;
	std::uint16_t __mxcsr_lo;
	std::uint32_t __tag;
	char __other[16];
};

enum __sse_support { __SSE_YES, __SSE_NO, __SSE_UNK };

std::uint32_t sim_x87_control;
std::uint32_t sim_x87_status;
std::uint32_t sim_x87_tag;
char sim_x87_other[16];
std::uint32_t sim_mxcsr;

enum __sse_support __has_sse;

int __test_sse(void);

inline std::uint32_t
get_mxcsr(const fenv_t &env)
{
	return (((std::uint32_t)env.__mxcsr_hi << 16) | env.__mxcsr_lo);
}

inline void
set_mxcsr(fenv_t &env, std::uint32_t x)
{
	env.__mxcsr_hi = (std::uint32_t)(x) >> 16;
	env.__mxcsr_lo = (std::uint16_t)(x);
}

inline void
fnstenv(fenv_t *addr)
{
	addr->__control = (std::uint16_t)sim_x87_control;
	addr->__status = (std::uint16_t)sim_x87_status;
	addr->__tag = sim_x87_tag;
	std::memcpy(addr->__other, sim_x87_other, 16);
	sim_x87_control |= FE_ALL_EXCEPT;
}

inline void
fldenv(const fenv_t *addr)
{
	sim_x87_control = addr->__control;
	sim_x87_status = addr->__status;
	sim_x87_tag = addr->__tag;
	std::memcpy(sim_x87_other, addr->__other, 16);
}

inline void
fldenvx(const fenv_t &env)
{
	sim_x87_control = env.__control;
	sim_x87_status = env.__status;
	sim_x87_tag = env.__tag;
	std::memcpy(sim_x87_other, env.__other, 16);
}

inline void
fnstcw(std::uint16_t *addr)
{
	*addr = (std::uint16_t)sim_x87_control;
}

inline void
fldcw(std::uint16_t *addr)
{
	sim_x87_control = *addr;
}

inline void
fnstsw(std::uint16_t *addr)
{
	*addr = (std::uint16_t)sim_x87_status;
}

inline void
stmxcsr(std::uint32_t *addr)
{
	*addr = sim_mxcsr;
}

inline void
ldmxcsr(std::uint32_t *addr)
{
	sim_mxcsr = *addr;
}

inline void
fnclex()
{
	sim_x87_status &= ~FE_ALL_EXCEPT;
}

inline void
fwait()
{
}

inline int
HAS_SSE()
{
#ifdef __SSE__
	return (1);
#else
	return (__has_sse == __SSE_YES ||
	    (__has_sse == __SSE_UNK && __test_sse()));
#endif
}

inline int
feclearexcept_int(int __excepts)
{
	fenv_t __env;
	std::uint32_t __mxcsr;

	if (__excepts == FE_ALL_EXCEPT) {
		fnclex();
	} else {
		fnstenv(&__env);
		__env.__status &= ~__excepts;
		fldenv(&__env);
	}
	if (HAS_SSE()) {
		stmxcsr(&__mxcsr);
		__mxcsr &= ~__excepts;
		ldmxcsr(&__mxcsr);
	}
	return (0);
}

inline int
fegetexceptflag_int(fexcept_t *__flagp, int __excepts)
{
	std::uint32_t __mxcsr;
	std::uint16_t __status;

	fnstsw(&__status);
	if (HAS_SSE())
		stmxcsr(&__mxcsr);
	else
		__mxcsr = 0;
	*__flagp = (__mxcsr | __status) & __excepts;
	return (0);
}

inline int
fetestexcept_int(int __excepts)
{
	std::uint32_t __mxcsr;
	std::uint16_t __status;

	fnstsw(&__status);
	if (HAS_SSE())
		stmxcsr(&__mxcsr);
	else
		__mxcsr = 0;
	return ((__status | __mxcsr) & __excepts);
}

inline int
fegetround_int(void)
{
	std::uint16_t __control;

	fnstcw(&__control);
	return (__control & _ROUND_MASK);
}

inline int
fesetround_int(int __round)
{
	std::uint32_t __mxcsr;
	std::uint16_t __control;

	if (__round & ~_ROUND_MASK)
		return (-1);

	fnstcw(&__control);
	__control &= ~_ROUND_MASK;
	__control |= __round;
	fldcw(&__control);

	if (HAS_SSE()) {
		stmxcsr(&__mxcsr);
		__mxcsr &= ~(_ROUND_MASK << _SSE_ROUND_SHIFT);
		__mxcsr |= __round << _SSE_ROUND_SHIFT;
		ldmxcsr(&__mxcsr);
	}

	return (0);
}

inline int
fesetenv_int(const fenv_t *__envp)
{
	fenv_t __env = *__envp;
	std::uint32_t __mxcsr;

	__mxcsr = get_mxcsr(__env);
	set_mxcsr(__env, 0xffffffff);
	fldenvx(__env);
	if (HAS_SSE())
		ldmxcsr(&__mxcsr);
	return (0);
}

} /* namespace pbsd::lib_msun_i387::b0315::detail */

export namespace pbsd::lib_msun_i387::b0315 {

using fexcept_t = detail::fexcept_t;
using fenv_t = detail::fenv_t;
enum __sse_support : int {
	__SSE_YES = detail::__SSE_YES,
	__SSE_NO = detail::__SSE_NO,
	__SSE_UNK = detail::__SSE_UNK
};

constexpr int FE_INVALID = detail::FE_INVALID;
constexpr int FE_DENORMAL = detail::FE_DENORMAL;
constexpr int FE_DIVBYZERO = detail::FE_DIVBYZERO;
constexpr int FE_OVERFLOW = detail::FE_OVERFLOW;
constexpr int FE_UNDERFLOW = detail::FE_UNDERFLOW;
constexpr int FE_INEXACT = detail::FE_INEXACT;
constexpr int FE_ALL_EXCEPT = detail::FE_ALL_EXCEPT;
constexpr int FE_TONEAREST = detail::FE_TONEAREST;
constexpr int FE_DOWNWARD = detail::FE_DOWNWARD;
constexpr int FE_UPWARD = detail::FE_UPWARD;
constexpr int FE_TOWARDZERO = detail::FE_TOWARDZERO;

extern const fenv_t __fe_dfl_env;

inline void
test_set_x87_control(std::uint32_t v)
{
	detail::sim_x87_control = v;
}

inline void
test_set_x87_status(std::uint32_t v)
{
	detail::sim_x87_status = v;
}

inline void
test_set_x87_tag(std::uint32_t v)
{
	detail::sim_x87_tag = v;
}

inline void
test_set_x87_other(const char *v)
{
	std::memcpy(detail::sim_x87_other, v, 16);
}

inline void
test_set_mxcsr(std::uint32_t v)
{
	detail::sim_mxcsr = v;
}

inline void
test_set_has_sse(enum __sse_support v)
{
	detail::__has_sse = (detail::__sse_support)v;
}

inline std::uint32_t
test_get_x87_control(void)
{
	return (detail::sim_x87_control);
}

inline std::uint32_t
test_get_x87_status(void)
{
	return (detail::sim_x87_status);
}

inline std::uint32_t
test_get_mxcsr(void)
{
	return (detail::sim_mxcsr);
}

inline int
test_get_has_sse(void)
{
	return ((int)detail::__has_sse);
}

inline void
test_reset(void)
{
	detail::sim_x87_control = detail::__INITIAL_NPXCW__;
	detail::sim_x87_status = 0;
	detail::sim_x87_tag = 0;
	std::memset(detail::sim_x87_other, 0, 16);
	detail::sim_mxcsr = detail::__INITIAL_MXCSR__;
}


inline int
feclearexcept(int excepts)
{
	return (detail::feclearexcept_int(excepts));
}

inline int
fegetexceptflag(fexcept_t *flagp, int excepts)
{
	return (detail::fegetexceptflag_int(flagp, excepts));
}

inline int
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
	fenv_t env;
	std::uint32_t mxcsr;

	detail::fnstenv(&env);
	env.__status &= ~excepts;
	env.__status |= *flagp & excepts;
	detail::fldenv(&env);

	if (detail::HAS_SSE()) {
		detail::stmxcsr(&mxcsr);
		mxcsr &= ~excepts;
		mxcsr |= *flagp & excepts;
		detail::ldmxcsr(&mxcsr);
	}

	return (0);
}

inline int
feraiseexcept(int excepts)
{
	fexcept_t ex = excepts;

	fesetexceptflag(&ex, excepts);
	detail::fwait();
	return (0);
}

inline int
fetestexcept(int excepts)
{
	return (detail::fetestexcept_int(excepts));
}

inline int
fegetround(void)
{
	return (detail::fegetround_int());
}

inline int
fesetround(int round)
{
	return (detail::fesetround_int(round));
}

inline int
fegetenv(fenv_t *envp)
{
	std::uint32_t mxcsr;

	detail::fnstenv(envp);
	/*
	 * fnstenv masks all exceptions, so we need to restore
	 * the old control word to avoid this side effect.
	 */
	detail::fldcw(&envp->__control);
	if (detail::HAS_SSE()) {
		detail::stmxcsr(&mxcsr);
		detail::set_mxcsr(*envp, mxcsr);
	}
	return (0);
}

inline int
feholdexcept(fenv_t *envp)
{
	std::uint32_t mxcsr;

	detail::fnstenv(envp);
	detail::fnclex();
	if (detail::HAS_SSE()) {
		detail::stmxcsr(&mxcsr);
		detail::set_mxcsr(*envp, mxcsr);
		mxcsr &= ~FE_ALL_EXCEPT;
		mxcsr |= FE_ALL_EXCEPT << detail::_SSE_EMASK_SHIFT;
		detail::ldmxcsr(&mxcsr);
	}
	return (0);
}

inline int
fesetenv(const fenv_t *envp)
{
	return (detail::fesetenv_int(envp));
}

inline int
feupdateenv(const fenv_t *envp)
{
	std::uint32_t mxcsr;
	std::uint16_t status;

	detail::fnstsw(&status);
	if (detail::HAS_SSE())
		detail::stmxcsr(&mxcsr);
	else
		mxcsr = 0;
	fesetenv(envp);
	feraiseexcept((mxcsr | status) & FE_ALL_EXCEPT);
	return (0);
}

inline int
__feenableexcept(int mask)
{
	std::uint32_t mxcsr, omask;
	std::uint16_t control;

	mask &= FE_ALL_EXCEPT;
	detail::fnstcw(&control);
	if (detail::HAS_SSE())
		detail::stmxcsr(&mxcsr);
	else
		mxcsr = 0;
	omask = ~(control | mxcsr >> detail::_SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control &= ~mask;
	detail::fldcw(&control);
	if (detail::HAS_SSE()) {
		mxcsr &= ~(mask << detail::_SSE_EMASK_SHIFT);
		detail::ldmxcsr(&mxcsr);
	}
	return (omask);
}

inline int
__fedisableexcept(int mask)
{
	std::uint32_t mxcsr, omask;
	std::uint16_t control;

	mask &= FE_ALL_EXCEPT;
	detail::fnstcw(&control);
	if (detail::HAS_SSE())
		detail::stmxcsr(&mxcsr);
	else
		mxcsr = 0;
	omask = ~(control | mxcsr >> detail::_SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control |= mask;
	detail::fldcw(&control);
	if (detail::HAS_SSE()) {
		mxcsr |= mask << detail::_SSE_EMASK_SHIFT;
		detail::ldmxcsr(&mxcsr);
	}
	return (omask);
}

inline int
feenableexcept(int mask)
{
	return (__feenableexcept(mask));
}

inline int
fedisableexcept(int mask)
{
	return (__fedisableexcept(mask));
}

} /* namespace pbsd::lib_msun_i387::b0315 */

namespace pbsd::lib_msun_i387::b0315::detail {

enum __sse_support __has_sse =
#ifdef __SSE__
	__SSE_YES;
#else
	__SSE_UNK;
#endif

} /* namespace pbsd::lib_msun_i387::b0315::detail */

namespace pbsd::lib_msun_i387::b0315 {

const fenv_t __fe_dfl_env = {
	detail::__INITIAL_NPXCW__,
	0x0000,
	0x0000,
	0x1f80,
	0xffffffff,
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (char)0xff, (char)0xff }
};

int
__test_sse(void)
{
	int flag, nflag;
	int dx_features;

	/* Am I a 486? */
	__asm __volatile("pushfl\n\tpopl %0" : "=mr" (flag));
	nflag = flag ^ 0x200000;
	__asm __volatile("pushl %0\n\tpopfl" : : "g" (nflag));
	__asm __volatile("pushfl\n\tpopl %0" : "=mr" (nflag));
	if (flag != nflag) {
		/* Not a 486, so CPUID should work. */
		__asm __volatile("pushl %%ebx\n\tmovl $1, %%eax\n\t"
				 "cpuid\n\tpopl %%ebx"
				: "=d" (dx_features) : : "eax", "ecx");
		if (dx_features & 0x2000000) {
			detail::__has_sse = detail::__SSE_YES;
			return (1);
		}
	}
	detail::__has_sse = detail::__SSE_NO;
	return (0);
}

} /* namespace pbsd::lib_msun_i387::b0315 */
