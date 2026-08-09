/*
 * harness.cpp -- differential test for PBSD batch b0160.
 *
 * Every ported entity is exercised against the ref_ oracle in oracle.c:
 *
 *   icmpcodes      (data, icmpcode.c)
 *   icmptypelist   (data, link dependency)
 *   ionames        (data, link dependency)
 *   dupmbt         (dupmbt.c)
 *   icmptypename   (icmptypename.c)
 *   getoptbyname   (optvalue.c)
 *   getoptbyvalue  (optvalue.c)
 *
 * dupmbt writes into a malloc'd mb_t.  malloc is interposed with
 * -Wl,--wrap=malloc so that every block handed to the port and to the oracle
 * starts out filled with the guard byte 0x7f and carries a 64 byte trailing
 * guard region.  That makes the whole destination buffer -- including the
 * bytes past the nominal bcopy() window and past the end of the object --
 * deterministic and comparable, so a copy that is one byte short or one byte
 * long is a hard failure rather than a coin flip.  Pointer results are only
 * ever compared as offsets from their own buffer base.
 */

#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sbin.ipf.libipf.b0160;

namespace P = pbsd::sbin_ipf_libipf::b0160;

extern "C" {
extern char *ref_icmpcodes[];
extern P::icmptype_t ref_icmptypelist[];
extern P::ipopt_names ref_ionames[];
P::mb_t *ref_dupmbt(P::mb_t *orig);
char *ref_icmptypename(int family, int type);
P::u_32_t ref_getoptbyname(char *optname);
P::u_32_t ref_getoptbyvalue(int optval);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr long SWEEP = 200000;
static constexpr int MAX_REPORT = 6;
static constexpr std::size_t MB_BUF_BYTES = sizeof(P::mb_t::mb_buf);

/* ---------------------------------------------------------------------- */
/* guard-filling malloc interposer                                        */
/* ---------------------------------------------------------------------- */

extern "C" void *__real_malloc(std::size_t n);

static constexpr std::size_t ALLOC_HDR = 32;
static constexpr std::size_t ALLOC_TAIL = 64;
static constexpr std::uint64_t ALLOC_MAGIC = 0x5042534430313630ULL;

extern "C" void *
__wrap_malloc(std::size_t n)
{
	unsigned char *base;
	std::uint64_t magic = ALLOC_MAGIC;

	base = (unsigned char *)__real_malloc(ALLOC_HDR + n + ALLOC_TAIL);
	if (base == nullptr)
		return nullptr;
	std::memset(base, GUARD, ALLOC_HDR + n + ALLOC_TAIL);
	std::memcpy(base, &magic, sizeof(magic));
	std::memcpy(base + sizeof(magic), &n, sizeof(n));
	return base + ALLOC_HDR;
}

static std::size_t
guard_size(const void *p)
{
	const unsigned char *base = (const unsigned char *)p - ALLOC_HDR;
	std::uint64_t magic;
	std::size_t n;

	std::memcpy(&magic, base, sizeof(magic));
	if (magic != ALLOC_MAGIC)
		return 0;
	std::memcpy(&n, base + sizeof(magic), sizeof(n));
	return n;
}

/* Every byte of the header padding and of the trailing region must be GUARD. */
static int
guard_intact(const void *p)
{
	const unsigned char *base = (const unsigned char *)p - ALLOC_HDR;
	std::size_t n = guard_size(p);
	std::size_t i;

	if (n == 0)
		return 0;
	for (i = sizeof(std::uint64_t) + sizeof(std::size_t); i < ALLOC_HDR;
	    i++) {
		if (base[i] != GUARD)
			return 0;
	}
	for (i = 0; i < ALLOC_TAIL; i++) {
		if (((const unsigned char *)p)[n + i] != GUARD)
			return 0;
	}
	return 1;
}

static void
guard_free(void *p)
{
	if (p == nullptr)
		return;
	if (guard_size(p) == 0) {
		std::free(p);
		return;
	}
	std::free((unsigned char *)p - ALLOC_HDR);
}

/* ---------------------------------------------------------------------- */
/* bookkeeping                                                            */
/* ---------------------------------------------------------------------- */

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_icmpcodes = { "icmpcodes[]", 0, 0, 0 };
static Stat st_icmptypelist = { "icmptypelist[]", 0, 0, 0 };
static Stat st_ionames = { "ionames[]", 0, 0, 0 };
static Stat st_dupmbt = { "dupmbt", 0, 0, 0 };
static Stat st_icmptypename = { "icmptypename", 0, 0, 0 };
static Stat st_getoptbyname = { "getoptbyname", 0, 0, 0 };
static Stat st_getoptbyvalue = { "getoptbyvalue", 0, 0, 0 };

static Stat *all_stats[] = { &st_icmpcodes, &st_icmptypelist, &st_ionames,
	&st_dupmbt, &st_icmptypename, &st_getoptbyname, &st_getoptbyvalue };

static void fail(Stat *st, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

static void
fail(Stat *st, const char *fmt, ...)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		va_list ap;

		st->reported++;
		std::printf("  FAIL %-14s ", st->name);
		va_start(ap, fmt);
		std::vprintf(fmt, ap);
		va_end(ap);
		std::printf("\n");
	}
}

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

