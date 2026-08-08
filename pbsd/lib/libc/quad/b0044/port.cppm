/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
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
 * Port of:
 *	lib/libc/quad/muldi3.c
 *	lib/libc/quad/qdivrem.c
 */

module;

#include <climits>
#include <cstdint>

export module pbsd.lib.libc.quad.b0044;

export namespace pbsd::lib_libc_quad::b0044 {

typedef long long quad_t;
typedef unsigned long long u_quad_t;

} // namespace pbsd::lib_libc_quad::b0044

namespace pbsd::lib_libc_quad::b0044 {

typedef std::int32_t quad_long;
typedef std::uint32_t quad_u_long;
typedef quad_u_long u_long;

union uu {
	quad_t	q;
	quad_t	uq;
	quad_long	sl[2];
	quad_u_long	ul[2];
};

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline constexpr int H = 0;
inline constexpr int L = 1;
#else
inline constexpr int H = 1;
inline constexpr int L = 0;
#endif

inline constexpr int QUAD_BITS = sizeof(quad_t) * CHAR_BIT;
inline constexpr int LONG_BITS = sizeof(quad_long) * CHAR_BIT;
inline constexpr int HALF_BITS = sizeof(quad_long) * CHAR_BIT / 2;

#define	HHALF(x)	((x) >> HALF_BITS)
#define	LHALF(x)	((x) & ((1L << HALF_BITS) - 1))
#define	LHUP(x)		((x) << HALF_BITS)

#ifndef __predict_false
#define	__predict_false(exp)	__builtin_expect((exp) != 0, 0)
#endif

static quad_t __lmulq(u_long, u_long);

} // namespace pbsd::lib_libc_quad::b0044

