// b0154s4 differential harness: C++23 port vs. the untouched C oracle.
//
// Every function of the batch is driven with hand-written edge cases and a
// fixed-seed randomised sweep of at least 200000 iterations.  Buffer-writing
// functions get two independently guarded buffers whose ENTIRE contents
// (including the 16 guard bytes on either side of the nominal window) are
// compared after the call.  Pointers are always compared as offsets from the
// respective buffer base.

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <errno.h>
#include <limits.h>
#include <resolv.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

import pbsd.lib.libc.nameser.b0154s4;

namespace P = pbsd::lib_libc_nameser::b0154s4;

extern "C" {
int	ref_ns_msg_getflag(ns_msg handle, int flag);
int	ref_ns_skiprr(const u_char *ptr, const u_char *eom, ns_sect section,
		      int count);
int	ref_ns_initparse(const u_char *msg, int msglen, ns_msg *handle);
int	ref_ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr);
void	ref_setsection(ns_msg *msg, ns_sect sect);
}

/* ---------------------------------------------------------------- report */

struct Stat {
	const char	*name;
	unsigned long	cases;
	unsigned long	fails;
	unsigned	shown;
};

enum { MAXSTAT = 16 };
static Stat	stats[MAXSTAT];
static int	nstat;

static Stat &
mkstat(const char *name)
{
	Stat &s = stats[nstat++];
	s.name = name;
	s.cases = 0;
	s.fails = 0;
	s.shown = 0;
	return s;
}

static void
note(Stat &s, bool ok, const char *fmt, ...)
{
	s.cases++;
	if (ok)
		return;
	s.fails++;
	if (s.shown < 8) {
		s.shown++;
		va_list ap;
		va_start(ap, fmt);
		fprintf(stderr, "  MISMATCH %s: ", s.name);
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
		va_end(ap);
	}
}

/* ------------------------------------------------------------------ rng */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x9E3779B97F4A7C15ULL;
}

static inline uint64_t
rnd64(void)
{
	uint64_t x = rng_state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x;
}

static inline uint32_t
rnd32(void)
{
	return (uint32_t)(rnd64() >> 16);
}

static inline unsigned
rndn(unsigned n)
{
	return (unsigned)(rnd32() % n);
}

/* -------------------------------------------------------------- buffers */

enum { PRE = 16, WIN = 512, POST = 16, RAW = PRE + WIN + POST };

struct Buf {
	unsigned char raw[RAW];

	void fill(void) { memset(raw, 0x7f, sizeof raw); }
	unsigned char *base(void) { return raw + PRE; }
};

static bool
sameraw(const Buf &a, const Buf &b)
{
	return memcmp(a.raw, b.raw, RAW) == 0;
}

static long
firstdiff(const Buf &a, const Buf &b)
{
	for (long i = 0; i < RAW; i++)
		if (a.raw[i] != b.raw[i])
			return i - PRE;
	return -1;
}

static long
poff(const void *p, const unsigned char *base)
{
	if (p == NULL)
		return -1000001;
	const unsigned char *q = (const unsigned char *)p;
	long d = (long)(q - base);
	if (d < -(1L << 20) || d > (1L << 20))
		return -1000002;
	return d;
}

/* ------------------------------------------------------- ns_msg snapshot */

struct Snap {
	long msg, eom, msg_ptr, sect, rrnum, id, flags;
	long counts[4];
	long sections[4];
};

static Snap
snap(const ns_msg &h, const unsigned char *base)
{
	Snap s;
	s.msg = poff(h._msg, base);
	s.eom = poff(h._eom, base);
	s.msg_ptr = poff(h._msg_ptr, base);
	s.sect = (long)(int)h._sect;
	s.rrnum = (long)h._rrnum;
	s.id = (long)h._id;
	s.flags = (long)h._flags;
	for (int i = 0; i < 4; i++) {
		s.counts[i] = (long)h._counts[i];
		s.sections[i] = poff(h._sections[i], base);
	}
	return s;
}

static bool
snapeq(const Snap &a, const Snap &b)
{
	return memcmp(&a, &b, sizeof a) == 0;
}

