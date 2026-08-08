/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 2003 Peter Wemm.
 * Copyright (c) 1990 Andrew Moore, Talke Studio
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * b0055 reference oracle.
 *
 * Contents:
 *   lib/libc/amd64/gen/fpgetprec.c
 *   lib/libc/amd64/gen/fpgetround.c
 *   lib/libc/amd64/gen/fpgetmask.c
 *   lib/libc/amd64/gen/fpgetsticky.c
 *
 * Each of the four batch sources is just
 *
 *	#define __IEEEFP_NOINLINES__ 1
 *	#include <ieeefp.h>
 *	<type> <name>(void) { return __<name>(); }
 *
 * <ieeefp.h> does not exist on this build host, so the declarations it would
 * have supplied are reproduced below verbatim from
 *   sys/x86/include/x86_ieeefp.h  (types, bit-field masks/offsets, asm macros)
 *   sys/amd64/include/ieeefp.h    (the static __inline __fpget*() bodies)
 * of the same source tree.  Only the four public wrapper names carry the ref_
 * prefix; no function body anywhere in this file has been altered.
 */

#define __IEEEFP_NOINLINES__ 1

/* ------------------------------------------------------------------ */
/* Missing declarations: sys/x86/include/x86_ieeefp.h                 */
/* ------------------------------------------------------------------ */

/*
 * Rounding modes.
 */
typedef enum {
	FP_RN=0,	/* round to nearest */
	FP_RM,		/* round down towards minus infinity */
	FP_RP,		/* round up towards plus infinity */
	FP_RZ		/* truncate */
} fp_rnd_t;

/*
 * Precision (i.e., rounding precision) modes.
 */
typedef enum {
	FP_PS=0,	/* 24 bit (single-precision) */
	FP_PRS,		/* reserved */
	FP_PD,		/* 53 bit (double-precision) */
	FP_PE		/* 64 bit (extended-precision) */
} fp_prec_t;

#define fp_except_t	int

/*
 * Exception bit masks.
 */
#define FP_X_INV	0x01	/* invalid operation */
#define FP_X_DNML	0x02	/* denormal */
#define FP_X_DZ		0x04	/* zero divide */
#define FP_X_OFL	0x08	/* overflow */
#define FP_X_UFL	0x10	/* underflow */
#define FP_X_IMP	0x20	/* (im)precision */
#define FP_X_STK	0x40	/* stack fault */

/*
 * FPU control word bit-field masks.
 */
#define FP_MSKS_FLD	0x3f	/* exception masks field */
#define FP_PRC_FLD	0x300	/* precision control field */
#define	FP_RND_FLD	0xc00	/* rounding control field */

/*
 * FPU status word bit-field masks.
 */
#define FP_STKY_FLD	0x3f	/* sticky flags field */

/*
 * FPU control word bit-field offsets (shift counts).
 */
#define FP_MSKS_OFF	0	/* exception masks offset */
#define FP_PRC_OFF	8	/* precision control offset */
#define	FP_RND_OFF	10	/* rounding control offset */

/*
 * FPU status word bit-field offsets (shift counts).
 */
#define FP_STKY_OFF	0	/* sticky flags offset */

#define	__fldcw(addr)	__asm__ __volatile__("fldcw %0" : : "m" (*(addr)))
#define	__fldenv(addr)	__asm__ __volatile__("fldenv %0" : : "m" (*(addr)))
#define	__fnclex()	__asm__ __volatile__("fnclex")
#define	__fnstcw(addr)	__asm__ __volatile__("fnstcw %0" : "=m" (*(addr)))
#define	__fnstenv(addr)	__asm__ __volatile__("fnstenv %0" : "=m" (*(addr)))
#define	__fnstsw(addr)	__asm__ __volatile__("fnstsw %0" : "=m" (*(addr)))
#define	__ldmxcsr(addr)	__asm__ __volatile__("ldmxcsr %0" : : "m" (*(addr)))
#define	__stmxcsr(addr)	__asm__ __volatile__("stmxcsr %0" : "=m" (*(addr)))

/* ------------------------------------------------------------------ */
/* Missing declarations: sys/amd64/include/ieeefp.h                   */
/* ------------------------------------------------------------------ */

/*
 * SSE mxcsr register bit-field masks.
 */
#define	SSE_STKY_FLD	0x3f	/* exception flags */
#define	SSE_DAZ_FLD	0x40	/* Denormals are zero */
#define	SSE_MSKS_FLD	0x1f80	/* exception masks field */
#define	SSE_RND_FLD	0x6000	/* rounding control */
#define	SSE_FZ_FLD	0x8000	/* flush to zero on underflow */

/*
 * SSE mxcsr register bit-field offsets (shift counts).
 */
#define	SSE_STKY_OFF	0	/* exception flags offset */
#define	SSE_DAZ_OFF	6	/* DAZ exception mask offset */
#define	SSE_MSKS_OFF	7	/* other exception masks offset */
#define	SSE_RND_OFF	13	/* rounding control offset */
#define	SSE_FZ_OFF	15	/* flush to zero offset */

/*
 * General notes about conflicting SSE vs FP status bits.
 * This code assumes that software will not fiddle with the control
 * bits of the SSE and x87 in such a way to get them out of sync and
 * still expect this to work.  Break this at your peril.
 * Because I based this on the i386 port, the x87 state is used for
 * the fpget*() functions, and is shadowed into the SSE state for
 * the fpset*() functions.  For dual source fpget*() functions, I
 * merge the two together.  I think.
 */

static __inline fp_rnd_t
__fpgetround(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((fp_rnd_t)((_cw & FP_RND_FLD) >> FP_RND_OFF));
}

/*
 * Get or set the rounding precision for x87 arithmetic operations.
 * There is no equivalent SSE mode or control.
 */

static __inline fp_prec_t
__fpgetprec(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((fp_prec_t)((_cw & FP_PRC_FLD) >> FP_PRC_OFF));
}

/*
 * Get or set the exception mask.
 * Note that the x87 mask bits are inverted by the API -- a mask bit of 1
 * means disable for x87 and SSE, but for fp*mask() it means enable.
 */

static __inline fp_except_t
__fpgetmask(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((~_cw & FP_MSKS_FLD) >> FP_MSKS_OFF);
}

static __inline fp_except_t
__fpgetsticky(void)
{
	unsigned _ex, _mxcsr;
	unsigned short _sw;

	__fnstsw(&_sw);
	_ex = (_sw & FP_STKY_FLD) >> FP_STKY_OFF;
	__stmxcsr(&_mxcsr);
	_ex |= (_mxcsr & SSE_STKY_FLD) >> SSE_STKY_OFF;
	return ((fp_except_t)_ex);
}

/* ------------------------------------------------------------------ */
/* lib/libc/amd64/gen/fpgetprec.c                                     */
/* ------------------------------------------------------------------ */

fp_prec_t ref_fpgetprec(void)
{
	return __fpgetprec();
}

/* ------------------------------------------------------------------ */
/* lib/libc/amd64/gen/fpgetround.c                                    */
/* ------------------------------------------------------------------ */

fp_rnd_t ref_fpgetround(void)
{
	return __fpgetround();
}

/* ------------------------------------------------------------------ */
/* lib/libc/amd64/gen/fpgetmask.c                                     */
/* ------------------------------------------------------------------ */

fp_except_t ref_fpgetmask(void)
{
	return __fpgetmask();
}

/* ------------------------------------------------------------------ */
/* lib/libc/amd64/gen/fpgetsticky.c                                   */
/* ------------------------------------------------------------------ */

fp_except_t ref_fpgetsticky(void)
{
	return __fpgetsticky();
}
