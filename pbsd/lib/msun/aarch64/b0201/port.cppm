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

module;

#include <cstdint>

export module pbsd.lib.msun.aarch64.b0201;

namespace pbsd::lib_msun_aarch64::b0201::detail {

/* The high 32 bits contain fpcr, low 32 contain fpsr. */
using fenv_t = std::uint64_t;
using fexcept_t = std::uint64_t;

/* Exception flags */
constexpr int FE_INVALID = 0x00000001;
constexpr int FE_DIVBYZERO = 0x00000002;
constexpr int FE_OVERFLOW = 0x00000004;
constexpr int FE_UNDERFLOW = 0x00000008;
constexpr int FE_INEXACT = 0x00000010;
constexpr int FE_ALL_EXCEPT = (FE_DIVBYZERO | FE_INEXACT |
    FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);

constexpr int FE_TONEAREST = 0x0;
constexpr int FE_UPWARD = 0x1;
constexpr int FE_DOWNWARD = 0x2;
constexpr int FE_TOWARDZERO = 0x3;
constexpr int _ROUND_MASK = (FE_TONEAREST | FE_DOWNWARD |
    FE_UPWARD | FE_TOWARDZERO);
constexpr int _ROUND_SHIFT = 22;

constexpr int _FPUSW_SHIFT = 8;
constexpr int _ENABLE_MASK = (FE_ALL_EXCEPT << _FPUSW_SHIFT);

static std::uint64_t sim_fpcr;
static std::uint64_t sim_fpsr;

inline void
mrs_fpcr(std::uint64_t &__r)
{
	__r = sim_fpcr;
}

inline void
msr_fpcr(std::uint64_t __r)
{
	sim_fpcr = __r;
}

inline void
mrs_fpsr(std::uint64_t &__r)
{
	__r = sim_fpsr;
}

inline void
msr_fpsr(std::uint64_t __r)
{
	sim_fpsr = __r;
}

inline int
feclearexcept_int(int __excepts)
{
	fexcept_t __r;

	mrs_fpsr(__r);
	__r &= ~__excepts;
	msr_fpsr(__r);
	return (0);
}

inline int
fegetexceptflag_int(fexcept_t *__flagp, int __excepts)
{
	fexcept_t __r;

	mrs_fpsr(__r);
	*__flagp = __r & __excepts;
	return (0);
}

inline int
fesetexceptflag_int(const fexcept_t *__flagp, int __excepts)
{
	fexcept_t __r;

	mrs_fpsr(__r);
	__r &= ~__excepts;
	__r |= *__flagp & __excepts;
	msr_fpsr(__r);
	return (0);
}

inline int
feraiseexcept_int(int __excepts)
{
	fexcept_t __r;

	mrs_fpsr(__r);
	__r |= __excepts;
	msr_fpsr(__r);
	return (0);
}

inline int
fetestexcept_int(int __excepts)
{
	fexcept_t __r;

	mrs_fpsr(__r);
	return (__r & __excepts);
}

inline int
fegetround_int(void)
{
	fenv_t __r;

	mrs_fpcr(__r);
	return ((__r >> _ROUND_SHIFT) & _ROUND_MASK);
}

inline int
fesetround_int(int __round)
{
	fenv_t __r;

	if (__round & ~_ROUND_MASK)
		return (-1);
	mrs_fpcr(__r);
	__r &= ~(_ROUND_MASK << _ROUND_SHIFT);
	__r |= __round << _ROUND_SHIFT;
	msr_fpcr(__r);
	return (0);
}

inline int
fegetenv_int(fenv_t *__envp)
{
	std::uint64_t fpcr;
	std::uint64_t fpsr;

	mrs_fpcr(fpcr);
	mrs_fpsr(fpsr);
	*__envp = fpsr | (fpcr << 32);

	return (0);
}

inline int
feholdexcept_int(fenv_t *__envp)
{
	fenv_t __r;

	mrs_fpcr(__r);
	*__envp = __r << 32;
	__r &= ~(_ENABLE_MASK);
	msr_fpcr(__r);

	mrs_fpsr(__r);
	*__envp |= (std::uint32_t)__r;
	__r &= ~(_ENABLE_MASK);
	msr_fpsr(__r);
	return (0);
}

inline int
fesetenv_int(const fenv_t *__envp)
{

	msr_fpcr((*__envp) >> 32);
	msr_fpsr((fenv_t)(std::uint32_t)*__envp);
	return (0);
}

inline int
feupdateenv_int(const fenv_t *__envp)
{
	fexcept_t __r;

	mrs_fpsr(__r);
	fesetenv_int(__envp);
	feraiseexcept_int(__r & FE_ALL_EXCEPT);
	return (0);
}

inline int
feenableexcept_int(int __mask)
{
	fenv_t __old_r, __new_r;

	mrs_fpcr(__old_r);
	__new_r = __old_r | ((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

inline int
fedisableexcept_int(int __mask)
{
	fenv_t __old_r, __new_r;

	mrs_fpcr(__old_r);
	__new_r = __old_r & ~((__mask & FE_ALL_EXCEPT) << _FPUSW_SHIFT);
	msr_fpcr(__new_r);
	return ((__old_r >> _FPUSW_SHIFT) & FE_ALL_EXCEPT);
}

inline int
fegetexcept_int(void)
{
	fenv_t __r;

	mrs_fpcr(__r);
	return ((__r & _ENABLE_MASK) >> _FPUSW_SHIFT);
}

} /* namespace pbsd::lib_msun_aarch64::b0201::detail */

export namespace pbsd::lib_msun_aarch64::b0201 {

using fenv_t = detail::fenv_t;
using fexcept_t = detail::fexcept_t;

constexpr int FE_INVALID = detail::FE_INVALID;
constexpr int FE_DIVBYZERO = detail::FE_DIVBYZERO;
constexpr int FE_OVERFLOW = detail::FE_OVERFLOW;
constexpr int FE_UNDERFLOW = detail::FE_UNDERFLOW;
constexpr int FE_INEXACT = detail::FE_INEXACT;
constexpr int FE_ALL_EXCEPT = detail::FE_ALL_EXCEPT;
constexpr int FE_TONEAREST = detail::FE_TONEAREST;
constexpr int FE_UPWARD = detail::FE_UPWARD;
constexpr int FE_DOWNWARD = detail::FE_DOWNWARD;
constexpr int FE_TOWARDZERO = detail::FE_TOWARDZERO;

/*
 * Hopefully the system ID byte is immutable, so it's valid to use
 * this as a default environment.
 */
extern const fenv_t __fe_dfl_env;

inline void
test_set_fpcr(std::uint64_t v)
{
	detail::sim_fpcr = v;
}

inline void
test_set_fpsr(std::uint64_t v)
{
	detail::sim_fpsr = v;
}

inline std::uint64_t
test_get_fpcr(void)
{
	return (detail::sim_fpcr);
}

inline std::uint64_t
test_get_fpsr(void)
{
	return (detail::sim_fpsr);
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
	return (detail::fesetexceptflag_int(flagp, excepts));
}

inline int
feraiseexcept(int excepts)
{
	return (detail::feraiseexcept_int(excepts));
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
	return (detail::fegetenv_int(envp));
}

inline int
feholdexcept(fenv_t *envp)
{
	return (detail::feholdexcept_int(envp));
}

inline int
fesetenv(const fenv_t *envp)
{
	return (detail::fesetenv_int(envp));
}

inline int
feupdateenv(const fenv_t *envp)
{
	return (detail::feupdateenv_int(envp));
}

inline int
feenableexcept(int mask)
{
	return (detail::feenableexcept_int(mask));
}

inline int
fedisableexcept(int mask)
{
	return (detail::fedisableexcept_int(mask));
}

inline int
fegetexcept(void)
{
	return (detail::fegetexcept_int());
}

} /* namespace pbsd::lib_msun_aarch64::b0201 */

namespace pbsd::lib_msun_aarch64::b0201 {

const fenv_t __fe_dfl_env = 0;

} /* namespace pbsd::lib_msun_aarch64::b0201 */
