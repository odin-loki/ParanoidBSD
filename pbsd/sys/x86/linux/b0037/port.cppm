/*-
 * Copyright (c) 2012 Konstantin Belousov <kib@FreeBSD.org>
 * Copyright (c) 2016, 2017, 2019, 2021 The FreeBSD Foundation
 * Copyright (c) 2021 Dmitry Chagin <dchagin@FreeBSD.org>
 *
 * Portions of this software were developed by Konstantin Belousov
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

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1994-1996 Søren Schmidt
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

#include <cstddef>
#include <cstdint>

export module pbsd.sys.x86.linux.b0037;

export namespace pbsd::sys_x86_linux::b0037 {

namespace detail {

static int _bsd_to_linux_trapcode[] = {
	255,	/* 0 LINUX_T_UNKNOWN */
	6,			/* 1  T_PRIVINFLT */
	255,	/* 2 */
	3,			/* 3  T_BPTFLT */
	255,	/* 4 */
	255,	/* 5 */
	16,			/* 6  T_ARITHTRAP */
	254,			/* 7  T_ASTFLT */
	255,	/* 8 */
	13,			/* 9  T_PROTFLT */
	1,			/* 10 T_TRCTRAP */
	255,	/* 11 */
	14,			/* 12 T_PAGEFLT */
	255,	/* 13 */
	17,			/* 14 T_ALIGNFLT */
	255,	/* 15 */
	255,	/* 16 */
	255,	/* 17 */
	0,			/* 18 T_DIVIDE */
	2,			/* 19 T_NMI */
	4,			/* 20 T_OFLOW */
	5,			/* 21 T_BOUND */
	7,			/* 22 T_DNA */
	8,			/* 23 T_DOUBLEFLT */
	9,			/* 24 T_FPOPFLT */
	10,			/* 25 T_TSSFLT */
	11,			/* 26 T_SEGNPFLT */
	12,			/* 27 T_STKFLT */
	18,			/* 28 T_MCHK */
	19,			/* 29 T_XMMFLT */
	15			/* 30 T_RESERVED */
};

} /* namespace detail */

using u_int = unsigned int;

inline constexpr u_int CPU_VENDOR_AMD = 0x1022;
inline constexpr u_int CPU_VENDOR_HYGON = 0x1d94;

inline constexpr u_int CPUID_SSE2 = 0x04000000;
inline constexpr u_int AMDID_RDTSCP = 0x08000000;
inline constexpr u_int CPUID_STDEXT_FSGSBASE = 0x00000001;
inline constexpr u_int CPUID_STDEXT2_RDPID = 0x00400000;

inline constexpr int LINUX_VDSO_CPU_DEFAULT = 0;
inline constexpr int LINUX_VDSO_CPU_RDPID = 1;
inline constexpr int LINUX_VDSO_CPU_RDTSCP = 2;
inline constexpr u_int LINUX_HWCAP2_FSGSBASE = 0x00000002;

inline constexpr int T_PROTFLT = 9;
inline constexpr int T_PAGEFLT = 12;
inline constexpr int T_DOUBLEFLT = 23;
inline constexpr int T_TSSFLT = 25;

inline constexpr int SIGBUS = 10;
inline constexpr int SIGSEGV = 11;

inline constexpr int LINUX_T_UNKNOWN = 255;

inline u_int cpu_feature = 0;
inline u_int cpu_vendor_id = 0;
inline u_int amd_feature = 0;
inline u_int cpu_stdext_feature = 0;
inline u_int cpu_stdext_feature2 = 0;

template <typename T, std::size_t N>
constexpr std::size_t
nitems(const T (&)[N])
{
	return (N);
}

int
linux_vdso_tsc_selector_idx(void)
{
	bool amd_cpu;

	if (cpu_feature != 0)
		return (2);	/* should not happen due to RDTSC */

	amd_cpu = (cpu_vendor_id == CPU_VENDOR_AMD ||
	    cpu_vendor_id == CPU_VENDOR_HYGON);

	if ((amd_feature & AMDID_RDTSCP) != 0)
		return (3);
	if ((cpu_feature & CPUID_SSE2) == 0)
		return (2);
	return (amd_cpu ? 1 : 0);
}

int
linux_vdso_cpu_selector_idx(void)
{

	if ((cpu_stdext_feature2 & CPUID_STDEXT2_RDPID) != 0)
		return (LINUX_VDSO_CPU_RDPID);

	return ((amd_feature & AMDID_RDTSCP) == 0 ?
	    LINUX_VDSO_CPU_DEFAULT : LINUX_VDSO_CPU_RDTSCP);
}

int
linux_translate_traps(int signal, int trap_code)
{
	if (signal != SIGBUS)
		return (signal);
	switch (trap_code) {
	case T_PROTFLT:
	case T_TSSFLT:
	case T_DOUBLEFLT:
	case T_PAGEFLT:
		return (SIGSEGV);
	default:
		return (signal);
	}
}

int
bsd_to_linux_trapcode(int code)
{

	return (code < nitems(detail::_bsd_to_linux_trapcode) ?
	    detail::_bsd_to_linux_trapcode[code] : LINUX_T_UNKNOWN);
}

u_int
linux_x86_elf_hwcap2(void)
{
	static u_int elf_hwcap2 = 0;
	static bool elf_hwcap2_valid = false;

	if (!elf_hwcap2_valid) {
		if ((cpu_stdext_feature & CPUID_STDEXT_FSGSBASE) != 0)
			elf_hwcap2 |= LINUX_HWCAP2_FSGSBASE;
		elf_hwcap2_valid = true;
	}
	return (elf_hwcap2);
}

} /* namespace pbsd::sys_x86_linux::b0037 */
