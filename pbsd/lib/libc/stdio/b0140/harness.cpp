/*
 * harness.cpp -- differential test for PBSD batch b0140.
 */

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#if __has_include(<xlocale.h>)
#include <xlocale.h>
#else
#include <locale.h>
#endif

import pbsd.lib.libc.stdio.b0140;

namespace P = pbsd::lib_libc_stdio::b0140;

extern "C" {
int	ref_putchar(int);
int	ref_putchar_unlocked(int);
int	ref_vdprintf(int, const char * __restrict, va_list);
int	ref___printf_arginfo_errno(const P::printf_info *, size_t, int *);
int	ref___printf_render_errno(P::__printf_io *,
    const P::printf_info *, const void *const *);
int	ref_printf(char const * __restrict, ...);
int	ref_printf_l(locale_t, char const * __restrict, ...);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t PRE = 16;
constexpr std::size_t USER = 512;
constexpr std::size_t POST = 16;
constexpr std::size_t ARENA = PRE + USER + POST;
constexpr int MAXPRINT = 8;
constexpr long RAND_ITERS = 200000;
constexpr int B0140_SYS_NERR = 134;

struct Stats {
	const char *name;
	long cases;
	long fails;
	int printed;
};

Stats g_stat[] = {
	{ "putchar", 0, 0, 0 },
	{ "putchar_unlocked", 0, 0, 0 },
	{ "vdprintf", 0, 0, 0 },
	{ "__printf_arginfo_errno", 0, 0, 0 },
	{ "__printf_render_errno", 0, 0, 0 },
	{ "printf", 0, 0, 0 },
	{ "printf_l", 0, 0, 0 },
};

std::uint64_t rng_state = 0xb0140facefeedULL;

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
fail_msg(int idx, const char *tag, const char *detail)
{
	Stats *st = &g_stat[idx];

	st->fails++;
	if (st->printed++ < MAXPRINT)
		std::printf("  FAIL %-24s %-16s %s\n", st->name, tag, detail);
}

void
case_inc(int idx)
{
	g_stat[idx].cases++;
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

/* ------------------------------------------------------------------------ */
/* putchar                                                                   */
/* ------------------------------------------------------------------------ */

struct PutcharObs {
	int ret;
	GuardArena out;
};

static PutcharObs
run_putchar_fn(int (*fn)(int), int c)
{
	PutcharObs obs{};
	char path[] = "/tmp/pbsd_b0140_pc_XXXXXX";
	int fd, saved_stdout;
	FILE *cap;

	obs.out.fill();
	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	obs.ret = fn(c);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(obs.out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return obs;
}

static void
putchar_case(int idx, int use_unlocked, int c, const char *tag)
{
	PutcharObs a, b;
	int bad;

	if (use_unlocked) {
		a = run_putchar_fn(ref_putchar_unlocked, c);
		b = run_putchar_fn(
		    [](int ch) -> int { return P::putchar_unlocked(ch); }, c);
	} else {
		a = run_putchar_fn(ref_putchar, c);
		b = run_putchar_fn(
		    [](int ch) -> int { return P::putchar(ch); }, c);
	}

	case_inc(idx);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (!a.out.eq(b.out))
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail), "c=%d ret=%d/%d", c,
		    a.ret, b.ret);
		fail_msg(idx, tag, detail);
	}
}

static void
putchar_edges(int idx, int use_unlocked)
{
	putchar_case(idx, use_unlocked, 'a', "a");
	putchar_case(idx, use_unlocked, 0, "nul");
	putchar_case(idx, use_unlocked, 0x7f, "0x7f");
	putchar_case(idx, use_unlocked, 0x80, "0x80");
	putchar_case(idx, use_unlocked, 0xff, "0xff");
	putchar_case(idx, use_unlocked, -1, "neg1");
	putchar_case(idx, use_unlocked, EOF, "eof");
}

static void
putchar_random(int idx, int use_unlocked, long n)
{
	for (long t = 0; t < n; t++) {
		int c = (int)(rnd_u32() & 0xff);
		if (rnd_u32() & 1)
			c = (int)((signed char)c);
		putchar_case(idx, use_unlocked, c, "random");
	}
}