static int
snapwhich(const Snap &a, const Snap &b)
{
	const long *x = (const long *)&a, *y = (const long *)&b;
	for (unsigned i = 0; i < sizeof a / sizeof(long); i++)
		if (x[i] != y[i])
			return (int)i;
	return -1;
}

static void
prefill_msg(ns_msg &h, const unsigned char *base)
{
	memset(&h, 0, sizeof h);
	h._msg = base + 5;
	h._eom = base + 6;
	h._id = 0x1234;
	h._flags = 0x5678;
	for (int i = 0; i < ns_s_max; i++) {
		h._counts[i] = (uint16_t)(0x4400 + i);
		h._sections[i] = base + 7 + i;
	}
	h._sect = (ns_sect)3;
	h._rrnum = 12345;
	h._msg_ptr = base + 11;
}

static void
prefill_rr(ns_rr &rr, const unsigned char *base)
{
	memset(&rr, 0x7f, sizeof rr);
	rr.rdata = base + 3;
}

static bool
rreq(const ns_rr &a, const unsigned char *ba, const ns_rr &b,
     const unsigned char *bb)
{
	if (memcmp(a.name, b.name, NS_MAXDNAME) != 0)
		return false;
	if (a.type != b.type || a.rr_class != b.rr_class)
		return false;
	if (a.ttl != b.ttl || a.rdlength != b.rdlength)
		return false;
	return poff(a.rdata, ba) == poff(b.rdata, bb);
}

/* -------------------------------------------------------- ns_parse.c */

static void
t_ns_msg_getflag(void)
{
	Stat &s = mkstat("ns_msg_getflag");
	ns_msg h;

	memset(&h, 0, sizeof h);
	for (unsigned long w = 0; w <= 0xffffu; w++) {
		h._flags = (uint16_t)w;
		for (int f = 0; f < 16; f++) {
			int ra = P::ns_msg_getflag(h, f);
			int rb = ref_ns_msg_getflag(h, f);
			note(s, ra == rb, "flags=%#lx flag=%d: %d/%d", w, f,
			    ra, rb);
		}
	}
}

static void
t_setsection(void)
{
	Stat &s = mkstat("setsection");
	Buf A, B;

	for (unsigned long it = 0; it < 200000; it++) {
		A.fill();
		B.fill();
		ns_msg ha, hb;
		prefill_msg(ha, A.base());
		prefill_msg(hb, B.base());
		for (int i = 0; i < ns_s_max; i++) {
			long off = (long)rndn(64);
			bool nul = rndn(8) == 0;
			ha._sections[i] = nul ? NULL : A.base() + off;
			hb._sections[i] = nul ? NULL : B.base() + off;
		}
		ha._rrnum = hb._rrnum = (int)rnd32() % 1000;
		ns_sect prev = (ns_sect)rndn(5);
		ha._sect = hb._sect = prev;
		long mp = (long)rndn(64);
		ha._msg_ptr = A.base() + mp;
		hb._msg_ptr = B.base() + mp;

		ns_sect sect = (ns_sect)(it % 5);
		if ((it & 1) == 0)
			sect = (ns_sect)rndn(5);

		P::setsection(&ha, sect);
		ref_setsection(&hb, sect);

		Snap sa = snap(ha, A.base()), sb = snap(hb, B.base());
		note(s, snapeq(sa, sb) && sameraw(A, B),
		    "sect=%d prev=%d: field %d differs", (int)sect, (int)prev,
		    snapwhich(sa, sb));
	}
	for (int i = 0; i <= ns_s_max; i++) {
		A.fill();
		B.fill();
		ns_msg ha, hb;
		prefill_msg(ha, A.base());
		prefill_msg(hb, B.base());
		for (int k = 0; k < ns_s_max; k++) {
			ha._sections[k] = A.base() + 3 * k;
			hb._sections[k] = B.base() + 3 * k;
		}
		P::setsection(&ha, (ns_sect)i);
		ref_setsection(&hb, (ns_sect)i);
		Snap sa = snap(ha, A.base()), sb = snap(hb, B.base());
		note(s, snapeq(sa, sb) && sameraw(A, B),
		    "edge sect=%d: field %d differs", i, snapwhich(sa, sb));
	}
}

/* --------------------------------------------------- message generator */

