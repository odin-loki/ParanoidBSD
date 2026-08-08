/*
 * Batch b0235 oracle: original HardenedBSD C sources concatenated.
 * Functions renamed with ref_ prefix; bodies otherwise unmodified.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef __predict_false
#define __predict_false(exp) __builtin_expect((exp), 0)
#endif
#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX >> 1)
#endif
typedef size_t rsize_t;
typedef int errno_t;

struct qsort_block_desc {
	void *isa;
	int flags;
	int reserved;
	void (*invoke)(void *, ...);
};
#define DECLARE_BLOCK(retTy, name, argTys, ...)\
	struct {\
		void *isa;\
		int flags;\
		int reserved;\
		retTy (*invoke)(void *, ...);\
	} *name
#define GET_BLOCK_FUNCTION(x) \
	(((struct qsort_block_desc *)(void *)(x))->invoke)

static void
__throw_constraint_handler_s(const char *msg, errno_t error)
{
	(void)msg;
	(void)error;
}


/*
 * Copyright (c) 2025 Robert Clausecker <fuz@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */


size_t
ref_memalignment(const void *p)
{
	uintptr_t align;

	if (p == NULL)
		return (0);

	align = (uintptr_t)p;
	align &= -align;

#if UINTPTR_MAX > SIZE_MAX
	/* if alignment overflows size_t, return maximum possible */
	if (align > SIZE_MAX)
		align = SIZE_MAX - SIZE_MAX/2;
#endif

	return (align);
}

/* qsort_r.c */
/*
 * This file is in the public domain.  Originally written by Garrett
 * A. Wollman.
 */
#define I_AM_QSORT_R
#define med3 med3_r
#define swapfunc swapfunc_r
#define cmp_t cmp_t_r
#define local_qsort local_qsort_r
#define CMP_r(t, x, y) (cmp((x), (y), (t)))
#define CMP CMP_r
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


#if defined(I_AM_QSORT_R)
typedef int		 cmp_t(const void *, const void *, void *);
#elif defined(I_AM_QSORT_R_COMPAT)
typedef int		 cmp_t(void *, const void *, const void *);
#elif defined(I_AM_QSORT_S)
typedef int		 cmp_t(const void *, const void *, void *);
#else
typedef int		 cmp_t(const void *, const void *);
#endif
static inline char	*med3(char *, char *, char *, cmp_t *, void *);

#define	MIN(a, b)	((a) < (b) ? a : b)

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

