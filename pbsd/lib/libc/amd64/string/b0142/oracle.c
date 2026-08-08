/*
 * oracle.c -- reference (specification) build for PBSD batch b0142.
 *
 * The original HardenedBSD source
 *
 *	hbsd/src/lib/libc/amd64/string/amd64_archlevel.c
 *
 * concatenated verbatim, with the exported function renamed with a "ref_"
 * prefix.  No function body has been modified.  This file is the
 * specification.
 *
 * The #include lines of the original name <sys/types.h>,
 * <machine/atomic.h>, <machine/cpufunc.h>, <machine/specialreg.h>,
 * <stddef.h>, <string.h>, "amd64_archlevel.h" and "libc_private.h".
 * The typedefs, constants and helpers those headers supply are
 * reproduced below so that the bodies compile verbatim.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef dlfunc_t
typedef void (*dlfunc_t)(void);
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

typedef unsigned int u_int;

/* ------------------------------------------------------------------ */
/* amd64_archlevel.h                                                  */
/* ------------------------------------------------------------------ */

#define X86_64_SCALAR    0
#define X86_64_BASELINE  1
#define X86_64_V2        2
#define X86_64_V3        3
#define X86_64_V4        4
#define X86_64_MAX       X86_64_V4
#define X86_64_UNDEFINED -1

/* ------------------------------------------------------------------ */
/* machine/specialreg.h (constants used by this batch)                */
/* ------------------------------------------------------------------ */

#define CPUID_FPU	0x00000001
#define CPUID_CX8	0x00000100
#define CPUID_CMOV	0x00008000
#define CPUID_MMX	0x00800000
#define CPUID_FXSR	0x01000000
#define CPUID_SSE	0x02000000
#define CPUID_SSE2	0x04000000

#define CPUID2_SSE3	0x00000001
#define CPUID2_SSSE3	0x00000200
#define CPUID2_CX16	0x00002000
#define CPUID2_SSE41	0x00080000
#define CPUID2_SSE42	0x00100000
#define CPUID2_POPCNT	0x00800000
#define CPUID2_FMA	0x00001000
#define CPUID2_MOVBE	0x00400000
#define CPUID2_OSXSAVE	0x08000000
#define CPUID2_AVX	0x10000000
#define CPUID2_F16C	0x20000000

#define AMDID2_LAHF	0x00000001
#define AMDID2_ABM	0x00000020

#define CPUID_STDEXT_BMI1	0x00000008
#define CPUID_STDEXT_AVX2	0x00000020
#define CPUID_STDEXT_BMI2	0x00000100
#define CPUID_STDEXT_AVX512F	0x00010000
#define CPUID_STDEXT_AVX512DQ	0x00020000
#define CPUID_STDEXT_AVX512CD	0x10000000
#define CPUID_STDEXT_AVX512BW	0x40000000
#define CPUID_STDEXT_AVX512VL	0x80000000

/* ------------------------------------------------------------------ */
/* machine/atomic.h                                                   */
/* ------------------------------------------------------------------ */

static int
atomic_load_int(volatile int *p)
{
	return (*(volatile int *)(p));
}

static int
atomic_cmpset_int(volatile u_int *dst, u_int expect, u_int src)
{
	u_int old = *dst;

	if (old != expect)
		return (0);
	*dst = src;
	return (1);
}

/* ------------------------------------------------------------------ */
/* machine/cpufunc.h                                                  */
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
			return (1);
		}
	}
	return (0);
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
do_cpuid(u_int level, u_int *p)
{
	if (ref_try_cpuid_mock(level, 0, p))
		return;
	ref_real_do_cpuid(level, p);
}

/* ------------------------------------------------------------------ */
/* libc_private.h                                                     */
/* ------------------------------------------------------------------ */

char **environ;

void
ref_set_test_environ(char **env)
{
	environ = env;
}

void
ref_reset_archlevel_state(void);

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/amd64_archlevel.c                   */
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2023 The FreeBSD Foundation
 *
 * This software was developed by Robert Clausecker <fuz@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

#define ARCHLEVEL_ENV	"ARCHLEVEL"

static volatile int amd64_archlevel = X86_64_UNDEFINED;

static const struct archlevel {
	char name[10];
	/* CPUID feature bits that need to be present */
	u_int feat_edx, feat_ecx, amd_ecx, ext_ebx;
} levels[] = {
	{
		.name = "scalar",
		.feat_edx = 0,
		.feat_ecx = 0,
		.amd_ecx = 0,
		.ext_ebx = 0,
	}, {
#define FEAT_EDX_BASELINE (CPUID_FPU | CPUID_CX8 | CPUID_CMOV | CPUID_MMX | \
	CPUID_FXSR | CPUID_SSE | CPUID_SSE2)
		.name = "baseline",
		.feat_edx = FEAT_EDX_BASELINE,
		.feat_ecx = 0,
		.amd_ecx = 0,
		.ext_ebx = 0,
	}, {
#define FEAT_ECX_V2 (CPUID2_SSE3 | CPUID2_SSSE3 | CPUID2_CX16 | CPUID2_SSE41 | \
	CPUID2_SSE42 | CPUID2_POPCNT)
#define AMD_ECX_V2 AMDID2_LAHF
		.name = "x86-64-v2",
		.feat_edx = FEAT_EDX_BASELINE,
		.feat_ecx = FEAT_ECX_V2,
		.amd_ecx = AMD_ECX_V2,
		.ext_ebx = 0,
	}, {
#define FEAT_ECX_V3 (FEAT_ECX_V2 | CPUID2_FMA | CPUID2_MOVBE | \
	CPUID2_OSXSAVE | CPUID2_AVX | CPUID2_F16C)