static std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static std::size_t
rnd_mod(std::size_t m)
{
	return m == 0 ? 0 : (std::size_t)(rnd() % (std::uint64_t)m);
}

static const char *
show(const char *s)
{
	return s != nullptr ? s : "(null)";
}

static int
same_string(const char *a, const char *b)
{
	if (a == nullptr || b == nullptr)
		return a == b;
	return std::strcmp(a, b) == 0;
}

/* ---------------------------------------------------------------------- */
/* icmpcodes[] -- the data table that is icmpcode.c                       */
/* ---------------------------------------------------------------------- */

static void
icmpcodes_case(int idx)
{
	const char *got = P::icmpcodes[idx];
	const char *ref = ref_icmpcodes[idx];

	st_icmpcodes.cases++;
	if (!same_string(got, ref))
		fail(&st_icmpcodes, "idx=%d port=%s ref=%s", idx, show(got),
		    show(ref));
}

static void
test_icmpcodes(void)
{
	int i;

	/* MAX_ICMPCODE sizes the array and is part of the ported interface. */
	st_icmpcodes.cases++;
	if (P::MAX_ICMPCODE != 16)
		fail(&st_icmpcodes, "MAX_ICMPCODE=%d want 16",
		    P::MAX_ICMPCODE);

	/* Every slot, including the NULL terminator at MAX_ICMPCODE. */
	for (i = 0; i <= 16; i++)
		icmpcodes_case(i);

	st_icmpcodes.cases++;
	if (P::icmpcodes[16] != nullptr || ref_icmpcodes[16] != nullptr)
		fail(&st_icmpcodes, "terminator not NULL port=%s ref=%s",
		    show(P::icmpcodes[16]), show(ref_icmpcodes[16]));

	rng_seed(0x1c3d5e7f9ab00160ULL);
	for (long n = 0; n < SWEEP; n++)
		icmpcodes_case((int)rnd_mod(17));
}

/* ---------------------------------------------------------------------- */
/* icmptypelist[] and ionames[] -- data the batch functions walk          */
/* ---------------------------------------------------------------------- */

static void
icmptypelist_case(int i)
{
	const P::icmptype_t *g = &P::icmptypelist[i];
	const P::icmptype_t *r = &ref_icmptypelist[i];

	st_icmptypelist.cases++;
	if (!same_string(g->it_name, r->it_name) || g->it_v4 != r->it_v4 ||
	    g->it_v6 != r->it_v6) {
		fail(&st_icmptypelist, "i=%d port={%s,%d,%d} ref={%s,%d,%d}",
		    i, show(g->it_name), g->it_v4, g->it_v6, show(r->it_name),
		    r->it_v4, r->it_v6);
	}
}

