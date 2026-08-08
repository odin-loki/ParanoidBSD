/*
 * oracle.c -- reference (specification) build for PBSD batch b0018.
 *
 * The original HardenedBSD source for this batch is reproduced below with the
 * resolver body renamed ref_memset_resolver.  Function bodies are otherwise
 * unmodified.
 *
 * READ_SPECIALREG(dczid_el0) and the three __memset_aarch64* callees are merely
 * declared in the original translation unit.  Mock and stub definitions are
 * supplied here so the unmodified reference body can be linked and executed;
 * they are support code, not part of the ported specification body.
 */

#include <stdint.h>
#include <stddef.h>

/* machine/ifunc.h */
struct __ifunc_arg_t
{
	unsigned long _size;
	unsigned long _hwcap;
	unsigned long _hwcap2;
	unsigned long _hwcap3;
	unsigned long _hwcap4;
};

/* elf.h */
#define	HWCAP2_MOPS		(1UL << 43)

/* machine/armreg.h */
#define DCZID_DZP		(1 << 4)
#define DCZID_BS_SHIFT		0
#define DCZID_BS_MASK		(0xf << DCZID_BS_SHIFT)
#define	DCZID_BS_SIZE(reg)	(((reg) & DCZID_BS_MASK) >> DCZID_BS_SHIFT)

/* mock READ_SPECIALREG(dczid_el0) for differential testing */
uint64_t test_dczid_el0 = 0;

#define	READ_SPECIALREG(reg)	test_dczid_el0

void *__memset_aarch64(void *, int, size_t);
void *__memset_aarch64_zva64(void *, int, size_t);
void *__memset_aarch64_mops(void *, int, size_t);

void *
__memset_aarch64(void *d, int c, size_t n)
{
	(void)c;
	(void)n;
	return (d);
}

void *
__memset_aarch64_zva64(void *d, int c, size_t n)
{
	(void)c;
	(void)n;
	return (d);
}

void *
__memset_aarch64_mops(void *d, int c, size_t n)
{
	(void)c;
	(void)n;
	return (d);
}

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/memset_resolver.c            */
/* ------------------------------------------------------------------ */

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

void *
ref_memset_resolver(uint64_t at_hwcap, const struct __ifunc_arg_t *ifunc_arg,
    uint64_t _arg3, uint64_t _arg4, uint64_t _arg5, uint64_t _arg6,
    uint64_t _arg7, uint64_t _arg8)
{
	uint64_t dczid;

	if (ifunc_arg->_hwcap2 & HWCAP2_MOPS)
		return (__memset_aarch64_mops);

	/*
	 * Check for the DC ZVA instruction, and it will
	 * zero 64 bytes (4 * 4byte words).
	 */
	dczid = READ_SPECIALREG(dczid_el0);
	if ((dczid & DCZID_DZP) == 0 && DCZID_BS_SIZE(dczid) == 4)
		return (__memset_aarch64_zva64);

	return (__memset_aarch64);
}
