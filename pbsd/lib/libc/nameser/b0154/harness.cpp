/*
 * Batch b0154 differential test: pbsd::lib_libc_nameser::b0154 vs ref_ oracle.
 */

#include <arpa/nameser.h>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>

import pbsd.lib.libc.nameser.b0154;

namespace P = pbsd::lib_libc_nameser::b0154;

extern "C" {
u_int	ref_ns_get16(const u_char *);
u_long	ref_ns_get32(const u_char *);
void	ref_ns_put16(u_int, u_char *);
void	ref_ns_put32(u_long, u_char *);
int	ref_ns_format_ttl(u_long, char *, size_t);
int	ref_ns_parse_ttl(const char *, u_long *);
int	ref_ns_samedomain(const char *, const char *);
int	ref_ns_subdomain(const char *, const char *);
int	ref_ns_makecanon(const char *, char *, size_t);
int	ref_ns_samename(const char *, const char *);
int	ref_ns_msg_getflag(ns_msg, int);
int	ref_ns_skiprr(const u_char *, const u_char *, ns_sect, int);
int	ref_ns_initparse(const u_char *, int, ns_msg *);
int	ref_ns_parserr(ns_msg *, ns_sect, int, ns_rr *);
int	ref_ns_parserr2(ns_msg *, ns_sect, int, P::ns_rr2 *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t BUF_CAP = 4096;
constexpr size_t OUT_PREFIX = 64;
constexpr unsigned long long RANDOM_ITERS = 200000;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "ns_get16", 0, 0 },
	{ "ns_get32", 0, 0 },
	{ "ns_put16", 0, 0 },
	{ "ns_put32", 0, 0 },
	{ "ns_format_ttl", 0, 0 },
	{ "ns_parse_ttl", 0, 0 },
	{ "ns_samedomain", 0, 0 },
	{ "ns_subdomain", 0, 0 },
	{ "ns_makecanon", 0, 0 },
	{ "ns_samename", 0, 0 },
	{ "ns_msg_getflag", 0, 0 },
	{ "ns_skiprr", 0, 0 },
	{ "ns_initparse", 0, 0 },
	{ "ns_parserr", 0, 0 },
	{ "ns_parserr2", 0, 0 },
};

enum fn_idx {
	F_GET16 = 0, F_GET32, F_PUT16, F_PUT32,
	F_FMTTTL, F_PARSETTL,
	F_SAMEDOM, F_SUBDOM, F_MAKECANON, F_SAMENAME,
	F_GETFLAG, F_SKIPRR, F_INITPARSE, F_PARSERR, F_PARSERR2,
	F_COUNT
};

unsigned long long reported = 0;
constexpr unsigned long long report_limit = 30;

std::uint64_t rng = 0x243F6A8885A308D3ULL;