static void
gput16(unsigned char *b, int &o, unsigned v)
{
	b[o++] = (unsigned char)(v >> 8);
	b[o++] = (unsigned char)v;
}

static void
gput32(unsigned char *b, int &o, unsigned long v)
{
	b[o++] = (unsigned char)(v >> 24);
	b[o++] = (unsigned char)(v >> 16);
	b[o++] = (unsigned char)(v >> 8);
	b[o++] = (unsigned char)v;
}

enum { MSGCAP = 400 };

static int
gen_msg(unsigned char *b)
{
	int o = 0;

	if (rndn(6) == 0) {
		int n = (int)rndn(15);
		for (int i = 0; i < n; i++)
			b[o++] = (unsigned char)rndn(256);
		return o;
	}

	gput16(b, o, rnd32() & 0xffffu);
	gput16(b, o, rnd32() & 0xffffu);
	int counts[4];
	for (int i = 0; i < 4; i++)
		counts[i] = (int)rndn(5);
	for (int i = 0; i < 4; i++)
		gput16(b, o, (unsigned)counts[i]);

	int noff[24];
	int nn = 0;
	for (int sect = 0; sect < 4; sect++) {
		for (int k = 0; k < counts[sect]; k++) {
			if (o > MSGCAP - 96)
				break;
			int nameoff = o;
			unsigned kind = rndn(nn ? 4 : 2);
			if (kind == 0) {
				b[o++] = 0;
			} else if (kind == 1) {
				int nl = 1 + (int)rndn(3);
				for (int l = 0; l < nl; l++) {
					int ll = 1 + (int)rndn(4);
					b[o++] = (unsigned char)ll;
					for (int c = 0; c < ll; c++)
						b[o++] = (unsigned char)
						    ('a' + rndn(26));
				}
				b[o++] = 0;
			} else if (kind == 2) {
				int t = noff[rndn((unsigned)nn)];
				b[o++] = (unsigned char)(0xc0 |
				    ((t >> 8) & 0x3f));
				b[o++] = (unsigned char)(t & 0xff);
			} else {
				int ll = 1 + (int)rndn(4);
				b[o++] = (unsigned char)ll;
				for (int c = 0; c < ll; c++)
					b[o++] = (unsigned char)
					    ('a' + rndn(26));
				int t = noff[rndn((unsigned)nn)];
				b[o++] = (unsigned char)(0xc0 |
				    ((t >> 8) & 0x3f));
				b[o++] = (unsigned char)(t & 0xff);
			}
			if (nn < 24)
				noff[nn++] = nameoff;
			gput16(b, o, rnd32() & 0xffffu);
			gput16(b, o, rnd32() & 0xffffu);
			if (sect != 0) {
				gput32(b, o, rnd32());
				int rdl = (int)rndn(4);
				gput16(b, o, (unsigned)rdl);
				for (int c = 0; c < rdl; c++)
					b[o++] = (unsigned char)rndn(256);
			}
		}
	}

	switch (rndn(8)) {
	case 0:
		if (o > 0)
			b[rndn((unsigned)o)] ^= (unsigned char)(1u << rndn(8));
		break;
	case 1:
		o -= 1 + (int)rndn(3);
		if (o < 0)
			o = 0;
		break;
	case 2: {
		int n = 1 + (int)rndn(3);
		for (int i = 0; i < n && o < MSGCAP; i++)
			b[o++] = (unsigned char)rndn(256);
		break;
	}
	case 3:
		if (o > 0)
			b[rndn((unsigned)o)] = (unsigned char)rndn(256);
		break;
	case 4:
		if (o >= 12) {
			int i = 4 + 2 * (int)rndn(4);
			if (rndn(2))
				b[i + 1] = (unsigned char)(b[i + 1] + 1);
			else
				b[i + 1] = (unsigned char)(b[i + 1] - 1);
		}
		break;
	default:
		break;
	}
	return o;
}

