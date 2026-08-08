/*
 * harness.cpp -- differential test for PBSD batch b0100.
 */

#define _GNU_SOURCE
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fcntl.h>
#include <unistd.h>
#include <xlocale.h>

import pbsd.lib.libc.stdio.b0100;

namespace port = pbsd::lib_libc_stdio::b0100;

extern "C" {
int ref_wprintf(const wchar_t *__restrict, ...);
int ref_wprintf_l(locale_t, const wchar_t *__restrict, ...);
void ref_clearerr(FILE *);
void ref_clearerr_unlocked(FILE *);
int ref_feof(FILE *);
int ref_feof_unlocked(FILE *);
extern int __isthreaded;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr int MAX_REPORT = 8;
constexpr unsigned RAND_ITERS = 200000u;
constexpr std::size_t OUT_PRE = 16;
constexpr std::size_t OUT_CAP = 512;
constexpr std::size_t OUT_POST = 16;
constexpr std::size_t OUT_TOTAL = OUT_PRE + OUT_CAP + OUT_POST;
constexpr std::size_t FILEBUF = 4096;

enum StatId {
	S_WPRINTF,
	S_WPRINTF_L,
	S_CLEARERR,
	S_CLEARERR_UNLOCKED,
	S_FEOF,
	S_FEOF_UNLOCKED,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int reported;
};

Stats g_stat[NSTAT] = {
	{ "wprintf",            0, 0, 0 },
	{ "wprintf_l",          0, 0, 0 },
	{ "clearerr",           0, 0, 0 },
	{ "clearerr_unlocked",  0, 0, 0 },
	{ "feof",               0, 0, 0 },
	{ "feof_unlocked",      0, 0, 0 },
};

std::uint64_t rng_state = 0xb0100facefeedULL;

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
	if (g_stat[which].reported++ < MAX_REPORT)
		std::printf("  FAIL %-18s %-20s %s\n", g_stat[which].name,
		    label, detail);
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

struct GuardedOut {
	unsigned char data[OUT_TOTAL];
};

void
guard_fill(GuardedOut *g)
{
	std::memset(g->data, GUARD, sizeof(g->data));
}

bool
guard_eq(const GuardedOut *a, const GuardedOut *b)
{
	return std::memcmp(a->data, b->data, sizeof(a->data)) == 0;
}

unsigned char *
out_user(GuardedOut *g)
{
	return g->data + OUT_PRE;
}

int saved_stdout = -1;

bool
push_stdout_pipe(int pfd[2])
{
	if (pipe(pfd) != 0)
		return false;
	if (saved_stdout < 0)
		saved_stdout = dup(STDOUT_FILENO);
	if (dup2(pfd[1], STDOUT_FILENO) < 0)
		return false;
	close(pfd[1]);
	std::setvbuf(stdout, nullptr, _IONBF, 0);
	fwide(stdout, 1);
	return true;
}

void
pop_stdout(int pfd_read)
{
	std::fflush(stdout);
	if (saved_stdout >= 0) {
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
		saved_stdout = -1;
	}
	if (pfd_read >= 0)
		close(pfd_read);
	clearerr(stdout);
	fwide(stdout, 1);
}

struct WprintfObs {
	int ret;
	GuardedOut out;
};

WprintfObs
run_ref_wprintf(const wchar_t *fmt, ...)
{
	WprintfObs obs{};
	int pfd[2];
	ssize_t n;
	va_list ap;

	guard_fill(&obs.out);
	if (!push_stdout_pipe(pfd))
		return obs;

	va_start(ap, fmt);
	obs.ret = ref_wprintf(fmt, ap);
	va_end(ap);

	std::fflush(stdout);
	n = read(pfd[0], out_user(&obs.out), OUT_CAP);
	if (n > 0)
		(void)n;
	pop_stdout(pfd[0]);
	return obs;
}

WprintfObs
run_port_wprintf(const wchar_t *fmt, ...)
{
	WprintfObs obs{};
	int pfd[2];
	ssize_t n;
	va_list ap;

	guard_fill(&obs.out);
	if (!push_stdout_pipe(pfd))
		return obs;

	va_start(ap, fmt);
	obs.ret = port::wprintf(fmt, ap);
	va_end(ap);

	std::fflush(stdout);
	n = read(pfd[0], out_user(&obs.out), OUT_CAP);
	if (n > 0)
		(void)n;
	pop_stdout(pfd[0]);
	return obs;
}

WprintfObs
run_ref_wprintf_l(locale_t loc, const wchar_t *fmt, ...)
{
	WprintfObs obs{};
	int pfd[2];
	ssize_t n;
	va_list ap;

	guard_fill(&obs.out);
	if (!push_stdout_pipe(pfd))
		return obs;

	va_start(ap, fmt);
	obs.ret = ref_wprintf_l(loc, fmt, ap);
	va_end(ap);

	std::fflush(stdout);
	n = read(pfd[0], out_user(&obs.out), OUT_CAP);
	if (n > 0)
		(void)n;
	pop_stdout(pfd[0]);
	return obs;
}

WprintfObs
run_port_wprintf_l(locale_t loc, const wchar_t *fmt, ...)
{
	WprintfObs obs{};
	int pfd[2];
	ssize_t n;
	va_list ap;

	guard_fill(&obs.out);
	if (!push_stdout_pipe(pfd))
		return obs;

	va_start(ap, fmt);
	obs.ret = port::wprintf_l(loc, fmt, ap);
	va_end(ap);

	std::fflush(stdout);
	n = read(pfd[0], out_user(&obs.out), OUT_CAP);
	if (n > 0)
		(void)n;
	pop_stdout(pfd[0]);
	return obs;
}

bool
test_wprintf_pair(StatId which, const char *label, locale_t loc, int use_l,
    const wchar_t *fmt, ...)
{
	WprintfObs r, p;
	va_list ap1, ap2;
	bool ok = true;

	guard_fill(&r.out);
	guard_fill(&p.out);

	if (use_l) {
		va_start(ap1, fmt);
		r = run_ref_wprintf_l(loc, fmt, ap1);
		va_end(ap1);
		va_start(ap2, fmt);
		p = run_port_wprintf_l(loc, fmt, ap2);
		va_end(ap2);
	} else {
		va_start(ap1, fmt);
		r = run_ref_wprintf(fmt, ap1);
		va_end(ap1);
		va_start(ap2, fmt);
		p = run_port_wprintf(fmt, ap2);
		va_end(ap2);
	}

	case_inc(which);
	if (r.ret != p.ret) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!guard_eq(&r.out, &p.out)) {
		fail_msg(which, label, "output/guard mismatch");
		ok = false;
	}
	return ok;
}

