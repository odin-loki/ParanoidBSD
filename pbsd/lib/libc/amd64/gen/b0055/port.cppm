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
 * PBSD port of:
 *   lib/libc/amd64/gen/fpgetprec.c
 *   lib/libc/amd64/gen/fpgetround.c
 *   lib/libc/amd64/gen/fpgetmask.c
 *   lib/libc/amd64/gen/fpgetsticky.c
 *
 * Each of the four batch sources is a two-line out-of-line wrapper compiled
 * with __IEEEFP_NOINLINES__ defined, so the substance of the port is the
 * matching static __inline in sys/amd64/include/ieeefp.h together with the
 * types, bit-field masks/offsets and asm macros from
 * sys/x86/include/x86_ieeefp.h.  Behaviour is preserved exactly, including the
 * asymmetry that the fpget*() side reads only x87 state except for
 * fpgetsticky(), which merges the x87 and SSE sticky fields, and including the
 * fact that fpgetmask() returns the ones' complement of the control word's
 * mask field so a "1" bit means enabled rather than disabled.
 */

export module pbsd.lib.libc.amd64.gen.b0055;

export namespace pbsd::lib_libc_amd64_gen::b0055 {

/*
 * IEEE floating point type, constant and function definitions.
 * XXX: {FP,SSE}*FLD and {FP,SSE}*OFF are undocumented pollution.
 */

/*
 * Rounding modes.
 */
enum fp_rnd_t {
	FP_RN = 0,	/* round to nearest */
	FP_RM,		/* round down towards minus infinity */
	FP_RP,		/* round up towards plus infinity */
	FP_RZ		/* truncate */
};

/*
 * Precision (i.e., rounding precision) modes.
 */
enum fp_prec_t {
	FP_PS = 0,	/* 24 bit (single-precision) */
	FP_PRS,		/* reserved */
	FP_PD,		/* 53 bit (double-precision) */
	FP_PE		/* 64 bit (extended-precision) */
};

/* #define fp_except_t	int */
using fp_except_t = int;

/*
 * Exception bit masks.
 */
inline constexpr int FP_X_INV	= 0x01;	/* invalid operation */
inline constexpr int FP_X_DNML	= 0x02;	/* denormal */
inline constexpr int FP_X_DZ	= 0x04;	/* zero divide */
inline constexpr int FP_X_OFL	= 0x08;	/* overflow */
inline constexpr int FP_X_UFL	= 0x10;	/* underflow */
inline constexpr int FP_X_IMP	= 0x20;	/* (im)precision */
inline constexpr int FP_X_STK	= 0x40;	/* stack fault */

/*
 * FPU control word bit-field masks.
 */
inline constexpr int FP_MSKS_FLD = 0x3f;	/* exception masks field */
inline constexpr int FP_PRC_FLD	 = 0x300;	/* precision control field */
inline constexpr int FP_RND_FLD	 = 0xc00;	/* rounding control field */

/*
 * FPU status word bit-field masks.
 */
inline constexpr int FP_STKY_FLD = 0x3f;	/* sticky flags field */

/*
 * FPU control word bit-field offsets (shift counts).
 */
inline constexpr int FP_MSKS_OFF = 0;		/* exception masks offset */
inline constexpr int FP_PRC_OFF	 = 8;		/* precision control offset */
inline constexpr int FP_RND_OFF	 = 10;		/* rounding control offset */

/*
 * FPU status word bit-field offsets (shift counts).
 */
inline constexpr int FP_STKY_OFF = 0;		/* sticky flags offset */

/*
 * SSE mxcsr register bit-field masks.
 */
inline constexpr unsigned SSE_STKY_FLD	= 0x3f;		/* exception flags */
inline constexpr unsigned SSE_DAZ_FLD	= 0x40;		/* Denormals are zero */
inline constexpr unsigned SSE_MSKS_FLD	= 0x1f80;	/* exception masks */
inline constexpr unsigned SSE_RND_FLD	= 0x6000;	/* rounding control */
inline constexpr unsigned SSE_FZ_FLD	= 0x8000;	/* flush to zero */

/*
 * SSE mxcsr register bit-field offsets (shift counts).
 */
inline constexpr int SSE_STKY_OFF = 0;		/* exception flags offset */
inline constexpr int SSE_DAZ_OFF  = 6;		/* DAZ exception mask offset */
inline constexpr int SSE_MSKS_OFF = 7;		/* other exception masks */
inline constexpr int SSE_RND_OFF  = 13;		/* rounding control offset */
inline constexpr int SSE_FZ_OFF	  = 15;		/* flush to zero offset */

} /* namespace pbsd::lib_libc_amd64_gen::b0055 */

namespace pbsd::lib_libc_amd64_gen::b0055 {

/*
 * #define	__fnstcw(addr)	__asm __volatile("fnstcw %0" : "=m" (*(addr)))
 * #define	__fnstsw(addr)	__asm __volatile("fnstsw %0" : "=m" (*(addr)))
 * #define	__stmxcsr(addr)	__asm __volatile("stmxcsr %0" : "=m" (*(addr)))
 */
inline void
__fnstcw(unsigned short *addr)
{
	__asm__ __volatile__("fnstcw %0" : "=m" (*(addr)));
}

inline void
__fnstsw(unsigned short *addr)
{
	__asm__ __volatile__("fnstsw %0" : "=m" (*(addr)));
}

inline void
__stmxcsr(unsigned *addr)
{
	__asm__ __volatile__("stmxcsr %0" : "=m" (*(addr)));
}

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

fp_rnd_t
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

fp_prec_t
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

fp_except_t
__fpgetmask(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((~_cw & FP_MSKS_FLD) >> FP_MSKS_OFF);
}

fp_except_t
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

} /* namespace pbsd::lib_libc_amd64_gen::b0055 */

export namespace pbsd::lib_libc_amd64_gen::b0055 {

/* lib/libc/amd64/gen/fpgetprec.c */
fp_prec_t fpgetprec(void)
{
	return __fpgetprec();
}

/* lib/libc/amd64/gen/fpgetround.c */
fp_rnd_t fpgetround(void)
{
	return __fpgetround();
}

/* lib/libc/amd64/gen/fpgetmask.c */
fp_except_t fpgetmask(void)
{
	return __fpgetmask();
}

/* lib/libc/amd64/gen/fpgetsticky.c */
fp_except_t fpgetsticky(void)
{
	return __fpgetsticky();
}

} /* namespace pbsd::lib_libc_amd64_gen::b0055 */
