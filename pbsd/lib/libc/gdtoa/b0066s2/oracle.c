/*
 * b0066s2 oracle -- reference implementation for the differential test.
 *
 * This is lib/libc/gdtoa/_hldtoa.c with every function renamed with a ref_
 * prefix.  The function bodies are the originals, unmodified.  Declarations
 * and defines that the batch source normally picks up from <fpmath.h>,
 * <machine/_fpmath.h> and "gdtoaimp.h" are supplied here.
 *
 * rv_alloc() and nrv_alloc() live in contrib/gdtoa outside this batch.  The
 * versions here are instrumented: storage is handed out from a per-side arena
 * that the harness pre-fills with the guard byte 0x7f.
 */

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* From lib/libc/include/fpmath.h (_BYTE_ORDER == _LITTLE_ENDIAN and	*/
/* _IEEE_WORD_ORDER == _LITTLE_ENDIAN, i.e. the amd64 branch).		*/
/* ------------------------------------------------------------------ */

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

/* ------------------------------------------------------------------ */
/* From contrib/gdtoa/gdtoaimp.h (rv_alloc / nrv_alloc declarations).	*/
/* ------------------------------------------------------------------ */

typedef unsigned int ULong;

extern char *rv_alloc(int);
extern char *nrv_alloc(const char *, char **, int);

/* ------------------------------------------------------------------ */
/* Instrumentation shared with harness.cpp.				*/
/* ------------------------------------------------------------------ */

#define	PBSD_ARENA_SIZE	512
#define	PBSD_MAX_ALLOC	2
#define	PBSD_STR_MAX	96

struct pbsd_log {
	int		alloc_calls;
	int		alloc_n[PBSD_MAX_ALLOC];
	long		alloc_off[PBSD_MAX_ALLOC];
	long		alloc_usable[PBSD_MAX_ALLOC];
	int		nrv_calls;
	int		nrv_n[PBSD_MAX_ALLOC];
	char		nrv_s[PBSD_MAX_ALLOC][PBSD_STR_MAX];
};

unsigned char	pbsd_arena[2][PBSD_ARENA_SIZE];
struct pbsd_log	pbsd_logs[2];
int		pbsd_side;

static long	pbsd_arena_used[2];

void
pbsd_case_begin(int side)
{

	pbsd_side = side;
	memset(pbsd_arena[side], 0x7f, PBSD_ARENA_SIZE);
	memset(&pbsd_logs[side], 0, sizeof(pbsd_logs[side]));
	pbsd_arena_used[side] = 0;
}

struct pbsd_Bigint {
	struct pbsd_Bigint	*next;
	int			k, maxwds, sign, wds;
	ULong			x[1];
};

static size_t
pbsd_block_bytes(int i)
{
	size_t j, x, len;
	int k;

	j = sizeof(ULong);
	for (k = 0;
	    sizeof(struct pbsd_Bigint) - sizeof(ULong) - sizeof(int) + j <=
	    (size_t)i;
	    j <<= 1)
		k++;
	x = (size_t)1 << k;
	len = (sizeof(struct pbsd_Bigint) + (x - 1) * sizeof(ULong) +
	    sizeof(double) - 1) / sizeof(double);
	return (len * sizeof(double));
}

char *
rv_alloc(int i)
{
	struct pbsd_log *lg;
	size_t usable;
	long off;
	int s;

	s = pbsd_side;
	lg = &pbsd_logs[s];
	usable = pbsd_block_bytes(i) - sizeof(int);
	off = pbsd_arena_used[s];
	if (off + (long)usable > PBSD_ARENA_SIZE) {
		fprintf(stderr, "b0066s2: arena exhausted (request %d)\n", i);
		abort();
	}
	if (lg->alloc_calls < PBSD_MAX_ALLOC) {
		lg->alloc_n[lg->alloc_calls] = i;
		lg->alloc_off[lg->alloc_calls] = off;
		lg->alloc_usable[lg->alloc_calls] = (long)usable;
	}
	lg->alloc_calls++;
	pbsd_arena_used[s] = off + (long)((usable + 7u) & ~(size_t)7u);
	return ((char *)pbsd_arena[s] + off);
}

char *
nrv_alloc(const char *s, char **rve, int n)
{
	struct pbsd_log *lg;
	char *rv, *t;
	int i;

	lg = &pbsd_logs[pbsd_side];
	i = lg->nrv_calls;
	if (i < PBSD_MAX_ALLOC) {
		lg->nrv_n[i] = n;
		strncpy(lg->nrv_s[i], s, PBSD_STR_MAX - 1);
	}
	lg->nrv_calls++;

	t = rv = rv_alloc(n);
	while ((*t = *s++) != 0)
		t++;
	if (rve)
		*rve = t;
	return (rv);
}

void
__freedtoa(char *s)
{
	(void)s;
}

/* ================================================================== */
/* lib/libc/gdtoa/_hldtoa.c						*/
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004-2008 David Schultz <das@FreeBSD.ORG>
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

#if (LDBL_MANT_DIG > DBL_MANT_DIG)

/* Strings values used by dtoa() */
#define	INFSTR	"Infinity"
#define	NANSTR	"NaN"

#ifdef LDBL_IMPLICIT_NBIT
#define	MANH_SIZE	LDBL_MANH_SIZE
#else
#define	MANH_SIZE	(LDBL_MANH_SIZE - 1)
#endif

#if MANH_SIZE > 32
typedef uint64_t manh_t;
#else
typedef uint32_t manh_t;
#endif

#if LDBL_MANL_SIZE > 32
typedef uint64_t manl_t;
#else
typedef uint32_t manl_t;
#endif

#define	LDBL_ADJ	(LDBL_MAX_EXP - 2)
#define	SIGFIGS		((LDBL_MANT_DIG + 3) / 4 + 1)

static const float one[] = { 1.0f, -1.0f };

/*
 * This is the long double version of __hdtoa().
 */
char *
ref___hldtoa(long double e, const char *xdigs, int ndigits, int *decpt,
    int *sign, char **rve)
{
	union IEEEl2bits u;
	char *s, *s0;
	manh_t manh;
	manl_t manl;
	int bufsize;
#ifdef __i386__
	fp_prec_t oldprec;
#endif

	u.e = e;
	*sign = u.bits.sign;

	switch (fpclassify(e)) {
	case FP_NORMAL:
		*decpt = u.bits.exp - LDBL_ADJ;
		break;
	case FP_ZERO:
		*decpt = 1;
		return (nrv_alloc("0", rve, 1));
	case FP_SUBNORMAL:
#ifdef __i386__
		oldprec = fpsetprec(FP_PE);
#endif
		u.e *= 0x1p514L;
		*decpt = u.bits.exp - (514 + LDBL_ADJ);
#ifdef __i386__
		fpsetprec(oldprec);
#endif
		break;
	case FP_INFINITE:
		*decpt = INT_MAX;
		return (nrv_alloc(INFSTR, rve, sizeof(INFSTR) - 1));
	default:	/* FP_NAN or unrecognized */
		*decpt = INT_MAX;
		return (nrv_alloc(NANSTR, rve, sizeof(NANSTR) - 1));
	}

	/* FP_NORMAL or FP_SUBNORMAL */

	if (ndigits == 0)		/* dtoa() compatibility */
		ndigits = 1;

	/*
	 * If ndigits < 0, we are expected to auto-size, so we allocate
	 * enough space for all the digits.
	 */
	bufsize = (ndigits > 0) ? ndigits : SIGFIGS;
	s0 = rv_alloc(bufsize);

	/* Round to the desired number of digits. */
	if (SIGFIGS > ndigits && ndigits > 0) {
		float redux = one[u.bits.sign];
		int offset = 4 * ndigits + LDBL_MAX_EXP - 4 - LDBL_MANT_DIG;
#ifdef __i386__
		oldprec = fpsetprec(FP_PE);
#endif
		u.bits.exp = offset;
		u.e += redux;
		u.e -= redux;
		*decpt += u.bits.exp - offset;
#ifdef __i386__
		fpsetprec(oldprec);
#endif
	}

	mask_nbit_l(u);
	manh = u.bits.manh;
	manl = u.bits.manl;	
	*s0 = '1';
	for (s = s0 + 1; s < s0 + bufsize; s++) {
		*s = xdigs[(manh >> (MANH_SIZE - 4)) & 0xf];
		manh = (manh << 4) | (manl >> (LDBL_MANL_SIZE - 4));
		manl <<= 4;
	}

	/* If ndigits < 0, we are expected to auto-size the precision. */
	if (ndigits < 0) {
		for (ndigits = SIGFIGS; s0[ndigits - 1] == '0'; ndigits--)
			;
	}

	s = s0 + ndigits;
	*s = '\0';
	if (rve != NULL)
		*rve = s;
	return (s0);
}

#else	/* (LDBL_MANT_DIG == DBL_MANT_DIG) */

char *ref___hdtoa(double, const char *, int, int *, int *, char **);

char *
ref___hldtoa(long double e, const char *xdigs, int ndigits, int *decpt,
    int *sign, char **rve)
{

	return (ref___hdtoa((double)e, xdigs, ndigits, decpt, sign, rve));
}

#endif	/* (LDBL_MANT_DIG == DBL_MANT_DIG) */