void
run_wprintf_edges(StatId which, locale_t loc, int use_l)
{
	static const wchar_t ws[] = { L'h', L'i', 0 };
	static const wchar_t wempty[] = { 0 };
	static const wchar_t whi[] = { (wchar_t)0x80, (wchar_t)0xff, 0 };
	static const wchar_t wnul[] = { L'a', L'\0', L'b', 0 };

	test_wprintf_pair(which, "empty", loc, use_l, L"");
	test_wprintf_pair(which, "pct", loc, use_l, L"%%");
	test_wprintf_pair(which, "int0", loc, use_l, L"%d", 0);
	test_wprintf_pair(which, "int-1", loc, use_l, L"%d", -1);
	test_wprintf_pair(which, "intmax", loc, use_l, L"%d", INT_MAX);
	test_wprintf_pair(which, "intmin", loc, use_l, L"%d", INT_MIN);
	test_wprintf_pair(which, "char", loc, use_l, L"%c", L'Z');
	test_wprintf_pair(which, "charhi", loc, use_l, L"%c", (wchar_t)0xff);
	test_wprintf_pair(which, "str", loc, use_l, L"%ls", ws);
	test_wprintf_pair(which, "strempty", loc, use_l, L"%ls", wempty);
	test_wprintf_pair(which, "strhi", loc, use_l, L"%ls", whi);
	test_wprintf_pair(which, "strnul", loc, use_l, L"%ls", wnul);
	test_wprintf_pair(which, "width", loc, use_l, L"%5d", 42);
	test_wprintf_pair(which, "hex", loc, use_l, L"%x", 0xdead);
	test_wprintf_pair(which, "mix", loc, use_l, L"%d %ls %c", 7, ws, L'!');
}

