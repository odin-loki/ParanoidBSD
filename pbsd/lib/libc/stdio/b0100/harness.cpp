/*
 * PBSD batch b0100 -- differential test: port vs. ref_ oracle.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <locale.h>
#include <stdarg.h>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0100;

namespace P = pbsd::lib_libc_stdio::b0100;

extern "C" {
typedef struct {
	unsigned char	*_p;
	int		_r;
	int		_w;
	short		_flags;
	short		_file;
} ref_FILE;

int		ref_wprintf(const wchar_t * __restrict, ...);
int		ref_wprintf_l(locale_t, const wchar_t * __restrict, ...);
void		ref_clearerr(ref_FILE *);
void		ref_clearerr_unlocked(ref_FILE *);
int		ref_feof(ref_FILE *);
int		ref_feof_unlocked(ref_FILE *);
extern int	__isthreaded;
}

enum {
	F_WPRINTF,
	F_WPRINTF_L,
	F_CLEARERR,
	F_CLEARERR_UNLOCKED,
	F_FEOF,
	F_FEOF_UNLOCKED,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"wprintf",
	"wprintf_l",
	"clearerr",
	"clearerr_unlocked",
	"feof",
	"feof_unlocked"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr long long SWEEP = 200000;

struct FileBlob {
	unsigned char	pre[32];
	ref_FILE	f;
	unsigned char	post[32];
};

struct PortBlob {
	unsigned char	pre[32];
	P::FILE		f;
	unsigned char	post[32];
};

static_assert(sizeof(ref_FILE) == sizeof(P::FILE));
static_assert(offsetof(ref_FILE, _flags) == offsetof(P::FILE, _flags));

static uint64_t rng = 0xb0100ULL;

static uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return rng;
}

static int
rnd16(void)
{
	return (int)(rnd() & 0xffffu);
}

static void
fill_blob(FileBlob *b)
{
	std::memset(b, GUARD, sizeof(*b));
}

static void
fill_port_blob(PortBlob *b)
{
	std::memset(b, GUARD, sizeof(*b));
}

static int
blob_equal(const FileBlob *refb, const PortBlob *portb)
{
	if (std::memcmp(refb->pre, portb->pre, sizeof(refb->pre)) != 0)
		return 0;
	if (std::memcmp(&refb->f._p, &portb->f._p, sizeof(refb->f._p)) != 0)
		return 0;
	if (refb->f._r != portb->f._r)
		return 0;
	if (refb->f._w != portb->f._w)
		return 0;
	if (refb->f._flags != portb->f._flags)
		return 0;
	if (refb->f._file != portb->f._file)
		return 0;
	return std::memcmp(refb->post, portb->post, sizeof(refb->post)) == 0;
}

static void
record_fail(int fn)
{
	if (nfail[fn] == 0)
		std::fprintf(stderr, "  FAIL %s\n", fname[fn]);
	nfail[fn]++;
}

static void
check_file(int fn, const FileBlob *refb, const PortBlob *portb, int refret,
    int portret)
{
	ncase[fn]++;

	if (refret != portret)
		record_fail(fn);
	if (!blob_equal(refb, portb))
		record_fail(fn);
}

static void
test_clearerr_pair(int fn, void (*ref_fn)(ref_FILE *),
    void (*port_fn)(P::FILE *), short flags, int threaded)
{
	FileBlob refb;
	PortBlob portb;

	__isthreaded = threaded;
	fill_blob(&refb);
	fill_port_blob(&portb);
	refb.f._flags = flags;
	portb.f._flags = flags;

	ref_fn(&refb.f);
	port_fn(&portb.f);
	check_file(fn, &refb, &portb, 0, 0);
}

static void
test_feof_pair(int fn, int (*ref_fn)(ref_FILE *), int (*port_fn)(P::FILE *),
    short flags, int threaded)
{
	FileBlob refb;
	PortBlob portb;
	int refret, portret;

	__isthreaded = threaded;
	fill_blob(&refb);
	fill_port_blob(&portb);
	refb.f._flags = flags;
	portb.f._flags = flags;

	refret = ref_fn(&refb.f);
	portret = port_fn(&portb.f);
	check_file(fn, &refb, &portb, refret, portret);
}

static int saved_stdout = -1;

static int
stdout_capture_open(void)
{
	int pipefd[2];

	if (saved_stdout < 0)
		saved_stdout = ::dup(STDOUT_FILENO);
	if (pipe(pipefd) != 0)
		std::abort();
	if (::dup2(pipefd[1], STDOUT_FILENO) != STDOUT_FILENO)
		std::abort();
	::close(pipefd[1]);
	return pipefd[0];
}

static void
stdout_restore(void)
{
	if (saved_stdout >= 0)
		::dup2(saved_stdout, STDOUT_FILENO);
	std::fflush(stdout);
}

static int
stdout_capture_close(int readfd, unsigned char *out, size_t outcap)
{
	std::memset(out, GUARD, outcap);
	std::fflush(stdout);
	stdout_restore();
	ssize_t n = ::read(readfd, out, outcap);
	::close(readfd);
	if (n < 0)
		return -1;
	return (int)n;
}

#define DO_WPRINTF_TEST(fn, refcall, portcall)                           \
	do {                                                                 \
		int refret, portret;                                         \
		int refbytes, portbytes;                                     \
		unsigned char refbuf[4096], portbuf[4096];                   \
		int reffd, portfd;                                           \
		std::memset(refbuf, GUARD, sizeof(refbuf));                  \
		std::memset(portbuf, GUARD, sizeof(portbuf));                \
		ncase[fn]++;                                                 \
		reffd = stdout_capture_open();                               \
		refret = (refcall);                                          \
		refbytes = stdout_capture_close(reffd, refbuf, sizeof(refbuf)); \
		portfd = stdout_capture_open();                              \
		portret = (portcall);                                        \
		portbytes = stdout_capture_close(portfd, portbuf, sizeof(portbuf)); \
		if (refret != portret || refbytes != portbytes ||            \
		    std::memcmp(refbuf, portbuf, sizeof(refbuf)) != 0)       \
			record_fail(fn);                                     \
	} while (0)

#define TEST_WPRINTF_CASE(fn, fmt, ...)                                \
	DO_WPRINTF_TEST(fn, ref_wprintf(fmt, ##__VA_ARGS__),             \
	    P::wprintf(fmt, ##__VA_ARGS__))

#define TEST_WPRINTF_L_CASE(fn, loc, fmt, ...)                         \
	DO_WPRINTF_TEST(fn, ref_wprintf_l(loc, fmt, ##__VA_ARGS__),        \
	    P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, ##__VA_ARGS__))

static void
edge_clearerr(int fn, void (*ref_fn)(ref_FILE *), void (*port_fn)(P::FILE *))
{
	static const short edge_flags[] = {
		0,
		0x0001, 0x0002, 0x0004, 0x0008, 0x0010,
		(short)0x0020, (short)0x0040, (short)0x0060,
		(short)0x001f, (short)0x0021, (short)0x003f, (short)0x0041,
		(short)0x0080, (short)0x00ff, (short)0x7f00, (short)-1,
		(short)0x8000, (short)0xfffe, (short)0xffff
	};

	for (short f : edge_flags)
		test_clearerr_pair(fn, ref_fn, port_fn, f, 0);
	for (short f : edge_flags)
		test_clearerr_pair(fn, ref_fn, port_fn, f, 1);
}

static void
edge_feof(int fn, int (*ref_fn)(ref_FILE *), int (*port_fn)(P::FILE *))
{
	static const short edge_flags[] = {
		0,
		0x0001, 0x0002, 0x0004, 0x0008, 0x0010,
		(short)0x0020, (short)0x0040, (short)0x0060,
		(short)0x001f, (short)0x0021, (short)0x003f, (short)0x0041,
		(short)0x0080, (short)0x00ff, (short)0x7f00, (short)-1,
		(short)0x8000, (short)0xfffe, (short)0xffff
	};

	for (short f : edge_flags)
		test_feof_pair(fn, ref_fn, port_fn, f, 0);
	for (short f : edge_flags)
		test_feof_pair(fn, ref_fn, port_fn, f, 1);
}

static void
sweep_clearerr(int fn, void (*ref_fn)(ref_FILE *), void (*port_fn)(P::FILE *))
{
	for (long long i = 0; i < SWEEP; i++)
		test_clearerr_pair(fn, ref_fn, port_fn, rnd16(), (int)(rnd() & 1u));
}

static void
sweep_feof(int fn, int (*ref_fn)(ref_FILE *), int (*port_fn)(P::FILE *))
{
	for (long long i = 0; i < SWEEP; i++)
		test_feof_pair(fn, ref_fn, port_fn, rnd16(), (int)(rnd() & 1u));
}

static wchar_t wbuf[256];

static void
edge_wprintf(int fn)
{
	static const wchar_t empty[] = { 0 };
	static const wchar_t one[] = { 'A', 0 };
	static const wchar_t hi[] = { (wchar_t)0x80, (wchar_t)0xff,
	    (wchar_t)0x100, (wchar_t)0xffff, 0 };
	static const wchar_t nuls[] = { 'x', 0, 'y', 0 };

	TEST_WPRINTF_CASE(fn, empty);
	TEST_WPRINTF_CASE(fn, L"");
	TEST_WPRINTF_CASE(fn, L"%d", 0);
	TEST_WPRINTF_CASE(fn, L"%d", -1);
	TEST_WPRINTF_CASE(fn, L"%d", INT_MAX);
	TEST_WPRINTF_CASE(fn, L"%d", INT_MIN);
	TEST_WPRINTF_CASE(fn, L"%u", 0u);
	TEST_WPRINTF_CASE(fn, L"%u", UINT_MAX);
	TEST_WPRINTF_CASE(fn, L"%x", 0);
	TEST_WPRINTF_CASE(fn, L"%x", 0xffffffffu);
	TEST_WPRINTF_CASE(fn, L"%c", L'Z');
	TEST_WPRINTF_CASE(fn, L"%c", (wchar_t)0x80);
	TEST_WPRINTF_CASE(fn, L"%c", (wchar_t)0xffff);
	TEST_WPRINTF_CASE(fn, L"%%");
	TEST_WPRINTF_CASE(fn, L"%s", empty);
	TEST_WPRINTF_CASE(fn, L"%s", one);
	TEST_WPRINTF_CASE(fn, L"%s", hi);
	TEST_WPRINTF_CASE(fn, L"%5d", 42);
	TEST_WPRINTF_CASE(fn, L"%05d", -7);
	TEST_WPRINTF_CASE(fn, L"%d %u %x %c", 1, 2u, 3u, L'Q');
	TEST_WPRINTF_CASE(fn, L"%ls", one);
	TEST_WPRINTF_CASE(fn, L"%ls", hi);
	TEST_WPRINTF_CASE(fn, L"plain text");
	TEST_WPRINTF_CASE(fn, L"%lc", (wchar_t)0x7f);
	TEST_WPRINTF_CASE(fn, L"%lc", (wchar_t)0x80);
	TEST_WPRINTF_CASE(fn, nuls);
}

static void
edge_wprintf_l(int fn, locale_t loc)
{
	static const wchar_t empty[] = { 0 };
	static const wchar_t one[] = { 'B', 0 };
	static const wchar_t hi[] = { (wchar_t)0xfe, (wchar_t)0xabcd, 0 };

	TEST_WPRINTF_L_CASE(fn, loc, empty);
	TEST_WPRINTF_L_CASE(fn, loc, L"");
	TEST_WPRINTF_L_CASE(fn, loc, L"%d", 0);
	TEST_WPRINTF_L_CASE(fn, loc, L"%d", -12345);
	TEST_WPRINTF_L_CASE(fn, loc, L"%d", INT_MAX);
	TEST_WPRINTF_L_CASE(fn, loc, L"%d", INT_MIN);
	TEST_WPRINTF_L_CASE(fn, loc, L"%u", UINT_MAX);
	TEST_WPRINTF_L_CASE(fn, loc, L"%x", 0xdeadbeefu);
	TEST_WPRINTF_L_CASE(fn, loc, L"%c", L'!');
	TEST_WPRINTF_L_CASE(fn, loc, L"%c", (wchar_t)0xff);
	TEST_WPRINTF_L_CASE(fn, loc, L"%%");
	TEST_WPRINTF_L_CASE(fn, loc, L"%s", one);
	TEST_WPRINTF_L_CASE(fn, loc, L"%s", hi);
	TEST_WPRINTF_L_CASE(fn, loc, L"%5d", 99);
	TEST_WPRINTF_L_CASE(fn, loc, L"%d %u %x", 7, 8u, 9u);
	TEST_WPRINTF_L_CASE(fn, loc, L"locale line");
}

static void
sweep_wprintf(int fn)
{
	static const wchar_t *fmts[] = {
		L"%d", L"%u", L"%x", L"%c", L"%s", L"%%", L"%5d", L"%05d",
		L"%d %u", L"%x %c"
	};
	const int nfmts = (int)(sizeof(fmts) / sizeof(fmts[0]));

	for (long long i = 0; i < SWEEP; i++) {
		const wchar_t *fmt = fmts[(int)(rnd() % nfmts)];
		int a = (int)(rnd() & 0x7fffffffu);
		unsigned b = (unsigned)(rnd() & 0xffffffffu);
		wchar_t c = (wchar_t)rnd16();
		int len = (int)(rnd() % 16u);

		for (int j = 0; j < len; j++)
			wbuf[j] = (wchar_t)rnd16();
		wbuf[len] = 0;

		ncase[fn]++;

		unsigned char refbuf[4096], portbuf[4096];
		int refret, portret;
		int refbytes, portbytes;
		int reffd, portfd;

		std::memset(refbuf, GUARD, sizeof(refbuf));
		std::memset(portbuf, GUARD, sizeof(portbuf));

		reffd = stdout_capture_open();
		if (std::wcscmp(fmt, L"%d") == 0)
			refret = ref_wprintf(fmt, a);
		else if (std::wcscmp(fmt, L"%u") == 0)
			refret = ref_wprintf(fmt, b);
		else if (std::wcscmp(fmt, L"%x") == 0)
			refret = ref_wprintf(fmt, b);
		else if (std::wcscmp(fmt, L"%c") == 0)
			refret = ref_wprintf(fmt, c);
		else if (std::wcscmp(fmt, L"%s") == 0)
			refret = ref_wprintf(fmt, wbuf);
		else if (std::wcscmp(fmt, L"%%") == 0)
			refret = ref_wprintf(fmt);
		else if (std::wcscmp(fmt, L"%5d") == 0)
			refret = ref_wprintf(fmt, a);
		else if (std::wcscmp(fmt, L"%05d") == 0)
			refret = ref_wprintf(fmt, a);
		else if (std::wcscmp(fmt, L"%d %u") == 0)
			refret = ref_wprintf(fmt, a, b);
		else
			refret = ref_wprintf(fmt, a, b, c);
		refbytes = stdout_capture_close(reffd, refbuf, sizeof(refbuf));

		portfd = stdout_capture_open();
		if (std::wcscmp(fmt, L"%d") == 0)
			portret = P::wprintf(fmt, a);
		else if (std::wcscmp(fmt, L"%u") == 0)
			portret = P::wprintf(fmt, b);
		else if (std::wcscmp(fmt, L"%x") == 0)
			portret = P::wprintf(fmt, b);
		else if (std::wcscmp(fmt, L"%c") == 0)
			portret = P::wprintf(fmt, c);
		else if (std::wcscmp(fmt, L"%s") == 0)
			portret = P::wprintf(fmt, wbuf);
		else if (std::wcscmp(fmt, L"%%") == 0)
			portret = P::wprintf(fmt);
		else if (std::wcscmp(fmt, L"%5d") == 0)
			portret = P::wprintf(fmt, a);
		else if (std::wcscmp(fmt, L"%05d") == 0)
			portret = P::wprintf(fmt, a);
		else if (std::wcscmp(fmt, L"%d %u") == 0)
			portret = P::wprintf(fmt, a, b);
		else
			portret = P::wprintf(fmt, a, b, c);
		portbytes = stdout_capture_close(portfd, portbuf, sizeof(portbuf));

		if (refret != portret || refbytes != portbytes ||
		    std::memcmp(refbuf, portbuf, sizeof(refbuf)) != 0)
			record_fail(fn);
	}
}

static void
sweep_wprintf_l(int fn, locale_t loc)
{
	static const wchar_t *fmts[] = {
		L"%d", L"%u", L"%x", L"%c", L"%s", L"%%", L"%5d", L"%d %u"
	};
	const int nfmts = (int)(sizeof(fmts) / sizeof(fmts[0]));

	for (long long i = 0; i < SWEEP; i++) {
		const wchar_t *fmt = fmts[(int)(rnd() % nfmts)];
		int a = (int)(rnd() & 0x7fffffffu);
		unsigned b = (unsigned)(rnd() & 0xffffffffu);
		wchar_t c = (wchar_t)rnd16();
		int len = (int)(rnd() % 16u);

		for (int j = 0; j < len; j++)
			wbuf[j] = (wchar_t)rnd16();
		wbuf[len] = 0;

		ncase[fn]++;

		unsigned char refbuf[4096], portbuf[4096];
		int refret, portret;
		int refbytes, portbytes;
		int reffd, portfd;

		std::memset(refbuf, GUARD, sizeof(refbuf));
		std::memset(portbuf, GUARD, sizeof(portbuf));

		reffd = stdout_capture_open();
		if (std::wcscmp(fmt, L"%d") == 0)
			refret = ref_wprintf_l(loc, fmt, a);
		else if (std::wcscmp(fmt, L"%u") == 0)
			refret = ref_wprintf_l(loc, fmt, b);
		else if (std::wcscmp(fmt, L"%x") == 0)
			refret = ref_wprintf_l(loc, fmt, b);
		else if (std::wcscmp(fmt, L"%c") == 0)
			refret = ref_wprintf_l(loc, fmt, c);
		else if (std::wcscmp(fmt, L"%s") == 0)
			refret = ref_wprintf_l(loc, fmt, wbuf);
		else if (std::wcscmp(fmt, L"%%") == 0)
			refret = ref_wprintf_l(loc, fmt);
		else if (std::wcscmp(fmt, L"%5d") == 0)
			refret = ref_wprintf_l(loc, fmt, a);
		else
			refret = ref_wprintf_l(loc, fmt, a, b);
		refbytes = stdout_capture_close(reffd, refbuf, sizeof(refbuf));

		portfd = stdout_capture_open();
		if (std::wcscmp(fmt, L"%d") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, a);
		else if (std::wcscmp(fmt, L"%u") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, b);
		else if (std::wcscmp(fmt, L"%x") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, b);
		else if (std::wcscmp(fmt, L"%c") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, c);
		else if (std::wcscmp(fmt, L"%s") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, wbuf);
		else if (std::wcscmp(fmt, L"%%") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt);
		else if (std::wcscmp(fmt, L"%5d") == 0)
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, a);
		else
			portret = P::wprintf_l(reinterpret_cast<P::locale_t>(loc), fmt, a, b);
		portbytes = stdout_capture_close(portfd, portbuf, sizeof(portbuf));

		if (refret != portret || refbytes != portbytes ||
		    std::memcmp(refbuf, portbuf, sizeof(refbuf)) != 0)
			record_fail(fn);
	}
}

int
main(void)
{
	locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
	if (loc == NULL)
		loc = LC_GLOBAL_LOCALE;

	__isthreaded = 0;

	edge_wprintf(F_WPRINTF);
	sweep_wprintf(F_WPRINTF);

	edge_wprintf_l(F_WPRINTF_L, loc);
	sweep_wprintf_l(F_WPRINTF_L, loc);

	edge_clearerr(F_CLEARERR, ref_clearerr, P::clearerr);
	sweep_clearerr(F_CLEARERR, ref_clearerr, P::clearerr);

	edge_clearerr(F_CLEARERR_UNLOCKED, ref_clearerr_unlocked,
	    P::clearerr_unlocked);
	sweep_clearerr(F_CLEARERR_UNLOCKED, ref_clearerr_unlocked,
	    P::clearerr_unlocked);

	edge_feof(F_FEOF, ref_feof, P::feof);
	sweep_feof(F_FEOF, ref_feof, P::feof);

	edge_feof(F_FEOF_UNLOCKED, ref_feof_unlocked, P::feof_unlocked);
	sweep_feof(F_FEOF_UNLOCKED, ref_feof_unlocked, P::feof_unlocked);

	stdout_restore();
	if (saved_stdout >= 0) {
		::close(saved_stdout);
		saved_stdout = -1;
	}
	if (loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);

	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fail += nfail[i];

	return total_fail == 0 ? 0 : 1;
}