/* ------------------------------------------------------------------------ */
/* vdprintf                                                                  */
/* ------------------------------------------------------------------------ */

struct VdprintfObs {
	int ret;
	int saved_errno;
	unsigned char out[USER];
	unsigned char tail[32];
};

static int
make_pipe_rw(int *readfd, int *writefd)
{
	int fds[2];

	if (pipe(fds) != 0)
		return -1;
	*readfd = fds[0];
	*writefd = fds[1];
	return 0;
}

static VdprintfObs
run_vdprintf_ref(int readfd, int writefd, const char *fmt, va_list ap)
{
	VdprintfObs obs{};
	va_list ap2;
	int ret;

	std::memset(obs.out, GUARD, sizeof(obs.out));
	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	va_copy(ap2, ap);
	errno = 0;
	ret = ref_vdprintf(writefd, fmt, ap2);
	va_end(ap2);
	obs.ret = ret;
	obs.saved_errno = errno;
	if (writefd >= 0 && writefd <= SHRT_MAX)
		close(writefd);
	if (readfd >= 0 && readfd <= SHRT_MAX) {
		ssize_t n;

		n = read(readfd, obs.out, sizeof(obs.out));
		if (n > 0)
			(void)read(readfd, obs.tail, sizeof(obs.tail));
		close(readfd);
	}
	return obs;
}

static VdprintfObs
run_vdprintf_port(int readfd, int writefd, const char *fmt, va_list ap)
{
	VdprintfObs obs{};
	va_list ap2;
	int ret;

	std::memset(obs.out, GUARD, sizeof(obs.out));
	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	va_copy(ap2, ap);
	errno = 0;
	ret = P::vdprintf(writefd, fmt, ap2);
	va_end(ap2);
	obs.ret = ret;
	obs.saved_errno = errno;
	if (writefd >= 0 && writefd <= SHRT_MAX)
		close(writefd);
	if (readfd >= 0 && readfd <= SHRT_MAX) {
		ssize_t n;

		n = read(readfd, obs.out, sizeof(obs.out));
		if (n > 0)
			(void)read(readfd, obs.tail, sizeof(obs.tail));
		close(readfd);
	}
	return obs;
}

static void
vdprintf_compare(const VdprintfObs &a, const VdprintfObs &b,
    const char *tag, const char *detail)
{
	int bad = 0;

	case_inc(2);
	if (a.ret != b.ret)
		bad = 1;
	if (a.saved_errno != b.saved_errno)
		bad = 1;
	if (std::memcmp(a.out, b.out, sizeof(a.out)) != 0)
		bad = 1;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad)
		fail_msg(2, tag, detail);
}

static void
vdprintf_case_va(int readfd, int writefd, const char *fmt, va_list ap,
    const char *tag, const char *detail)
{
	va_list ap_a, ap_b;
	VdprintfObs a, b;

	va_copy(ap_a, ap);
	va_copy(ap_b, ap);
	a = run_vdprintf_ref(readfd, writefd, fmt, ap_a);
	b = run_vdprintf_port(readfd, writefd, fmt, ap_b);
	va_end(ap_a);
	va_end(ap_b);
	vdprintf_compare(a, b, tag, detail);
}

static void
vdprintf_case_pipes(const char *tag, const char *fmt, ...)
{
	int rfd, wfd;
	va_list ap;
	char detail[160];

	if (make_pipe_rw(&rfd, &wfd) != 0)
		return;
	va_start(ap, fmt);
	std::snprintf(detail, sizeof(detail), "%s fmt='%s'", tag, fmt);
	vdprintf_case_va(rfd, wfd, fmt, ap, tag, detail);
	va_end(ap);
}

static void
vdprintf_case_overflow(int fd, const char *tag, const char *fmt, ...)
{
	va_list ap;
	char detail[160];

	va_start(ap, fmt);
	std::snprintf(detail, sizeof(detail), "%s fd=%d", tag, fd);
	vdprintf_case_va(-1, fd, fmt, ap, tag, detail);
	va_end(ap);
}