static inline void
swapfunc(char *a, char *b, size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc(a, b, n)


static inline char *
med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk
#if !defined(I_AM_QSORT_R) && !defined(I_AM_QSORT_R_COMPAT) && !defined(I_AM_QSORT_S)
__unused
#endif
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
local_qsort(void *a, size_t n, size_t es, cmp_t *cmp, void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d1, d2;
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
				swapfunc(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			size_t d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swapfunc(a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swapfunc(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swapfunc(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc(pb, pc, es);
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort(a, d1 / es, es, cmp, thunk);
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
			local_qsort(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

#if defined(I_AM_QSORT_R)
void
ref_qsort_r(void *a, size_t n, size_t es, cmp_t_r *cmp, void *thunk)
{
	local_qsort_r(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_R_COMPAT)
void
__qsort_r_compat(void *a, size_t n, size_t es, void *thunk, cmp_t *cmp)
{
	local_qsort_r_compat(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_S)
errno_t
qsort_s(void *a, rsize_t n, rsize_t es, cmp_t *cmp, void *thunk)
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
#else
void
qsort(void *a, size_t n, size_t es, cmp_t *cmp)
{
	local_qsort(a, n, es, cmp, NULL);
}
#endif
#undef med3
#undef swapfunc
#undef cmp_t
#undef local_qsort
#undef CMP
#undef CMP_r
#undef I_AM_QSORT_R

/* qsort_r_compat.c (qsort.c portion) */
#define I_AM_QSORT_R_COMPAT
#define med3 med3_compat
#define swapfunc swapfunc_compat
#define cmp_t cmp_t_compat
#define local_qsort local_qsort_r_compat
#define CMP_compat(t, x, y) (cmp((t), (x), (y)))
#define CMP CMP_compat
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


#if defined(I_AM_QSORT_R)
typedef int		 cmp_t(const void *, const void *, void *);
#elif defined(I_AM_QSORT_R_COMPAT)
typedef int		 cmp_t(void *, const void *, const void *);
#elif defined(I_AM_QSORT_S)
typedef int		 cmp_t(const void *, const void *, void *);
#else
typedef int		 cmp_t(const void *, const void *);
#endif
static inline char	*med3(char *, char *, char *, cmp_t *, void *);

#define	MIN(a, b)	((a) < (b) ? a : b)

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

static inline void
swapfunc(char *a, char *b, size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc(a, b, n)


static inline char *
med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk
#if !defined(I_AM_QSORT_R) && !defined(I_AM_QSORT_R_COMPAT) && !defined(I_AM_QSORT_S)
__unused
#endif
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
local_qsort(void *a, size_t n, size_t es, cmp_t *cmp, void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d1, d2;
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
				swapfunc(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			size_t d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swapfunc(a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swapfunc(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swapfunc(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc(pb, pc, es);
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort(a, d1 / es, es, cmp, thunk);
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
			local_qsort(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

#if defined(I_AM_QSORT_R)
void
(qsort_r)(void *a, size_t n, size_t es, cmp_t *cmp, void *thunk)
{
	local_qsort_r(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_R_COMPAT)
void
ref___qsort_r_compat(void *a, size_t n, size_t es, void *thunk, cmp_t_compat *cmp)
{
	local_qsort_r_compat(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_S)
errno_t
qsort_s(void *a, rsize_t n, rsize_t es, cmp_t *cmp, void *thunk)
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
#else
void
qsort(void *a, size_t n, size_t es, cmp_t *cmp)
{
	local_qsort(a, n, es, cmp, NULL);
}
#endif
#undef med3
#undef swapfunc
#undef cmp_t
#undef local_qsort
#undef CMP
#undef CMP_compat
#undef I_AM_QSORT_R_COMPAT
/*
 * This file is in the public domain.  Originally written by Garrett
 * A. Wollman.
 */
/*
 * This file is in the public domain.  Originally written by Garrett
 * A. Wollman.
 */

typedef DECLARE_BLOCK(int, qsort_block, const void *, const void *);

void
ref_qsort_b(void *base, size_t nel, size_t width, qsort_block compar)
{
	ref___qsort_r_compat(base, nel, width, compar,
		(int (*)(void *, const void *, const void *))
		GET_BLOCK_FUNCTION(compar));
}


/* qsort_s.c */
/*
 * This file is in the public domain.  Originally written by Garrett
 * A. Wollman.
 */
#define I_AM_QSORT_S
#define med3 med3_s
#define swapfunc swapfunc_s
#define cmp_t cmp_t_s
#define local_qsort local_qsort_s
#define CMP_s(t, x, y) (cmp((x), (y), (t)))
#define CMP CMP_s
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


#if defined(I_AM_QSORT_R)
typedef int		 cmp_t(const void *, const void *, void *);
#elif defined(I_AM_QSORT_R_COMPAT)
typedef int		 cmp_t(void *, const void *, const void *);
#elif defined(I_AM_QSORT_S)
typedef int		 cmp_t(const void *, const void *, void *);
#else
typedef int		 cmp_t(const void *, const void *);
#endif
static inline char	*med3(char *, char *, char *, cmp_t *, void *);

#define	MIN(a, b)	((a) < (b) ? a : b)

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

static inline void
swapfunc(char *a, char *b, size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#define	vecswap(a, b, n)				\
	if ((n) > 0) swapfunc(a, b, n)


static inline char *
med3(char *a, char *b, char *c, cmp_t *cmp, void *thunk
#if !defined(I_AM_QSORT_R) && !defined(I_AM_QSORT_R_COMPAT) && !defined(I_AM_QSORT_S)
__unused
#endif
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
local_qsort(void *a, size_t n, size_t es, cmp_t *cmp, void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d1, d2;
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
				swapfunc(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			size_t d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swapfunc(a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swapfunc(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swapfunc(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc(pb, pc, es);
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort(a, d1 / es, es, cmp, thunk);
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
			local_qsort(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

#if defined(I_AM_QSORT_R)
void
(qsort_r)(void *a, size_t n, size_t es, cmp_t *cmp, void *thunk)
{
	local_qsort_r(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_R_COMPAT)
void
__qsort_r_compat(void *a, size_t n, size_t es, void *thunk, cmp_t *cmp)
{
	local_qsort_r_compat(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_S)
errno_t
ref_qsort_s(void *a, rsize_t n, rsize_t es, cmp_t_s *cmp, void *thunk)
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
#else
void
qsort(void *a, size_t n, size_t es, cmp_t *cmp)
{
	local_qsort(a, n, es, cmp, NULL);
}
#endif
#undef med3
#undef swapfunc
#undef cmp_t
#undef local_qsort
#undef CMP
#undef CMP_s
#undef I_AM_QSORT_S