export namespace pbsd::lib_libc_quad::b0044 {

/*
 * Multiply two quads.
 *
 * Our algorithm is based on the following.  Split incoming quad values
 * u and v (where u,v >= 0) into
 *
 *	u = 2^n u1  *  u0	(n = number of bits in `u_long', usu. 32)
 *
 * and
 *
 *	v = 2^n v1  *  v0
 *
 * Then
 *
 *	uv = 2^2n u1 v1  +  2^n u1 v0  +  2^n v1 u0  +  u0 v0
 *	   = 2^2n u1 v1  +     2^n (u1 v0 + v1 u0)   +  u0 v0
 *
 * Now add 2^n u1 v1 to the first term and subtract it from the middle,
 * and add 2^n u0 v0 to the last term and subtract it from the middle.
 * This gives:
 *
 *	uv = (2^2n + 2^n) (u1 v1)  +
 *	         (2^n)    (u1 v0 - u1 v1 + u0 v1 - u0 v0)  +
 *	       (2^n + 1)  (u0 v0)
 *
 * Factoring the middle a bit gives us:
 *
 *	uv = (2^2n + 2^n) (u1 v1)  +			[u1v1 = high]
 *		 (2^n)    (u1 - u0) (v0 - v1)  +	[(u1-u0)... = mid]
 *	       (2^n + 1)  (u0 v0)			[u0v0 = low]
 *
 * The terms (u1 v1), (u1 - u0) (v0 - v1), and (u0 v0) can all be done
 * in just half the precision of the original.  (Note that either or both
 * of (u1 - u0) or (v0 - v1) may be negative.)
 *
 * This algorithm is from Knuth vol. 2 (2nd ed), section 4.3.3, p. 278.
 *
 * Since C does not give us a `long * long = quad' operator, we split
 * our input quads into two longs, then split the two longs into two
 * shorts.  We can then calculate `short * short = long' in native
 * arithmetic.
 *
 * Our product should, strictly speaking, be a `long quad', with 128
 * bits, but we are going to discard the upper 64.  In other words,
 * we are not interested in uv, but rather in (uv mod 2^2n).  This
 * makes some of the terms above vanish, and we get:
 *
 *	(2^n)(high) + (2^n)(mid) + (2^n + 1)(low)
 *
 * or
 *
 *	(2^n)(high + mid + low) + low
 *
 * Furthermore, `high' and `mid' can be computed mod 2^n, as any factor
 * of 2^n in either one will also vanish.  Only `low' need be computed
 * mod 2^2n, and only because of the final term above.
 */
quad_t
__muldi3(quad_t a, quad_t b)
{
	union uu u, v, low, prod;
	u_long high, mid, udiff, vdiff;
	int negall, negmid;
#define	u1	u.ul[H]
#define	u0	u.ul[L]
#define	v1	v.ul[H]
#define	v0	v.ul[L]

	/*
	 * Get u and v such that u, v >= 0.  When this is finished,
	 * u1, u0, v1, and v0 will be directly accessible through the
	 * longword fields.
	 */
	if (a >= 0)
		u.q = a, negall = 0;
	else
		u.q = -a, negall = 1;
	if (b >= 0)
		v.q = b;
	else
		v.q = -b, negall ^= 1;

	if (u1 == 0 && v1 == 0) {
		/*
		 * An (I hope) important optimization occurs when u1 and v1
		 * are both 0.  This should be common since most numbers
		 * are small.  Here the product is just u0*v0.
		 */
		prod.q = __lmulq(u0, v0);
	} else {
		/*
		 * Compute the three intermediate products, remembering
		 * whether the middle term is negative.  We can discard
		 * any upper bits in high and mid, so we can use native
		 * u_long * u_long => u_long arithmetic.
		 */
		low.q = __lmulq(u0, v0);

		if (u1 >= u0)
			negmid = 0, udiff = u1 - u0;
		else
			negmid = 1, udiff = u0 - u1;
		if (v0 >= v1)
			vdiff = v0 - v1;
		else
			vdiff = v1 - v0, negmid ^= 1;
		mid = udiff * vdiff;

		high = u1 * v1;

		/*
		 * Assemble the final product.
		 */
		prod.ul[H] = high + (negmid ? -mid : mid) + low.ul[L] +
		    low.ul[H];
		prod.ul[L] = low.ul[L];
	}
	return (negall ? -prod.q : prod.q);
#undef u1
#undef u0
#undef v1
#undef v0
}

#define	B	(1L << HALF_BITS)	/* digit base */

/* Combine two `digits' to make a single two-digit number. */
#define	COMBINE(a, b) (((u_long)(a) << HALF_BITS) | (b))

/* select a type for digits in base B: use unsigned short if they fit */
#if ULONG_MAX == 0xffffffff && USHRT_MAX >= 0xffff
typedef unsigned short digit;
#else
typedef u_long digit;
#endif

/*
 * Shift p[0]..p[len] left `sh' bits, ignoring any bits that
 * `fall out' the left (there never will be any such anyway).
 * We may assume len >= 0.  NOTE THAT THIS WRITES len+1 DIGITS.
 */
static void
shl(digit *p, int len, int sh)
{
	int i;

	for (i = 0; i < len; i++)
		p[i] = LHALF(p[i] << sh) | (p[i + 1] >> (HALF_BITS - sh));
	p[i] = LHALF(p[i] << sh);
}

/*
 * __qdivrem(u, v, rem) returns u/v and, optionally, sets *rem to u%v.
 *
 * We do this in base 2-sup-HALF_BITS, so that all intermediate products
 * fit within u_long.  As a consequence, the maximum length dividend and
 * divisor are 4 `digits' in this base (they are shorter if they have
 * leading zeros).
 */
u_quad_t
__qdivrem(u_quad_t uq, u_quad_t vq, u_quad_t *arq)
{
	union uu tmp;
	digit *u, *v, *q;
	digit v1, v2;
	u_long qhat, rhat, t;
	int m, n, d, j, i;
	digit uspace[5], vspace[5], qspace[5];

	/*
	 * Take care of special cases: divide by zero, and u < v.
	 */
	if (__predict_false(vq == 0)) {
		/* divide by zero. */
		static volatile const unsigned int zero = 0;

		tmp.ul[H] = tmp.ul[L] = 1 / zero;
		if (arq)
			*arq = uq;
		return (tmp.q);
	}
	if (uq < vq) {
		if (arq)
			*arq = uq;
		return (0);
	}
	u = &uspace[0];
	v = &vspace[0];
	q = &qspace[0];

	/*
	 * Break dividend and divisor into digits in base B, then
	 * count leading zeros to determine m and n.  When done, we
	 * will have:
	 *	u = (u[1]u[2]...u[m+n]) sub B
	 *	v = (v[1]v[2]...v[n]) sub B
	 *	v[1] != 0
	 *	1 < n <= 4 (if n = 1, we use a different division algorithm)
	 *	m >= 0 (otherwise u < v, which we already checked)
	 *	m + n = 4
	 * and thus
	 *	m = 4 - n <= 2
	 */
	tmp.uq = uq;
	u[0] = 0;
	u[1] = HHALF(tmp.ul[H]);
	u[2] = LHALF(tmp.ul[H]);
	u[3] = HHALF(tmp.ul[L]);
	u[4] = LHALF(tmp.ul[L]);
	tmp.uq = vq;
	v[1] = HHALF(tmp.ul[H]);
	v[2] = LHALF(tmp.ul[H]);
	v[3] = HHALF(tmp.ul[L]);
	v[4] = LHALF(tmp.ul[L]);
	for (n = 4; v[1] == 0; v++) {
		if (--n == 1) {
			u_long rbj;	/* r*B+u[j] (not root boy jim) */
			digit q1, q2, q3, q4;

			/*
			 * Change of plan, per exercise 16.
			 *	r = 0;
			 *	for j = 1..4:
			 *		q[j] = floor((r*B + u[j]) / v),
			 *		r = (r*B + u[j]) % v;
			 * We unroll this completely here.
			 */
			t = v[2];	/* nonzero, by definition */
			q1 = u[1] / t;
			rbj = COMBINE(u[1] % t, u[2]);
			q2 = rbj / t;
			rbj = COMBINE(rbj % t, u[3]);
			q3 = rbj / t;
			rbj = COMBINE(rbj % t, u[4]);
			q4 = rbj / t;
			if (arq)
				*arq = rbj % t;
			tmp.ul[H] = COMBINE(q1, q2);
			tmp.ul[L] = COMBINE(q3, q4);
			return (tmp.q);
		}
	}

	/*
	 * By adjusting q once we determine m, we can guarantee that
	 * there is a complete four-digit quotient at &qspace[1] when
	 * we finally stop.
	 */
	for (m = 4 - n; u[1] == 0; u++)
		m--;
	for (i = 4 - m; --i >= 0;)
		q[i] = 0;
	q += 4 - m;

	/*
	 * Here we run Program D, translated from MIX to C and acquiring
	 * a few minor changes.
	 *
	 * D1: choose multiplier 1 << d to ensure v[1] >= B/2.
	 */
	d = 0;
	for (t = v[1]; t < B / 2; t <<= 1)
		d++;
	if (d > 0) {
		shl(&u[0], m + n, d);		/* u <<= d */
		shl(&v[1], n - 1, d);		/* v <<= d */
	}
	/*
	 * D2: j = 0.
	 */
	j = 0;
	v1 = v[1];	/* for D3 -- note that v[1..n] are constant */
	v2 = v[2];	/* for D3 */
	do {
		digit uj0, uj1, uj2;

		/*
		 * D3: Calculate qhat (\^q, in TeX notation).
		 * Let qhat = min((u[j]*B + u[j+1])/v[1], B-1), and
		 * let rhat = (u[j]*B + u[j+1]) mod v[1].
		 * While rhat < B and v[2]*qhat > rhat*B+u[j+2],
		 * decrement qhat and increase rhat correspondingly.
		 * Note that if rhat >= B, v[2]*qhat < rhat*B.
		 */
		uj0 = u[j + 0];	/* for D3 only -- note that u[j+...] change */
		uj1 = u[j + 1];	/* for D3 only */
		uj2 = u[j + 2];	/* for D3 only */
		if (uj0 == v1) {
			qhat = B;
			rhat = uj1;
			goto qhat_too_big;
		} else {
			u_long n = COMBINE(uj0, uj1);
			qhat = n / v1;
			rhat = n % v1;
		}
		while (v2 * qhat > COMBINE(rhat, uj2)) {
	qhat_too_big:
			qhat--;
			if ((rhat += v1) >= B)
				break;
		}
		/*
		 * D4: Multiply and subtract.
		 * The variable `t' holds any borrows across the loop.
		 * We split this up so that we do not require v[0] = 0,
		 * and to eliminate a final special case.
		 */
		for (t = 0, i = n; i > 0; i--) {
			t = u[i + j] - v[i] * qhat - t;
			u[i + j] = LHALF(t);
			t = (B - HHALF(t)) & (B - 1);
		}
		t = u[j] - t;
		u[j] = LHALF(t);
		/*
		 * D5: test remainder.
		 * There is a borrow if and only if HHALF(t) is nonzero;
		 * in that (rare) case, qhat was too large (by exactly 1).
		 * Fix it by adding v[1..n] to u[j..j+n].
		 */
		if (HHALF(t)) {
			qhat--;
			for (t = 0, i = n; i > 0; i--) { /* D6: add back. */
				t += u[i + j] + v[i];
				u[i + j] = LHALF(t);
				t = HHALF(t);
			}
			u[j] = LHALF(u[j] + t);
		}
		q[j] = qhat;
	} while (++j <= m);		/* D7: loop on j. */

	/*
	 * If caller wants the remainder, we have to calculate it as
	 * u[m..m+n] >> d (this is at most n digits and thus fits in
	 * u[m+1..m+n], but we may need more source digits).
	 */
	if (arq) {
		if (d) {
			for (i = m + n; i > m; --i)
				u[i] = (u[i] >> d) |
				    LHALF(u[i - 1] << (HALF_BITS - d));
			u[i] = 0;
		}
		tmp.ul[H] = COMBINE(uspace[1], uspace[2]);
		tmp.ul[L] = COMBINE(uspace[3], uspace[4]);
		*arq = tmp.q;
	}

	tmp.ul[H] = COMBINE(qspace[1], qspace[2]);
	tmp.ul[L] = COMBINE(qspace[3], qspace[4]);
	return (tmp.q);
}

} // namespace pbsd::lib_libc_quad::b0044