std::uint64_t
nxt()
{
	std::uint64_t z = (rng += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

unsigned
u32()
{
	return (unsigned)(nxt() & 0xffffffffu);
}

void
fail(int f, const char *why)
{
	tbl[f].failures++;
	if (reported < report_limit) {
		reported++;
		std::printf("FAIL %s: %s\n", tbl[f].name, why);
	}
}

void
fill_guard(unsigned char *b, size_t n)
{
	std::memset(b, GUARD, n);
}

bool
bufs_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

void
put16(unsigned char *p, unsigned v)
{
	p[0] = (unsigned char)(v >> 8);
	p[1] = (unsigned char)v;
}

size_t
write_label_name(unsigned char *dst, const char *host)
{
	size_t n = 0;

	while (*host != '\0') {
		const char *dot = std::strchr(host, '.');
		size_t lablen = dot ? (size_t)(dot - host) : std::strlen(host);

		if (lablen == 0 || lablen > NS_MAXLABEL)
			return 0;
		dst[n++] = (unsigned char)lablen;
		std::memcpy(dst + n, host, lablen);
		n += lablen;
		if (dot == nullptr)
			break;
		host = dot + 1;
	}
	dst[n++] = 0;
	return n;
}

size_t
build_msg(unsigned char *buf, size_t cap, unsigned id, unsigned flags,
    unsigned qd, unsigned an, unsigned ns, unsigned ar,
    const char *qname, unsigned qtype, unsigned qclass,
    unsigned ttl, const unsigned char *rdata, unsigned rdlen)
{
	size_t off = 12;

	if (cap < 12)
		return 0;
	put16(buf + 0, id);
	put16(buf + 2, flags);
	put16(buf + 4, qd);
	put16(buf + 6, an);
	put16(buf + 8, ns);
	put16(buf + 10, ar);

	if (qd > 0) {
		size_t nl = write_label_name(buf + off, qname);

		if (nl == 0 || off + nl + 4 > cap)
			return 0;
		off += nl;
		put16(buf + off, qtype);
		off += 2;
		put16(buf + off, qclass);
		off += 2;
	}

	for (unsigned i = 0; i < an + ns + ar; i++) {
		size_t nl;

		if (off + 1 > cap)
			return 0;
		if (i == 0 && qd > 0) {
			buf[off++] = 0xc0;
			buf[off++] = 12;
			nl = 2;
		} else {
			nl = write_label_name(buf + off, "x.test.");
			if (nl == 0)
				return 0;
			off += nl;
		}
		if (off + 10 + rdlen > cap)
			return 0;
		put16(buf + off, 1);
		off += 2;
		put16(buf + off, 1);
		off += 2;
		put16(buf + off, (unsigned)(ttl >> 16));
		put16(buf + off + 2, (unsigned)(ttl & 0xffff));
		off += 4;
		put16(buf + off, rdlen);
		off += 2;
		if (rdlen > 0) {
			std::memcpy(buf + off, rdata, rdlen);
			off += rdlen;
		}
	}

	return off;
}

void
check_get16(const unsigned char *src, int f)
{
	tbl[f].cases++;
	unsigned got = P::ns_get16(src);
	unsigned want = ref_ns_get16(src);
	if (got != want)
		fail(f, "ret");
}

void
check_get32(const unsigned char *src, int f)
{
	tbl[f].cases++;
	u_long got = P::ns_get32(src);
	u_long want = ref_ns_get32(src);
	if (got != want)
		fail(f, "ret");
}

void
check_put16(unsigned v, int f)
{
	unsigned char pbuf[BUF_CAP], rbuf[BUF_CAP];

	tbl[f].cases++;
	fill_guard(pbuf, sizeof(pbuf));
	fill_guard(rbuf, sizeof(rbuf));
	P::ns_put16(v, pbuf + OUT_PREFIX);
	ref_ns_put16(v, rbuf + OUT_PREFIX);
	if (!bufs_eq(pbuf, rbuf, sizeof(pbuf)))
		fail(f, "buf");
}

void
check_put32(u_long v, int f)
{
	unsigned char pbuf[BUF_CAP], rbuf[BUF_CAP];

	tbl[f].cases++;
	fill_guard(pbuf, sizeof(pbuf));
	fill_guard(rbuf, sizeof(rbuf));
	P::ns_put32(v, pbuf + OUT_PREFIX);
	ref_ns_put32(v, rbuf + OUT_PREFIX);
	if (!bufs_eq(pbuf, rbuf, sizeof(pbuf)))
		fail(f, "buf");
}

void
check_format_ttl(u_long ttl, size_t dstlen, int f)
{
	unsigned char pbuf[BUF_CAP], rbuf[BUF_CAP];
	int pr, rr;

	tbl[f].cases++;
	fill_guard(pbuf, sizeof(pbuf));
	fill_guard(rbuf, sizeof(rbuf));
	pr = P::ns_format_ttl(ttl, (char *)(pbuf + OUT_PREFIX), dstlen);
	rr = ref_ns_format_ttl(ttl, (char *)(rbuf + OUT_PREFIX), dstlen);
	if (pr != rr)
		fail(f, "ret");
	if (!bufs_eq(pbuf, rbuf, sizeof(pbuf)))
		fail(f, "buf");
}

void
check_parse_ttl(const char *src, int f)
{
	u_long pttl = 0x55555555UL, rttl = 0x55555555UL;
	int pr, rr, perrno, rerrno;

	tbl[f].cases++;
	errno = 0;
	pr = P::ns_parse_ttl(src, &pttl);
	perrno = errno;
	errno = 0;
	rr = ref_ns_parse_ttl(src, &rttl);
	rerrno = errno;
	if (pr != rr)
		fail(f, "ret");
	if (pr == 0 && pttl != rttl)
		fail(f, "dst");
	if (pr != 0 && perrno != rerrno)
		fail(f, "errno");
}

void
check_samedomain(const char *a, const char *b, int f)
{
	tbl[f].cases++;
	int got = P::ns_samedomain(a, b);
	int want = ref_ns_samedomain(a, b);
	if (got != want)
		fail(f, "ret");
}

void
check_subdomain(const char *a, const char *b, int f)
{
	tbl[f].cases++;
	int got = P::ns_subdomain(a, b);
	int want = ref_ns_subdomain(a, b);
	if (got != want)
		fail(f, "ret");
}

void
check_makecanon(const char *src, size_t dstsize, int f)
{
	unsigned char pbuf[BUF_CAP], rbuf[BUF_CAP];
	int pr, rr, perrno, rerrno;

	tbl[f].cases++;
	fill_guard(pbuf, sizeof(pbuf));
	fill_guard(rbuf, sizeof(rbuf));
	errno = 0;
	pr = P::ns_makecanon(src, (char *)(pbuf + OUT_PREFIX), dstsize);
	perrno = errno;
	errno = 0;
	rr = ref_ns_makecanon(src, (char *)(rbuf + OUT_PREFIX), dstsize);
	rerrno = errno;
	if (pr != rr)
		fail(f, "ret");
	if (pr == 0 && !bufs_eq(pbuf, rbuf, sizeof(pbuf)))
		fail(f, "buf");
	if (pr != 0 && perrno != rerrno)
		fail(f, "errno");
}

void
check_samename(const char *a, const char *b, int f)
{
	tbl[f].cases++;
	int got = P::ns_samename(a, b);
	int want = ref_ns_samename(a, b);
	if (got != want)
		fail(f, "ret");
}

void
check_msg_getflag(unsigned flags, int flag, int f)
{
	ns_msg h{};

	tbl[f].cases++;
	h._flags = flags;
	int got = P::ns_msg_getflag(h, flag);
	int want = ref_ns_msg_getflag(h, flag);
	if (got != want)
		fail(f, "ret");
}

void
check_skiprr(const unsigned char *pkt, size_t pktlen, ns_sect sect, int count,
    int f)
{
	const u_char *eom = pkt + pktlen;
	int pr, rr, perrno, rerrno;

	tbl[f].cases++;
	errno = 0;
	pr = P::ns_skiprr(pkt + 12, eom, sect, count);
	perrno = errno;
	errno = 0;
	rr = ref_ns_skiprr(pkt + 12, eom, sect, count);
	rerrno = errno;
	if (pr != rr)
		fail(f, "ret");
	if (pr < 0 && perrno != rerrno)
		fail(f, "errno");
}

void
check_initparse(const unsigned char *pkt, int msglen, int f)
{
	ns_msg ph{}, rh{};
	int pr, rr, perrno, rerrno;

	tbl[f].cases++;
	errno = 0;
	pr = P::ns_initparse(pkt, msglen, &ph);
	perrno = errno;
	errno = 0;
	rr = ref_ns_initparse(pkt, msglen, &rh);
	rerrno = errno;
	if (pr != rr)
		fail(f, "ret");
	if (pr != 0 && perrno != rerrno)
		fail(f, "errno");
	if (pr == 0) {
		if (ph._id != rh._id || ph._flags != rh._flags)
			fail(f, "hdr");
		for (int i = 0; i < ns_s_max; i++) {
			if (ph._counts[i] != rh._counts[i])
				fail(f, "count");
			if ((ph._sections[i] == nullptr) !=
			    (rh._sections[i] == nullptr))
				fail(f, "sect-null");
			if (ph._sections[i] != nullptr && rh._sections[i] !=
			    nullptr &&
			    (ph._sections[i] - pkt) != (rh._sections[i] - pkt))
				fail(f, "sect-off");
		}
	}
}

void
check_parserr(const unsigned char *pkt, int msglen, ns_sect sect, int rrnum,
    int f)
{
	ns_msg ph{}, rh{};
	ns_rr prr{}, rrr{};
	int pi, ri, perrno, rerrno;

	tbl[f].cases++;
	if (P::ns_initparse(pkt, msglen, &ph) != 0)
		return;
	if (ref_ns_initparse(pkt, msglen, &rh) != 0)
		return;
	std::memset(&prr, 0x7f, sizeof(prr));
	std::memset(&rrr, 0x7f, sizeof(rrr));
	errno = 0;
	pi = P::ns_parserr(&ph, sect, rrnum, &prr);
	perrno = errno;
	errno = 0;
	ri = ref_ns_parserr(&rh, sect, rrnum, &rrr);
	rerrno = errno;
	if (pi != ri)
		fail(f, "ret");
	if (pi != 0 && perrno != rerrno)
		fail(f, "errno");
	if (pi == 0) {
		if (std::strcmp(prr.name, rrr.name) != 0)
			fail(f, "name");
		if (prr.type != rrr.type || prr.rr_class != rrr.rr_class ||
		    prr.ttl != rrr.ttl || prr.rdlength != rrr.rdlength)
			fail(f, "fields");
		if ((prr.rdata == nullptr) != (rrr.rdata == nullptr))
			fail(f, "rdata-null");
		if (prr.rdata != nullptr && rrr.rdata != nullptr &&
		    (prr.rdata - pkt) != (rrr.rdata - pkt))
			fail(f, "rdata-off");
		if (ph._rrnum != rh._rrnum || ph._sect != rh._sect)
			fail(f, "state");
		if ((ph._msg_ptr == nullptr) != (rh._msg_ptr == nullptr))
			fail(f, "ptr-null");
		if (ph._msg_ptr != nullptr && rh._msg_ptr != nullptr &&
		    (ph._msg_ptr - pkt) != (rh._msg_ptr - pkt))
			fail(f, "ptr-off");
	}
}

void
check_parserr2(const unsigned char *pkt, int msglen, ns_sect sect, int rrnum,
    int f)
{
	ns_msg ph{}, rh{};
	P::ns_rr2 prr{}, rrr{};
	int pi, ri, perrno, rerrno;

	tbl[f].cases++;
	if (P::ns_initparse(pkt, msglen, &ph) != 0)
		return;
	if (ref_ns_initparse(pkt, msglen, &rh) != 0)
		return;
	std::memset(&prr, 0x7f, sizeof(prr));
	std::memset(&rrr, 0x7f, sizeof(rrr));
	errno = 0;
	pi = P::ns_parserr2(&ph, sect, rrnum, &prr);
	perrno = errno;
	errno = 0;
	ri = ref_ns_parserr2(&rh, sect, rrnum, &rrr);
	rerrno = errno;
	if (pi != ri)
		fail(f, "ret");
	if (pi != 0 && perrno != rerrno)
		fail(f, "errno");
	if (pi == 0) {
		if (prr.nnamel != rrr.nnamel ||
		    std::memcmp(prr.nname, rrr.nname, prr.nnamel) != 0)
			fail(f, "nname");
		if (prr.type != rrr.type || prr.rr_class != rrr.rr_class ||
		    prr.ttl != rrr.ttl || prr.rdlength != rrr.rdlength)
			fail(f, "fields");
		if ((prr.rdata == nullptr) != (rrr.rdata == nullptr))
			fail(f, "rdata-null");
		if (prr.rdata != nullptr && rrr.rdata != nullptr &&
		    (prr.rdata - pkt) != (rrr.rdata - pkt))
			fail(f, "rdata-off");
		if (ph._rrnum != rh._rrnum || ph._sect != rh._sect)
			fail(f, "state");
	}
}

void
edge_netint()
{
	unsigned char b[8] = {0};

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			b[0] = (unsigned char)i;
			b[1] = (unsigned char)j;
			check_get16(b, F_GET16);
		}
	}
	for (int a = 0; a < 256; a++) {
		b[0] = (unsigned char)a;
		b[1] = 0x12; b[2] = 0x34; b[3] = 0x56;
		check_get32(b, F_GET32);
	}
	check_put16(0, F_PUT16);
	check_put16(0xffff, F_PUT16);
	check_put16(0x8000, F_PUT16);
	check_put32(0, F_PUT32);
	check_put32(0xffffffffUL, F_PUT32);
	check_put32(0x80000000UL, F_PUT32);
}