static void
vdprintf_edges(void)
{
	char big[BUFSIZ + 64];
	std::size_t i;

	vdprintf_case_pipes("empty", "");
	vdprintf_case_pipes("pct", "%%");
	vdprintf_case_pipes("d0", "%d", 0);
	vdprintf_case_pipes("d-1", "%d", -1);
	vdprintf_case_pipes("dmax", "%d", INT_MAX);
	vdprintf_case_pipes("dmin", "%d", INT_MIN);
	vdprintf_case_pipes("c80", "%c", 0x80);
	vdprintf_case_pipes("cff", "%c", 0xff);
	vdprintf_case_pipes("s-empty", "%s", "");
	vdprintf_case_pipes("s-hi", "%s", "\x80\xff");
	vdprintf_case_pipes("mix", "%d %u %x", 42, 7u, 0xab);

	for (i = 0; i < sizeof(big) - 1; i++)
		big[i] = (char)('A' + (i % 26));
	big[sizeof(big) - 1] = '\0';
	vdprintf_case_pipes("big", "%s", big);

	vdprintf_case_overflow(SHRT_MAX, "shrt_max", "%d", 1);
	vdprintf_case_overflow(SHRT_MAX + 1, "shrt_max+1", "%d", 1);
	vdprintf_case_overflow(-1, "badfd", "%d", 1);
}

static void
vdprintf_random(long n)
{
	char str[128];

	for (long t = 0; t < n; t++) {
		unsigned pick = rnd_u32() % 10u;

		if (pick < 8u) {
			if (make_pipe_rw(nullptr, nullptr) != 0)
				continue;
			/* make_pipe_rw needs valid pointers - fix below */
		}

		int rfd, wfd;

		if (make_pipe_rw(&rfd, &wfd) != 0)
			continue;

		switch (pick) {
		case 0:
			vdprintf_case_pipes("random", "");
			break;
		case 1:
			vdprintf_case_pipes("random", "%d",
			    (int)((int)rnd_u32() ^ (int)(rnd_u32() & 1 ? 0 :
			    INT_MIN)));
			break;
		case 2:
			vdprintf_case_pipes("random", "%u", rnd_u32());
			break;
		case 3:
			vdprintf_case_pipes("random", "%x", rnd_u32());
			break;
		case 4:
			vdprintf_case_pipes("random", "%c",
			    (int)(rnd_u32() & 0xff));
			break;
		case 5: {
			std::size_t slen = rnd_mod(sizeof(str));
			for (std::size_t j = 0; j < slen; j++)
				str[j] = (char)(unsigned char)(rnd_u32() & 0xff);
			str[slen] = '\0';
			vdprintf_case_pipes("random", "%s", str);
			break;
		}
		default:
			vdprintf_case_pipes("random", "%d %u %c",
			    (int)rnd_u32(), rnd_u32(),
			    (int)(rnd_u32() & 0xff));
			break;
		}

		close(wfd);
		close(rfd);

		if (rnd_u32() & 1u) {
			int fd = SHRT_MAX + (int)(rnd_u32() % 4u);
			vdprintf_case_overflow(fd, "random", "%d",
			    (int)rnd_u32());
		}
	}
}

/* ------------------------------------------------------------------------ */
/* __printf_arginfo_errno / __printf_render_errno                            */
/* ------------------------------------------------------------------------ */

struct ArginfoObs {
	int ret;
	int argt[8];
	unsigned char tail[32];
};