static void
t_ns_skiprr(void)
{
	Stat &s = mkstat("ns_skiprr");
	Buf A, B;
	unsigned char msg[MSGCAP];

	for (int len = 0; len <= 12; len++) {
		for (int cnt = -1; cnt <= 2; cnt++) {
			for (int sect = 0; sect < 4; sect++) {
				A.fill();
				B.fill();
				for (int i = 0; i < len; i++) {
					A.base()[i] = (unsigned char)(i * 7);
					B.base()[i] = (unsigned char)(i * 7);
				}
				errno = 0;
				int ra = P::ns_skiprr(A.base(),
				    A.base() + len, (ns_sect)sect, cnt);
				int ea = errno;
				errno = 0;
				int rb = ref_ns_skiprr(B.base(),
				    B.base() + len, (ns_sect)sect, cnt);
				int eb = errno;
				note(s, ra == rb && ea == eb && sameraw(A, B),
				    "edge len=%d cnt=%d sect=%d: r %d/%d "
				    "errno %d/%d", len, cnt, sect, ra, rb,
				    ea, eb);
			}
		}
	}

	for (unsigned long it = 0; it < 200000; it++) {
		int len;
		if ((it & 1) == 0) {
			len = gen_msg(msg);
		} else {
			len = (int)rndn(40);
			for (int i = 0; i < len; i++)
				msg[i] = (unsigned char)rndn(256);
		}
		A.fill();
		B.fill();
		memcpy(A.base(), msg, (size_t)len);
		memcpy(B.base(), msg, (size_t)len);

		int off;
		switch (rndn(4)) {
		case 0: off = 0; break;
		case 1: off = len < 12 ? len : 12; break;
		case 2: off = len; break;
		default: off = len ? (int)rndn((unsigned)len + 1) : 0; break;
		}
		int cnt;
		switch (rndn(5)) {
		case 0: cnt = 0; break;
		case 1: cnt = 1; break;
		case 2: cnt = -(int)rndn(4); break;
		case 3: cnt = (int)rndn(6); break;
		default: cnt = (int)rndn(300); break;
		}
		ns_sect sect = (ns_sect)rndn(4);

		errno = 0;
		int ra = P::ns_skiprr(A.base() + off, A.base() + len, sect,
		    cnt);
		int ea = errno;
		errno = 0;
		int rb = ref_ns_skiprr(B.base() + off, B.base() + len, sect,
		    cnt);
		int eb = errno;
		note(s, ra == rb && ea == eb && sameraw(A, B),
		    "len=%d off=%d cnt=%d sect=%d: r %d/%d errno %d/%d buf@%ld",
		    len, off, cnt, (int)sect, ra, rb, ea, eb, firstdiff(A, B));
	}
}

