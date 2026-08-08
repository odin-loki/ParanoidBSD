/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Conrad Meyer <cem@FreeBSD.org>
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
 * PBSD batch b0146s1 differential harness: the C++23 port in
 * pbsd.sys.kern.b0146s1 against the ref_ oracle built from the original C.
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.sys.kern.b0146s1;

namespace P = pbsd::sys_kern::b0146s1;

extern "C" {

struct thread {
	long	td_retval[2];
};

struct getrandom_args {
	void		*buf;
	std::size_t	 buflen;
	unsigned int	 flags;
};

struct rr_observation {
	int		 calls;
	int		 nonblock;
	const void	*iov_base;
	std::size_t	 iov_len;
	int		 iovcnt;
	long long	 offset;
	long long	 resid_in;
	long long	 resid_out;
	int		 segflg;
	int		 rw;
	const void	*td;
};

extern int rr_error;
extern std::size_t rr_consume;
extern int rr_block;
extern struct rr_observation rr_obs;

int ref_kern_getrandom(struct thread *td, void *user_buf, std::size_t buflen,
    unsigned int flags);
int ref_sys_getrandom(struct thread *td, struct getrandom_args *uap);

} /* extern "C" */

/*
 * Buffers are 256 bytes; the stand-in read_random_uio() never writes more than
 * MAX_CONSUME bytes starting at most MAX_OFF bytes in, so everything from
 * MAX_OFF + MAX_CONSUME upwards must still hold the guard byte.  The whole
 * buffer is compared regardless.
 */
static constexpr std::size_t BUFSZ = 256;
static constexpr std::size_t MAX_OFF = 8;
static constexpr std::size_t MAX_CONSUME = 64;
static constexpr unsigned char GUARD = 0x7f;

static constexpr long SENT0 = 0x5a5a5a5a5a5a5a5aL;
static constexpr long SENT1 = 0x3c3c3c3c3c3c3c3cL;

enum entry { ENT_KERN, ENT_SYS, ENT_NUM };

static const char *const entry_name[ENT_NUM] = {
	"kern_getrandom",
	"sys_getrandom",
};

struct testcase {
	std::size_t	buflen;
	unsigned int	flags;
	std::size_t	bufoff;
	int		err;
	std::size_t	consume;
	int		block;
	bool		nullbuf;
};

static long ncases[ENT_NUM];
static long nfail[ENT_NUM];
static int nreported;

static int
td_kind(const void *p, const void *expect)
{
	if (p == nullptr)
		return (0);
	if (p == expect)
		return (1);
	return (2);
}

static long
buf_off(const void *p, const unsigned char *base)
{
	if (p == nullptr)
		return (-1);
	return (long)((const unsigned char *)p - base);
}

static void
report(entry e, const testcase &c, const char *what, long long a, long long b)
{
	nfail[e]++;
	if (nreported >= 20)
		return;
	nreported++;
	std::printf("FAIL %-14s %-12s ref=%lld port=%lld  "
	    "buflen=%zu flags=0x%08x off=%zu err=%d consume=%zu block=%d "
	    "nullbuf=%d\n",
	    entry_name[e], what, a, b, c.buflen, c.flags, c.bufoff, c.err,
	    c.consume, c.block, (int)c.nullbuf);
}

