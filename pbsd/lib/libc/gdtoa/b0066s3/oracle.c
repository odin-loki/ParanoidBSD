/*
 * b0066s3 oracle -- the reference implementation for the differential test.
 *
 * This is lib/libc/gdtoa/_hdtoa.c with the function renamed ref___hdtoa.
 * The function body is the original, unmodified.  Declarations and the
 * instrumented rv_alloc()/nrv_alloc() helpers the source calls but does not
 * define have been added so the batch is self-contained.
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

#define	DBL_MANH_SIZE	20
#define	DBL_MANL_SIZE	32

typedef unsigned int ULong;

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
		fprintf(stderr, "b0066s3: arena exhausted (request %d)\n", i);
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

/* ================================================================== */
/* lib/libc/gdtoa/_hdtoa.c						*/
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

/* Strings values used by dtoa() */
#define	INFSTR	"Infinity"
#define	NANSTR	"NaN"

#define	DBL_ADJ	(DBL_MAX_EXP - 2)
#define	SIGFIGS	((DBL_MANT_DIG + 3) / 4 + 1)

static const float one[] = { 1.0f, -1.0f };

/*
 * This procedure converts a double-precision number in IEEE format
 * into a string of hexadecimal digits and an exponent of 2.  Its
 * behavior is bug-for-bug compatible with dtoa() in mode 2, with the
 * following exceptions:
 *
 * - An ndigits < 0 causes it to use as many digits as necessary to
 *   represent the number exactly.
 * - The additional xdigs argument should point to either the string
 *   "0123456789ABCDEF" or the string "0123456789abcdef", depending on
 *   which case is desired.
 * - This routine does not repeat dtoa's mistake of setting decpt
 *   to 9999 in the case of an infinity or NaN.  INT_MAX is used
 *   for this purpose instead.
 *
 * Note that the C99 standard does not specify what the leading digit
 * should be for a non-zero number.  For instance, 0x1.3p3 is the same
 * as 0x2.6p2 is the same as 0x4.cp3.  This implementation always makes
 * the leading digit a 1. This ensures that the exponent printed is the
 * actual base-2 exponent, i.e., ilogb(d).
 *
 * Inputs:	d, xdigs, ndigits
 * Outputs:	decpt, sign, rve
 */
char *
ref___hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve)
{
	union IEEEd2bits u;
	char *s, *s0;
	int bufsize;
	uint32_t manh, manl;

	u.d = d;
	*sign = u.bits.sign;

	switch (fpclassify(d)) {
	case FP_NORMAL:
		*decpt = u.bits.exp - DBL_ADJ;
		break;
	case FP_ZERO:
		*decpt = 1;
		return (nrv_alloc("0", rve, 1));
	case FP_SUBNORMAL:
		u.d *= 0x1p514;
		*decpt = u.bits.exp - (514 + DBL_ADJ);
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
		int offset = 4 * ndigits + DBL_MAX_EXP - 4 - DBL_MANT_DIG;
		u.bits.exp = offset;
		u.d += redux;
		u.d -= redux;
		*decpt += u.bits.exp - offset;
	}

	manh = u.bits.manh;
	manl = u.bits.manl;
	*s0 = '1';
	for (s = s0 + 1; s < s0 + bufsize; s++) {
		*s = xdigs[(manh >> (DBL_MANH_SIZE - 4)) & 0xf];
		manh = (manh << 4) | (manl >> (DBL_MANL_SIZE - 4));
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