#define AMD_ECX_V3 (AMD_ECX_V2 | AMDID2_ABM)
#define EXT_EBX_V3 (CPUID_STDEXT_BMI1 | CPUID_STDEXT_AVX2 | CPUID_STDEXT_BMI2)
		.name = "x86-64-v3",
		.feat_edx = FEAT_EDX_BASELINE,
		.feat_ecx = FEAT_ECX_V3,
		.amd_ecx = AMD_ECX_V3,
		.ext_ebx = EXT_EBX_V3,
	}, {
#define EXT_EBX_V4 (EXT_EBX_V3 | CPUID_STDEXT_AVX512F | \
	CPUID_STDEXT_AVX512DQ | CPUID_STDEXT_AVX512CD | \
	CPUID_STDEXT_AVX512BW | CPUID_STDEXT_AVX512VL)
		.name = "x86-64-v4",
		.feat_edx = FEAT_EDX_BASELINE,
		.feat_ecx = FEAT_ECX_V3,
		.amd_ecx = AMD_ECX_V3,
		.ext_ebx = EXT_EBX_V4,
	}
};

static int
supported_archlevel(u_int feat_edx, u_int feat_ecx, u_int ext_ebx, u_int ext_ecx)
{
	int level;
	u_int p[4], max_leaf;
	u_int amd_ecx = 0;

	(void)ext_ecx;

	do_cpuid(0x80000000, p);
	max_leaf = p[0];

	if (max_leaf >= 0x80000001) {
		do_cpuid(0x80000001, p);
		amd_ecx = p[2];
	}

	for (level = X86_64_BASELINE; level <= X86_64_MAX; level++) {
		const struct archlevel *lvl = &levels[level];

		if ((lvl->feat_edx & feat_edx) != lvl->feat_edx ||
		    (lvl->feat_ecx & feat_ecx) != lvl->feat_ecx ||
		    (lvl->amd_ecx & amd_ecx) != lvl->amd_ecx ||
		    (lvl->ext_ebx & ext_ebx) != lvl->ext_ebx)
			return (level - 1);
	}

	return (X86_64_MAX);
}

static int
match_archlevel(const char *str, int *force)
{
	int level, want_force = 0;

	*force = 0;

	if (str[0] == '!') {
		str++;
		want_force = 1;
	}

	for (level = 0; level <= X86_64_MAX; level++) {
		size_t i;
		const char *candidate = levels[level].name;

		/* can't use strcmp here: would recurse during ifunc resolution */
		for (i = 0; str[i] == candidate[i]; i++)
			/* suffixes starting with : or + are ignored for future extensions */
			if (str[i] == '\0' || str[i] == ':' || str[i] == '+') {
				if (want_force)
					*force = 1;

				return (level);
			}
	}

	return (X86_64_UNDEFINED);
}

/*
 * We can't use getenv(), strcmp(), and a bunch of other functions here as
 * they may in turn call SIMD-optimised string functions.
 *
 * *force is set to 1 if the architecture level is valid and begins with a !
 * and to 0 otherwise.
 */
static int
env_archlevel(int *force)
{
	size_t i;

	if (environ == NULL)
		return (X86_64_UNDEFINED);

	for (i = 0; environ[i] != NULL; i++) {
		size_t j;

		for (j = 0; environ[i][j] == ARCHLEVEL_ENV "="[j]; j++)
			if (environ[i][j] == '=')
				return (match_archlevel(&environ[i][j + 1], force));
	}

	*force = 0;

	return (X86_64_UNDEFINED);

}

/*
 * Determine the architecture level by checking the CPU capabilities
 * and the environment:
 *
 * 1. If environment variable ARCHLEVEL starts with a ! and is followed
 *    by a valid architecture level, that level is returned.
 * 2. Else if ARCHLEVEL is set to a valid architecture level that is
 *    supported by the CPU, that level is returned.
 * 3. Else the highest architecture level supported by the CPU is
 *    returned.
 *
 * Valid architecture levels are those defined in the levels array.
 * The architecture level "scalar" indicates that SIMD enhancements
 * shall not be used.
 */
static int
archlevel(u_int feat_edx, u_int feat_ecx, u_int ext_ebx, u_int ext_ecx)
{
	int islevel, wantlevel, hwlevel, force;

	islevel = atomic_load_int(&amd64_archlevel);
	if (islevel != X86_64_UNDEFINED)
		return (islevel);

	wantlevel = env_archlevel(&force);
	if (!force) {
		hwlevel = supported_archlevel(feat_edx, feat_ecx, ext_ebx, ext_ecx);
		if (wantlevel == X86_64_UNDEFINED || wantlevel > hwlevel)
			wantlevel = hwlevel;
	}

	/*
	 * Ensure amd64_archlevel is set only once and
	 * all calls agree on what it was set to.
	 */
	if (atomic_cmpset_int(&amd64_archlevel, islevel, wantlevel))
		return (wantlevel);
	else
		return (atomic_load_int(&amd64_archlevel));
}

void
ref_reset_archlevel_state(void)
{
	amd64_archlevel = X86_64_UNDEFINED;
}

/*
 * Helper function for SIMD ifunc dispatch: select the highest level
 * implementation up to the current architecture level.
 */
dlfunc_t
ref___archlevel_resolve(u_int feat_edx, u_int feat_ecx, u_int ext_ebx,
    u_int ext_ecx, int32_t funcs[static X86_64_MAX + 1])
{
	int level;

	for (level = archlevel(feat_edx, feat_ecx, ext_ebx, ext_ecx); level >= 0; level--)
		if (funcs[level] != 0)
			return (dlfunc_t)((uintptr_t)funcs + (ptrdiff_t)funcs[level]);

	/* no function is present -- what now? */
	__builtin_trap();
}