namespace pbsd::lib_libc_quad::b0044 {

/*
 * Multiply two 2N-bit longs to produce a 4N-bit quad, where N is half
 * the number of bits in a long (whatever that is---the code below
 * does not care as long as quad.h does its part of the bargain---but
 * typically N==16).
 *
 * We use the same algorithm from Knuth, but this time the modulo refinement
 * does not apply.  On the other hand, since N is half the size of a long,
 * we can get away with native multiplication---none of our input terms
 * exceeds (ULONG_MAX >> 1).
 *
 * Note that, for u_long l, the quad-precision result
 *
 *	l << N
 *
 * splits into high and low longs as HHALF(l) and LHUP(l) respectively.
 */
static quad_t
__lmulq(u_long u, u_long v)
{
	u_long u1, u0, v1, v0, udiff, vdiff, high, mid, low;
	u_long prodh, prodl, was;
	union uu prod;
	int neg;

	u1 = HHALF(u);
	u0 = LHALF(u);
	v1 = HHALF(v);
	v0 = LHALF(v);

	low = u0 * v0;

	/* This is the same small-number optimization as before. */
	if (u1 == 0 && v1 == 0)
		return (low);

	if (u1 >= u0)
		udiff = u1 - u0, neg = 0;
	else
		udiff = u0 - u1, neg = 1;
	if (v0 >= v1)
		vdiff = v0 - v1;
	else
		vdiff = v1 - v0, neg ^= 1;
	mid = udiff * vdiff;

	high = u1 * v1;

	/* prod = (high << 2N) + (high << N); */
	prodh = high + HHALF(high);
	prodl = LHUP(high);

	/* if (neg) prod -= mid << N; else prod += mid << N; */
	if (neg) {
		was = prodl;
		prodl -= LHUP(mid);
		prodh -= HHALF(mid) + (prodl > was);
	} else {
		was = prodl;
		prodl += LHUP(mid);
		prodh += HHALF(mid) + (prodl < was);
	}

	/* prod += low << N */
	was = prodl;
	prodl += LHUP(low);
	prodh += HHALF(low) + (prodl < was);
	/* ... + low; */
	if ((prodl += low) < low)
		prodh++;

	/* return 4N-bit product */
	prod.ul[H] = prodh;
	prod.ul[L] = prodl;
	return (prod.q);
}

} // namespace pbsd::lib_libc_quad::b0044
