/*
 * PBSD batch b0235 -- C++23 port of:
 *   lib/libc/stdlib/qsort_r.c
 *   lib/libc/stdlib/qsort_s.c
 *   lib/libc/stdlib/memalignment.c
 *   lib/libc/stdlib/qsort_r_compat.c
 *
 * qsort_r.c, qsort_s.c and qsort_r_compat.c each consist of a single #define
 * followed by #include "qsort.c"; the three macro expansions of qsort.c are
 * therefore written out below, one section each.  See skipped.txt for the
 * part of qsort_r_compat.c that is not portable.
 *
 * Original copyright headers follow.
 */

/*
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 * This file is in the public domain.  Originally written by Garrett
 * A. Wollman.
 */

module;

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef __predict_false
#define	__predict_false(exp)	__builtin_expect(((exp) != 0), 0)
#endif

#ifndef RSIZE_MAX
#define	RSIZE_MAX		(SIZE_MAX >> 1)
#endif

#define	MIN(a, b)	((a) < (b) ? a : b)

export module pbsd.lib.libc.stdlib.b0235;

namespace pbsd::lib_libc_stdlib::b0235 {

using errno_t = int;
using rsize_t = ::size_t;

/*
 * Stand-in for libc_private.h's __throw_constraint_handler_s().  The default
 * FreeBSD constraint handler is ignore_handler_s(), i.e. it has no effect on
 * control flow; the diagnostic it receives is recorded here so the
 * differential harness can observe which constraint fired.  The identical
 * shim exists on the C oracle side.
 */
char		constraint_msg_buf[256];
int		constraint_last_error;
unsigned long	constraint_call_count;

void
__throw_constraint_handler_s(const char *msg, errno_t error)
{
	::size_t i;

	constraint_call_count++;
	constraint_last_error = error;
	for (i = 0; i + 1 < sizeof(constraint_msg_buf) && msg != nullptr &&
	    msg[i] != '\0'; i++)
		constraint_msg_buf[i] = msg[i];
	constraint_msg_buf[i] = '\0';
}

} /* namespace */

export namespace pbsd::lib_libc_stdlib::b0235 {

/* Observation points for the constraint-handler shim above. */
unsigned long
constraint_handler_count()
{
	return (constraint_call_count);
}

int
constraint_handler_error()
{
	return (constraint_last_error);
}

const char *
constraint_handler_msg()
{
	return (constraint_msg_buf);
}

void
constraint_handler_reset()
{
	constraint_call_count = 0;
	constraint_last_error = 0;
	constraint_msg_buf[0] = '\0';
}

} /* namespace */

/* ------------------------------------------------------------------ */
/* lib/libc/stdlib/memalignment.c                                     */
/* ------------------------------------------------------------------ */

export namespace pbsd::lib_libc_stdlib::b0235 {

::size_t
memalignment(const void *p)
{
	::uintptr_t align;

	if (p == NULL)
		return (0);

	align = (::uintptr_t)p;
	align &= -align;

#if UINTPTR_MAX > SIZE_MAX
	/* if alignment overflows size_t, return maximum possible */
	if (align > SIZE_MAX)
		align = SIZE_MAX - SIZE_MAX/2;
#endif

	return (align);
}

} /* namespace */

/* ------------------------------------------------------------------ */
/* lib/libc/stdlib/qsort_r.c   (qsort.c with I_AM_QSORT_R)            */
/* ------------------------------------------------------------------ */

export namespace pbsd::lib_libc_stdlib::b0235 {
typedef int		 cmp_r_t(const void *, const void *, void *);
}