static int icmptypelist_n;

static void
test_icmptypelist(void)
{
	int i;

	for (i = 0; ; i++) {
		int gend = P::icmptypelist[i].it_name == nullptr;
		int rend = ref_icmptypelist[i].it_name == nullptr;

		st_icmptypelist.cases++;
		if (gend != rend) {
			fail(&st_icmptypelist, "terminator disagrees at i=%d "
			    "port_end=%d ref_end=%d", i, gend, rend);
			break;
		}
		icmptypelist_case(i);
		if (gend)
			break;
	}
	icmptypelist_n = i + 1;

	rng_seed(0x2b4d6e8fa1c00160ULL);
	for (long n = 0; n < SWEEP; n++)
		icmptypelist_case((int)rnd_mod((std::size_t)icmptypelist_n));
}

static void
ionames_case(int i)
{
	const P::ipopt_names *g = &P::ionames[i];
	const P::ipopt_names *r = &ref_ionames[i];

	st_ionames.cases++;
	if (!same_string(g->on_name, r->on_name) ||
	    g->on_value != r->on_value || g->on_bit != r->on_bit ||
	    g->on_siz != r->on_siz) {
		fail(&st_ionames,
		    "i=%d port={%d,%#x,%d,%s} ref={%d,%#x,%d,%s}", i,
		    g->on_value, (unsigned)g->on_bit, g->on_siz,
		    show(g->on_name), r->on_value, (unsigned)r->on_bit,
		    r->on_siz, show(r->on_name));
	}
}

static int ionames_n;

static void
test_ionames(void)
{
	int i;

	for (i = 0; ; i++) {
		int gend = P::ionames[i].on_name == nullptr;
		int rend = ref_ionames[i].on_name == nullptr;

		st_ionames.cases++;
		if (gend != rend) {
			fail(&st_ionames, "terminator disagrees at i=%d "
			    "port_end=%d ref_end=%d", i, gend, rend);
			break;
		}
		ionames_case(i);
		if (gend)
			break;
	}
	ionames_n = i + 1;

	rng_seed(0x3c5e7f90b2d00160ULL);
	for (long n = 0; n < SWEEP; n++)
		ionames_case((int)rnd_mod((std::size_t)ionames_n));
}

/* ---------------------------------------------------------------------- */
/* dupmbt                                                                 */
/* ---------------------------------------------------------------------- */

static P::mb_t orig_p;
static P::mb_t orig_r;
static unsigned char saved_buf[MB_BUF_BYTES];

/*
 * Source bytes are chosen so that none of them is ever the guard byte.  A
 * destination byte therefore says unambiguously whether bcopy() wrote it or
 * left it alone, which is what makes an off-by-one copy length detectable
 * rather than a 255-in-256 coin flip.
 */
static unsigned char
src_byte(int pattern, std::size_t i, std::uint64_t *s)
{
	unsigned char c;

	switch (pattern) {
	case 0:
		c = 0x00;			/* NUL-heavy */
		break;
	case 1:
		c = 0xff;
		break;
	case 2:
		c = 0x80;			/* high-bit */
		break;
	case 3:
		c = (unsigned char)(0x80 + (i & 0x7f));	/* 0x80..0xff */
		break;
	case 4:
		c = (unsigned char)((i & 1) ? 0xff : 0x00);
		break;
	case 5:
		c = (unsigned char)(i & 0xff);
		break;
	default:
		*s = *s * 6364136223846793005ULL + 1442695040888963407ULL;
		c = (unsigned char)(*s >> 33);
		break;
	}
	return c == GUARD ? (unsigned char)0x80 : c;
}

