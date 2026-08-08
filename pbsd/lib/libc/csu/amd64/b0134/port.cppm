/*
 * PBSD batch b0134 -- C++23 port of the HardenedBSD libc csu amd64
 * reloc.c relocation helpers.
 *
 * Original copyright headers are retained above each ported function.
 * Behaviour, signedness and evaluation order are otherwise unchanged.
 *
 * The Elf typedefs, relocation constants and cpuid helpers from the
 * machine headers are not part of this batch; they are reproduced here
 * as scaffolding so the ported functions compile and behave identically.
 */

module;

#include <cstdint>
#include <cstring>

export module pbsd.lib.libc.csu.amd64.b0134;

export namespace pbsd::lib_libc_csu_amd64::b0134 {

using u_int = unsigned int;

/* ------------------------------------------------------------------ */
/* machine/cpufunc.h scaffolding                                      */
/* ------------------------------------------------------------------ */

struct cpuid_mock_entry {
	u_int level;
	u_int leaf;
	u_int p[4];
};

inline constexpr int CPUID_MOCK_MAX = 8;

cpuid_mock_entry cpuid_mocks[CPUID_MOCK_MAX];
int cpuid_mock_count;

void reset_cpuid_mocks() noexcept
{
	cpuid_mock_count = 0;
}

void add_cpuid_mock(u_int level, u_int leaf, u_int p0, u_int p1, u_int p2,
    u_int p3) noexcept
{
	if (cpuid_mock_count >= CPUID_MOCK_MAX)
		return;
	cpuid_mocks[cpuid_mock_count].level = level;
	cpuid_mocks[cpuid_mock_count].leaf = leaf;
	cpuid_mocks[cpuid_mock_count].p[0] = p0;
	cpuid_mocks[cpuid_mock_count].p[1] = p1;
	cpuid_mocks[cpuid_mock_count].p[2] = p2;
	cpuid_mocks[cpuid_mock_count].p[3] = p3;
	cpuid_mock_count++;
}

static int try_cpuid_mock(u_int level, u_int leaf, u_int *p) noexcept
{
	for (int i = 0; i < cpuid_mock_count; i++) {
		if (cpuid_mocks[i].level == level &&
		    cpuid_mocks[i].leaf == leaf) {
			p[0] = cpuid_mocks[i].p[0];
			p[1] = cpuid_mocks[i].p[1];
			p[2] = cpuid_mocks[i].p[2];
			p[3] = cpuid_mocks[i].p[3];
			return 1;
		}
	}
	return 0;
}

static void real_do_cpuid(u_int level, u_int *p) noexcept
{
#if defined(__i386__) || defined(__x86_64__)
	unsigned int a, b, c, d;

	__asm__ volatile("cpuid"
	    : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
	    : "a"(level), "c"(0));
	p[0] = a;
	p[1] = b;
	p[2] = c;
	p[3] = d;
#else
	p[0] = p[1] = p[2] = p[3] = 0;
	(void)level;
#endif
}

static void real_cpuid_count(u_int level, u_int leaf, u_int *p) noexcept
{
#if defined(__i386__) || defined(__x86_64__)
	unsigned int a, b, c, d;

	__asm__ volatile("cpuid"
	    : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
	    : "a"(level), "c"(leaf));
	p[0] = a;
	p[1] = b;
	p[2] = c;
	p[3] = d;
#else
	p[0] = p[1] = p[2] = p[3] = 0;
	(void)level;
	(void)leaf;
#endif
}

static void do_cpuid(u_int level, u_int *p) noexcept
{
	if (try_cpuid_mock(level, 0, p))
		return;
	real_do_cpuid(level, p);
}

static void cpuid_count(u_int level, u_int leaf, u_int *p) noexcept
{
	if (try_cpuid_mock(level, leaf, p))
		return;
	real_cpuid_count(level, leaf, p);
}

/* ------------------------------------------------------------------ */
/* ELF scaffolding                                                    */
/* ------------------------------------------------------------------ */

using Elf_Addr = std::uint64_t;

struct Elf_Auxinfo {
	Elf_Addr a_type;
	union {
		Elf_Addr a_val;
	} a_un;
};

struct Elf_Rela {
	Elf_Addr r_offset;
	Elf_Addr r_info;
	long r_addend;
};

inline constexpr Elf_Addr R_X86_64_IRELATIVE = 37;

inline constexpr Elf_Addr ELF_R_TYPE(Elf_Addr info) noexcept
{
	return info & 0xffffffffUL;
}

std::uint32_t cpu_feature, cpu_feature2;
std::uint32_t cpu_stdext_feature, cpu_stdext_feature2;

/* ------------------------------------------------------------------ */
/* reloc.c                                                            */
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2018 The FreeBSD Foundation
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

void ifunc_init(const Elf_Auxinfo *aux) noexcept
{
	u_int p[4];

	do_cpuid(1, p);
	cpu_feature = p[3];
	cpu_feature2 = p[2];
	do_cpuid(0, p);
	if (p[0] >= 7) {
		cpuid_count(7, 0, p);
		cpu_stdext_feature = p[1];
		cpu_stdext_feature2 = p[2];
	} else {
		cpu_stdext_feature = 0;
		cpu_stdext_feature2 = 0;
	}
	(void)aux;
}

void crt1_handle_rela(const Elf_Rela *r) noexcept
{
	Elf_Addr *ptr, *where, target;

	switch (ELF_R_TYPE(r->r_info)) {
	case R_X86_64_IRELATIVE:
		ptr = (Elf_Addr *)r->r_addend;
		where = (Elf_Addr *)r->r_offset;
		target = ((Elf_Addr (*)(std::uint32_t, std::uint32_t, std::uint32_t,
		    std::uint32_t))ptr)(cpu_feature, cpu_feature2,
		    cpu_stdext_feature, cpu_stdext_feature2);
		*where = target;
		break;
	}
}

} /* namespace pbsd::lib_libc_csu_amd64::b0134 */
