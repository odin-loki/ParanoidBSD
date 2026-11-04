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

export module pbsd.lib.msun.amd64.b0250;

namespace pbsd::lib_msun_amd64::b0250::detail {

using fexcept_t = std::uint16_t;

/* Exception flags */
constexpr int FE_INVALID = 0x01;
constexpr int FE_DENORMAL = 0x02;
constexpr int FE_DIVBYZERO = 0x04;
constexpr int FE_OVERFLOW = 0x08;
constexpr int FE_UNDERFLOW = 0x10;
constexpr int FE_INEXACT = 0x20;
constexpr int FE_ALL_EXCEPT = (FE_DIVBYZERO | FE_DENORMAL | FE_INEXACT |
    FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);

/* Rounding modes */
constexpr int FE_TONEAREST = 0x0000;
constexpr int FE_DOWNWARD = 0x0400;
constexpr int FE_UPWARD = 0x0800;
constexpr int FE_TOWARDZERO = 0x0c00;
constexpr int _ROUND_MASK = (FE_TONEAREST | FE_DOWNWARD |
    FE_UPWARD | FE_TOWARDZERO);

constexpr int _SSE_ROUND_SHIFT = 3;
constexpr int _SSE_EMASK_SHIFT = 7;

constexpr std::uint32_t __INITIAL_FPUCW__ = 0x037F;
constexpr std::uint32_t __INITIAL_MXCSR__ = 0x1F80;

struct fenv_x87 {
	std::uint32_t __control;
	std::uint32_t __status;
	std::uint32_t __tag;
	char __other[16];
};

struct fenv_t {
	fenv_x87 __x87;
	std::uint32_t __mxcsr;
};

std::uint32_t sim_x87_control;
std::uint32_t sim_x87_status;
std::uint32_t sim_x87_tag;
char sim_x87_other[16];
std::uint32_t sim_mxcsr;

inline void
fnstenv(fenv_x87 *addr)
{
	addr->__control = sim_x87_control;
	addr->__status = sim_x87_status;
	addr->__tag = sim_x87_tag;
	std::memcpy(addr->__other, sim_x87_other, 16);
	sim_x87_control |= FE_ALL_EXCEPT;
}

inline void
fldenv(const fenv_x87 *addr)
{
	sim_x87_control = addr->__control;
	sim_x87_status = addr->__status;
	sim_x87_tag = addr->__tag;
	std::memcpy(sim_x87_other, addr->__other, 16);
}

inline void
fldenvx(const fenv_x87 &env)
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
fldcw(std::uint32_t *addr)
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
ldmxcsr(const std::uint32_t *addr)
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
feclearexcept_int(int __excepts)
{
	fenv_t __env;

	if (__excepts == FE_ALL_EXCEPT) {
		fnclex();
	} else {
		fnstenv(&__env.__x87);
		__env.__x87.__status &= ~__excepts;
		fldenv(&__env.__x87);
	}
	stmxcsr(&__env.__mxcsr);
	__env.__mxcsr &= ~__excepts;
	ldmxcsr(&__env.__mxcsr);
	return (0);
}

inline int
fegetexceptflag_int(fexcept_t *__flagp, int __excepts)
{
	std::uint32_t __mxcsr;
	std::uint16_t __status;

	stmxcsr(&__mxcsr);
	fnstsw(&__status);
	*__flagp = (__mxcsr | __status) & __excepts;
	return (0);
}

inline int
fetestexcept_int(int __excepts)
{
	std::uint32_t __mxcsr;
	std::uint16_t __status;

	stmxcsr(&__mxcsr);
	fnstsw(&__status);
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

	stmxcsr(&__mxcsr);
	__mxcsr &= ~(_ROUND_MASK << _SSE_ROUND_SHIFT);
	__mxcsr |= __round << _SSE_ROUND_SHIFT;
	ldmxcsr(&__mxcsr);

	return (0);
}

inline int
fesetenv_int(const fenv_t *__envp)
{

	fldenvx(__envp->__x87);
	ldmxcsr(&__envp->__mxcsr);
	return (0);
}

} /* namespace pbsd::lib_msun_amd64::b0250::detail */

export namespace pbsd::lib_msun_amd64::b0250 {

using fexcept_t = detail::fexcept_t;
using fenv_x87 = detail::fenv_x87;
using fenv_t = detail::fenv_t;

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

inline void
test_reset(void)
{
	detail::sim_x87_control = detail::__INITIAL_FPUCW__;
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

	detail::fnstenv(&env.__x87);
	env.__x87.__status &= ~excepts;
	env.__x87.__status |= *flagp & excepts;
	detail::fldenv(&env.__x87);

	detail::stmxcsr(&env.__mxcsr);
	env.__mxcsr &= ~excepts;
	env.__mxcsr |= *flagp & excepts;
	detail::ldmxcsr(&env.__mxcsr);

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

	detail::fnstenv(&envp->__x87);
	detail::stmxcsr(&envp->__mxcsr);
	/*
	 * fnstenv masks all exceptions, so we need to restore the
	 * control word to avoid this side effect.
	 */
	detail::fldcw(&envp->__x87.__control);
	return (0);
}

inline int
feholdexcept(fenv_t *envp)
{
	std::uint32_t mxcsr;

	detail::stmxcsr(&mxcsr);
	detail::fnstenv(&envp->__x87);
	detail::fnclex();
	envp->__mxcsr = mxcsr;
	mxcsr &= ~FE_ALL_EXCEPT;
	mxcsr |= FE_ALL_EXCEPT << detail::_SSE_EMASK_SHIFT;
	detail::ldmxcsr(&mxcsr);
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
	detail::stmxcsr(&mxcsr);
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
	detail::stmxcsr(&mxcsr);
	omask = ~(control | mxcsr >> detail::_SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control &= ~mask;
	detail::fldcw(&control);
	mxcsr &= ~(mask << detail::_SSE_EMASK_SHIFT);
	detail::ldmxcsr(&mxcsr);
	return (omask);
}

inline int
__fedisableexcept(int mask)
{
	std::uint32_t mxcsr, omask;
	std::uint16_t control;

	mask &= FE_ALL_EXCEPT;
	detail::fnstcw(&control);
	detail::stmxcsr(&mxcsr);
	omask = ~(control | mxcsr >> detail::_SSE_EMASK_SHIFT) & FE_ALL_EXCEPT;
	control |= mask;
	detail::fldcw(&control);
	mxcsr |= mask << detail::_SSE_EMASK_SHIFT;
	detail::ldmxcsr(&mxcsr);
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

} /* namespace pbsd::lib_msun_amd64::b0250 */

namespace pbsd::lib_msun_amd64::b0250 {

const fenv_t __fe_dfl_env = {
	{ detail::__INITIAL_FPUCW__ | 0xffff0000u,
	  0xffff0000u,
	  0xffffffffu,
	  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (char)0xff, (char)0xff }
	},
	detail::__INITIAL_MXCSR__
};

} /* namespace pbsd::lib_msun_amd64::b0250 */
