/*
 * b0109 differential test: pbsd::lib_libc_stdio::b0109 vs. the ref_ oracle.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fcntl.h>
#include <locale.h>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0109;

namespace port = pbsd::lib_libc_stdio::b0109;

extern "C" {
int ref_fgetc(FILE *);
wint_t ref_putwc(wchar_t wc, FILE *fp);
wint_t ref_putwc_l(wchar_t wc, FILE *fp, locale_t locale);
int ref_swprintf(wchar_t * __restrict s, size_t n,
    const wchar_t * __restrict fmt, ...);
int ref_swprintf_l(wchar_t * __restrict s, size_t n, locale_t locale,
    const wchar_t * __restrict fmt, ...);
void ref_rewind(FILE *fp);
wint_t fputwc_l(wchar_t wc, FILE *fp, locale_t loc);
wint_t putwc_l(wchar_t wc, FILE *fp, locale_t locale);
}

extern "C" wint_t
fputwc_l(wchar_t wc, FILE *fp, locale_t loc)
{
	locale_t old = uselocale(loc);
	wint_t w = fputwc(wc, fp);

	uselocale(old);
	return w;
}

extern "C" wint_t
putwc_l(wchar_t wc, FILE *fp, locale_t locale)
{
	locale_t old = uselocale(locale);
	wint_t w = fputwc(wc, fp);

	uselocale(old);
	return w;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_USER = 512;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_CAP = BUF_PRE + BUF_USER + BUF_POST;

constexpr int MAXPRINT = 8;
constexpr unsigned RAND_ITERS = 200000u;

enum StatId {
	S_FGETC,
	S_PUTWC,
	S_PUTWC_L,
	S_SWPRINTF,
	S_SWPRINTF_L,
	S_REWIND,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "fgetc",      0, 0, 0 },
	{ "putwc",      0, 0, 0 },
	{ "putwc_l",    0, 0, 0 },
	{ "swprintf",   0, 0, 0 },
	{ "swprintf_l", 0, 0, 0 },
	{ "rewind",     0, 0, 0 },
};

std::uint64_t rng_state = 0xb0109cafebabeULL;

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

int
rnd_i32(void)
{
	return (int)rnd_u32();
}

void
fail_msg(StatId which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].printed++ < MAXPRINT)
		std::printf("  FAIL %-10s %-24s %s\n", g_stat[which].name, label,
		    detail);
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

FILE *
make_temp_copy(const unsigned char *data, std::size_t len)
{
	char path[] = "/tmp/pbsd_b0109_XXXXXX";
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
	unlink(path);
	return fp;
}

FILE *
make_temp_readonly(const unsigned char *data, std::size_t len)
{
	char path[] = "/tmp/pbsd_b0109_ro_XXXXXX";
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
	fp = fopen(path, "r");
	unlink(path);
	return fp;
}

struct GuardedWBuf {
	static constexpr std::size_t WPRE = 8;
	static constexpr std::size_t WUSER = 64;
	static constexpr std::size_t WPOST = 8;
	static constexpr std::size_t WCAP = WPRE + WUSER + WPOST;
	unsigned char bytes[WCAP * sizeof(wchar_t)];

	void fill_guard(void)
	{
		std::memset(bytes, GUARD, sizeof(bytes));
	}

	wchar_t *user(void)
	{
		return reinterpret_cast<wchar_t *>(bytes + WPRE * sizeof(wchar_t));
	}

	bool eq_bytes(const GuardedWBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof(bytes)) == 0;
	}
};

/* ------------------------------------------------------------------------ */
/* fgetc                                                                     */
/* ------------------------------------------------------------------------ */

bool
test_fgetc_stream(const unsigned char *data, std::size_t len, const char *label)
{
	FILE *rf = make_temp_copy(data, len);
	FILE *pf = make_temp_copy(data, len);
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(S_FGETC, label, "temp file failed");
		case_inc(S_FGETC);
		return false;
	}

	for (;;) {
		int cr = ref_fgetc(rf);
		int cp = port::fgetc(pf);
		if (cr != cp) {
			fail_msg(S_FGETC, label, "return mismatch");
			ok = false;
			break;
		}
		if (cr == EOF)
			break;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(S_FGETC);
	return ok;
}

