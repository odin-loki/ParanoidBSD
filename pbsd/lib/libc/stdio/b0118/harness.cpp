/*
 * harness.cpp -- differential test for PBSD batch b0118.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fcntl.h>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0118;

namespace P = pbsd::lib_libc_stdio::b0118;

extern "C" {

typedef struct b0118_stream {
	const unsigned char *data;
	size_t len;
	size_t pos;
	int err_on_refill;
} b0118_stream;

typedef struct b0118_write_ctx {
	unsigned char *out;
	size_t out_cap;
	size_t out_len;
	size_t max_write;
	int fail;
} b0118_write_ctx;

typedef struct {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct {
		unsigned char *_base;
		int _size;
	} _bf;
	int _lbfsize;
	void *_cookie;
	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	long (*_seek)(void *, long, int);
	int (*_write)(void *, const char *, int);
	struct {
		unsigned char *_base;
		int _size;
	} _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct {
		unsigned char *_base;
		int _size;
	} _lb;
	int _blksize;
	long _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	mbstate_t _mbstate;
	int _flags2;
} ref_FILE;

char		*ref_fgets(char * __restrict, int, ref_FILE * __restrict);
size_t		ref_fwrite(const void * __restrict, size_t, size_t, ref_FILE * __restrict);
size_t		ref_fwrite_unlocked(const void * __restrict, size_t, size_t,
		    ref_FILE * __restrict);
wchar_t		*ref_fgetws(wchar_t * __restrict, int, ref_FILE * __restrict);
wchar_t		*ref_fgetws_l(wchar_t * __restrict, int, ref_FILE * __restrict,
		    void *);
ref_FILE	*ref_fdopen(int, const char *);
void		b0118_set_sfp_target(ref_FILE *);
void		b0118_oracle_init(void);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr long SWEEP = 50000;
constexpr int MAX_REPORT = 8;

#define	__SRD	0x0004
#define	__SWR	0x0008
#define	__SEOF	0x0020
#define	__SERR	0x0040
#define	__SAPP	0x0100
#define	__S2OAP	0x0001

enum StatId {
	S_FGETS,
	S_FWRITE,
	S_FWRITE_UNLOCKED,
	S_FGETWS,
	S_FGETWS_L,
	S_FDOPEN,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "fgets", 0, 0, 0 },
	{ "fwrite", 0, 0, 0 },
	{ "fwrite_unlocked", 0, 0, 0 },
	{ "fgetws", 0, 0, 0 },
	{ "fgetws_l", 0, 0, 0 },
	{ "fdopen", 0, 0, 0 },
};

std::uint64_t rng = 0xb0118feedfaceULL;

std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

std::size_t
rnd_mod(std::size_t m)
{
	return m == 0 ? 0 : (std::size_t)(rnd() % (std::uint64_t)m);
}

void
fail_msg(StatId which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].printed < MAX_REPORT) {
		g_stat[which].printed++;
		std::printf("  FAIL %-16s %-20s %s\n", g_stat[which].name, label,
		    detail);
	}
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

static_assert(sizeof(ref_FILE) == sizeof(P::FILE));

struct ReadCoreR {
	unsigned char pre[32];
	ref_FILE fp;
	unsigned char iobuf[512];
	b0118_stream stream;
	unsigned char post[32];
};

struct ReadCoreP {
	unsigned char pre[32];
	P::FILE fp;
	unsigned char iobuf[512];
	b0118_stream stream;
	unsigned char post[32];
};

struct WriteCoreR {
	unsigned char pre[32];
	ref_FILE fp;
	b0118_write_ctx wctx;
	unsigned char outbuf[512];
	unsigned char post[32];
};

struct WriteCoreP {
	unsigned char pre[32];
	P::FILE fp;
	b0118_write_ctx wctx;
	unsigned char outbuf[512];
	unsigned char post[32];
};

struct FdCoreR {
	unsigned char pre[64];
	ref_FILE fp;
	unsigned char post[64];
};

struct FdCoreP {
	unsigned char pre[64];
	P::FILE fp;
	unsigned char post[64];
};

struct CharBuf {
	unsigned char pre[16];
	char user[256];
	unsigned char post[16];
};

struct WCharBuf {
	unsigned char pre[16];
	wchar_t user[128];
	unsigned char post[16];
};

void
fill_guard(void *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

void
setup_read(ReadCoreR &r, ReadCoreP &p, const unsigned char *data, std::size_t len,
    std::size_t preload, int err_on_refill)
{
	fill_guard(&r, sizeof(r));
	fill_guard(&p, sizeof(p));

	r.stream.data = data;
	r.stream.len = len;
	r.stream.pos = preload;
	r.stream.err_on_refill = err_on_refill;
	p.stream = r.stream;

	r.fp._cookie = &r.stream;
	p.fp._cookie = &p.stream;
	r.fp._flags = __SRD;
	p.fp._flags = __SRD;
	r.fp._orientation = 0;
	p.fp._orientation = 0;
	std::memset(&r.fp._mbstate, 0, sizeof(mbstate_t));
	std::memset(&p.fp._mbstate, 0, sizeof(mbstate_t));

	if (preload > 0 && preload <= len) {
		std::memcpy(r.iobuf, data, preload);
		std::memcpy(p.iobuf, data, preload);
		r.fp._p = r.iobuf;
		p.fp._p = p.iobuf;
		r.fp._r = (int)preload;
		p.fp._r = (int)preload;
	} else {
		r.fp._r = 0;
		p.fp._r = 0;
	}
}

void
setup_write(WriteCoreR &r, WriteCoreP &p, std::size_t max_write, int fail)
{
	fill_guard(&r, sizeof(r));
	fill_guard(&p, sizeof(p));

	r.wctx.out = r.outbuf;
	r.wctx.out_cap = sizeof(r.outbuf);
	r.wctx.out_len = 0;
	r.wctx.max_write = max_write;
	r.wctx.fail = fail;
	p.wctx = r.wctx;
	p.wctx.out = p.outbuf;

	r.fp._cookie = &r.wctx;
	p.fp._cookie = &p.wctx;
	r.fp._flags = __SWR;
	p.fp._flags = __SWR;
}

bool
test_fgets_one(StatId which, const char *label, const unsigned char *data,
    std::size_t len, std::size_t preload, int n, int err_on_refill)
{
	ReadCoreR rc;
	ReadCoreP pc;
	CharBuf rb, pb;
	char *rr, *rp;
	long roff, poff;
	bool ok = true;

	setup_read(rc, pc, data, len, preload, err_on_refill);
	fill_guard(&rb, sizeof(rb));
	fill_guard(&pb, sizeof(pb));

	rr = ref_fgets(rb.user, n, &rc.fp);
	rp = P::fgets(pb.user, n, &pc.fp);

	case_inc(which);

	if ((rr == nullptr) != (rp == nullptr))
		ok = false;
	if (rr != nullptr && rp != nullptr) {
		roff = rr - rb.user;
		poff = rp - pb.user;
		if (roff != poff)
			ok = false;
	}

	if (std::memcmp(&rb, &pb, sizeof(rb)) != 0)
		ok = false;
	if (std::memcmp(&rc, &pc, sizeof(rc)) != 0)
		ok = false;

	if (!ok)
		fail_msg(which, label, "mismatch");
	return ok;
}

bool
test_fwrite_one(StatId which, const char *label, const unsigned char *src,
    std::size_t size, std::size_t count, std::size_t max_write, int fail)
{
	WriteCoreR rc;
	WriteCoreP pc;
	unsigned char srcbuf[256];
	std::size_t n = size * count;
	size_t rr, rp;
	bool ok = true;

	if (n > sizeof(srcbuf))
		n = sizeof(srcbuf);
	std::memcpy(srcbuf, src, n);

	setup_write(rc, pc, max_write, fail);

	if (which == S_FWRITE_UNLOCKED) {
		rr = ref_fwrite_unlocked(srcbuf, size, count, &rc.fp);
		rp = P::fwrite_unlocked(srcbuf, size, count, &pc.fp);
	} else {
		rr = ref_fwrite(srcbuf, size, count, &rc.fp);
		rp = P::fwrite(srcbuf, size, count, &pc.fp);
	}

	case_inc(which);

	if (rr != rp)
		ok = false;
	if (rc.wctx.out_len != pc.wctx.out_len)
		ok = false;
	if (std::memcmp(rc.outbuf, pc.outbuf, sizeof(rc.outbuf)) != 0)
		ok = false;
	if (std::memcmp(&rc, &pc, sizeof(rc)) != 0)
		ok = false;

	if (!ok)
		fail_msg(which, label, "mismatch");
	return ok;
}

bool
test_fgetws_one(StatId which, const char *label, const unsigned char *data,
    std::size_t len, std::size_t preload, int n, int err_on_refill)
{
	ReadCoreR rc;
	ReadCoreP pc;
	WCharBuf rb, pb;
	wchar_t *rr, *rp;
	long roff, poff;
	bool ok = true;

	setup_read(rc, pc, data, len, preload, err_on_refill);
	fill_guard(&rb, sizeof(rb));
	fill_guard(&pb, sizeof(pb));

	if (which == S_FGETWS_L) {
		rr = ref_fgetws_l(rb.user, n, &rc.fp, nullptr);
		rp = P::fgetws_l(pb.user, n, &pc.fp, nullptr);
	} else {
		rr = ref_fgetws(rb.user, n, &rc.fp);
		rp = P::fgetws(pb.user, n, &pc.fp);
	}

	case_inc(which);

	if ((rr == nullptr) != (rp == nullptr))
		ok = false;
	if (rr != nullptr && rp != nullptr) {
		roff = rr - rb.user;
		poff = rp - pb.user;
		if (roff != poff)
			ok = false;
	}

	if (std::memcmp(&rb, &pb, sizeof(rb)) != 0)
		ok = false;
	if (std::memcmp(&rc, &pc, sizeof(rc)) != 0)
		ok = false;

	if (!ok)
		fail_msg(which, label, "mismatch");
	return ok;
}

bool
test_fdopen_one(const char *label, int fd, const char *mode, bool set_target)
{
	FdCoreR rc;
	FdCoreP pc;
	ref_FILE *rr;
	P::FILE *rp;
	int re, pe;
	bool ok = true;

	fill_guard(&rc, sizeof(rc));
	fill_guard(&pc, sizeof(pc));
	rc.fp._flags = 0;
	pc.fp._flags = 0;

	if (set_target) {
		b0118_set_sfp_target(&rc.fp);
		rr = ref_fdopen(fd, mode);
		b0118_set_sfp_target(nullptr);

		b0118_set_sfp_target(reinterpret_cast<ref_FILE *>(&pc.fp));
		rp = P::fdopen(fd, mode);
		b0118_set_sfp_target(nullptr);
	} else {
		rr = ref_fdopen(fd, mode);
		rp = P::fdopen(fd, mode);
	}

	re = errno;
	pe = errno;
	case_inc(S_FDOPEN);

	if ((rr == nullptr) != (rp == nullptr))
		ok = false;
	if (re != pe)
		ok = false;

	if (rr != nullptr && rp != nullptr) {
		if (rr->_flags != rp->_flags)
			ok = false;
		if (rr->_flags2 != rp->_flags2)
			ok = false;
		if (rr->_file != rp->_file)
			ok = false;
	}

	if (std::memcmp(&rc, &pc, sizeof(rc)) != 0)
		ok = false;

	if (!ok)
		fail_msg(S_FDOPEN, label, "mismatch");
	return ok;
}

int
make_read_fd(const unsigned char *data, std::size_t len)
{
	int pfd[2];

	if (pipe(pfd) != 0)
		return -1;
	if (len > 0) {
		std::size_t off = 0;
		while (off < len) {
			ssize_t w = write(pfd[1], data + off, len - off);
			if (w <= 0) {
				close(pfd[0]);
				close(pfd[1]);
				return -1;
			}
			off += (std::size_t)w;
		}
	}
	close(pfd[1]);
	return pfd[0];
}

int
make_write_fd(void)
{
	int pfd[2];

	if (pipe(pfd) != 0)
		return -1;
	close(pfd[0]);
	return pfd[1];
}

void
run_fgets_edges(void)
{
	static const unsigned char empty[] = { '\n' };
	static const unsigned char one[] = { 'a', '\n' };
	static const unsigned char hi[] = { 0x80, 0xff, '\n' };
	static const unsigned char nul[] = { 'a', '\0', 'b', '\n' };
	static const unsigned char longln[] = {
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', '\n'
	};
	static const unsigned char no_nl[] = { 'x', 'y', 'z' };
	unsigned char chunk[200];

	for (int i = 0; i < 200; i++)
		chunk[i] = (unsigned char)('A' + (i % 26));

	test_fgets_one(S_FGETS, "n zero", one, 1, 0, 0, 0);
	test_fgets_one(S_FGETS, "n neg", one, 1, 0, -1, 0);
	test_fgets_one(S_FGETS, "n one", one, 2, 0, 1, 0);
	test_fgets_one(S_FGETS, "n two empty", empty, 1, 0, 2, 0);
	test_fgets_one(S_FGETS, "one char", one, 2, 0, 2, 0);
	test_fgets_one(S_FGETS, "one preload", one, 2, 2, 8, 0);
	test_fgets_one(S_FGETS, "highbit", hi, 3, 0, 16, 0);
	test_fgets_one(S_FGETS, "nul byte", nul, 4, 0, 16, 0);
	test_fgets_one(S_FGETS, "long line", longln, sizeof(longln), 0, 5, 0);
	test_fgets_one(S_FGETS, "no newline eof", no_nl, 3, 0, 16, 0);
	test_fgets_one(S_FGETS, "partial then eof", no_nl, 3, 1, 16, 0);
	test_fgets_one(S_FGETS, "empty stream", empty, 0, 0, 8, 0);
	test_fgets_one(S_FGETS, "refill err", one, 2, 0, 8, 1);
	test_fgets_one(S_FGETS, "big chunk", chunk, sizeof(chunk), 0, 50, 0);
	test_fgets_one(S_FGETS, "tiny n", chunk, 20, 0, 3, 0);
}

void
run_fgets_random(void)
{
	unsigned char data[128];

	for (long i = 0; i < SWEEP; i++) {
		std::size_t len = rnd_mod(120) + 1;
		std::size_t preload = rnd_mod(len + 1);
		int n = (int)(rnd_mod(64) + 1);
		int err = (int)(rnd() % 997u == 0);

		for (std::size_t j = 0; j < len; j++) {
			unsigned v = (unsigned)rnd();
			if ((v & 7u) == 0)
				data[j] = (unsigned char)(0x80 + (v & 0x7fu));
			else if ((v & 7u) == 1)
				data[j] = '\n';
			else if ((v & 7u) == 2)
				data[j] = '\0';
			else
				data[j] = (unsigned char)('a' + (v % 26u));
		}

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_fgets_one(S_FGETS, label, data, len, preload, n, err);
	}
}

void
run_fwrite_edges(StatId which)
{
	static const unsigned char b1[] = { 'x' };
	static const unsigned char b3[] = { 0x80, 0xff, 0x7f };
	unsigned char big[300];

	for (int i = 0; i < 300; i++)
		big[i] = (unsigned char)(rnd() & 0xff);

	test_fwrite_one(which, "size zero", b1, 0, 5, SIZE_MAX, 0);
	test_fwrite_one(which, "count zero", b1, 4, 0, SIZE_MAX, 0);
	test_fwrite_one(which, "one byte", b1, 1, 1, SIZE_MAX, 0);
	test_fwrite_one(which, "three high", b3, 1, 3, SIZE_MAX, 0);
	test_fwrite_one(which, "elem 4 x 3", b3, 4, 3, SIZE_MAX, 0);
	test_fwrite_one(which, "partial write", big, 1, 100, 17, 0);
	test_fwrite_one(which, "write fail", b1, 1, 1, SIZE_MAX, 1);
	test_fwrite_one(which, "overflow", b1, (size_t)0x10000, (size_t)0x10000,
	    SIZE_MAX, 0);
	test_fwrite_one(which, "big ok", big, 7, 11, SIZE_MAX, 0);
}

void
run_fwrite_random(StatId which)
{
	unsigned char src[200];

	for (long i = 0; i < SWEEP; i++) {
		std::size_t size = rnd_mod(16) + 1;
		std::size_t count = rnd_mod(32) + 1;
		std::size_t maxw = rnd_mod(200);
		int fail = (int)(rnd() % 500u == 0);

		for (std::size_t j = 0; j < sizeof(src); j++)
			src[j] = (unsigned char)(rnd() & 0xff);

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_fwrite_one(which, label, src, size, count, maxw, fail);
	}
}

void
run_fgetws_edges(StatId which)
{
	static const unsigned char asc[] = { 'h', 'i', '\n' };
	static const unsigned char nulw[] = { 'a', '\0', 'b', '\n' };
	static const unsigned char hi[] = { 0xc3, 0xa9, '\n' };
	static const unsigned char bad[] = { 0xff, '\n' };

	test_fgetws_one(which, "n zero", asc, 3, 0, 0, 0);
	test_fgetws_one(which, "n neg", asc, 3, 0, -2, 0);
	test_fgetws_one(which, "n one", asc, 3, 0, 1, 0);
	test_fgetws_one(which, "ascii", asc, 3, 0, 16, 0);
	test_fgetws_one(which, "nul multibyte", nulw, 4, 0, 16, 0);
	test_fgetws_one(which, "utf8", hi, 3, 0, 16, 0);
	test_fgetws_one(which, "preload", asc, 3, 2, 16, 0);
	test_fgetws_one(which, "eof empty", asc, 0, 0, 8, 0);
	test_fgetws_one(which, "refill err", asc, 3, 0, 8, 1);
	test_fgetws_one(which, "bad seq", bad, 2, 0, 8, 0);
}

void
run_fgetws_random(StatId which)
{
	unsigned char data[128];

	for (long i = 0; i < SWEEP; i++) {
		std::size_t len = rnd_mod(100) + 1;
		std::size_t preload = rnd_mod(len + 1);
		int n = (int)(rnd_mod(40) + 1);
		int err = (int)(rnd() % 997u == 0);

		for (std::size_t j = 0; j < len; j++) {
			unsigned v = (unsigned)rnd();
			if ((v & 7u) == 0)
				data[j] = (unsigned char)(0x80 + (v & 0x7fu));
			else if ((v & 7u) == 1)
				data[j] = '\n';
			else if ((v & 7u) == 2)
				data[j] = '\0';
			else
				data[j] = (unsigned char)('a' + (v % 26u));
		}

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_fgetws_one(which, label, data, len, preload, n, err);
	}
}

void
run_fdopen_edges(void)
{
	static const unsigned char msg[] = { 't', 'e', 's', 't' };
	int rfd, wfd;

	rfd = make_read_fd(msg, sizeof(msg));
	if (rfd >= 0) {
		test_fdopen_one("read ok", rfd, "r", true);
		close(rfd);
	}

	wfd = make_write_fd();
	if (wfd >= 0) {
		test_fdopen_one("write ok", wfd, "w", true);
		close(wfd);
	}

	rfd = make_read_fd(msg, sizeof(msg));
	if (rfd >= 0) {
		test_fdopen_one("bad mode w", rfd, "w", true);
		close(rfd);
	}

	test_fdopen_one("fd huge", SHRT_MAX + 1, "r", false);
	test_fdopen_one("bad mode", 0, "q", false);
	test_fdopen_one("null mode", 0, "", false);

	rfd = make_read_fd(msg, sizeof(msg));
	if (rfd >= 0) {
		int clo = fcntl(rfd, F_GETFD);
		if (clo >= 0)
			fcntl(rfd, F_SETFD, clo & ~FD_CLOEXEC);
		test_fdopen_one("cloexec e", rfd, "re", true);
		close(rfd);
	}

	wfd = make_write_fd();
	if (wfd >= 0) {
		test_fdopen_one("append a", wfd, "a", true);
		close(wfd);
	}

	wfd = make_write_fd();
	if (wfd >= 0) {
		fcntl(wfd, F_SETFL, O_APPEND);
		test_fdopen_one("fd append", wfd, "w", true);
		close(wfd);
	}
}

void
run_fdopen_random(void)
{
	static const char *modes[] = { "r", "w", "a", "r+", "w+", "a+", "re",
	    "ae" };
	unsigned char msg[8];

	for (long i = 0; i < SWEEP; i++) {
		std::size_t n = rnd_mod(8) + 1;
		int fd;
		const char *mode = modes[rnd_mod(sizeof(modes) / sizeof(modes[0]))];

		for (std::size_t j = 0; j < n; j++)
			msg[j] = (unsigned char)(rnd() & 0xff);

		if (mode[0] == 'r' || (mode[0] == 'a' && mode[1] != '+'))
			fd = make_read_fd(msg, n);
		else
			fd = make_write_fd();

		if (fd < 0)
			continue;

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_fdopen_one(label, fd, mode, true);
		close(fd);
	}
}

void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("------------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-16s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("------------------------------------------------\n");
	std::printf("%-16s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

} /* namespace */

int
main(void)
{
	b0118_oracle_init();

	run_fgets_edges();
	run_fgets_random();
	run_fwrite_edges(S_FWRITE_UNLOCKED);
	run_fwrite_random(S_FWRITE_UNLOCKED);
	run_fwrite_edges(S_FWRITE);
	run_fwrite_random(S_FWRITE);
	run_fgetws_edges(S_FGETWS);
	run_fgetws_random(S_FGETWS);
	run_fgetws_edges(S_FGETWS_L);
	run_fgetws_random(S_FGETWS_L);
	run_fdopen_edges();
	run_fdopen_random();

	report();

	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