static void
run(entry e, const testcase &c)
{
	static unsigned char bufa[BUFSZ];
	static unsigned char bufb[BUFSZ];
	struct thread tda;
	P::thread tdb;
	struct rr_observation oa, ob;
	void *pa, *pb;
	int ra, rb;

	ncases[e]++;

	std::memset(bufa, GUARD, BUFSZ);
	std::memset(bufb, GUARD, BUFSZ);

	tda.td_retval[0] = SENT0;
	tda.td_retval[1] = SENT1;
	tdb.td_retval[0] = SENT0;
	tdb.td_retval[1] = SENT1;

	pa = c.nullbuf ? nullptr : (void *)(bufa + c.bufoff);
	pb = c.nullbuf ? nullptr : (void *)(bufb + c.bufoff);

	rr_error = c.err;
	rr_consume = c.consume;
	rr_block = c.block;

	std::memset(&rr_obs, 0, sizeof(rr_obs));
	if (e == ENT_KERN) {
		ra = ref_kern_getrandom(&tda, pa, c.buflen, c.flags);
	} else {
		struct getrandom_args ua;

		ua.buf = pa;
		ua.buflen = c.buflen;
		ua.flags = c.flags;
		ra = ref_sys_getrandom(&tda, &ua);
	}
	oa = rr_obs;

	std::memset(&rr_obs, 0, sizeof(rr_obs));
	if (e == ENT_KERN) {
		rb = P::kern_getrandom(&tdb, pb, c.buflen, c.flags);
	} else {
		P::getrandom_args ub;

		ub.buf = pb;
		ub.buflen = c.buflen;
		ub.flags = c.flags;
		rb = P::sys_getrandom(&tdb, &ub);
	}
	ob = rr_obs;

	if (ra != rb) {
		report(e, c, "retcode", ra, rb);
		return;
	}
	if (tda.td_retval[0] != tdb.td_retval[0]) {
		report(e, c, "td_retval0", tda.td_retval[0], tdb.td_retval[0]);
		return;
	}
	if (tda.td_retval[1] != tdb.td_retval[1]) {
		report(e, c, "td_retval1", tda.td_retval[1], tdb.td_retval[1]);
		return;
	}
	if (std::memcmp(bufa, bufb, BUFSZ) != 0) {
		std::size_t i;

		for (i = 0; i < BUFSZ; i++) {
			if (bufa[i] != bufb[i])
				break;
		}
		report(e, c, "buffer", (long long)((i << 16) | bufa[i]),
		    (long long)((i << 16) | bufb[i]));
		return;
	}
	if (oa.calls != ob.calls) {
		report(e, c, "rr:calls", oa.calls, ob.calls);
		return;
	}
	if (oa.calls == 0)
		return;
	if (oa.nonblock != ob.nonblock) {
		report(e, c, "rr:nonblock", oa.nonblock, ob.nonblock);
		return;
	}
	if (buf_off(oa.iov_base, bufa) != buf_off(ob.iov_base, bufb)) {
		report(e, c, "rr:iov_base", buf_off(oa.iov_base, bufa),
		    buf_off(ob.iov_base, bufb));
		return;
	}
	if (oa.iov_len != ob.iov_len) {
		report(e, c, "rr:iov_len", (long long)oa.iov_len,
		    (long long)ob.iov_len);
		return;
	}
	if (oa.iovcnt != ob.iovcnt) {
		report(e, c, "rr:iovcnt", oa.iovcnt, ob.iovcnt);
		return;
	}
	if (oa.offset != ob.offset) {
		report(e, c, "rr:offset", oa.offset, ob.offset);
		return;
	}
	if (oa.resid_in != ob.resid_in) {
		report(e, c, "rr:resid_in", oa.resid_in, ob.resid_in);
		return;
	}
	if (oa.resid_out != ob.resid_out) {
		report(e, c, "rr:resid_out", oa.resid_out, ob.resid_out);
		return;
	}
	if (oa.segflg != ob.segflg) {
		report(e, c, "rr:segflg", oa.segflg, ob.segflg);
		return;
	}
	if (oa.rw != ob.rw) {
		report(e, c, "rr:rw", oa.rw, ob.rw);
		return;
	}
	if (td_kind(oa.td, &tda) != td_kind(ob.td, &tdb)) {
		report(e, c, "rr:uio_td", td_kind(oa.td, &tda),
		    td_kind(ob.td, &tdb));
		return;
	}
}

static void
run_both(const testcase &c)
{
	run(ENT_KERN, c);
	run(ENT_SYS, c);
}

/*
 * Hand-written cases: a full cross product over the interesting values of
 * every input.  The buflen list straddles 0, the 64-byte write window and
 * IOSIZE_MAX from both sides; the flags list covers each individual valid bit,
 * every valid combination, and invalid bits both adjacent to and far above the
 * valid mask.
 */