static void
build_orig(std::size_t off, std::size_t len, int pattern, std::uint64_t seed)
{
	unsigned char *b = (unsigned char *)orig_p.mb_buf;
	std::uint64_t s = seed | 1;
	std::size_t i;

	std::memset(&orig_p, GUARD, sizeof(orig_p));
	for (i = 0; i < MB_BUF_BYTES; i++)
		b[i] = src_byte(pattern, i, &s);

	orig_p.mb_next = (P::mb_t *)(std::uintptr_t)0x5a5a5a5a5a5a5a5aULL;
	orig_p.mb_ifp = (void *)(std::uintptr_t)0xa5a5a5a5a5a5a5a5ULL;
	orig_p.mb_flags = 0x13572468;
	orig_p.mb_len = (int)len;
	orig_p.mb_data = (char *)orig_p.mb_buf + off;

	std::memcpy(&orig_r, &orig_p, sizeof(orig_r));
	orig_r.mb_data = (char *)orig_r.mb_buf + off;
	std::memcpy(saved_buf, b, MB_BUF_BYTES);
}

static std::ptrdiff_t
data_off(const P::mb_t *m)
{
	return m->mb_data - (const char *)m->mb_buf;
}

/*
 * Compare two mb_t objects in full, skipping only the mb_data field, whose
 * raw value is an address and is compared separately as an offset.
 */
static int
mb_equal_except_data(const P::mb_t *a, const P::mb_t *b)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;
	std::size_t head = offsetof(P::mb_t, mb_data);
	std::size_t tail = head + sizeof(a->mb_data);

	if (std::memcmp(pa, pb, head) != 0)
		return 0;
	return std::memcmp(pa + tail, pb + tail, sizeof(P::mb_t) - tail) == 0;
}

static std::size_t
first_diff(const P::mb_t *a, const P::mb_t *b)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;
	std::size_t lo = offsetof(P::mb_t, mb_data);
	std::size_t hi = lo + sizeof(a->mb_data);
	std::size_t i;

	for (i = 0; i < sizeof(P::mb_t); i++) {
		if (pa[i] != pb[i] && (i < lo || i >= hi))
			return i;
	}
	return sizeof(P::mb_t);
}

static void
dupmbt_case(std::size_t off, std::size_t len, int pattern, std::uint64_t seed)
{
	P::mb_t *got;
	P::mb_t *ref;

	build_orig(off, len, pattern, seed);

	got = P::dupmbt(&orig_p);
	ref = ref_dupmbt(&orig_r);

	st_dupmbt.cases++;

	if ((got == nullptr) != (ref == nullptr)) {
		fail(&st_dupmbt, "off=%zu len=%zu NULL-ness port=%d ref=%d",
		    off, len, got == nullptr, ref == nullptr);
	} else if (got == nullptr) {
		/* Both declined to allocate; nothing else to compare. */
	} else if (std::memcmp(orig_p.mb_buf, saved_buf, MB_BUF_BYTES) != 0 ||
	    std::memcmp(orig_r.mb_buf, saved_buf, MB_BUF_BYTES) != 0) {
		fail(&st_dupmbt, "off=%zu len=%zu source buffer modified", off,
		    len);
	} else if (orig_p.mb_len != (int)len || orig_r.mb_len != (int)len ||
	    data_off(&orig_p) != (std::ptrdiff_t)off ||
	    data_off(&orig_r) != (std::ptrdiff_t)off ||
	    !mb_equal_except_data(&orig_p, &orig_r)) {
		fail(&st_dupmbt, "off=%zu len=%zu source header modified", off,
		    len);
	} else if (data_off(got) != data_off(ref)) {
		fail(&st_dupmbt,
		    "off=%zu len=%zu mb_data offset port=%td ref=%td", off,
		    len, data_off(got), data_off(ref));
	} else if (!mb_equal_except_data(got, ref)) {
		std::size_t d = first_diff(got, ref);

		fail(&st_dupmbt, "off=%zu len=%zu pat=%d differ at mb_buf+%td "
		    "port=%02x ref=%02x", off, len, pattern,
		    (std::ptrdiff_t)d -
		    (std::ptrdiff_t)offsetof(P::mb_t, mb_buf),
		    ((const unsigned char *)got)[d],
		    ((const unsigned char *)ref)[d]);
	} else if (got->mb_next != nullptr || ref->mb_next != nullptr) {
		fail(&st_dupmbt, "off=%zu len=%zu mb_next port=%p ref=%p", off,
		    len, (void *)got->mb_next, (void *)ref->mb_next);
	} else if (got->mb_len != (int)len || ref->mb_len != (int)len) {
		fail(&st_dupmbt, "off=%zu len=%zu mb_len port=%d ref=%d", off,
		    len, got->mb_len, ref->mb_len);
	} else if (!guard_intact(got) || !guard_intact(ref)) {
		fail(&st_dupmbt, "off=%zu len=%zu wrote outside the object "
		    "(port_ok=%d ref_ok=%d)", off, len, guard_intact(got),
		    guard_intact(ref));
	} else if (len > 0 &&
	    std::memcmp(got->mb_data, saved_buf + off, len) != 0) {
		fail(&st_dupmbt, "off=%zu len=%zu payload not copied", off,
		    len);
	} else if (data_off(got) != (std::ptrdiff_t)off) {
		fail(&st_dupmbt, "off=%zu len=%zu mb_data offset %td", off,
		    len, data_off(got));
	}

	guard_free(got);
	guard_free(ref);
}

