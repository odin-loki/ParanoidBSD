/*
 * harness.cpp -- differential test for PBSD batch b0139.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

import pbsd.lib.libc.stdio.b0139;

namespace port = pbsd::lib_libc_stdio::b0139;

extern "C" {
typedef struct {
	unsigned char	*_p;
	int		_r;
	int		_w;
	short		_flags;
	short		_file;
} ref_FILE;

int ref_putc(int, FILE *);
int ref_putc_unlocked(int, FILE *);
char *ref_tmpnam(char *);
int ref_getchar(void);
int ref_getchar_unlocked(void);
int ref_fileno(ref_FILE *);
int ref_fileno_unlocked(ref_FILE *);
extern int __isthreaded;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t PRE = 16;
constexpr std::size_t USER = 512;
constexpr std::size_t POST = 16;
constexpr std::size_t ARENA = PRE + USER + POST;
constexpr int MAXPRINT = 8;
constexpr long RAND_ITERS = 200000;

enum StatId {
	S_PUTC,
	S_PUTC_UNLOCKED,
	S_TMPNAM,
	S_GETCHAR,
	S_GETCHAR_UNLOCKED,
	S_FILENO,
	S_FILENO_UNLOCKED,
	NSTAT
};

struct Stats {
	const char *name;
	long cases;
	long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "putc",             0, 0, 0 },
	{ "putc_unlocked",    0, 0, 0 },
	{ "tmpnam",           0, 0, 0 },
	{ "getchar",          0, 0, 0 },
	{ "getchar_unlocked", 0, 0, 0 },
	{ "fileno",           0, 0, 0 },
	{ "fileno_unlocked",  0, 0, 0 },
};

std::uint64_t rng_state = 0xb0139facefeedULL;

std::uint64_t
rnd_u64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned
rnd_u32(void)
{
	return (unsigned)(rnd_u64() & 0xffffffffu);
}

std::size_t
rnd_mod(std::size_t m)
{
	return m == 0 ? 0 : (std::size_t)(rnd_u64() % (std::uint64_t)m);
}

void
fail_msg(StatId id, const char *tag, const char *detail)
{
	g_stat[id].fails++;
	if (g_stat[id].printed++ < MAXPRINT)
		std::printf("  FAIL %-20s %-20s %s\n", g_stat[id].name, tag,
		    detail);
}

void
case_inc(StatId id)
{
	g_stat[id].cases++;
}

struct GuardArena {
	unsigned char bytes[ARENA];

	void fill(void)
	{
		std::memset(bytes, GUARD, ARENA);
	}

	bool eq(const GuardArena &o) const
	{
		return std::memcmp(bytes, o.bytes, ARENA) == 0;
	}
};

FILE *
temp_file_rw(const unsigned char *data, std::size_t len, char *path_out)
{
	char path[] = "/tmp/pbsd_b0139_XXXXXX";
	int fd;
	FILE *fp;

	fd = mkstemp(path);
	if (fd < 0)
		return nullptr;
	if (len > 0 && write(fd, data, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return nullptr;
	}
	close(fd);
	fp = fopen(path, "r+b");
	if (fp == nullptr) {
		unlink(path);
		return nullptr;
	}
	std::strcpy(path_out, path);
	return fp;
}

void
rewind_stream(FILE *fp)
{
	std::rewind(fp);
	clearerr(fp);
}

struct PutcObs {
	int ret;
	long pos;
	unsigned char filebuf[ARENA];
};

template <typename Fn>
PutcObs
run_putc(Fn fn, FILE *fp, int c)
{
	PutcObs obs{};

	std::memset(obs.filebuf, GUARD, sizeof(obs.filebuf));
	rewind_stream(fp);
	obs.ret = fn(c, fp);
	fflush(fp);
	obs.pos = ftell(fp);
	rewind_stream(fp);
	(void)fread(obs.filebuf + PRE, 1, USER, fp);
	return obs;
}

static void
putc_case(StatId id, int use_unlocked, int c, const char *tag)
{
	char pa[64], pb[64];
	FILE *fa, *fb;
	PutcObs a, b;
	int bad;

	fa = temp_file_rw(nullptr, 0, pa);
	fb = temp_file_rw(nullptr, 0, pb);
	if (fa == nullptr || fb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file putc\n");
		std::exit(2);
	}

	if (use_unlocked) {
		a = run_putc(ref_putc_unlocked, fa, c);
		b = run_putc(port::putc_unlocked, fb, c);
	} else {
		a = run_putc(ref_putc, fa, c);
		b = run_putc(port::putc, fb, c);
	}

	case_inc(id);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.filebuf, b.filebuf, sizeof(a.filebuf)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "c=%d ret=%d/%d pos=%ld/%ld", c, a.ret, b.ret, a.pos,
		    b.pos);
		fail_msg(id, tag, detail);
	}

	fclose(fa);
	fclose(fb);
	unlink(pa);
	unlink(pb);
}

static void
putc_edges(void)
{
	putc_case(S_PUTC, 0, 'a', "a");
	putc_case(S_PUTC, 0, 0, "nul");
	putc_case(S_PUTC, 0, 0x7f, "0x7f");
	putc_case(S_PUTC, 0, 0x80, "0x80");
	putc_case(S_PUTC, 0, 0xff, "0xff");
	putc_case(S_PUTC, 0, -1, "neg1");
	putc_case(S_PUTC, 0, EOF, "eof");

	putc_case(S_PUTC_UNLOCKED, 1, 'a', "a");
	putc_case(S_PUTC_UNLOCKED, 1, 0, "nul");
	putc_case(S_PUTC_UNLOCKED, 1, 0x7f, "0x7f");
	putc_case(S_PUTC_UNLOCKED, 1, 0x80, "0x80");
	putc_case(S_PUTC_UNLOCKED, 1, 0xff, "0xff");
	putc_case(S_PUTC_UNLOCKED, 1, -1, "neg1");
	putc_case(S_PUTC_UNLOCKED, 1, EOF, "eof");
}

static void
putc_random(StatId id, int use_unlocked, long n)
{
	for (long t = 0; t < n; t++) {
		int c = (int)(rnd_u32() & 0xff);
		if (rnd_u32() & 1)
			c = (int)((signed char)c);
		putc_case(id, use_unlocked, c, "random");
	}
}

struct TmpnamObs {
	char *ret;
	long off;
	unsigned char arena[PRE + L_tmpnam + POST];
};

static void
tmpnam_case(int use_null_buf, unsigned char seed_byte, const char *tag)
{
	GuardArena ga, gb;
	char *ra, *rb;
	long off_a, off_b;
	int bad;
	std::size_t i;

	ga.fill();
	gb.fill();
	for (i = PRE; i < PRE + L_tmpnam; i++) {
		ga.bytes[i] = seed_byte;
		gb.bytes[i] = seed_byte;
	}

	if (use_null_buf) {
		ra = ref_tmpnam(nullptr);
		rb = port::tmpnam(nullptr);
		off_a = 0;
		off_b = 0;
	} else {
		ra = ref_tmpnam((char *)(ga.bytes + PRE));
		rb = port::tmpnam((char *)(gb.bytes + PRE));
		off_a = ra - (char *)(ga.bytes + PRE);
		off_b = rb - (char *)(gb.bytes + PRE);
	}

	case_inc(S_TMPNAM);
	bad = 0;
	if (off_a != off_b)
		bad = 1;
	if (ra == nullptr || rb == nullptr) {
		if (ra != rb)
			bad = 1;
	} else if (std::strcmp(ra, rb) != 0) {
		bad = 1;
	}
	if (!ga.eq(gb))
		bad = 1;
	if (bad) {
		char detail[192];
		std::snprintf(detail, sizeof(detail),
		    "null=%d off=%ld/%ld str=%s/%s", use_null_buf, off_a,
		    off_b, ra ? ra : "(null)", rb ? rb : "(null)");
		fail_msg(S_TMPNAM, tag, detail);
	}
}

static void
tmpnam_edges(void)
{
	tmpnam_case(1, 0x00, "null-buf");
	tmpnam_case(0, 0x00, "user-nul");
	tmpnam_case(0, 0x7f, "user-7f");
	tmpnam_case(0, (unsigned char)0x80, "user-80");
	tmpnam_case(0, (unsigned char)0xff, "user-ff");
	tmpnam_case(1, 0x7f, "null-7f");
}

static void
tmpnam_random(long n)
{
	for (long t = 0; t < n; t++) {
		int use_null = (int)(rnd_u32() & 1);
		unsigned char seed = (unsigned char)(rnd_u32() & 0xff);
		tmpnam_case(use_null, seed, "random");
	}
}

static int saved_stdin = -1;

static bool
redirect_stdin_from(const unsigned char *data, std::size_t len)
{
	char path[] = "/tmp/pbsd_b0139_in_XXXXXX";
	int fd;

	fd = mkstemp(path);
	if (fd < 0)
		return false;
	if (len > 0 && write(fd, data, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return false;
	}
	close(fd);
	if (freopen(path, "rb", stdin) == nullptr) {
		unlink(path);
		return false;
	}
	unlink(path);
	clearerr(stdin);
	return true;
}

static void
save_stdin(void)
{
	if (saved_stdin < 0)
		saved_stdin = dup(STDIN_FILENO);
}

static void
restore_stdin(void)
{
	if (saved_stdin >= 0) {
		dup2(saved_stdin, STDIN_FILENO);
		clearerr(stdin);
	}
}

struct GetcharObs {
	int ret;
	long pos;
	unsigned char tail[64];
};

template <typename Fn>
GetcharObs
run_getchar(Fn fn, const unsigned char *data, std::size_t len, int pre_reads)
{
	GetcharObs obs{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	if (!redirect_stdin_from(data, len)) {
		obs.ret = -9999;
		return obs;
	}
	for (int i = 0; i < pre_reads; i++)
		(void)fn();
	obs.ret = fn();
	obs.pos = ftell(stdin);
	if (obs.pos >= 0)
		(void)fread(obs.tail, 1, sizeof(obs.tail), stdin);
	restore_stdin();
	return obs;
}

static void
getchar_case(StatId id, int use_unlocked, const unsigned char *data,
    std::size_t len, int pre_reads, const char *tag)
{
	GetcharObs a, b;
	int bad;

	save_stdin();
	if (use_unlocked) {
		a = run_getchar(ref_getchar_unlocked, data, len, pre_reads);
		b = run_getchar(port::getchar_unlocked, data, len, pre_reads);
	} else {
		a = run_getchar(ref_getchar, data, len, pre_reads);
		b = run_getchar(port::getchar, data, len, pre_reads);
	}

	case_inc(id);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu pre=%d ret=%d/%d pos=%ld/%ld",
		    len, pre_reads, a.ret, b.ret, a.pos, b.pos);
		fail_msg(id, tag, detail);
	}
}

static void
getchar_edges(void)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one_a[] = { 'a' };
	static const unsigned char one_nul[] = { 0x00 };
	static const unsigned char one_7f[] = { 0x7f };
	static const unsigned char one_80[] = { (unsigned char)0x80 };
	static const unsigned char one_ff[] = { (unsigned char)0xff };
	static const unsigned char hi[] = {
		0x00, 0x7f, (unsigned char)0x80, (unsigned char)0xff, 'Z'
	};
	unsigned char buf[USER];
	std::size_t i;

	for (i = 0; i < sizeof(buf); i++)
		buf[i] = (unsigned char)(0x80 + (i & 0x7f));

	getchar_case(S_GETCHAR, 0, empty, 0, 0, "empty@0");
	getchar_case(S_GETCHAR, 0, one_a, 1, 0, "a@0");
	getchar_case(S_GETCHAR, 0, one_a, 1, 1, "a@eof");
	getchar_case(S_GETCHAR, 0, one_nul, 1, 0, "nul@0");
	getchar_case(S_GETCHAR, 0, one_7f, 1, 0, "0x7f@0");
	getchar_case(S_GETCHAR, 0, one_80, 1, 0, "0x80@0");
	getchar_case(S_GETCHAR, 0, one_ff, 1, 0, "0xff@0");
	getchar_case(S_GETCHAR, 0, hi, sizeof(hi), 0, "hi@0");
	getchar_case(S_GETCHAR, 0, hi, sizeof(hi), 2, "hi@2");
	getchar_case(S_GETCHAR, 0, hi, sizeof(hi), 4, "hi@eof");
	getchar_case(S_GETCHAR, 0, buf, sizeof(buf), 0, "big@0");
	getchar_case(S_GETCHAR, 0, buf, sizeof(buf), 127, "big@127");
	getchar_case(S_GETCHAR, 0, buf, sizeof(buf), (int)sizeof(buf) - 1,
	    "big@last");

	getchar_case(S_GETCHAR_UNLOCKED, 1, empty, 0, 0, "empty@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, one_a, 1, 0, "a@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, one_a, 1, 1, "a@eof");
	getchar_case(S_GETCHAR_UNLOCKED, 1, one_nul, 1, 0, "nul@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, one_7f, 1, 0, "0x7f@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, one_80, 1, 0, "0x80@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, one_ff, 1, 0, "0xff@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, hi, sizeof(hi), 0, "hi@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, hi, sizeof(hi), 2, "hi@2");
	getchar_case(S_GETCHAR_UNLOCKED, 1, hi, sizeof(hi), 4, "hi@eof");
	getchar_case(S_GETCHAR_UNLOCKED, 1, buf, sizeof(buf), 0, "big@0");
	getchar_case(S_GETCHAR_UNLOCKED, 1, buf, sizeof(buf), 127, "big@127");
	getchar_case(S_GETCHAR_UNLOCKED, 1, buf, sizeof(buf),
	    (int)sizeof(buf) - 1, "big@last");
}

static void
getchar_random(StatId id, int use_unlocked, long n)
{
	unsigned char data[256];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data) + 1);
		int pre = (int)rnd_mod(len + 2);

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd_u32();
		getchar_case(id, use_unlocked, data, len, pre, "random");
	}
}

struct FileBlob {
	unsigned char	pre[32];
	ref_FILE	f;
	unsigned char	post[32];
};

struct PortBlob {
	unsigned char	pre[32];
	port::FILE	f;
	unsigned char	post[32];
};

static_assert(sizeof(ref_FILE) == sizeof(port::FILE));
static_assert(offsetof(ref_FILE, _file) == offsetof(port::FILE, _file));

static void
fill_ref_blob(FileBlob *b)
{
	std::memset(b, GUARD, sizeof(*b));
}

static void
fill_port_blob(PortBlob *b)
{
	std::memset(b, GUARD, sizeof(*b));
}

static bool
blob_equal(const FileBlob *refb, const PortBlob *portb)
{
	if (std::memcmp(refb->pre, portb->pre, sizeof(refb->pre)) != 0)
		return false;
	if (std::memcmp(&refb->f._p, &portb->f._p, sizeof(refb->f._p)) != 0)
		return false;
	if (refb->f._r != portb->f._r)
		return false;
	if (refb->f._w != portb->f._w)
		return false;
	if (refb->f._flags != portb->f._flags)
		return false;
	if (refb->f._file != portb->f._file)
		return false;
	return std::memcmp(refb->post, portb->post, sizeof(refb->post)) == 0;
}

static void
fileno_case(StatId id, int use_unlocked, short file_val, int threaded,
    const char *tag)
{
	FileBlob rb;
	PortBlob pb;
	int ret_r, ret_p, err_r, err_p;
	int bad;

	fill_ref_blob(&rb);
	fill_port_blob(&pb);
	rb.f._file = file_val;
	pb.f._file = file_val;

	__isthreaded = threaded;
	errno = 0;
	if (use_unlocked)
		ret_r = ref_fileno_unlocked(&rb.f);
	else
		ret_r = ref_fileno(&rb.f);
	err_r = errno;

	__isthreaded = threaded;
	errno = 0;
	if (use_unlocked)
		ret_p = port::fileno_unlocked(&pb.f);
	else
		ret_p = port::fileno(&pb.f);
	err_p = errno;

	case_inc(id);
	bad = 0;
	if (ret_r != ret_p)
		bad = 1;
	if (err_r != err_p)
		bad = 1;
	if (!blob_equal(&rb, &pb))
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "file=%d thr=%d ret=%d/%d err=%d/%d", (int)file_val,
		    threaded, ret_r, ret_p, err_r, err_p);
		fail_msg(id, tag, detail);
	}
}

static void
fileno_edges(void)
{
	static const short vals[] = {
		-1, 0, 1, 2, 127, 255, 256, 32767, -2, -128, -32768
	};
	int threaded;

	for (threaded = 0; threaded <= 1; threaded++) {
		for (std::size_t i = 0; i < sizeof(vals) / sizeof(vals[0]);
		    i++) {
			char tag[32];
			std::snprintf(tag, sizeof(tag), "v%d-t%d",
			    (int)vals[i], threaded);
			fileno_case(S_FILENO, 0, vals[i], threaded, tag);
			fileno_case(S_FILENO_UNLOCKED, 1, vals[i], threaded,
			    tag);
		}
	}
}

static void
fileno_random(StatId id, int use_unlocked, long n)
{
	for (long t = 0; t < n; t++) {
		short file_val = (short)(rnd_u32() & 0xffffu);
		int threaded = (int)(rnd_u32() & 1);
		fileno_case(id, use_unlocked, file_val, threaded, "random");
	}
}

} /* namespace */

int
main(void)
{
	long total_fails = 0;
	int rc = 0;

	save_stdin();

	putc_edges();
	putc_random(S_PUTC, 0, RAND_ITERS);
	putc_random(S_PUTC_UNLOCKED, 1, RAND_ITERS);

	tmpnam_edges();
	tmpnam_random(RAND_ITERS);

	getchar_edges();
	getchar_random(S_GETCHAR, 0, RAND_ITERS);
	getchar_random(S_GETCHAR_UNLOCKED, 1, RAND_ITERS);

	fileno_edges();
	fileno_random(S_FILENO, 0, RAND_ITERS);
	fileno_random(S_FILENO_UNLOCKED, 1, RAND_ITERS);

	if (saved_stdin >= 0)
		close(saved_stdin);

	for (int i = 0; i < NSTAT; i++)
		total_fails += g_stat[i].fails;

	std::printf("\n");
	std::printf("+----------------------+----------+----------+\n");
	std::printf("| function             |     cases|    fails |\n");
	std::printf("+----------------------+----------+----------+\n");
	for (int i = 0; i < NSTAT; i++)
		std::printf("| %-20s | %9ld| %9ld|\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
	std::printf("+----------------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