void
run_wprintf_random(StatId which, locale_t loc, int use_l)
{
	wchar_t fmt[32];
	wchar_t str[64];
	int val;

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		unsigned kind = rnd_u32() % 8;
		char label[32];

		std::snprintf(label, sizeof(label), "rnd%u", i);
		switch (kind) {
		case 0:
			test_wprintf_pair(which, label, loc, use_l, L"%d",
			    rnd_i32());
			break;
		case 1:
			test_wprintf_pair(which, label, loc, use_l, L"%c",
			    (wchar_t)(rnd_u32() & 0xffff));
			break;
		case 2:
			for (std::size_t j = 0; j < 8; j++)
				str[j] = (wchar_t)(rnd_u32() & 0xff);
			str[8] = 0;
			test_wprintf_pair(which, label, loc, use_l, L"%ls", str);
			break;
		case 3:
			test_wprintf_pair(which, label, loc, use_l, L"%%");
			break;
		case 4:
			test_wprintf_pair(which, label, loc, use_l, L"");
			break;
		case 5:
			val = rnd_i32();
			test_wprintf_pair(which, label, loc, use_l, L"%#x", val);
			break;
		case 6:
			test_wprintf_pair(which, label, loc, use_l, L"%d %d",
			    rnd_i32(), rnd_i32());
			break;
		default:
			std::swprintf(fmt, 32, L"%%%du", (int)(rnd_u32() % 20));
			test_wprintf_pair(which, label, loc, use_l, fmt,
			    rnd_i32());
			break;
		}
	}
}

FILE *
make_temp_copy(const unsigned char *data, std::size_t len, const char *tag)
{
	char path[] = "/tmp/pbsd_b0100_XXXXXX";
	int fd;
	FILE *fp;

	(void)tag;
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

struct StreamObs {
	int ret;
	int feof_u;
	int ferror_u;
	long pos;
	unsigned char tail[64];
};

void
guard_tail(unsigned char *t)
{
	std::memset(t, GUARD, 64);
}

StreamObs
observe_stream(FILE *fp)
{
	StreamObs o{};

	guard_tail(o.tail);
	o.feof_u = feof_unlocked(fp);
	o.ferror_u = ferror_unlocked(fp);
	o.pos = ftell(fp);
	if (o.pos >= 0)
		(void)fread(o.tail, 1, sizeof(o.tail), fp);
	return o;
}

void
prep_stream(FILE *fp, const unsigned char *data, std::size_t len,
    std::size_t read_count, int set_error, int at_eof)
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
		c = fgetc(fp);
		if (c == EOF)
			break;
	}
	if (at_eof) {
		while (fgetc(fp) != EOF)
			(void)0;
	}
	if (set_error) {
		(void)fputc('x', fp);
		clearerr_unlocked(fp);
		if (at_eof) {
			while (fgetc(fp) != EOF)
				(void)0;
		}
		(void)fputc('y', fp);
	}
}

StreamObs
call_ref_feof(FILE *fp)
{
	StreamObs o{};

	o.ret = ref_feof(fp);
	o = observe_stream(fp);
	o.ret = ref_feof(fp);
	return o;
}

StreamObs
call_port_feof(FILE *fp)
{
	StreamObs o{};

	o.ret = port::feof(fp);
	o = observe_stream(fp);
	o.ret = port::feof(fp);
	return o;
}

StreamObs
call_ref_feof_unlocked(FILE *fp)
{
	StreamObs o{};

	o.ret = ref_feof_unlocked(fp);
	o.feof_u = feof_unlocked(fp);
	o.ferror_u = ferror_unlocked(fp);
	o.pos = ftell(fp);
	guard_tail(o.tail);
	if (o.pos >= 0)
		(void)fread(o.tail, 1, sizeof(o.tail), fp);
	return o;
}

StreamObs
call_port_feof_unlocked(FILE *fp)
{
	StreamObs o{};

	o.ret = port::feof_unlocked(fp);
	o.feof_u = feof_unlocked(fp);
	o.ferror_u = ferror_unlocked(fp);
	o.pos = ftell(fp);
	guard_tail(o.tail);
	if (o.pos >= 0)
		(void)fread(o.tail, 1, sizeof(o.tail), fp);
	return o;
}

void
call_ref_clearerr(FILE *fp)
{
	ref_clearerr(fp);
}

void
call_port_clearerr(FILE *fp)
{
	port::clearerr(fp);
}

void
call_ref_clearerr_unlocked(FILE *fp)
{
	ref_clearerr_unlocked(fp);
}

void
call_port_clearerr_unlocked(FILE *fp)
{
	port::clearerr_unlocked(fp);
}

bool
stream_obs_eq(const StreamObs *a, const StreamObs *b)
{
	if (a->ret != b->ret)
		return false;
	if (a->feof_u != b->feof_u)
		return false;
	if (a->ferror_u != b->ferror_u)
		return false;
	if (a->pos != b->pos)
		return false;
	if (std::memcmp(a->tail, b->tail, sizeof(a->tail)) != 0)
		return false;
	return true;
}