static void
edge_cases(void)
{
	static const std::size_t buflens[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 65,
		127, 128, 129, 255, 256, 257, 1000, 65535, 65536,
		(std::size_t)INT_MAX - 1,
		(std::size_t)INT_MAX,
		(std::size_t)INT_MAX + 1,
		(std::size_t)INT_MAX + 2,
		(std::size_t)UINT_MAX,
		(std::size_t)1 << 32,
		SIZE_MAX / 2,
		SIZE_MAX - 1,
		SIZE_MAX,
	};
	static const unsigned int flagses[] = {
		0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 0xcu, 0xfu, 0x10u,
		0x80u, 0x100u, 0x8000u, 0x40000000u, 0x80000000u, 0xfffffff8u,
		0xffffffffu,
	};
	static const int errs[] = { 0, EAGAIN, EINVAL, 1, 42 };
	static const std::size_t consumes[] = { 0, 1, 2, 3, 63, 64 };
	static const std::size_t offs[] = { 0, 1, 7 };
	testcase c;

	for (std::size_t bi = 0; bi < sizeof(buflens) / sizeof(buflens[0]); bi++) {
		for (std::size_t fi = 0; fi < sizeof(flagses) / sizeof(flagses[0]); fi++) {
			for (std::size_t ei = 0; ei < sizeof(errs) / sizeof(errs[0]); ei++) {
				for (std::size_t ci = 0; ci < sizeof(consumes) / sizeof(consumes[0]); ci++) {
					for (std::size_t oi = 0; oi < sizeof(offs) / sizeof(offs[0]); oi++) {
						for (int blk = 0; blk < 2; blk++) {
							c.buflen = buflens[bi];
							c.flags = flagses[fi];
							c.bufoff = offs[oi];
							c.err = errs[ei];
							c.consume = consumes[ci];
							c.block = blk;
							c.nullbuf = false;
							run_both(c);
						}
					}
				}
			}
		}
	}

	/* NUL/high-bit payloads are produced by the stand-in itself; here we
	 * additionally exercise a NULL user buffer, which the syscall accepts
	 * for buflen == 0 and passes straight through otherwise. */
	for (std::size_t bi = 0; bi < sizeof(buflens) / sizeof(buflens[0]); bi++) {
		for (std::size_t fi = 0; fi < sizeof(flagses) / sizeof(flagses[0]); fi++) {
			c.buflen = buflens[bi];
			c.flags = flagses[fi];
			c.bufoff = 0;
			c.err = (bi & 1) ? 0 : EAGAIN;
			c.consume = (bi % 3) * 17;
			c.block = (int)(fi & 1);
			c.nullbuf = true;
			run_both(c);
		}
	}
}

static std::uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static std::size_t
pick_buflen(std::uint64_t r)
{
	std::uint64_t v = r >> 8;

	switch (r % 20) {
	case 0:
		return (0);
	case 1:
		return (1);
	case 2:
		return (2);
	case 3:
		return (3);
	case 4:
		return ((std::size_t)(v % 65));
	case 5:
		return ((std::size_t)(63 + v % 5));
	case 6:
		return ((std::size_t)(127 + v % 5));
	case 7:
		return ((std::size_t)(v % 300));
	case 8:
		return ((std::size_t)(v % 100000));
	case 9:
		return ((std::size_t)INT_MAX - (std::size_t)(v % 4));
	case 10:
		return ((std::size_t)INT_MAX);
	case 11:
		return ((std::size_t)INT_MAX + 1 + (std::size_t)(v % 4));
	case 12:
		return ((std::size_t)1 << (31 + v % 33));
	case 13:
		return (SIZE_MAX - (std::size_t)(v % 4));
	case 14:
		return (SIZE_MAX / 2);
	case 15:
		return ((std::size_t)UINT_MAX + (std::size_t)(v % 3));
	case 16:
		return ((std::size_t)v);
	case 17:
		return ((std::size_t)(v & 0xffff));
	case 18:
		return ((std::size_t)INT_MAX / 2 + (std::size_t)(v % 3));
	default:
		return ((std::size_t)(v % 9));
	}
}

static unsigned int
pick_flags(std::uint64_t r)
{
	unsigned int f = (unsigned int)(r & 7);

	switch ((r >> 3) % 8) {
	case 0:
	case 1:
	case 2:
	case 3:
		return (f);
	case 4:
		return (f | 8u);
	case 5:
		return (f | (1u << (unsigned int)((r >> 8) % 32)));
	case 6:
		return ((unsigned int)(r >> 16));
	default:
		return (f | 0x80000000u);
	}
}

static void
random_sweep(void)
{
	static const int errs[] = { 0, 0, 0, EAGAIN, EINVAL, 1, 4, 5 };
	const long iters = 200000;
	testcase c;
	long i;

	for (i = 0; i < iters; i++) {
		c.buflen = pick_buflen(rnd());
		c.flags = pick_flags(rnd());
		c.bufoff = (std::size_t)(rnd() % (MAX_OFF + 1));
		c.err = errs[rnd() % 8];
		c.consume = (std::size_t)(rnd() % (MAX_CONSUME + 1));
		c.block = (int)(rnd() & 1);
		c.nullbuf = (rnd() % 64) == 0;
		run_both(c);
	}
}

int
main(void)
{
	long total_cases = 0, total_fail = 0;
	int i;

	edge_cases();
	random_sweep();

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	for (i = 0; i < ENT_NUM; i++) {
		std::printf("%-20s %12ld %12ld\n", entry_name[i], ncases[i],
		    nfail[i]);
		total_cases += ncases[i];
		total_fail += nfail[i];
	}
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	std::printf("%-20s %12ld %12ld\n", "TOTAL", total_cases, total_fail);
	std::printf("\n%s\n", total_fail == 0 ? "PASS" : "FAIL");
	return (total_fail == 0 ? 0 : 1);
}