namespace pbsd::lib_libc_stdlib::b0235 {

static inline char	*med3_r(char *, char *, char *, cmp_r_t *, void *);

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

static inline void
swapfunc_r(char *a, char *b, ::size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc_r(a, b, n)

#define	CMP(t, x, y) (cmp((x), (y), (t)))

static inline char *
med3_r(char *a, char *b, char *c, cmp_r_t *cmp, void *thunk
)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
	      :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

/*
 * The actual qsort() implementation is static to avoid preemptible calls when
 * recursing. Also give them different names for improved debugging.
 */
static void
local_qsort_r(void *a, ::size_t n, ::size_t es, cmp_r_t *cmp, void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	::size_t d1, d2;
	int cmp_result;

	/* if there are less than 2 elements, then sorting is not needed */
	if (__predict_false(n < 2))
		return;
loop:
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; 
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swapfunc_r(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			::size_t d = (n / 8) * es;

			pl = med3_r(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3_r(pm - d, pm, pm + d, cmp, thunk);
			pn = med3_r(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3_r(pl, pm, pn, cmp, thunk);
	}
	swapfunc_r((char *)a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swapfunc_r(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swapfunc_r(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc_r(pb, pc, es);
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap((char *)a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (::ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort_r(a, d1 / es, es, cmp, thunk);
		}
		if (d2 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	} else {
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es) {
			local_qsort_r(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

} /* namespace */

export namespace pbsd::lib_libc_stdlib::b0235 {

void
(qsort_r)(void *a, ::size_t n, ::size_t es, cmp_r_t *cmp, void *thunk)
{
	local_qsort_r(a, n, es, cmp, thunk);
}

} /* namespace */

/* ------------------------------------------------------------------ */
/* lib/libc/stdlib/qsort_r_compat.c (qsort.c with I_AM_QSORT_R_COMPAT)*/
/* ------------------------------------------------------------------ */

export namespace pbsd::lib_libc_stdlib::b0235 {
typedef int		 cmp_rc_t(void *, const void *, const void *);
}

namespace pbsd::lib_libc_stdlib::b0235 {

static inline char	*med3_rc(char *, char *, char *, cmp_rc_t *, void *);

static inline void
swapfunc_rc(char *a, char *b, ::size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#undef	vecswap
#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc_rc(a, b, n)

#undef	CMP
#define	CMP(t, x, y) (cmp((t), (x), (y)))

static inline char *
med3_rc(char *a, char *b, char *c, cmp_rc_t *cmp, void *thunk
)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
	      :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

static void
local_qsort_r_compat(void *a, ::size_t n, ::size_t es, cmp_rc_t *cmp,
    void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	::size_t d1, d2;
	int cmp_result;

	/* if there are less than 2 elements, then sorting is not needed */
	if (__predict_false(n < 2))
		return;
loop:
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; 
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swapfunc_rc(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			::size_t d = (n / 8) * es;

			pl = med3_rc(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3_rc(pm - d, pm, pm + d, cmp, thunk);
			pn = med3_rc(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3_rc(pl, pm, pn, cmp, thunk);
	}
	swapfunc_rc((char *)a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swapfunc_rc(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swapfunc_rc(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc_rc(pb, pc, es);
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap((char *)a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (::ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort_r_compat(a, d1 / es, es, cmp, thunk);
		}
		if (d2 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	} else {
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es) {
			local_qsort_r_compat(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

} /* namespace */

export namespace pbsd::lib_libc_stdlib::b0235 {

void
__qsort_r_compat(void *a, ::size_t n, ::size_t es, void *thunk, cmp_rc_t *cmp)
{
	local_qsort_r_compat(a, n, es, cmp, thunk);
}

} /* namespace */

/* ------------------------------------------------------------------ */
/* lib/libc/stdlib/qsort_s.c   (qsort.c with I_AM_QSORT_S)            */
/* ------------------------------------------------------------------ */

export namespace pbsd::lib_libc_stdlib::b0235 {
typedef int		 cmp_s_t(const void *, const void *, void *);
}

namespace pbsd::lib_libc_stdlib::b0235 {

static inline char	*med3_s(char *, char *, char *, cmp_s_t *, void *);

static inline void
swapfunc_s(char *a, char *b, ::size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#undef	vecswap
#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc_s(a, b, n)

#undef	CMP
#define	CMP(t, x, y) (cmp((x), (y), (t)))

static inline char *
med3_s(char *a, char *b, char *c, cmp_s_t *cmp, void *thunk
)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
	      :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

static void
local_qsort_s(void *a, ::size_t n, ::size_t es, cmp_s_t *cmp, void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	::size_t d1, d2;
	int cmp_result;

	/* if there are less than 2 elements, then sorting is not needed */
	if (__predict_false(n < 2))
		return;
loop:
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; 
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swapfunc_s(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = (char *)a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			::size_t d = (n / 8) * es;

			pl = med3_s(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3_s(pm - d, pm, pm + d, cmp, thunk);
			pn = med3_s(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3_s(pl, pm, pn, cmp, thunk);
	}
	swapfunc_s((char *)a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swapfunc_s(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swapfunc_s(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc_s(pb, pc, es);
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap((char *)a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (::ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort_s(a, d1 / es, es, cmp, thunk);
		}
		if (d2 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	} else {
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es) {
			local_qsort_s(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

} /* namespace */

export namespace pbsd::lib_libc_stdlib::b0235 {

errno_t
qsort_s(void *a, rsize_t n, rsize_t es, cmp_s_t *cmp, void *thunk)
{
	if (n > RSIZE_MAX) {
		__throw_constraint_handler_s("qsort_s : n > RSIZE_MAX", EINVAL);
		return (EINVAL);
	} else if (es > RSIZE_MAX) {
		__throw_constraint_handler_s("qsort_s : es > RSIZE_MAX",
		    EINVAL);
		return (EINVAL);
	} else if (n != 0) {
		if (a == NULL) {
			__throw_constraint_handler_s("qsort_s : a == NULL",
			    EINVAL);
			return (EINVAL);
		} else if (cmp == NULL) {
			__throw_constraint_handler_s("qsort_s : cmp == NULL",
			    EINVAL);
			return (EINVAL);
		} else if (es <= 0) {
			__throw_constraint_handler_s("qsort_s : es <= 0",
			    EINVAL);
			return (EINVAL);
		}
	}

	local_qsort_s(a, n, es, cmp, thunk);
	return (0);
}

} /* namespace */