static ArginfoObs
run_arginfo_ref(size_t n, int *argt)
{
	ArginfoObs obs{};
	P::printf_info pi{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	obs.ret = ref___printf_arginfo_errno(&pi, n, argt);
	if (n < 8)
		std::memcpy(obs.tail, argt + n,
		    sizeof(obs.tail));
	return obs;
}

static ArginfoObs
run_arginfo_port(size_t n, int *argt)
{
	ArginfoObs obs{};
	P::printf_info pi{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	obs.ret = P::__printf_arginfo_errno(&pi, n, argt);
	if (n < 8)
		std::memcpy(obs.tail, argt + n, sizeof(obs.tail));
	return obs;
}

static void
arginfo_case(size_t n, const char *tag)
{
	int argt_a[8], argt_b[8];
	ArginfoObs a, b;
	int bad;

	for (int i = 0; i < 8; i++) {
		argt_a[i] = (int)GUARD;
		argt_b[i] = (int)GUARD;
	}
	a = run_arginfo_ref(n, argt_a);
	b = run_arginfo_port(n, argt_b);

	case_inc(3);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (argt_a[0] != argt_b[0])
		bad = 1;
	for (int i = 1; i < 8; i++) {
		if (argt_a[i] != argt_b[i])
			bad = 1;
	}
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "n=%zu ret=%d/%d argt=%d/%d", n, a.ret, b.ret,
		    argt_a[0], argt_b[0]);
		fail_msg(3, tag, detail);
	}
}

static void
arginfo_edges(void)
{
	arginfo_case(1, "n1");
	arginfo_case(2, "n2");
	arginfo_case(4, "n4");
	arginfo_case(8, "n8");
}

static void
arginfo_random(long n)
{
	for (long t = 0; t < n; t++)
		arginfo_case(1 + rnd_mod(7), "random");
}

struct RenderObs {
	int ret;
	int flush_count;
	GuardArena out;
};

static RenderObs
run_render_ref(int error, const P::printf_info *pi)
{
	RenderObs obs{};
	P::__printf_io io{};
	const void *arg[1];
	int errcopy = error;

	io.out = obs.out.bytes + PRE;
	io.out_cap = USER;
	io.out_len = 0;
	io.flush_count = 0;
	obs.out.fill();
	arg[0] = &errcopy;
	obs.ret = ref___printf_render_errno(&io, pi, arg);
	obs.flush_count = io.flush_count;
	return obs;
}

static RenderObs
run_render_port(int error, const P::printf_info *pi)
{
	RenderObs obs{};
	P::__printf_io io{};
	const void *arg[1];
	int errcopy = error;

	io.out = obs.out.bytes + PRE;
	io.out_cap = USER;
	io.out_len = 0;
	io.flush_count = 0;
	obs.out.fill();
	arg[0] = &errcopy;
	obs.ret = P::__printf_render_errno(&io, pi, arg);
	obs.flush_count = io.flush_count;
	return obs;
}

static void
render_case(int error, const P::printf_info *pi, const char *tag)
{
	RenderObs a, b;
	int bad;

	a = run_render_ref(error, pi);
	b = run_render_port(error, pi);

	case_inc(4);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.flush_count != b.flush_count)
		bad = 1;
	if (!a.out.eq(b.out))
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "err=%d ret=%d/%d flush=%d/%d", error, a.ret, b.ret,
		    a.flush_count, b.flush_count);
		fail_msg(4, tag, detail);
	}
}

static void
render_edges(void)
{
	P::printf_info zpi{};
	P::printf_info wpi{};

	std::memset(&zpi, 0, sizeof(zpi));
	std::memset(&wpi, 0, sizeof(wpi));
	wpi.width = 20;
	wpi.pad = '0';
	wpi.left = 0;

	render_case(-1, &zpi, "neg1");
	render_case(0, &zpi, "zero");
	render_case(1, &zpi, "one");
	render_case(EINVAL, &zpi, "einval");
	render_case(B0140_SYS_NERR - 1, &zpi, "nerr-1");
	render_case(B0140_SYS_NERR, &zpi, "nerr");
	render_case(B0140_SYS_NERR + 1, &zpi, "nerr+1");
	render_case(INT_MAX, &zpi, "max");
	render_case(INT_MIN, &zpi, "min");
	render_case(999, &wpi, "fallback-pad");
	render_case(-5, &wpi, "fallback-neg");
}

