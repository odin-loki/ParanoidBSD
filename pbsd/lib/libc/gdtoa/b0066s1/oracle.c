/*
 * b0066s1 oracle -- the reference implementation for the differential test.
 *
 * This is lib/libc/gdtoa/_ldtoa.c concatenated, with the function renamed
 * with a ref_ prefix.  The function bodies are the originals, unmodified.  Only
 * declarations, #defines and #undefs have been added, so that the file
 * can share one translation unit and so that the definitions they would
 * normally pick up from <fpmath.h>, <machine/_fpmath.h>, "gdtoa.h" and
 * "gdtoaimp.h" are available here.
 *
 * This file additionally supplies the three helpers the batch sources call but
 * do not define -- rv_alloc(), nrv_alloc() and gdtoa().  Those live in
 * contrib/gdtoa, outside this batch.  The versions here are instrumented:
 * storage is handed out from a per-side arena that the harness pre-fills with
 * the guard byte 0x7f, and every call is recorded in a log.  The port and the
 * oracle are linked against these very same helpers, so anything the harness
 * sees differ was produced by the code under test.
 */

#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * HardenedBSD's <float.h> defines FLT_ROUNDS as __flt_rounds(), which tracks
 * fesetround(3).  GCC's <float.h> hardcodes it to 1.  Restore the target
 * definition, otherwise __ldtoa()'s rounding-mode fixup can never be reached
 * with a rounding mode other than "to nearest".
 */
extern int __flt_rounds(void);
#undef FLT_ROUNDS
#define	FLT_ROUNDS	__flt_rounds()

int
__flt_rounds(void)
{

	switch (fegetround()) {
	case FE_TOWARDZERO:
		return (0);
	case FE_TONEAREST:
		return (1);
	case FE_UPWARD:
		return (2);
	case FE_DOWNWARD:
		return (3);
	}
	return (-1);
}

/* ------------------------------------------------------------------ */
/* From lib/libc/include/fpmath.h (_BYTE_ORDER == _LITTLE_ENDIAN and	*/
/* _IEEE_WORD_ORDER == _LITTLE_ENDIAN, i.e. the amd64 branch).		*/
/* ------------------------------------------------------------------ */

#define	DBL_MANH_SIZE	20
#define	DBL_MANL_SIZE	32

union IEEEd2bits {
	double	d;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
	} bits;
};

/* ------------------------------------------------------------------ */
/* From lib/libc/amd64/_fpmath.h.					*/
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

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)(u).bits.manh;		\
} while (0)

/* ------------------------------------------------------------------ */
/* From contrib/gdtoa/gdtoa.h and contrib/gdtoa/gdtoaimp.h.		*/
/* ------------------------------------------------------------------ */

typedef unsigned int ULong;

enum {	/* return values from strtodg */
	STRTOG_Zero	= 0,
	STRTOG_Normal	= 1,
	STRTOG_Denormal	= 2,
	STRTOG_Infinite	= 3,
	STRTOG_NaN	= 4,
	STRTOG_NaNbits	= 5,
	STRTOG_NoNumber	= 6,
	STRTOG_Retmask	= 7,

	STRTOG_Neg	= 0x08,
	STRTOG_Inexlo	= 0x10,
	STRTOG_Inexhi	= 0x20,
	STRTOG_Inexact	= 0x30,
	STRTOG_Underflow= 0x40,
	STRTOG_Overflow	= 0x80
};

typedef struct
FPI {
	int nbits;
	int emin;
	int emax;
	int rounding;
	int sudden_underflow;
	} FPI;

extern char *rv_alloc(int);
extern char *nrv_alloc(const char *, char **, int);
extern char *gdtoa(FPI *fpi, int be, ULong *bits, int *kindp,
		   int mode, int ndigits, int *decpt, char **rve);

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
	int		gdtoa_calls;
	int		fpi_nbits;
	int		fpi_emin;
	int		fpi_emax;
	int		fpi_rounding;
	int		fpi_sudden_underflow;
	int		g_be;
	int		g_kind;
	int		g_mode;
	int		g_ndigits;
	unsigned int	g_bits[2];
};

unsigned char	pbsd_arena[2][PBSD_ARENA_SIZE];
struct pbsd_log	pbsd_logs[2];
int		pbsd_side;
int		pbsd_gdtoa_decpt;

static long	pbsd_arena_used[2];

void
pbsd_case_begin(int side)
{

	pbsd_side = side;
	memset(pbsd_arena[side], 0x7f, PBSD_ARENA_SIZE);
	memset(&pbsd_logs[side], 0, sizeof(pbsd_logs[side]));
	pbsd_arena_used[side] = 0;
}