bool
test_feof_case(StatId which, const char *label, const unsigned char *data,
    std::size_t len, std::size_t read_count, int at_eof, int use_unlocked)
{
	FILE *ra = make_temp_copy(data, len, label);
	FILE *pa = make_temp_copy(data, len, label);
	StreamObs r, p;
	bool ok = true;

	if (ra == nullptr || pa == nullptr) {
		std::fprintf(stderr, "harness bug: temp file for feof\n");
		std::exit(2);
	}

	prep_stream(ra, data, len, read_count, 0, at_eof);
	prep_stream(pa, data, len, read_count, 0, at_eof);

	if (use_unlocked) {
		r = call_ref_feof_unlocked(ra);
		p = call_port_feof_unlocked(pa);
	} else {
		r.ret = ref_feof(ra);
		p.ret = port::feof(pa);
		r.feof_u = feof_unlocked(ra);
		p.feof_u = feof_unlocked(pa);
		r.ferror_u = ferror_unlocked(ra);
		p.ferror_u = ferror_unlocked(pa);
		r.pos = ftell(ra);
		p.pos = ftell(pa);
		guard_tail(r.tail);
		guard_tail(p.tail);
		if (r.pos >= 0)
			(void)fread(r.tail, 1, sizeof(r.tail), ra);
		if (p.pos >= 0)
			(void)fread(p.tail, 1, sizeof(p.tail), pa);
	}

	case_inc(which);
	if (!stream_obs_eq(&r, &p)) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu rd=%zu eof=%d ret %d/%d feof %d/%d",
		    len, read_count, at_eof, r.ret, p.ret, r.feof_u, p.feof_u);
		fail_msg(which, label, detail);
		ok = false;
	}

	fclose(ra);
	fclose(pa);
	return ok;
}

bool
test_clearerr_case(StatId which, const char *label, const unsigned char *data,
    std::size_t len, std::size_t read_count, int set_error, int at_eof,
    int use_unlocked)
{
	FILE *ra = make_temp_copy(data, len, label);
	FILE *pa = make_temp_copy(data, len, label);
	StreamObs r, p;
	bool ok = true;

	if (ra == nullptr || pa == nullptr) {
		std::fprintf(stderr, "harness bug: temp file for clearerr\n");
		std::exit(2);
	}

	prep_stream(ra, data, len, read_count, set_error, at_eof);
	prep_stream(pa, data, len, read_count, set_error, at_eof);

	if (use_unlocked) {
		call_ref_clearerr_unlocked(ra);
		call_port_clearerr_unlocked(pa);
	} else {
		call_ref_clearerr(ra);
		call_port_clearerr(pa);
	}

	r.feof_u = feof_unlocked(ra);
	p.feof_u = feof_unlocked(pa);
	r.ferror_u = ferror_unlocked(ra);
	p.ferror_u = ferror_unlocked(pa);
	r.pos = ftell(ra);
	p.pos = ftell(pa);
	guard_tail(r.tail);
	guard_tail(p.tail);
	if (r.pos >= 0)
		(void)fread(r.tail, 1, sizeof(r.tail), ra);
	if (p.pos >= 0)
		(void)fread(p.tail, 1, sizeof(p.tail), pa);

	case_inc(which);
	if (!stream_obs_eq(&r, &p)) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu rd=%zu err=%d eof=%d feof %d/%d errf %d/%d",
		    len, read_count, set_error, at_eof, r.feof_u, p.feof_u,
		    r.ferror_u, p.ferror_u);
		fail_msg(which, label, detail);
		ok = false;
	}

	fclose(ra);
	fclose(pa);
	return ok;
}

void
run_feof_edges(StatId which, int use_unlocked)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one[] = { 'x' };
	static const unsigned char hi[] = { 0x00, 0x7f, 0x80, 0xff };
	unsigned char buf[FILEBUF];
	std::size_t i;

	for (i = 0; i < sizeof(buf); i++)
		buf[i] = (unsigned char)(0x80 + (i & 0x7f));

	test_feof_case(which, "empty@0", empty, 0, 0, 0, use_unlocked);
	test_feof_case(which, "empty@eof", empty, 0, 0, 1, use_unlocked);
	test_feof_case(which, "one@0", one, 1, 0, 0, use_unlocked);
	test_feof_case(which, "one@1", one, 1, 1, 0, use_unlocked);
	test_feof_case(which, "one@eof", one, 1, 0, 1, use_unlocked);
	test_feof_case(which, "hi@0", hi, sizeof(hi), 0, 0, use_unlocked);
	test_feof_case(which, "hi@2", hi, sizeof(hi), 2, 0, use_unlocked);
	test_feof_case(which, "hi@eof", hi, sizeof(hi), 0, 1, use_unlocked);
	test_feof_case(which, "big@0", buf, sizeof(buf), 0, 0, use_unlocked);
	test_feof_case(which, "big@mid", buf, sizeof(buf), 127, 0, use_unlocked);
	test_feof_case(which, "big@eof", buf, sizeof(buf), 0, 1, use_unlocked);
}

