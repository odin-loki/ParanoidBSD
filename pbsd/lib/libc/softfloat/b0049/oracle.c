/*
 * oracle.c -- reference implementation ("specification") for PBSD batch b0049.
 *
 * The four HardenedBSD sources of this batch are concatenated below verbatim,
 * with each function renamed with a ref_ prefix.  No function body has been
 * touched.
 *
 * The originals #include "softfloat-for-gcc.h", "milieu.h", "softfloat.h",
 * "namespace.h" and <ieeefp.h>, none of which can be compiled standalone here,
 * so the declarations they provide are reproduced in the prologue below.  The
 * prologue also carries the softfloat.c entities that this batch *calls* but
 * does not define (float32_eq, float64_eq, float_raise and the
 * float_exception_* globals).  Those deliberately keep their real names: the
 * C++ port links against these very definitions, so both sides of the
 * differential test share a single dependency implementation and any observed
 * divergence can only come from the ported code itself.
 *
 * float_raise() here only records the flag word; it does not raise SIGFPE, so
 * that the harness can drive the signalling-NaN paths without dying.  Again,
 * both sides use this same function.
 */

/* ------------------------------------------------------------------------ */
/* prologue: milieu.h -- SoftFloat integer types (x86-64 / LP64)            */
/* ------------------------------------------------------------------------ */

typedef int flag;

typedef unsigned int uint8;
typedef signed int int8;
typedef unsigned int uint16;
typedef int int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned long long int uint64;
typedef signed long long int int64;

typedef unsigned int bits8;
typedef signed int sbits8;
typedef unsigned int bits16;
typedef signed int sbits16;
typedef unsigned int bits32;
typedef signed int sbits32;
typedef unsigned long long int bits64;
typedef signed long long int sbits64;

#define LIT64( a ) a##ULL
#define INLINE static inline

/* ------------------------------------------------------------------------ */
/* prologue: <ieeefp.h> (x86)                                               */
/* ------------------------------------------------------------------------ */

typedef int fp_except;

#define FP_X_INV	0x01	/* invalid operation exception */
#define FP_X_DNML	0x02	/* denormalization exception */
#define FP_X_DZ		0x04	/* divide-by-zero exception */
#define FP_X_OFL	0x08	/* overflow exception */
#define FP_X_UFL	0x10	/* underflow exception */
#define FP_X_IMP	0x20	/* imprecise (loss of precision) */
#define FP_X_STK	0x40	/* stack fault */

/* ------------------------------------------------------------------------ */
/* prologue: softfloat.h -- IEC/IEEE floating-point types and enums         */
/* ------------------------------------------------------------------------ */

typedef unsigned int float32;
typedef unsigned long long int float64;

/*
 * Enable the quadruple-precision type used by negtf2.c.
 */
#define FLOAT128

typedef struct {
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	bits64 high, low;
#else
	bits64 low, high;
#endif
} float128;

/*
 * On every target except the old mixed-endian ARM soft-float ABI these are
 * the identity.
 */
#define FLOAT64_DEMANGLE(a)	(a)
#define FLOAT64_MANGLE(a)	(a)

enum {
	float_flag_inexact   = FP_X_IMP,
	float_flag_underflow = FP_X_UFL,
	float_flag_overflow  = FP_X_OFL,
	float_flag_divbyzero = FP_X_DZ,
	float_flag_invalid   = FP_X_INV
};

/* ------------------------------------------------------------------------ */
/* prologue: softfloat.c dependencies of this batch (NOT part of the batch)  */
/* ------------------------------------------------------------------------ */

fp_except float_exception_flags = 0;
fp_except float_exception_mask = 0;

void
float_raise(fp_except flags)
{

	float_exception_flags |= flags;
}

INLINE bits32 extractFloat32Frac( float32 a )
{

	return a & 0x007FFFFF;
}

INLINE int16 extractFloat32Exp( float32 a )
{

	return ( a>>23 ) & 0xFF;
}

INLINE bits64 extractFloat64Frac( float64 a )
{

	return a & LIT64( 0x000FFFFFFFFFFFFF );
}

INLINE int16 extractFloat64Exp( float64 a )
{

	return ( a>>52 ) & 0x7FF;
}

flag float32_is_signaling_nan( float32 a )
{

	return ( ( ( a>>22 ) & 0x1FF ) == 0x1FE ) && ( a & 0x003FFFFF );
}

flag float64_is_signaling_nan( float64 a )
{

	return
	       ( ( ( a>>51 ) & 0xFFF ) == 0xFFE )
	    && ( a & LIT64( 0x0007FFFFFFFFFFFF ) );
}

flag float32_eq( float32 a, float32 b )
{

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
	     || ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
	   ) {
		if ( float32_is_signaling_nan( a )
		  || float32_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid );
		}
		return 0;
	}
	return ( a == b ) || ( (bits32) ( ( a | b )<<1 ) == 0 );
}

flag float64_eq( float64 a, float64 b )
{

	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
	     || ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
	   ) {
		if ( float64_is_signaling_nan( a )
		  || float64_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid );
		}
		return 0;
	}
	return ( a == b ) || ( (bits64) ( ( a | b )<<1 ) == 0 );
}

/* ======================================================================== */
/* lib/libc/softfloat/negtf2.c                                              */
/* ======================================================================== */

/* $NetBSD: negtf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */

/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#ifdef FLOAT128

float128 ref___negtf2(float128);

float128
ref___negtf2(float128 a)
{

	/* libgcc1.c says -a */
	a.high ^= FLOAT64_MANGLE(0x8000000000000000ULL);
	return a;
}

#endif /* FLOAT128 */

/* ======================================================================== */
/* lib/libc/softfloat/unorddf2.c                                            */
/* ======================================================================== */

/* $NetBSD: unorddf2.c,v 1.1 2003/05/06 08:58:19 rearnsha Exp $ */

/*
 * Written by Richard Earnshaw, 2003.  This file is in the Public Domain.
 */

flag ref___unorddf2(float64, float64);

flag
ref___unorddf2(float64 a, float64 b)
{
	/*
	 * The comparison is unordered if either input is a NaN.
	 * Test for this by comparing each operand with itself.
	 * We must perform both comparisons to correctly check for
	 * signalling NaNs.
	 */
	return 1 ^ (float64_eq(a, a) & float64_eq(b, b));
}

/* ======================================================================== */
/* lib/libc/softfloat/unordsf2.c                                            */
/* ======================================================================== */

/* $NetBSD: unordsf2.c,v 1.1 2003/05/06 08:58:20 rearnsha Exp $ */

/*
 * Written by Richard Earnshaw, 2003.  This file is in the Public Domain.
 */

flag ref___unordsf2(float32, float32);

flag
ref___unordsf2(float32 a, float32 b)
{
	/*
	 * The comparison is unordered if either input is a NaN.
	 * Test for this by comparing each operand with itself.
	 * We must perform both comparisons to correctly check for
	 * signalling NaNs.
	 */
	return 1 ^ (float32_eq(a, a) & float32_eq(b, b));
}

/* ======================================================================== */
/* lib/libc/softfloat/fpgetmask.c                                           */
/* ======================================================================== */

/* $NetBSD: fpgetmask.c,v 1.4 2008/04/28 20:23:00 martin Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Neil A. Carson and Mark Brinicombe
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

fp_except
ref_fpgetmask(void)
{
	return float_exception_mask;
}