static void
render_random(long n)
{
	P::printf_info pi{};

	for (long t = 0; t < n; t++) {
		int error;
		unsigned pick = rnd_u32() % 6u;

		std::memset(&pi, 0, sizeof(pi));
		if (rnd_u32() & 1u) {
			pi.width = (int)(rnd_u32() % 32u);
			pi.pad = (rnd_u32() & 1u) ? '0' : ' ';
			pi.left = rnd_u32() & 1u;
		}

		switch (pick) {
		case 0:
			error = (int)(rnd_u32() % (unsigned)B0140_SYS_NERR);
			break;
		case 1:
			error = B0140_SYS_NERR + (int)(rnd_u32() % 1000u);
			break;
		case 2:
			error = -(int)(rnd_u32() % 1000u);
			break;
		case 3:
			error = INT_MAX;
			break;
		case 4:
			error = INT_MIN;
			break;
		default:
			error = (int)rnd_u32();
			break;
		}
		render_case(error, &pi, "random");
	}
}

/* ------------------------------------------------------------------------ */
/* printf / printf_l                                                         */
/* ------------------------------------------------------------------------ */

struct PrintfObs {
	int ret;
	GuardArena out;
};

static PrintfObs
capture_printf_ref(const char *fmt, va_list ap)
{
	PrintfObs obs{};
	char path[] = "/tmp/pbsd_b0140_pr_XXXXXX";
	int fd, saved_stdout;
	va_list ap2;
	FILE *cap;

	obs.out.fill();
	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	va_copy(ap2, ap);
	obs.ret = ref_printf(fmt, ap2);
	va_end(ap2);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(obs.out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return obs;
}

static PrintfObs
capture_printf_port(const char *fmt, va_list ap)
{
	PrintfObs obs{};
	char path[] = "/tmp/pbsd_b0140_pr_XXXXXX";
	int fd, saved_stdout;
	va_list ap2;
	FILE *cap;

	obs.out.fill();
	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	va_copy(ap2, ap);
	obs.ret = P::printf(fmt, ap2);
	va_end(ap2);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(obs.out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return obs;
}

static PrintfObs
capture_printf_l_ref(locale_t loc, const char *fmt, va_list ap)
{
	PrintfObs obs{};
	char path[] = "/tmp/pbsd_b0140_pl_XXXXXX";
	int fd, saved_stdout;
	va_list ap2;
	FILE *cap;

	obs.out.fill();
	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	va_copy(ap2, ap);
	obs.ret = ref_printf_l(loc, fmt, ap2);
	va_end(ap2);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(obs.out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return obs;
}

static PrintfObs
capture_printf_l_port(locale_t loc, const char *fmt, va_list ap)
{
	PrintfObs obs{};
	char path[] = "/tmp/pbsd_b0140_pl_XXXXXX";
	int fd, saved_stdout;
	va_list ap2;
	FILE *cap;

	obs.out.fill();
	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		obs.ret = -9998;
		return obs;
	}
	va_copy(ap2, ap);
	obs.ret = P::printf_l(loc, fmt, ap2);
	va_end(ap2);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(obs.out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return obs;
}

static void
printf_compare(int idx, PrintfObs a, PrintfObs b, const char *tag,
    const char *detail)
{
	int bad = 0;

	case_inc(idx);
	if (a.ret != b.ret)
		bad = 1;
	if (!a.out.eq(b.out))
		bad = 1;
	if (bad)
		fail_msg(idx, tag, detail);
}

static void
printf_case_va(int idx, locale_t loc, int use_l, const char *fmt, va_list ap,
    const char *tag)
{
	va_list ap_a, ap_b;
	PrintfObs a, b;
	char detail[128];

	va_copy(ap_a, ap);
	va_copy(ap_b, ap);
	if (use_l) {
		a = capture_printf_l_ref(loc, fmt, ap_a);
		b = capture_printf_l_port(loc, fmt, ap_b);
	} else {
		a = capture_printf_ref(fmt, ap_a);
		b = capture_printf_port(fmt, ap_b);
	}
	va_end(ap_a);
	va_end(ap_b);
	std::snprintf(detail, sizeof(detail), "%s", tag);
	printf_compare(idx, a, b, tag, detail);
}

static void
printf_case(int idx, locale_t loc, int use_l, const char *tag,
    const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	printf_case_va(idx, loc, use_l, fmt, ap, tag);
	va_end(ap);
}

static void
printf_edges(locale_t loc)
{
	printf_case(5, loc, 0, "empty", "");
	printf_case(5, loc, 0, "pct", "%%");
	printf_case(5, loc, 0, "d0", "%d", 0);
	printf_case(5, loc, 0, "d-1", "%d", -1);
	printf_case(5, loc, 0, "dmax", "%d", INT_MAX);
	printf_case(5, loc, 0, "c80", "%c", 0x80);
	printf_case(5, loc, 0, "cff", "%c", 0xff);
	printf_case(5, loc, 0, "s-hi", "%s", "\x80\xff");

	printf_case(6, loc, 1, "empty", "");
	printf_case(6, loc, 1, "pct", "%%");
	printf_case(6, loc, 1, "d0", "%d", 0);
	printf_case(6, loc, 1, "d-1", "%d", -1);
	printf_case(6, loc, 1, "dmax", "%d", INT_MAX);
	printf_case(6, loc, 1, "c80", "%c", 0x80);
	printf_case(6, loc, 1, "cff", "%c", 0xff);
	printf_case(6, loc, 1, "s-hi", "%s", "\x80\xff");
}

static void
printf_random(locale_t loc, long n)
{
	char str[64];

	for (long t = 0; t < n; t++) {
		unsigned pick = rnd_u32() % 8u;

		switch (pick) {
		case 0:
			printf_case(5, loc, 0, "random", "");
			printf_case(6, loc, 1, "random", "");
			break;
		case 1:
			printf_case(5, loc, 0, "random", "%d",
			    (int)rnd_u32());
			printf_case(6, loc, 1, "random", "%d",
			    (int)rnd_u32());
			break;
		case 2:
			printf_case(5, loc, 0, "random", "%u", rnd_u32());
			printf_case(6, loc, 1, "random", "%u", rnd_u32());
			break;
		case 3:
			printf_case(5, loc, 0, "random", "%x", rnd_u32());
			printf_case(6, loc, 1, "random", "%x", rnd_u32());
			break;
		case 4:
			printf_case(5, loc, 0, "random", "%c",
			    (int)(rnd_u32() & 0xff));
			printf_case(6, loc, 1, "random", "%c",
			    (int)(rnd_u32() & 0xff));
			break;
		case 5: {
			std::size_t slen = rnd_mod(sizeof(str));
			for (std::size_t j = 0; j < slen; j++)
				str[j] = (char)(unsigned char)(rnd_u32() & 0xff);
			str[slen] = '\0';
			printf_case(5, loc, 0, "random", "%s", str);
			printf_case(6, loc, 1, "random", "%s", str);
			break;
		}
		default:
			printf_case(5, loc, 0, "random", "%d %u %c",
			    (int)rnd_u32(), rnd_u32(),
			    (int)(rnd_u32() & 0xff));
			printf_case(6, loc, 1, "random", "%d %u %c",
			    (int)rnd_u32(), rnd_u32(),
			    (int)(rnd_u32() & 0xff));
			break;
		}
	}
}

} /* namespace */

int
main(void)
{
	locale_t loc;
	long total_fails = 0;
	int rc = 0;

	setlocale(LC_ALL, "C");
	loc = newlocale(LC_ALL_MASK, "C", nullptr);
	if (loc == nullptr)
		loc = (locale_t)0;

	putchar_edges(0, 0);
	putchar_edges(1, 1);
	putchar_random(0, 0, RAND_ITERS);
	putchar_random(1, 1, RAND_ITERS);

	vdprintf_edges();
	vdprintf_random(RAND_ITERS);

	arginfo_edges();
	arginfo_random(RAND_ITERS);

	render_edges();
	render_random(RAND_ITERS);

	printf_edges(loc);
	printf_random(loc, RAND_ITERS);

	if (loc != nullptr && loc != (locale_t)0)
		freelocale(loc);

	for (int i = 0; i < 7; i++)
		total_fails += g_stat[i].fails;

	std::printf("\n");
	std::printf("+--------------------------+----------+----------+\n");
	std::printf("| function                 |     cases|    fails |\n");
	std::printf("+--------------------------+----------+----------+\n");
	for (int i = 0; i < 7; i++)
		std::printf("| %-24s | %9ld| %9ld|\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
	std::printf("+--------------------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