static void
test_dupmbt(void)
{
	static const std::size_t N = MB_BUF_BYTES;
	int pat;

	/* Hand-written edges: both sides of every boundary that exists. */
	for (pat = 0; pat <= 6; pat++) {
		dupmbt_case(0, 0, pat, 1);		/* empty, at base */
		dupmbt_case(0, 1, pat, 2);		/* single byte */
		dupmbt_case(0, 2, pat, 3);
		dupmbt_case(1, 0, pat, 4);		/* empty, offset 1 */
		dupmbt_case(1, 1, pat, 5);
		dupmbt_case(7, 1, pat, 6);		/* unaligned */
		dupmbt_case(8, 8, pat, 7);
		dupmbt_case(0, N - 1, pat, 8);
		dupmbt_case(0, N, pat, 9);		/* whole buffer */
		dupmbt_case(1, N - 1, pat, 10);		/* ends exactly */
		dupmbt_case(N - 1, 1, pat, 11);		/* last byte */
		dupmbt_case(N, 0, pat, 12);		/* one past the end */
		dupmbt_case(N / 2, N / 2, pat, 13);
		dupmbt_case(N / 2, 0, pat, 14);
		dupmbt_case(N / 2 - 1, 2, pat, 15);
		dupmbt_case(3, 5, pat, 16);
		dupmbt_case(4096, 4096, pat, 17);
	}

	rng_seed(0x4d6f809ac3e00160ULL);
	for (long n = 0; n < SWEEP; n++) {
		std::size_t off;
		std::size_t len;
		int p = (int)rnd_mod(7);

		/*
		 * Half the draws stay small so that short copies and small
		 * offsets are common; the rest span the whole buffer,
		 * including the off + len == sizeof(mb_buf) boundary.
		 */
		if ((rnd() & 1) != 0) {
			off = rnd_mod(65);
			len = rnd_mod(65);
			if (off + len > N)
				len = N - off;
		} else {
			off = rnd_mod(N + 1);
			len = rnd_mod(N - off + 1);
			if ((rnd() & 7) == 0)
				len = N - off;	/* exactly to the end */
		}
		dupmbt_case(off, len, p, rnd());
	}
}

/* ---------------------------------------------------------------------- */
/* icmptypename                                                           */
/* ---------------------------------------------------------------------- */

