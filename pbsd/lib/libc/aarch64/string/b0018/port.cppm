/*
 * PBSD batch b0018 -- C++23 port of:
 *   hbsd/src/lib/libc/aarch64/string/memset_resolver.c
 *
 * Faithful transliteration of memset_resolver: same evaluation order,
 * same signedness, same pointer arithmetic, same branch structure.
 */

module;

#include <cstddef>
#include <cstdint>

export module pbsd.lib.libc.aarch64.string.b0018;

extern "C" std::uint64_t test_dczid_el0;

extern "C" void *__memset_aarch64(void *, int, std::size_t);
extern "C" void *__memset_aarch64_zva64(void *, int, std::size_t);
extern "C" void *__memset_aarch64_mops(void *, int, std::size_t);

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Arm Ltd
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

#define	HWCAP2_MOPS		(1UL << 43)

#define DCZID_DZP		(1 << 4)
#define DCZID_BS_SHIFT		0
#define DCZID_BS_MASK		(0xf << DCZID_BS_SHIFT)
#define	DCZID_BS_SIZE(reg)	(((reg) & DCZID_BS_MASK) >> DCZID_BS_SHIFT)

#define	READ_SPECIALREG(reg)	test_dczid_el0

export namespace pbsd::lib_libc_aarch64_string::b0018 {

struct __ifunc_arg_t
{
	unsigned long _size;
	unsigned long _hwcap;
	unsigned long _hwcap2;
	unsigned long _hwcap3;
	unsigned long _hwcap4;
};

void *
memset_resolver(std::uint64_t at_hwcap, const __ifunc_arg_t *ifunc_arg,
    std::uint64_t _arg3, std::uint64_t _arg4, std::uint64_t _arg5,
    std::uint64_t _arg6, std::uint64_t _arg7, std::uint64_t _arg8)
{
	std::uint64_t dczid;

	if (ifunc_arg->_hwcap2 & HWCAP2_MOPS)
		return (reinterpret_cast<void *>(__memset_aarch64_mops));

	/*
	 * Check for the DC ZVA instruction, and it will
	 * zero 64 bytes (4 * 4byte words).
	 */
	dczid = READ_SPECIALREG(dczid_el0);
	if ((dczid & DCZID_DZP) == 0 && DCZID_BS_SIZE(dczid) == 4)
		return (reinterpret_cast<void *>(__memset_aarch64_zva64));

	return (reinterpret_cast<void *>(__memset_aarch64));
}

} /* namespace pbsd::lib_libc_aarch64_string::b0018 */