/*
 * Size of the block contrib/gdtoa's Balloc()/rv_alloc() pair would hand back
 * for a request of i bytes.  Reproduced here so that the slack past the
 * nominal write window -- which the real rv_alloc() also provides, and which
 * _hdtoa() and friends rely on when they store the terminating NUL -- has the
 * same size in the test as it does in libc.
 */
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
		fprintf(stderr, "b0066s1: arena exhausted (request %d)\n", i);
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

/*
 * Instrumented stand-in for contrib/gdtoa's gdtoa().  __ldtoa()'s contract
 * with gdtoa() is the thing under test, so every argument is recorded and also
 * rendered into the returned string; *decpt comes from a value the harness
 * chooses, which is how the -32768 fixup gets driven from both sides.
 */
char *
gdtoa(FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits,
    int *decpt, char **rve)
{
	struct pbsd_log *lg;
	char tmp[192];

	lg = &pbsd_logs[pbsd_side];
	lg->gdtoa_calls++;
	lg->fpi_nbits = fpi->nbits;
	lg->fpi_emin = fpi->emin;
	lg->fpi_emax = fpi->emax;
	lg->fpi_rounding = fpi->rounding;
	lg->fpi_sudden_underflow = fpi->sudden_underflow;
	lg->g_be = be;
	lg->g_kind = *kindp;
	lg->g_mode = mode;
	lg->g_ndigits = ndigits;
	lg->g_bits[0] = bits[0];
	lg->g_bits[1] = bits[1];

	snprintf(tmp, sizeof(tmp), "%d %d %d %d %d %d %d %d %d %08x %08x",
	    fpi->nbits, fpi->emin, fpi->emax, fpi->rounding,
	    fpi->sudden_underflow, be, *kindp, mode, ndigits,
	    bits[0], bits[1]);

	*decpt = pbsd_gdtoa_decpt;
	return (nrv_alloc(tmp, rve, (int)strlen(tmp)));
}

/* ================================================================== */
/* lib/libc/gdtoa/_ldtoa.c						*/
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 David Schultz <das@FreeBSD.ORG>
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

/*
 * ldtoa() is a wrapper for gdtoa() that makes it smell like dtoa(),
 * except that the floating point argument is passed by reference.
 * When dtoa() is passed a NaN or infinity, it sets expt to 9999.
 * However, a long double could have a valid exponent of 9999, so we
 * use INT_MAX in ldtoa() instead.
 */
char *
ref___ldtoa(long double *ld, int mode, int ndigits, int *decpt, int *sign,
    char **rve)
{
	FPI fpi = {
		LDBL_MANT_DIG,			/* nbits */
		LDBL_MIN_EXP - LDBL_MANT_DIG,	/* emin */
		LDBL_MAX_EXP - LDBL_MANT_DIG,	/* emax */
		FLT_ROUNDS,	       		/* rounding */
#ifdef Sudden_Underflow	/* unused, but correct anyway */
		1
#else
		0
#endif
	};
	int be, kind;
	char *ret;
	union IEEEl2bits u;
	uint32_t bits[(LDBL_MANT_DIG + 31) / 32];
	void *vbits = bits;

	u.e = *ld;

	/*
	 * gdtoa doesn't know anything about the sign of the number, so
	 * if the number is negative, we need to swap rounding modes of
	 * 2 (upwards) and 3 (downwards).
	 */
	*sign = u.bits.sign;
	fpi.rounding ^= (fpi.rounding >> 1) & u.bits.sign;

	be = u.bits.exp - (LDBL_MAX_EXP - 1) - (LDBL_MANT_DIG - 1);
	LDBL_TO_ARRAY32(u, bits);

	switch (fpclassify(u.e)) {
	case FP_NORMAL:
		kind = STRTOG_Normal;
#ifdef	LDBL_IMPLICIT_NBIT
		bits[LDBL_MANT_DIG / 32] |= 1 << ((LDBL_MANT_DIG - 1) % 32);
#endif /* LDBL_IMPLICIT_NBIT */
		break;
	case FP_ZERO:
		kind = STRTOG_Zero;
		break;
	case FP_SUBNORMAL:
		kind = STRTOG_Denormal;
		be++;
		break;
	case FP_INFINITE:
		kind = STRTOG_Infinite;
		break;
	case FP_NAN:
		kind = STRTOG_NaN;
		break;
	default:
		abort();
	}

	ret = gdtoa(&fpi, be, vbits, &kind, mode, ndigits, decpt, rve);
	if (*decpt == -32768)
		*decpt = INT_MAX;
	return ret;
}

