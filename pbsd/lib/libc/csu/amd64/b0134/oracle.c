/*
 * oracle.c -- reference implementation for PBSD batch b0134.
 *
 * The original HardenedBSD C source
 *
 *	lib/libc/csu/amd64/reloc.c
 *
 * concatenated, with each ported function renamed with a ref_ prefix.
 * The function bodies are UNMODIFIED.  This file is the specification.
 *
 * The #include lines of the original name <sys/cdefs.h>,
 * <machine/specialreg.h> and <machine/cpufunc.h>.  The Elf typedefs,
 * relocation constants and cpuid helpers those headers supply are
 * reproduced below so that the bodies compile verbatim.
 */

#include <stdint.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

#ifndef __unused
#define __unused
#endif

typedef unsigned int u_int;

/* ------------------------------------------------------------------ */
/* machine/cpufunc.h scaffolding                                      */
/* ------------------------------------------------------------------ */

typedef struct {
	u_int level;
	u_int leaf;
	u_int p[4];
} cpuid_mock_entry;

#define REF_CPUID_MOCK_MAX	8

static cpuid_mock_entry ref_cpuid_mocks[REF_CPUID_MOCK_MAX];
static int ref_cpuid_mock_count;

void
ref_reset_cpuid_mocks(void)
{
	ref_cpuid_mock_count = 0;
}

void
ref_add_cpuid_mock(u_int level, u_int leaf, u_int p0, u_int p1, u_int p2,
    u_int p3)
{
	if (ref_cpuid_mock_count >= REF_CPUID_MOCK_MAX)
		return;
	ref_cpuid_mocks[ref_cpuid_mock_count].level = level;
	ref_cpuid_mocks[ref_cpuid_mock_count].leaf = leaf;
	ref_cpuid_mocks[ref_cpuid_mock_count].p[0] = p0;
	ref_cpuid_mocks[ref_cpuid_mock_count].p[1] = p1;
	ref_cpuid_mocks[ref_cpuid_mock_count].p[2] = p2;
	ref_cpuid_mocks[ref_cpuid_mock_count].p[3] = p3;
	ref_cpuid_mock_count++;
}

static int
ref_try_cpuid_mock(u_int level, u_int leaf, u_int *p)
{
	int i;

	for (i = 0; i < ref_cpuid_mock_count; i++) {
		if (ref_cpuid_mocks[i].level == level &&
		    ref_cpuid_mocks[i].leaf == leaf) {
			p[0] = ref_cpuid_mocks[i].p[0];
			p[1] = ref_cpuid_mocks[i].p[1];
			p[2] = ref_cpuid_mocks[i].p[2];
			p[3] = ref_cpuid_mocks[i].p[3];
			return 1;
		}
	}
	return 0;
}

static void
ref_real_do_cpuid(u_int level, u_int *p)
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

static void
ref_real_cpuid_count(u_int level, u_int leaf, u_int *p)
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

static void
do_cpuid(u_int level, u_int *p)
{
	if (ref_try_cpuid_mock(level, 0, p))
		return;
	ref_real_do_cpuid(level, p);
}

static void
cpuid_count(u_int level, u_int leaf, u_int *p)
{
	if (ref_try_cpuid_mock(level, leaf, p))
		return;
	ref_real_cpuid_count(level, leaf, p);
}

/* ------------------------------------------------------------------ */
/* ELF scaffolding                                                    */
/* ------------------------------------------------------------------ */

typedef uint64_t Elf_Addr;

typedef struct {
	Elf_Addr a_type;
	union {
		Elf_Addr a_val;
	} a_un;
} Elf_Auxinfo;

typedef struct {
	Elf_Addr r_offset;
	Elf_Addr r_info;
	long r_addend;
} Elf_Rela;

#define R_X86_64_IRELATIVE	37

#define ELF_R_TYPE(info)	((info) & 0xffffffff)

uint32_t cpu_feature, cpu_feature2;
uint32_t cpu_stdext_feature, cpu_stdext_feature2;

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

void
ref_ifunc_init(const Elf_Auxinfo *aux __unused)
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
}

void
ref_crt1_handle_rela(const Elf_Rela *r)
{
	Elf_Addr *ptr, *where, target;

	switch (ELF_R_TYPE(r->r_info)) {
	case R_X86_64_IRELATIVE:
		ptr = (Elf_Addr *)r->r_addend;
		where = (Elf_Addr *)r->r_offset;
		target = ((Elf_Addr (*)(uint32_t, uint32_t, uint32_t,
		    uint32_t))ptr)(cpu_feature, cpu_feature2,
		    cpu_stdext_feature, cpu_stdext_feature2);
		*where = target;
		break;
	}
}