static void
icmptypename_case(int family, int type)
{
	char *got = P::icmptypename(family, type);
	char *ref = ref_icmptypename(family, type);

	st_icmptypename.cases++;
	if (!same_string(got, ref))
		fail(&st_icmptypename, "family=%d type=%d port=%s ref=%s",
		    family, type, show(got), show(ref));
}

static const int fam_list[] = { 2, 28, 0, 1, 3, 10, -1, 255, 256, -28,
	INT_MIN, INT_MAX };
static constexpr std::size_t N_FAM =
    sizeof(fam_list) / sizeof(fam_list[0]);

static void
test_icmptypename(void)
{
	std::size_t f;
	int t;

	/*
	 * Sweep every type in [-8, 300] for every interesting family.  That
	 * covers each table row, both sides of the (type < 0) test, both
	 * sides of the (type > 255) test, and the negative types that
	 * separate (type < 0) || (type > 255) from its && mutant -- rows
	 * exist with it_v4 == -1 and with it_v6 == -1, so a mutant that
	 * lets type == -1 reach the loop returns a name instead of NULL.
	 */
	for (f = 0; f < N_FAM; f++) {
		for (t = -8; t <= 300; t++)
			icmptypename_case(fam_list[f], t);
		icmptypename_case(fam_list[f], INT_MIN);
		icmptypename_case(fam_list[f], INT_MAX);
		icmptypename_case(fam_list[f], -256);
		icmptypename_case(fam_list[f], 65536);
	}

	rng_seed(0x5e7091abd4f00160ULL);
	for (long n = 0; n < SWEEP; n++) {
		int family;
		int type;
		std::uint64_t r = rnd();

		switch (r & 3) {
		case 0:
			family = 2;			/* AF_INET */
			break;
		case 1:
			family = 28;			/* AF_INET6 */
			break;
		case 2:
			family = fam_list[rnd_mod(N_FAM)];
			break;
		default:
			family = (int)(std::int32_t)rnd();
			break;
		}
		if ((r & 4) != 0)
			type = (int)(std::int32_t)rnd();
		else
			type = (int)rnd_mod(330) - 16;
		icmptypename_case(family, type);
	}
}

/* ---------------------------------------------------------------------- */
/* getoptbyname                                                           */
/* ---------------------------------------------------------------------- */

static constexpr std::size_t NAMEBUF = 256;
static constexpr std::size_t NAMEOFF = 64;

/*
 * Two buffers, both filled with the guard byte, both given the same input.
 * The whole 256 bytes of each are compared afterwards, so a port that writes
 * anywhere in its argument buffer is caught even though getoptbyname() is
 * only supposed to read.
 */
static void
getoptbyname_case(const char *name, std::size_t nlen)
{
	char pbuf[NAMEBUF];
	char rbuf[NAMEBUF];
	P::u_32_t got;
	P::u_32_t ref;

	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(rbuf, GUARD, sizeof(rbuf));
	std::memcpy(pbuf + NAMEOFF, name, nlen);
	pbuf[NAMEOFF + nlen] = '\0';
	std::memcpy(rbuf + NAMEOFF, name, nlen);
	rbuf[NAMEOFF + nlen] = '\0';

	got = P::getoptbyname(pbuf + NAMEOFF);
	ref = ref_getoptbyname(rbuf + NAMEOFF);

	st_getoptbyname.cases++;
	if (got != ref) {
		fail(&st_getoptbyname, "name=\"%s\" port=%#x ref=%#x",
		    rbuf + NAMEOFF, (unsigned)got, (unsigned)ref);
	} else if (std::memcmp(pbuf, rbuf, sizeof(pbuf)) != 0) {
		fail(&st_getoptbyname, "name=\"%s\" argument buffer diverged",
		    rbuf + NAMEOFF);
	}
}

static void
getoptbyname_str(const char *name)
{
	getoptbyname_case(name, std::strlen(name));
}