static void
t_ns_parse(void)
{
	Stat &sI = mkstat("ns_initparse");
	Stat &sR = mkstat("ns_parserr");
	Buf A, B;
	unsigned char msg[MSGCAP];
	unsigned long ok_init = 0;
	unsigned long ok_rr = 0;

	static const int sectv[] = { -1, 0, 1, 2, 3, 4, 5, 7 };

	for (unsigned long it = 0; it < 200000; it++) {
		int len;
		if ((it % 7) == 0) {
			len = (int)rndn(20);
			for (int i = 0; i < len; i++)
				msg[i] = (unsigned char)rndn(256);
		} else {
			len = gen_msg(msg);
		}
		A.fill();
		B.fill();
		memcpy(A.base(), msg, (size_t)len);
		memcpy(B.base(), msg, (size_t)len);

		ns_msg ha, hb;
		prefill_msg(ha, A.base());
		prefill_msg(hb, B.base());

		errno = 0;
		int ra = P::ns_initparse(A.base(), len, &ha);
		int ea = errno;
		errno = 0;
		int rb = ref_ns_initparse(B.base(), len, &hb);
		int eb = errno;

		Snap sa = snap(ha, A.base()), sb = snap(hb, B.base());
		note(sI, ra == rb && ea == eb && snapeq(sa, sb) &&
		    sameraw(A, B),
		    "len=%d: r %d/%d errno %d/%d field %d buf@%ld", len, ra,
		    rb, ea, eb, snapwhich(sa, sb), firstdiff(A, B));

		if (ra != 0 || rb != 0)
			continue;
		ok_init++;

		for (int sect = 0; sect < ns_s_max; sect++) {
			int guard = (int)ha._counts[sect] + 3;
			for (int step = 0; step < guard; step++) {
				ns_rr rra, rrb;
				prefill_rr(rra, A.base());
				prefill_rr(rrb, B.base());
				errno = 0;
				int pa = P::ns_parserr(&ha, (ns_sect)sect, -1,
				    &rra);
				int ea2 = errno;
				errno = 0;
				int pb = ref_ns_parserr(&hb, (ns_sect)sect, -1,
				    &rrb);
				int eb2 = errno;
				Snap qa = snap(ha, A.base());
				Snap qb = snap(hb, B.base());
				note(sR, pa == pb && ea2 == eb2 &&
				    snapeq(qa, qb) &&
				    rreq(rra, A.base(), rrb, B.base()) &&
				    sameraw(A, B),
				    "iter len=%d sect=%d step=%d: r %d/%d "
				    "errno %d/%d field %d rr(%u/%u,%u/%u,"
				    "%lu/%lu,%u/%u)", len, sect, step, pa, pb,
				    ea2, eb2, snapwhich(qa, qb),
				    rra.type, rrb.type, rra.rr_class,
				    rrb.rr_class, (unsigned long)rra.ttl,
				    (unsigned long)rrb.ttl, rra.rdlength,
				    rrb.rdlength);
				if (pa != 0 || pb != 0)
					break;
				ok_rr++;
			}
		}

		unsigned steps = 3 + rndn(5);
		for (unsigned step = 0; step < steps; step++) {
			int sect;
			if (rndn(8) == 0)
				sect = sectv[rndn(sizeof sectv /
				    sizeof sectv[0])];
			else
				sect = (int)rndn(4);
			int rrnum;
			switch (rndn(5)) {
			case 0: rrnum = -1; break;
			case 1: rrnum = 0; break;
			case 2: rrnum = (int)rndn(4); break;
			case 3: rrnum = -2 - (int)rndn(3); break;
			default:
				rrnum = sect >= 0 && sect < ns_s_max
				    ? (int)ha._counts[sect] - 1
				    : (int)rndn(4);
				break;
			}
			ns_rr rra, rrb;
			prefill_rr(rra, A.base());
			prefill_rr(rrb, B.base());
			errno = 0;
			int pa = P::ns_parserr(&ha, (ns_sect)sect, rrnum, &rra);
			int ea2 = errno;
			errno = 0;
			int pb = ref_ns_parserr(&hb, (ns_sect)sect, rrnum,
			    &rrb);
			int eb2 = errno;
			Snap qa = snap(ha, A.base());
			Snap qb = snap(hb, B.base());
			note(sR, pa == pb && ea2 == eb2 && snapeq(qa, qb) &&
			    rreq(rra, A.base(), rrb, B.base()) &&
			    sameraw(A, B),
			    "walk len=%d sect=%d rrnum=%d: r %d/%d errno "
			    "%d/%d field %d rr(%u/%u,%u/%u,%lu/%lu,%u/%u)",
			    len, sect, rrnum, pa, pb, ea2, eb2,
			    snapwhich(qa, qb), rra.type, rrb.type,
			    rra.rr_class, rrb.rr_class,
			    (unsigned long)rra.ttl, (unsigned long)rrb.ttl,
			    rra.rdlength, rrb.rdlength);
			if (pa == 0)
				ok_rr++;
		}
	}
	printf("  (ns_initparse succeeded on %lu messages; %lu successful "
	    "ns_parserr calls)\n", ok_init, ok_rr);
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	rng_seed(0x0154B4C0FFEE1234ULL);

	printf("=== b0154s4 nameser differential test ===\n");

	t_ns_msg_getflag();
	t_setsection();
	t_ns_skiprr();
	t_ns_parse();

	unsigned long tc = 0, tf = 0;
	printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	printf("--------------------------------------------------\n");
	for (int i = 0; i < nstat; i++) {
		printf("%-24s %12lu %12lu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		tc += stats[i].cases;
		tf += stats[i].fails;
	}
	printf("--------------------------------------------------\n");
	printf("%-24s %12lu %12lu\n", "TOTAL", tc, tf);
	printf("\n%s\n", tf == 0 ? "PASS" : "FAIL");
	return tf == 0 ? 0 : 1;
}