void
edge_ttl()
{
	static const u_long ttls[] = {
		0, 1, 59, 60, 61, 3599, 3600, 3601, 86399, 86400, 86401,
		604799, 604800, 604801, 0x7fffffffUL, 0xffffffffUL,
	};
	static const char *good[] = {
		"0", "1S", "60S", "1M", "1H", "1D", "1W",
		"1W2D", "2D3H4M5S", "90S", "3600", "1w2d3h4m5s",
	};
	static const char *bad[] = {
		"", "S", "1X", "1S2", "1S1S", "1W1", "\x7f", "\xff",
		"1\x80S", " 1S", "1 S",
	};

	for (unsigned i = 0; i < sizeof(ttls) / sizeof(ttls[0]); i++) {
		check_format_ttl(ttls[i], 256, F_FMTTTL);
		check_format_ttl(ttls[i], 1, F_FMTTTL);
		check_format_ttl(ttls[i], 0, F_FMTTTL);
	}
	for (unsigned i = 0; i < sizeof(good) / sizeof(good[0]); i++)
		check_parse_ttl(good[i], F_PARSETTL);
	for (unsigned i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
		check_parse_ttl(bad[i], F_PARSETTL);
}

void
edge_domain()
{
	static const char *pairs[][2] = {
		{ "", "" },
		{ "a", "" },
		{ "host.example.com", "example.com" },
		{ "host.example.com.", "example.com." },
		{ "host.example.com", "bar.example.com" },
		{ "foobar.com", "bar.com" },
		{ "foo\\.bar.com", "bar.com" },
		{ "foo\\\\.bar.com", "bar.com" },
		{ "x", "y" },
		{ "a.b", "a.b" },
		{ "a.b.", "a.b" },
		{ "foo..", "foo." },
		{ "foo\\.", "foo\\." },
		{ "foo\\\\.", "foo\\\\." },
	};

	for (unsigned i = 0; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
		check_samedomain(pairs[i][0], pairs[i][1], F_SAMEDOM);
		check_subdomain(pairs[i][0], pairs[i][1], F_SUBDOM);
		check_samename(pairs[i][0], pairs[i][1], F_SAMENAME);
	}
	check_makecanon("", 4, F_MAKECANON);
	check_makecanon("a", 4, F_MAKECANON);
	check_makecanon("foo", 3, F_MAKECANON);
	check_makecanon("foo.", 8, F_MAKECANON);
	check_makecanon("foo..", 16, F_MAKECANON);
	check_makecanon("foo\\.", 16, F_MAKECANON);
}

void
edge_parse()
{
	unsigned char pkt[512];
	unsigned char rdata[] = {0xde, 0xad};
	size_t len;

	for (int flag = 0; flag < 10; flag++)
		for (unsigned fl = 0; fl <= 0xffff; fl += 0x1111)
			check_msg_getflag(fl, flag, F_GETFLAG);
	check_msg_getflag(0xffff, 9, F_GETFLAG);
	check_msg_getflag(0x0000, 0, F_GETFLAG);

	len = build_msg(pkt, sizeof(pkt), 0x1234, 0x8180, 1, 0, 0, 0,
	    "a.example.com", 1, 1, 0, nullptr, 0);
	check_initparse(pkt, (int)len, F_INITPARSE);
	check_skiprr(pkt, len, ns_s_qd, 1, F_SKIPRR);
	check_parserr(pkt, (int)len, ns_s_qd, 0, F_PARSERR);
	check_parserr2(pkt, (int)len, ns_s_qd, 0, F_PARSERR2);
	check_parserr(pkt, (int)len, ns_s_qd, -1, F_PARSERR);
	check_parserr2(pkt, (int)len, ns_s_qd, -1, F_PARSERR2);

	len = build_msg(pkt, sizeof(pkt), 0xbeef, 0x0000, 1, 1, 0, 0,
	    "x.test", 28, 1, 300, rdata, 2);
	check_initparse(pkt, (int)len, F_INITPARSE);
	check_skiprr(pkt, len, ns_s_an, 1, F_SKIPRR);
	check_parserr(pkt, (int)len, ns_s_an, 0, F_PARSERR);
	check_parserr2(pkt, (int)len, ns_s_an, 0, F_PARSERR2);

	check_initparse(pkt, 0, F_INITPARSE);
	check_initparse(pkt, 1, F_INITPARSE);
	check_initparse(pkt, 11, F_INITPARSE);
	check_skiprr(pkt, len, ns_s_qd, 99, F_SKIPRR);
	check_parserr(pkt, (int)len, ns_s_max, 0, F_PARSERR);
	check_parserr(pkt, (int)len, ns_s_qd, 99, F_PARSERR);
}

void
rand_netint()
{
	unsigned char b[8];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		b[0] = (unsigned char)u32();
		b[1] = (unsigned char)u32();
		check_get16(b, F_GET16);
		b[0] = (unsigned char)u32();
		b[1] = (unsigned char)u32();
		b[2] = (unsigned char)u32();
		b[3] = (unsigned char)u32();
		check_get32(b, F_GET32);
		check_put16((unsigned)u32(), F_PUT16);
		check_put32((u_long)nxt(), F_PUT32);
	}
}