void
run_fgetc_edges(void)
{
	static const unsigned char empty[] = {};
	static const unsigned char one_a[] = { 'a' };
	static const unsigned char nul[] = { 0x00, 'b', 0x00, 'c' };
	static const unsigned char hi[] = { 0x80, 0x81, 0xfe, 0xff };
	static const unsigned char mix[] = {
		0x7f, 0x80, 0x00, 0xff, 'x', '\n', 0x00, 'y'
	};
	static const unsigned char boundary[256];

	test_fgetc_stream(empty, 0, "empty");
	test_fgetc_stream(one_a, 1, "one");
	test_fgetc_stream(nul, sizeof(nul), "nul");
	test_fgetc_stream(hi, sizeof(hi), "hi");
	test_fgetc_stream(mix, sizeof(mix), "mix");
	test_fgetc_stream(boundary, sizeof(boundary), "boundary");
}

void
run_fgetc_random(void)
{
	unsigned char buf[256];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t n = rnd_u32() % (sizeof(buf) + 1u);
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 3u) == 0)
				buf[j] = (unsigned char)(0x80 + (v & 0x7fu));
			else if ((v & 3u) == 1)
				buf[j] = 0;
			else
				buf[j] = (unsigned char)(v & 0xffu);
		}
		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_fgetc_stream(buf, n, label);
	}
}

/* ------------------------------------------------------------------------ */
/* putwc / putwc_l                                                           */
/* ------------------------------------------------------------------------ */

FILE *
mk_wchar_write(void)
{
	FILE *f = std::tmpfile();
	if (f)
		fwide(f, 1);
	return f;
}

bool
test_putwc_stream(StatId which, locale_t loc, int use_l,
    const wchar_t *ws, std::size_t n, const char *label)
{
	FILE *rf = mk_wchar_write();
	FILE *pf = mk_wchar_write();
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	for (std::size_t i = 0; i < n; i++) {
		wint_t wr, wp;
		if (use_l) {
			wr = ref_putwc_l(ws[i], rf, loc);
			wp = port::putwc_l(ws[i], pf, loc);
		} else {
			wr = ref_putwc(ws[i], rf);
			wp = port::putwc(ws[i], pf);
		}
		if (wr != wp) {
			fail_msg(which, label, "return mismatch");
			ok = false;
			break;
		}
	}

	if (ok) {
		std::rewind(rf);
		std::rewind(pf);
		for (std::size_t i = 0; i < n; i++) {
			wint_t cr = std::fgetwc(rf);
			wint_t cp = std::fgetwc(pf);
			if (cr != cp) {
				fail_msg(which, label, "readback mismatch");
				ok = false;
				break;
			}
		}
		wint_t cr = std::fgetwc(rf);
		wint_t cp = std::fgetwc(pf);
		if (cr != cp) {
			fail_msg(which, label, "eof mismatch");
			ok = false;
		}
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(which);
	return ok;
}

void
run_putwc_edges(StatId which, locale_t loc, int use_l)
{
	static const wchar_t wempty[] = {};
	static const wchar_t wone[] = { 'A' };
	static const wchar_t wnul[] = { '\0', 'B' };
	static const wchar_t whi[] = {
		(wchar_t)0x80, (wchar_t)0xff, (wchar_t)0xffff
	};

	test_putwc_stream(which, loc, use_l, wempty, 0, "empty");
	test_putwc_stream(which, loc, use_l, wone, 1, "one");
	test_putwc_stream(which, loc, use_l, wnul, 2, "nul");
	test_putwc_stream(which, loc, use_l, whi, 3, "hi");
}

void
run_putwc_random(StatId which, locale_t loc, int use_l)
{
	wchar_t buf[48];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t n = rnd_u32() % 40u;
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 3u) == 0)
				buf[j] = (wchar_t)(0x80 + (v & 0x7fu));
			else if ((v & 3u) == 1)
				buf[j] = (wchar_t)'\0';
			else
				buf[j] = (wchar_t)(v & 0xffffu);
		}
		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_putwc_stream(which, loc, use_l, buf, n, label);
	}
}

/* ------------------------------------------------------------------------ */
/* swprintf / swprintf_l                                                     */
/* ------------------------------------------------------------------------ */

bool
swprintf_cmp(StatId which, const char *label, GuardedWBuf &rb, GuardedWBuf &pb,
    int rr, int rp)
{
	bool ok = true;

	case_inc(which);
	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!rb.eq_bytes(pb)) {
		fail_msg(which, label, "buffer mismatch");
		ok = false;
	}
	return ok;
}

