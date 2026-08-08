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

export module pbsd.lib.libc.amd64.gen.b0055;

export namespace pbsd::lib_libc_amd64_gen::b0055 {

/*
 * x86_ieeefp.h / machine/amd64/ieeefp.h types and constants used by the
 * four fpget*() wrappers in this batch.
 */
typedef enum {
	FP_RN=0,	/* round to nearest */
	FP_RM,		/* round down towards minus infinity */
	FP_RP,		/* round up towards plus infinity */
	FP_RZ		/* truncate */
} fp_rnd_t;

typedef enum {
	FP_PS=0,	/* 24 bit (single-precision) */
	FP_PRS,		/* reserved */
	FP_PD,		/* 53 bit (double-precision) */
	FP_PE		/* 64 bit (extended-precision) */
} fp_prec_t;

using fp_except_t = int;

#define FP_MSKS_FLD	0x3f	/* exception masks field */
#define FP_PRC_FLD	0x300	/* precision control field */
#define	FP_RND_FLD	0xc00	/* rounding control field */

#define FP_STKY_FLD	0x3f	/* sticky flags field */

#define FP_MSKS_OFF	0	/* exception masks offset */
#define FP_PRC_OFF	8	/* precision control offset */
#define	FP_RND_OFF	10	/* rounding control offset */

#define FP_STKY_OFF	0	/* sticky flags offset */

#define	SSE_STKY_FLD	0x3f	/* exception flags */
#define	SSE_STKY_OFF	0	/* exception flags offset */

#define	__fnstcw(addr)	__asm __volatile("fnstcw %0" : "=m" (*(addr)))
#define	__fnstsw(addr)	__asm __volatile("fnstsw %0" : "=m" (*(addr)))
#define	__stmxcsr(addr)	__asm __volatile("stmxcsr %0" : "=m" (*(addr)))

static inline fp_rnd_t
__fpgetround(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((fp_rnd_t)((_cw & FP_RND_FLD) >> FP_RND_OFF));
}

static inline fp_prec_t
__fpgetprec(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((fp_prec_t)((_cw & FP_PRC_FLD) >> FP_PRC_OFF));
}

static inline fp_except_t
__fpgetmask(void)
{
	unsigned short _cw;

	__fnstcw(&_cw);
	return ((~_cw & FP_MSKS_FLD) >> FP_MSKS_OFF);
}

static inline fp_except_t
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

fp_prec_t
fpgetprec(void)
{
	return __fpgetprec();
}

fp_rnd_t
fpgetround(void)
{
	return __fpgetround();
}

fp_except_t
fpgetmask(void)
{
	return __fpgetmask();
}

fp_except_t
fpgetsticky(void)
{
	return __fpgetsticky();
}

} /* namespace pbsd::lib_libc_amd64_gen::b0055 */