static const char *known_names[] = { "nop", "rr", "zsu", "mtup", "mtur",
	"encode", "ts", "tr", "sec", "sec-class", "lsrr", "e-sec", "cipso",
	"satid", "ssrr", "addext", "visa", "imitd", "eip", "finn", "dps",
	"sdb", "nsapa", "rtralrt", "ump", "ah" };
static constexpr std::size_t N_KNOWN =
    sizeof(known_names) / sizeof(known_names[0]);

static void
test_getoptbyname(void)
{
	std::size_t i;
	std::size_t j;
	char buf[NAMEBUF];

	getoptbyname_str("");
	getoptbyname_str("\x7f");
	getoptbyname_str("\x80");
	getoptbyname_str("\xff");
	getoptbyname_str("\xff\xfe\x80\x81");
	getoptbyname_str("\x80nop");
	getoptbyname_str("bogus");
	getoptbyname_str("secclass");
	getoptbyname_str("sec-clas");
	getoptbyname_str("sec-classs");
	getoptbyname_str("n");
	getoptbyname_str("no");
	getoptbyname_str("nopp");
	getoptbyname_str("op");
	getoptbyname_str("a");
	getoptbyname_str("z");
	getoptbyname_str("zz");

	/* Names that stop early at an embedded NUL. */
	getoptbyname_case("nop\0rr", 6);
	getoptbyname_case("\0nop", 4);
	getoptbyname_case("ah\0\0\0", 5);

	/* Every table name, plus case variants and single-character edits. */
	for (i = 0; i < N_KNOWN; i++) {
		std::size_t n = std::strlen(known_names[i]);

		getoptbyname_str(known_names[i]);

		for (j = 0; j < n; j++) {
			char c = known_names[i][j];

			buf[j] = (c >= 'a' && c <= 'z') ?
			    (char)(c - 'a' + 'A') : c;
		}
		buf[n] = '\0';
		getoptbyname_str(buf);			/* upper cased */

		for (j = 0; j < n; j++) {
			char c = known_names[i][j];

			buf[j] = ((j & 1) != 0 && c >= 'a' && c <= 'z') ?
			    (char)(c - 'a' + 'A') : c;
		}
		buf[n] = '\0';
		getoptbyname_str(buf);			/* mixed case */

		std::memcpy(buf, known_names[i], n);
		buf[n] = 'x';
		buf[n + 1] = '\0';
		getoptbyname_str(buf);			/* one char longer */

		if (n > 0) {
			std::memcpy(buf, known_names[i], n);
			buf[n - 1] = '\0';
			getoptbyname_str(buf);		/* one char shorter */
		}

		for (j = 0; j < n; j++) {
			std::memcpy(buf, known_names[i], n + 1);
			buf[j] = (char)(buf[j] ^ 0x01);
			getoptbyname_str(buf);		/* one bit flipped */
			std::memcpy(buf, known_names[i], n + 1);
			buf[j] = (char)0x80;
			getoptbyname_str(buf);		/* high-bit byte */
		}
	}

	rng_seed(0x6f81a2bce5000160ULL);
	for (long n = 0; n < SWEEP; n++) {
		std::uint64_t r = rnd();
		std::size_t len;

		switch (r % 5) {
		case 0: {				/* exact, random case */
			const char *src = known_names[rnd_mod(N_KNOWN)];

			len = std::strlen(src);
			for (i = 0; i < len; i++) {
				char c = src[i];

				if ((rnd() & 1) != 0 && c >= 'a' && c <= 'z')
					c = (char)(c - 'a' + 'A');
				buf[i] = c;
			}
			break;
		}
		case 1: {				/* one byte perturbed */
			const char *src = known_names[rnd_mod(N_KNOWN)];

			len = std::strlen(src);
			std::memcpy(buf, src, len);
			buf[rnd_mod(len)] = (char)(rnd() & 0xff);
			break;
		}
		case 2: {				/* length perturbed */
			const char *src = known_names[rnd_mod(N_KNOWN)];

			len = std::strlen(src);
			std::memcpy(buf, src, len);
			if ((rnd() & 1) != 0) {
				len--;
			} else {
				buf[len] = (char)('a' + rnd_mod(26));
				len++;
			}
			break;
		}
		case 3: {				/* small alphabet */
			static const char alpha[] = "abcdeimnoprstuz-";

			len = rnd_mod(12);
			for (i = 0; i < len; i++)
				buf[i] = alpha[rnd_mod(sizeof(alpha) - 1)];
			break;
		}
		default:				/* arbitrary bytes */
			len = rnd_mod(40);
			for (i = 0; i < len; i++) {
				unsigned char c;

				do {
					c = (unsigned char)(rnd() & 0xff);
				} while (c == 0);
				buf[i] = (char)c;
			}
			break;
		}
		buf[len] = '\0';
		getoptbyname_case(buf, len);
	}
}