#define SWPRINTF_CASE(which, loc, use_l, label, fmt, ...)                      \
	do {                                                                       \
		GuardedWBuf rb, pb;                                                    \
		rb.fill_guard();                                                       \
		pb.fill_guard();                                                       \
		int rr, rp;                                                            \
		if (use_l) {                                                           \
			rr = ref_swprintf_l(rb.user(), GuardedWBuf::WUSER + 1, loc,     \
			    fmt, __VA_ARGS__);                                       \
			rp = port::swprintf_l(pb.user(), GuardedWBuf::WUSER + 1, loc,  \
			    fmt, __VA_ARGS__);                                       \
		} else {                                                               \
			rr = ref_swprintf(rb.user(), GuardedWBuf::WUSER + 1, fmt,      \
			    __VA_ARGS__);                                            \
			rp = port::swprintf(pb.user(), GuardedWBuf::WUSER + 1, fmt,    \
			    __VA_ARGS__);                                            \
		}                                                                      \
		swprintf_cmp(which, label, rb, pb, rr, rp);                           \
	} while (0)

void
run_swprintf_edges(StatId which, locale_t loc, int use_l)
{
	static const wchar_t ws[] = { L'h', L'i', 0 };
	static const wchar_t wempty[] = { 0 };
	static const wchar_t whi[] = { (wchar_t)0x80, (wchar_t)0xff, 0 };
	static const wchar_t wnul[] = { L'a', L'\0', L'b', 0 };

	SWPRINTF_CASE(which, loc, use_l, "empty", L"");
	SWPRINTF_CASE(which, loc, use_l, "pct", L"%%");
	SWPRINTF_CASE(which, loc, use_l, "int0", L"%d", 0);
	SWPRINTF_CASE(which, loc, use_l, "int-1", L"%d", -1);
	SWPRINTF_CASE(which, loc, use_l, "intmax", L"%d", INT_MAX);
	SWPRINTF_CASE(which, loc, use_l, "intmin", L"%d", INT_MIN);
	SWPRINTF_CASE(which, loc, use_l, "char", L"%c", L'Z');
	SWPRINTF_CASE(which, loc, use_l, "charhi", L"%c", (wchar_t)0xff);
	SWPRINTF_CASE(which, loc, use_l, "str", L"%ls", ws);
	SWPRINTF_CASE(which, loc, use_l, "strempty", L"%ls", wempty);
	SWPRINTF_CASE(which, loc, use_l, "strhi", L"%ls", whi);
	SWPRINTF_CASE(which, loc, use_l, "strnul", L"%ls", wnul);
	SWPRINTF_CASE(which, loc, use_l, "width", L"%5d", 42);
	SWPRINTF_CASE(which, loc, use_l, "hex", L"%x", 0xdead);
	SWPRINTF_CASE(which, loc, use_l, "n0", L"%d", 7);
	SWPRINTF_CASE(which, loc, use_l, "n1", L"%d", 1);
	SWPRINTF_CASE(which, loc, use_l, "mix", L"%d %ls %c", 7, ws, L'!');
}

void
run_swprintf_random(StatId which, locale_t loc, int use_l)
{
	wchar_t fmt[32];
	wchar_t str[64];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		unsigned kind = rnd_u32() % 8;
		char label[32];

		std::snprintf(label, sizeof(label), "rnd%u", i);
		switch (kind) {
		case 0:
			SWPRINTF_CASE(which, loc, use_l, label, L"%d", rnd_i32());
			break;
		case 1:
			SWPRINTF_CASE(which, loc, use_l, label, L"%c",
			    (wchar_t)(rnd_u32() & 0xffff));
			break;
		case 2:
			for (std::size_t j = 0; j < 8; j++)
				str[j] = (wchar_t)(rnd_u32() & 0xff);
			str[8] = 0;
			SWPRINTF_CASE(which, loc, use_l, label, L"%ls", str);
			break;
		case 3:
			SWPRINTF_CASE(which, loc, use_l, label, L"%%");
			break;
		case 4:
			SWPRINTF_CASE(which, loc, use_l, label, L"");
			break;
		case 5:
			SWPRINTF_CASE(which, loc, use_l, label, L"%#x",
			    rnd_i32());
			break;
		case 6:
			SWPRINTF_CASE(which, loc, use_l, label, L"%d %d",
			    rnd_i32(), rnd_i32());
			break;
		default:
			std::swprintf(fmt, 32, L"%%%du",
			    (int)(rnd_u32() % 20));
			SWPRINTF_CASE(which, loc, use_l, label, fmt, rnd_i32());
			break;
		}
	}
}

/* ------------------------------------------------------------------------ */
/* rewind                                                                    */
/* ------------------------------------------------------------------------ */

struct RewindObs {
	int errno_after;
	int feof_u;
	int ferror_u;
	long pos;
	unsigned char tail[64];
	int first_c;
};

void
guard_tail(unsigned char *t)
{
	std::memset(t, GUARD, 64);
}