void
run_feof_random(StatId which, int use_unlocked)
{
	unsigned char data[FILEBUF];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t len = rnd_u32() % (sizeof(data) + 1);
		std::size_t read_count = rnd_u32() % (len + 5);
		int at_eof = (int)(rnd_u32() & 1);
		char label[32];

		for (std::size_t j = 0; j < len; j++)
			data[j] = (unsigned char)rnd_u32();
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_feof_case(which, label, data, len, read_count, at_eof,
		    use_unlocked);
	}
}

void
run_clearerr_edges(StatId which, int use_unlocked)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one[] = { 'x' };
	static const unsigned char hi[] = { 0x80, 0xff, 0x00, 0xfe };
	unsigned char buf[FILEBUF];
	std::size_t i;

	for (i = 0; i < sizeof(buf); i++)
		buf[i] = (unsigned char)(0x80 + (i & 0x7f));

	test_clearerr_case(which, "empty", empty, 0, 0, 0, 0, use_unlocked);
	test_clearerr_case(which, "empty+eof", empty, 0, 0, 0, 1, use_unlocked);
	test_clearerr_case(which, "one@0", one, 1, 0, 0, 0, use_unlocked);
	test_clearerr_case(which, "one@eof", one, 1, 0, 0, 1, use_unlocked);
	test_clearerr_case(which, "one@err", one, 1, 0, 1, 0, use_unlocked);
	test_clearerr_case(which, "one@both", one, 1, 0, 1, 1, use_unlocked);
	test_clearerr_case(which, "hi@eof", hi, sizeof(hi), 0, 0, 1,
	    use_unlocked);
	test_clearerr_case(which, "hi@err", hi, sizeof(hi), 2, 1, 0,
	    use_unlocked);
	test_clearerr_case(which, "big@eof", buf, sizeof(buf), 0, 0, 1,
	    use_unlocked);
	test_clearerr_case(which, "big@err", buf, sizeof(buf), 255, 1, 0,
	    use_unlocked);
}

void
run_clearerr_random(StatId which, int use_unlocked)
{
	unsigned char data[FILEBUF];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t len = rnd_u32() % (sizeof(data) + 1);
		std::size_t read_count = rnd_u32() % (len + 5);
		int set_error = (int)(rnd_u32() & 1);
		int at_eof = (int)(rnd_u32() & 1);
		char label[32];

		for (std::size_t j = 0; j < len; j++)
			data[j] = (unsigned char)rnd_u32();
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_clearerr_case(which, label, data, len, read_count,
		    set_error, at_eof, use_unlocked);
	}
}

} /* namespace */

int
main(void)
{
	locale_t loc;
	long long total_fails = 0;
	int rc = 0;
	int saved_isthreaded = __isthreaded;

	setlocale(LC_ALL, "C.UTF-8");
	loc = newlocale(LC_ALL_MASK, "C.UTF-8", nullptr);
	if (loc == nullptr)
		loc = duplocale(LC_GLOBAL_LOCALE);

	run_wprintf_edges(S_WPRINTF, loc, 0);
	run_wprintf_random(S_WPRINTF, loc, 0);
	run_wprintf_edges(S_WPRINTF_L, loc, 1);
	run_wprintf_random(S_WPRINTF_L, loc, 1);

	__isthreaded = 0;
	run_feof_edges(S_FEOF, 0);
	run_feof_random(S_FEOF, 0);
	run_feof_edges(S_FEOF_UNLOCKED, 1);
	run_feof_random(S_FEOF_UNLOCKED, 1);

	run_clearerr_edges(S_CLEARERR, 0);
	run_clearerr_random(S_CLEARERR, 0);
	run_clearerr_edges(S_CLEARERR_UNLOCKED, 1);
	run_clearerr_random(S_CLEARERR_UNLOCKED, 1);

	__isthreaded = 1;
	run_feof_edges(S_FEOF, 0);
	run_clearerr_edges(S_CLEARERR, 0);

	__isthreaded = saved_isthreaded;

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	std::printf("\n");
	std::printf("+----------------------+----------+----------+\n");
	std::printf("| function             |     cases|    fails |\n");
	std::printf("+----------------------+----------+----------+\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("| %-20s | %9lld| %9lld|\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		total_fails += g_stat[i].fails;
	}
	std::printf("+----------------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