/* ---------------------------------------------------------------------- */
/* getoptbyvalue                                                          */
/* ---------------------------------------------------------------------- */

static void
getoptbyvalue_case(int optval)
{
	P::u_32_t got = P::getoptbyvalue(optval);
	P::u_32_t ref = ref_getoptbyvalue(optval);

	st_getoptbyvalue.cases++;
	if (got != ref)
		fail(&st_getoptbyvalue, "optval=%d port=%#x ref=%#x", optval,
		    (unsigned)got, (unsigned)ref);
}

static void
test_getoptbyvalue(void)
{
	int i;

	/*
	 * Every on_value in the table plus both neighbours, so the ==
	 * comparison is driven to true and to false from either side.
	 */
	for (i = 0; ref_ionames[i].on_name != nullptr; i++) {
		getoptbyvalue_case(ref_ionames[i].on_value - 1);
		getoptbyvalue_case(ref_ionames[i].on_value);
		getoptbyvalue_case(ref_ionames[i].on_value + 1);
	}

	/* Exhaustive over the range the table lives in, and past both ends. */
	for (i = -8; i <= 512; i++)
		getoptbyvalue_case(i);

	getoptbyvalue_case(INT_MIN);
	getoptbyvalue_case(INT_MIN + 1);
	getoptbyvalue_case(INT_MAX - 1);
	getoptbyvalue_case(INT_MAX);
	getoptbyvalue_case(0x1000000);

	rng_seed(0x7092b3cdf6100160ULL);
	for (long n = 0; n < SWEEP; n++) {
		std::uint64_t r = rnd();
		int v;

		switch (r & 3) {
		case 0:
			v = ref_ionames[rnd_mod((std::size_t)ionames_n)]
			    .on_value;
			break;
		case 1:
			v = (int)rnd_mod(600) - 32;
			break;
		case 2:
			v = ref_ionames[rnd_mod((std::size_t)ionames_n)]
			    .on_value + (int)rnd_mod(3) - 1;
			break;
		default:
			v = (int)(std::int32_t)rnd();
			break;
		}
		getoptbyvalue_case(v);
	}
}

/* ---------------------------------------------------------------------- */

int
main(void)
{
	std::size_t i;
	long cases = 0;
	long fails = 0;

	test_icmpcodes();
	test_icmptypelist();
	test_ionames();
	test_dupmbt();
	test_icmptypename();
	test_getoptbyname();
	test_getoptbyvalue();

	std::printf("\n%-16s %10s %10s  %s\n", "entity", "cases", "fails",
	    "result");
	for (i = 0; i < sizeof(all_stats) / sizeof(all_stats[0]); i++) {
		Stat *s = all_stats[i];

		cases += s->cases;
		fails += s->fails;
		std::printf("%-16s %10ld %10ld  %s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "ok" : "FAIL");
	}
	std::printf("%-16s %10ld %10ld  %s\n", "TOTAL", cases, fails,
	    fails == 0 ? "PASS" : "FAIL");

	return fails == 0 ? 0 : 1;
}