void
prep_rewindable(FILE *fp, const unsigned char *data, std::size_t len,
    std::size_t read_count, int cause_error, int drain_eof)
{
	std::size_t i;
	int c;

	std::rewind(fp);
	if (len > 0) {
		if (fwrite(data, 1, len, fp) != len)
			(void)0;
		std::fflush(fp);
		std::rewind(fp);
	}
	for (i = 0; i < read_count; i++) {
		c = std::fgetc(fp);
		if (c == EOF)
			break;
	}
	if (drain_eof) {
		while (std::fgetc(fp) != EOF)
			(void)0;
	}
	if (cause_error) {
		(void)fputc('x', fp);
	}
}

RewindObs
call_ref_rewind(FILE *fp, int errno_before)
{
	RewindObs o{};

	errno = errno_before;
	ref_rewind(fp);
	o.errno_after = errno;
	o.feof_u = feof_unlocked(fp);
	o.ferror_u = ferror_unlocked(fp);
	o.pos = ftell(fp);
	guard_tail(o.tail);
	if (o.pos >= 0)
		(void)fread(o.tail, 1, sizeof(o.tail), fp);
	std::rewind(fp);
	o.first_c = std::fgetc(fp);
	return o;
}

RewindObs
call_port_rewind(FILE *fp, int errno_before)
{
	RewindObs o{};

	errno = errno_before;
	port::rewind(fp);
	o.errno_after = errno;
	o.feof_u = feof_unlocked(fp);
	o.ferror_u = ferror_unlocked(fp);
	o.pos = ftell(fp);
	guard_tail(o.tail);
	if (o.pos >= 0)
		(void)fread(o.tail, 1, sizeof(o.tail), fp);
	std::rewind(fp);
	o.first_c = std::fgetc(fp);
	return o;
}

bool
rewind_obs_eq(const RewindObs &a, const RewindObs &b)
{
	if (a.errno_after != b.errno_after)
		return false;
	if (a.feof_u != b.feof_u)
		return false;
	if (a.ferror_u != b.ferror_u)
		return false;
	if (a.pos != b.pos)
		return false;
	if (a.first_c != b.first_c)
		return false;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		return false;
	return true;
}

bool
test_rewind_file(const unsigned char *data, std::size_t len,
    std::size_t read_count, int cause_error, int drain_eof,
    int errno_before, const char *label)
{
	FILE *ra = make_temp_copy(data, len);
	FILE *pa = make_temp_copy(data, len);
	RewindObs r, p;
	bool ok = true;

	if (!ra || !pa) {
		if (ra)
			std::fclose(ra);
		if (pa)
			std::fclose(pa);
		fail_msg(S_REWIND, label, "temp file failed");
		case_inc(S_REWIND);
		return false;
	}

	prep_rewindable(ra, data, len, read_count, cause_error, drain_eof);
	prep_rewindable(pa, data, len, read_count, cause_error, drain_eof);

	r = call_ref_rewind(ra, errno_before);
	p = call_port_rewind(pa, errno_before);

	if (!rewind_obs_eq(r, p)) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu rd=%zu err=%d ce=%d de=%d errno %d/%d pos %ld/%ld",
		    len, read_count, errno_before, cause_error, drain_eof,
		    r.errno_after, p.errno_after, r.pos, p.pos);
		fail_msg(S_REWIND, label, detail);
		ok = false;
	}

	std::fclose(ra);
	std::fclose(pa);
	case_inc(S_REWIND);
	return ok;
}

bool
test_rewind_readonly(const unsigned char *data, std::size_t len,
    int errno_before, const char *label)
{
	FILE *ra = make_temp_readonly(data, len);
	FILE *pa = make_temp_readonly(data, len);
	RewindObs r, p;
	bool ok = true;

	if (!ra || !pa) {
		if (ra)
			std::fclose(ra);
		if (pa)
			std::fclose(pa);
		fail_msg(S_REWIND, label, "readonly temp failed");
		case_inc(S_REWIND);
		return false;
	}

	(void)std::fgetc(ra);
	(void)std::fgetc(pa);
	(void)fputc('x', ra);
	(void)fputc('x', pa);

	r = call_ref_rewind(ra, errno_before);
	p = call_port_rewind(pa, errno_before);

	if (!rewind_obs_eq(r, p)) {
		fail_msg(S_REWIND, label, "readonly mismatch");
		ok = false;
	}

	std::fclose(ra);
	std::fclose(pa);
	case_inc(S_REWIND);
	return ok;
}

