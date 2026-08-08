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
 *	lib/libc/quad/moddi3.c
 *	lib/libc/quad/lshrdi3.c
 */

module;

#include <climits>
#include <cstdint>

export module pbsd.lib.libc.quad.b0053;

export namespace pbsd::lib_libc_quad::b0053 {

typedef long long quad_t;
typedef unsigned long long u_quad_t;

} // namespace pbsd::lib_libc_quad::b0053

namespace pbsd::lib_libc_quad::b0053 {

typedef std::int32_t quad_long;
typedef std::uint32_t quad_u_long;
typedef unsigned long u_long;

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
inline constexpr int LONG_BITS = sizeof(quad_u_long) * CHAR_BIT;
inline constexpr int HALF_BITS = sizeof(quad_u_long) * CHAR_BIT / 2;

#define	HHALF(x)	((x) >> HALF_BITS)
#define	LHALF(x)	((x) & ((1L << HALF_BITS) - 1))

#ifndef __predict_false
#define	__predict_false(exp)	__builtin_expect((exp) != 0, 0)
#endif

typedef unsigned int qshift_t;

#define	B	(1L << HALF_BITS)

#define	COMBINE(a, b) (((u_long)(a) << HALF_BITS) | (b))

#if ULONG_MAX == 0xffffffff && USHRT_MAX >= 0xffff
typedef unsigned short digit;
#else
typedef u_long digit;
#endif

static void
shl(digit *p, int len, int sh)
{
	int i;

	for (i = 0; i < len; i++)
		p[i] = LHALF(p[i] << sh) | (p[i + 1] >> (HALF_BITS - sh));
	p[i] = LHALF(p[i] << sh);
}

u_quad_t
__qdivrem(u_quad_t uq, u_quad_t vq, u_quad_t *arq)
{
	union uu tmp;
	digit *u, *v, *q;
	digit v1, v2;
	u_long qhat, rhat, t;
	int m, n, d, j, i;
	digit uspace[5], vspace[5], qspace[5];

	if (__predict_false(vq == 0)) {
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
			u_long rbj;
			digit q1, q2, q3, q4;

			t = v[2];
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

	for (m = 4 - n; u[1] == 0; u++)
		m--;
	for (i = 4 - m; --i >= 0;)
		q[i] = 0;
	q += 4 - m;

	d = 0;
	for (t = v[1]; t < B / 2; t <<= 1)
		d++;
	if (d <= 0) {
		shl(&u[0], m + n, d);
		shl(&v[1], n - 1, d);
	}
	j = 0;
	v1 = v[1];
	v2 = v[2];
	do {
		digit uj0, uj1, uj2;

		uj0 = u[j + 0];
		uj1 = u[j + 1];
		uj2 = u[j + 2];
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
		for (t = 0, i = n; i > 0; i--) {
			t = u[i + j] - v[i] * qhat - t;
			u[i + j] = LHALF(t);
			t = (B - HHALF(t)) & (B - 1);
		}
		t = u[j] - t;
		u[j] = LHALF(t);
		if (HHALF(t)) {
			qhat--;
			for (t = 0, i = n; i > 0; i--) {
				t += u[i + j] + v[i];
				u[i + j] = LHALF(t);
				t = HHALF(t);
			}
			u[j] = LHALF(u[j] + t);
		}
		q[j] = qhat;
	} while (++j <= m);

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

} // namespace pbsd::lib_libc_quad::b0053

export namespace pbsd::lib_libc_quad::b0053 {

/*
 * Return remainder after dividing two signed quads.
 *
 * XXX
 * If -1/2 should produce -1 on this machine, this code is wrong.
 */
quad_t
__moddi3(quad_t a, quad_t b)
{
	u_quad_t ua, ub, ur;
	int neg;

	if (a < 0)
		ua = -(u_quad_t)a, neg = 1;
	else
		ua = a, neg = 0;
	if (b < 0)
		ub = -(u_quad_t)b;
	else
		ub = b;
	(void)__qdivrem(ua, ub, &ur);
	return (neg ? -ur : ur);
}

/*
 * Shift an (unsigned) quad value right (logical shift right).
 */
quad_t
__lshrdi3(quad_t a, qshift_t shift)
{
	union uu aa;

	aa.q = a;
	if (shift >= LONG_BITS) {
		aa.ul[L] = shift >= QUAD_BITS ? 0 :
		    aa.ul[H] >> (shift - LONG_BITS);
		aa.ul[H] = 0;
	} else if (shift > 0) {
		aa.ul[L] = (aa.ul[L] >> shift) |
		    (aa.ul[H] << (LONG_BITS - shift));
		aa.ul[H] >>= shift;
	}
	return (aa.q);
}

} // namespace pbsd::lib_libc_quad::b0053