void
rand_ttl()
{
	char src[64];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		u_long ttl = (u_long)nxt();
		size_t cap = (size_t)(u32() % 128);

		check_format_ttl(ttl, cap, F_FMTTTL);

		unsigned n = u32() % 20;
		for (unsigned j = 0; j < n; j++) {
			unsigned v = u32() % 128;
			if (v < 10)
				src[j] = (char)('0' + v);
			else if (v < 20)
				src[j] = (char)("wdhmsWDHMS"[v - 10]);
			else
				src[j] = (char)(32 + (v % 95));
		}
		src[n] = '\0';
		check_parse_ttl(src, F_PARSETTL);
	}
}

void
rand_domain()
{
	char a[128], b[128];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		unsigned na = u32() % 80;
		unsigned nb = u32() % 80;

		for (unsigned j = 0; j < na; j++)
			a[j] = (char)(u32() % 256);
		a[na] = '\0';
		for (unsigned j = 0; j < nb; j++)
			b[j] = (char)(u32() % 256);
		b[nb] = '\0';
		check_samedomain(a, b, F_SAMEDOM);
		check_subdomain(a, b, F_SUBDOM);
		check_samename(a, b, F_SAMENAME);
		check_makecanon(a, (size_t)(u32() % NS_MAXDNAME), F_MAKECANON);
	}
}