bool
test_rewind_pipe(int errno_before, const char *label)
{
	int pr[2], pp[2];
	FILE *ra, *pa;
	RewindObs r, p;
	bool ok = true;

	if (pipe(pr) != 0 || pipe(pp) != 0) {
		fail_msg(S_REWIND, label, "pipe failed");
		case_inc(S_REWIND);
		return false;
	}
	const char probe[] = "abc";
	write(pr[1], probe, 3);
	write(pp[1], probe, 3);
	close(pr[1]);
	close(pp[1]);
	ra = fdopen(pr[0], "r");
	pa = fdopen(pp[0], "r");
	if (!ra || !pa) {
		if (ra)
			fclose(ra);
		if (pa)
			fclose(pa);
		fail_msg(S_REWIND, label, "fdopen failed");
		case_inc(S_REWIND);
		return false;
	}

	(void)std::fgetc(ra);
	(void)std::fgetc(pa);

	r = call_ref_rewind(ra, errno_before);
	p = call_port_rewind(pa, errno_before);

	if (!rewind_obs_eq(r, p)) {
		char detail[96];
		std::snprintf(detail, sizeof(detail),
		    "errno %d/%d feof %d/%d ferr %d/%d",
		    r.errno_after, p.errno_after, r.feof_u, p.feof_u,
		    r.ferror_u, p.ferror_u);
		fail_msg(S_REWIND, label, detail);
		ok = false;
	}

	fclose(ra);
	fclose(pa);
	case_inc(S_REWIND);
	return ok;
}

void
run_rewind_edges(void)
{
	static const unsigned char empty[] = {};
	static const unsigned char one[] = { 'x' };
	static const unsigned char mix[] = {
		0x7f, 0x80, 0x00, 0xff, 'a', 'b', 'c', '\n'
	};

	test_rewind_file(empty, 0, 0, 0, 0, 0, "empty-e0");
	test_rewind_file(empty, 0, 0, 0, 0, 42, "empty-e42");
	test_rewind_file(one, 1, 0, 0, 0, 5, "one-e5");
	test_rewind_file(one, 1, 1, 0, 0, 7, "one-read1");
	test_rewind_file(mix, sizeof(mix), 3, 0, 0, 11, "mix-rd3");
	test_rewind_file(mix, sizeof(mix), 0, 0, 1, 13, "mix-drain");
	test_rewind_file(mix, sizeof(mix), 2, 1, 0, 17, "mix-error");
	test_rewind_readonly(one, 1, 19, "ro-error");
	test_rewind_readonly(mix, sizeof(mix), 23, "ro-mix");
	test_rewind_pipe(0, "pipe-e0");
	test_rewind_pipe(31, "pipe-e31");
}

void
run_rewind_random(void)
{
	unsigned char buf[128];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t len = rnd_u32() % (sizeof(buf) + 1u);
		std::size_t read_count = rnd_u32() % (len + 4u);
		int cause_error = (rnd_u32() & 1u) ? 1 : 0;
		int drain_eof = (rnd_u32() & 2u) ? 1 : 0;
		int errno_before = (rnd_u32() & 4u) ? (int)(rnd_u32() % 40u + 1u) : 0;
		char label[48];

		for (std::size_t j = 0; j < len; j++)
			buf[j] = (unsigned char)rnd_u32();
		std::snprintf(label, sizeof(label), "rnd%u", i);

		if ((rnd_u32() & 8u) == 0)
			test_rewind_file(buf, len, read_count, cause_error,
			    drain_eof, errno_before, label);
		else if ((rnd_u32() & 16u) == 0)
			test_rewind_pipe(errno_before, label);
		else
			test_rewind_readonly(buf, len, errno_before, label);
	}
}

void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-12s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-12s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

} /* namespace */

int
main(void)
{
	locale_t cloc = newlocale(LC_ALL_MASK, "C", nullptr);

	run_fgetc_edges();
	run_fgetc_random();
	run_putwc_edges(S_PUTWC, nullptr, 0);
	run_putwc_random(S_PUTWC, nullptr, 0);
	run_putwc_edges(S_PUTWC_L, cloc, 1);
	run_putwc_random(S_PUTWC_L, cloc, 1);
	run_swprintf_edges(S_SWPRINTF, nullptr, 0);
	run_swprintf_random(S_SWPRINTF, nullptr, 0);
	run_swprintf_edges(S_SWPRINTF_L, cloc, 1);
	run_swprintf_random(S_SWPRINTF_L, cloc, 1);
	run_rewind_edges();
	run_rewind_random();

	if (cloc)
		freelocale(cloc);

	report();
	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