void
rand_parse()
{
	unsigned char pkt[512];
	unsigned char rdata[64];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		unsigned id = u32() & 0xffff;
		unsigned flags = u32() & 0xffff;
		unsigned qd = u32() % 3;
		unsigned an = u32() % 3;
		unsigned ttl = u32();
		unsigned rdlen = u32() % sizeof(rdata);
		size_t len;
		int msglen;

		for (unsigned j = 0; j < rdlen; j++)
			rdata[j] = (unsigned char)u32();
		len = build_msg(pkt, sizeof(pkt), id, flags, qd, an, 0, 0,
		    "h.example.org", (unsigned)(1 + (u32() % 50)),
		    (unsigned)(1 + (u32() % 4)), ttl, rdata, rdlen);
		msglen = (int)(u32() % (len + 8));
		check_initparse(pkt, msglen, F_INITPARSE);
		if (len > 12) {
			check_skiprr(pkt, len, (ns_sect)(u32() % ns_s_max),
			    (int)(u32() % 5), F_SKIPRR);
			check_msg_getflag(flags, (int)(u32() % 10), F_GETFLAG);
			if (qd > 0) {
				check_parserr(pkt, (int)len, ns_s_qd,
				    (int)(u32() % 4) - 1, F_PARSERR);
				check_parserr2(pkt, (int)len, ns_s_qd,
				    (int)(u32() % 4) - 1, F_PARSERR2);
			}
			if (an > 0) {
				check_parserr(pkt, (int)len, ns_s_an, 0,
				    F_PARSERR);
				check_parserr2(pkt, (int)len, ns_s_an, 0,
				    F_PARSERR2);
			}
		}
	}
}

} /* namespace */

int
main()
{
	edge_netint();
	edge_ttl();
	edge_domain();
	edge_parse();
	rand_netint();
	rand_ttl();
	rand_domain();
	rand_parse();

	unsigned long long total_cases = 0;
	unsigned long long total_fail = 0;

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "--------", "-----", "--------");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-16s %12llu %12llu\n", tbl[i].name, tbl[i].cases,
		    tbl[i].failures);
		total_cases += tbl[i].cases;
		total_fail += tbl[i].failures;
	}
	std::printf("%-16s %12llu %12llu\n", "TOTAL", total_cases,
	    total_fail);

	return total_fail == 0 ? 0 : 1;
}
