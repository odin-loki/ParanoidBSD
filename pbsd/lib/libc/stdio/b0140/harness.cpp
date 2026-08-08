// harness.cpp -- differential test: PBSD b0140 port vs. the ref_ C oracle.
//
// Every ported function is exercised with hand-written edge cases and with a
// fixed-seed randomised sweep of at least 200000 iterations.  For each case
// both implementations are called and everything observable is compared:
// return values, the whole of every buffer they may write to (guard-filled
// with 0x7f, bytes past the nominal write window included), the whole of the
// __printf_io sink they emit through, the bytes they write to stdout, and -
// for the assert() in __printf_arginfo_errno() - whether the call survived at
// all, by running it in a forked child.

#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>

#include <string>

// ---------------------------------------------------------------------------
// The types and symbols that live in the private FreeBSD headers.  These
// definitions are token-identical to the ones in oracle.c and port.cppm.
// ---------------------------------------------------------------------------

enum { SINK_CAP = 1024 };

extern "C" {

struct printf_info {
	int	prec;
	int	width;
	wchar_t	spec;
	unsigned is_long_double:1;
	unsigned is_char:1;
	unsigned is_short:1;
	unsigned is_long:1;
	unsigned is_long_long:1;
	unsigned is_intmax:1;
	unsigned is_ptrdiff:1;
	unsigned is_size:1;
	unsigned alt:1;
	unsigned space:1;
	unsigned left:1;
	unsigned showsign:1;
	unsigned group:1;
	unsigned wide:1;
	unsigned vsep:2;
	wchar_t	pad;
	void	*loc;
};

// libc's __printf_io is private to xprintf.c; this is the instrumented stand
// in that both sides emit through.  Nothing but this file touches its guts.
struct __printf_io {
	unsigned char	*buf;
	size_t		len;
	int		calls;
	int		flushes;
	int		overflow;
	int		last_spec;
	int		last_width;
	int		last_prec;
	int		last_pad;
	long		requested;
};

extern const int __hidden_sys_nerr;

int ref_putchar(int c);
int ref_putchar_unlocked(int ch);
int ref___printf_arginfo_errno(const struct printf_info *pi, size_t n,
    int *argt);
int ref___printf_render_errno(struct __printf_io *io,
    const struct printf_info *pi, const void *const *arg);
int ref_printf(char const *fmt, ...);

} // extern "C"

// sys_nerr as both sides see it.  Fixed, so that the `error < nerr' boundary
// is a stable place to aim test cases at.
extern "C" const int __hidden_sys_nerr = 135;

// Stand-in for libc's __printf_out(): pads according to pi->width/left/pad
// the way the real one does (clamped, to keep the sink bounded), appends to
// the sink, and records what it was asked for.
extern "C" int
__printf_out(struct __printf_io *io, const struct printf_info *pi,
    const void *buf, size_t len)
{
	size_t pad = 0;
	unsigned char padc = ' ';
	size_t total;

	io->calls++;
	io->requested += (long)len;
	if (pi != NULL) {
		io->last_spec = (int)pi->spec;
		io->last_width = pi->width;
		io->last_prec = pi->prec;
		io->last_pad = (int)pi->pad;
		if (pi->width > 0 && (size_t)pi->width > len)
			pad = (size_t)pi->width - len;
		if (pad > 64)
			pad = 64;
		if (pi->pad != 0)
			padc = (unsigned char)pi->pad;
	}
	total = len + pad;
	if (io->len + total > (size_t)SINK_CAP - 1) {
		io->overflow++;
		return (-1);
	}
	if (pi != NULL && pi->left) {
		memcpy(io->buf + io->len, buf, len);
		memset(io->buf + io->len + len, padc, pad);
	} else {
		memset(io->buf + io->len, padc, pad);
		memcpy(io->buf + io->len + pad, buf, len);
	}
	io->len += total;
	return ((int)total);
}

extern "C" void
__printf_flush(struct __printf_io *io)
{

	io->flushes++;
}

import pbsd.lib.libc.stdio.b0140;

namespace P = pbsd::lib_libc_stdio::b0140;

// ---------------------------------------------------------------------------
// Bookkeeping
// ---------------------------------------------------------------------------

struct Stats {
	const char	*name;
	long		cases;
	long		fails;
	int		reported;
};

static Stats st_putchar	  = { "putchar",		0, 0, 0 };
static Stats st_putchar_u = { "putchar_unlocked",	0, 0, 0 };
static Stats st_arginfo	  = { "__printf_arginfo_errno",	0, 0, 0 };
static Stats st_render	  = { "__printf_render_errno",	0, 0, 0 };
static Stats st_printf	  = { "printf",			0, 0, 0 };

static int report_fd = 2;	// stderr dup: usable while stdout is captured

static void
fail(Stats &s, const char *fmt, ...)
{
	char msg[512];
	char line[640];
	va_list ap;

	s.fails++;
	if (s.reported >= 8)
		return;
	s.reported++;
	va_start(ap, fmt);
	int n = vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	if (n < 0)
		return;
	int m = snprintf(line, sizeof(line), "FAIL %s: %s\n", s.name, msg);
	if (m > 0 && write(report_fd, line, (size_t)m) < 0)
		/* nothing useful to do about it */;
}

// splitmix64: deterministic and self-contained.
static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{

	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static uint32_t
rng_u32(void)
{

	return ((uint32_t)(rng_next() >> 32));
}

static int
rng_range(int lo, int hi)		// inclusive
{
	uint32_t span = (uint32_t)(hi - lo) + 1u;

	return (lo + (int)(rng_u32() % span));
}

static void
pi_zero(struct printf_info &pi)
{

	memset(&pi, 0, sizeof(pi));
}

static void
pi_random(struct printf_info &pi)
{

	pi_zero(pi);
	pi.prec = rng_range(-4, 40);
	pi.width = rng_range(-8, 90);
	pi.spec = (wchar_t)rng_range(1, 127);
	pi.left = (unsigned)(rng_u32() & 1u);
	pi.alt = (unsigned)(rng_u32() & 1u);
	pi.space = (unsigned)(rng_u32() & 1u);
	pi.showsign = (unsigned)(rng_u32() & 1u);
	pi.pad = (rng_u32() & 1u) ? (wchar_t)'0' : (wchar_t)' ';
}

// ---------------------------------------------------------------------------
// __printf_arginfo_errno
// ---------------------------------------------------------------------------

enum { ARGT_N = 8 };

static void
arginfo_case(const struct printf_info &tmpl, size_t n)
{
	int pa[ARGT_N], pb[ARGT_N];
	struct printf_info pia = tmpl, pib = tmpl;

	st_arginfo.cases++;
	memset(pa, 0x7f, sizeof(pa));
	memset(pb, 0x7f, sizeof(pb));

	int ra = P::__printf_arginfo_errno(&pia, n, pa);
	int rb = ref___printf_arginfo_errno(&pib, n, pb);

	if (ra != rb)
		fail(st_arginfo, "n=%zu ret port=%d ref=%d", n, ra, rb);
	if (memcmp(pa, pb, sizeof(pa)) != 0)
		fail(st_arginfo, "n=%zu argt differs: port=%d,%d,%d,%d "
		    "ref=%d,%d,%d,%d", n, pa[0], pa[1], pa[2], pa[3],
		    pb[0], pb[1], pb[2], pb[3]);
	if (memcmp(&pia, &pib, sizeof(pia)) != 0)
		fail(st_arginfo, "n=%zu printf_info was modified", n);
}

// assert(n >= 1) can only be observed by whether the call survives, so run it
// in a child and compare how the child died.
struct ChildOut {
	int	ret;
	int	completed;
	int	argt[ARGT_N];
};

static ChildOut *
shared_alloc(void)
{
	void *p = mmap(NULL, sizeof(ChildOut), PROT_READ | PROT_WRITE,
	    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (p == MAP_FAILED) {
		perror("mmap");
		exit(2);
	}
	return ((ChildOut *)p);
}

static void
arginfo_child(bool use_port, const struct printf_info *pi, size_t n,
    ChildOut *out, int *status)
{

	out->ret = -12345;
	out->completed = 0;
	memset(out->argt, 0x7f, sizeof(out->argt));
	fflush(NULL);

	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}
	if (pid == 0) {
		int devnull = open("/dev/null", O_WRONLY);
		if (devnull >= 0) {
			dup2(devnull, 2);	// assert() chatter
			close(devnull);
		}
		int r = use_port ?
		    P::__printf_arginfo_errno(pi, n, out->argt) :
		    ref___printf_arginfo_errno(pi, n, out->argt);
		out->ret = r;
		out->completed = 1;
		_exit(0);
	}
	if (waitpid(pid, status, 0) < 0) {
		perror("waitpid");
		exit(2);
	}
}

static void
arginfo_fork_case(const struct printf_info &tmpl, size_t n)
{
	static ChildOut *oa = shared_alloc();
	static ChildOut *ob = shared_alloc();
	int sa = 0, sb = 0;
	struct printf_info pia = tmpl, pib = tmpl;

	st_arginfo.cases++;
	arginfo_child(true, &pia, n, oa, &sa);
	arginfo_child(false, &pib, n, ob, &sb);

	if (sa != sb)
		fail(st_arginfo, "n=%zu wait status port=0x%x ref=0x%x "
		    "(signal port=%d ref=%d)", n, sa, sb,
		    WIFSIGNALED(sa) ? WTERMSIG(sa) : 0,
		    WIFSIGNALED(sb) ? WTERMSIG(sb) : 0);
	if (oa->completed != ob->completed)
		fail(st_arginfo, "n=%zu completed port=%d ref=%d", n,
		    oa->completed, ob->completed);
	if (oa->ret != ob->ret)
		fail(st_arginfo, "n=%zu child ret port=%d ref=%d", n,
		    oa->ret, ob->ret);
	if (memcmp(oa->argt, ob->argt, sizeof(oa->argt)) != 0)
		fail(st_arginfo, "n=%zu child argt differs", n);
}

// ---------------------------------------------------------------------------
// __printf_render_errno
// ---------------------------------------------------------------------------

struct Sink {
	unsigned char		store[SINK_CAP];
	struct __printf_io	io;

	void reset(void)
	{
		memset(store, 0x7f, sizeof(store));
		memset(&io, 0, sizeof(io));
		io.buf = store;
	}
};

static Sink sink_a, sink_b;

static void
render_case(int error, int other, const struct printf_info &tmpl)
{
	struct printf_info pia = tmpl, pib = tmpl;

	st_render.cases++;
	sink_a.reset();
	sink_b.reset();

	// Distinct storage per side, and distinct values per slot, so that a
	// mutated subscript reads a different value instead of the same one.
	int ea = error, eb = error;
	int oa1 = other, ob1 = other;
	int oa2 = ~other, ob2 = ~other;
	int oa3 = other ^ 0x5a5a5a5a, ob3 = other ^ 0x5a5a5a5a;
	const void *arga[4] = { &ea, &oa1, &oa2, &oa3 };
	const void *argb[4] = { &eb, &ob1, &ob2, &ob3 };

	int ra = P::__printf_render_errno(&sink_a.io, &pia, arga);
	int rb = ref___printf_render_errno(&sink_b.io, &pib, argb);

	if (ra != rb)
		fail(st_render, "error=%d ret port=%d ref=%d", error, ra, rb);
	if (sink_a.io.len != sink_b.io.len)
		fail(st_render, "error=%d sink len port=%zu ref=%zu", error,
		    sink_a.io.len, sink_b.io.len);
	if (sink_a.io.calls != sink_b.io.calls ||
	    sink_a.io.flushes != sink_b.io.flushes ||
	    sink_a.io.overflow != sink_b.io.overflow ||
	    sink_a.io.requested != sink_b.io.requested)
		fail(st_render, "error=%d io calls %d/%d flushes %d/%d "
		    "overflow %d/%d requested %ld/%ld", error,
		    sink_a.io.calls, sink_b.io.calls,
		    sink_a.io.flushes, sink_b.io.flushes,
		    sink_a.io.overflow, sink_b.io.overflow,
		    sink_a.io.requested, sink_b.io.requested);
	if (sink_a.io.last_spec != sink_b.io.last_spec ||
	    sink_a.io.last_width != sink_b.io.last_width ||
	    sink_a.io.last_prec != sink_b.io.last_prec ||
	    sink_a.io.last_pad != sink_b.io.last_pad)
		fail(st_render, "error=%d printf_info seen by the sink "
		    "differs", error);
	// The whole sink, guard bytes past the write window included.
	if (memcmp(sink_a.store, sink_b.store, sizeof(sink_a.store)) != 0) {
		size_t i = 0;
		while (i < sizeof(sink_a.store) &&
		    sink_a.store[i] == sink_b.store[i])
			i++;
		fail(st_render, "error=%d sink differs at %zu: port=0x%02x "
		    "ref=0x%02x port=[%.40s] ref=[%.40s]", error, i,
		    sink_a.store[i], sink_b.store[i],
		    (const char *)sink_a.store, (const char *)sink_b.store);
	}
	if (memcmp(&pia, &pib, sizeof(pia)) != 0)
		fail(st_render, "error=%d printf_info was modified", error);
	if (ea != eb || oa1 != ob1 || oa2 != ob2 || oa3 != ob3)
		fail(st_render, "error=%d argument storage was modified",
		    error);
}

// ---------------------------------------------------------------------------
// stdout capture, for putchar()/putchar_unlocked()/printf()
// ---------------------------------------------------------------------------

class Capture {
public:
	void begin(void)
	{
		char path[] = "/tmp/pbsd-b0140-XXXXXX";

		fflush(stdout);
		saved_ = dup(1);
		fd_ = mkstemp(path);
		if (saved_ < 0 || fd_ < 0) {
			perror("capture");
			exit(2);
		}
		unlink(path);
		dup2(fd_, 1);
	}

	void end(void)
	{
		fflush(stdout);
		dup2(saved_, 1);
		close(saved_);
		close(fd_);
		saved_ = fd_ = -1;
	}

	// Offset just past everything written to stdout so far.
	long pos(void)
	{
		fflush(stdout);
		return ((long)lseek(fd_, 0, SEEK_CUR));
	}

	void range(long from, long to, std::string &out)
	{
		out.assign((size_t)(to - from), '\0');
		if (to > from) {
			ssize_t got = pread(fd_, &out[0], (size_t)(to - from),
			    (off_t)from);
			if (got != (ssize_t)(to - from)) {
				perror("pread");
				exit(2);
			}
		}
	}

	void trim(void)
	{
		if (pos() > (1L << 20)) {
			fflush(stdout);
			lseek(fd_, 0, SEEK_SET);
			if (ftruncate(fd_, 0) != 0) {
				perror("ftruncate");
				exit(2);
			}
		}
	}

private:
	int	saved_ = -1;
	int	fd_ = -1;
};

static Capture cap;

static void
compare_output(Stats &s, const char *what, int ra, int rb, long a, long b,
    long c)
{
	std::string sa, sb;

	if (ra != rb)
		fail(s, "%s ret port=%d ref=%d", what, ra, rb);
	cap.range(a, b, sa);
	cap.range(b, c, sb);
	if (sa != sb)
		fail(s, "%s output port=%zu bytes [%.32s] ref=%zu bytes "
		    "[%.32s]", what, sa.size(), sa.c_str(), sb.size(),
		    sb.c_str());
}

static void
putchar_case(int c)
{
	char what[32];

	st_putchar.cases++;
	cap.trim();
	long a = cap.pos();
	int ra = P::putchar(c);
	long b = cap.pos();
	int rb = ref_putchar(c);
	long d = cap.pos();
	snprintf(what, sizeof(what), "c=%d", c);
	compare_output(st_putchar, what, ra, rb, a, b, d);
}

static void
putchar_unlocked_case(int c)
{
	char what[32];

	st_putchar_u.cases++;
	cap.trim();
	long a = cap.pos();
	int ra = P::putchar_unlocked(c);
	long b = cap.pos();
	int rb = ref_putchar_unlocked(c);
	long d = cap.pos();
	snprintf(what, sizeof(what), "c=%d", c);
	compare_output(st_putchar_u, what, ra, rb, a, b, d);
}

// ---------------------------------------------------------------------------
// printf
// ---------------------------------------------------------------------------

enum { PRINTF_KINDS = 12 };

struct PrintfCase {
	int		kind;
	int		i;
	int		j;
	unsigned	u;
	long		l;
	const char	*s;
	const char	*s2;
};

#define PF_CALL(...) (port ? P::printf(__VA_ARGS__) : ref_printf(__VA_ARGS__))

static int
printf_invoke(bool port, const PrintfCase &pc)
{

	switch (pc.kind) {
	case 0:
		return (PF_CALL(""));
	case 1:
		return (PF_CALL("%s", pc.s));
	case 2:
		return (PF_CALL("%d", pc.i));
	case 3:
		return (PF_CALL("%c", pc.i));
	case 4:
		return (PF_CALL("%%"));
	case 5:
		return (PF_CALL("[%5s|%-5s]", pc.s, pc.s2));
	case 6:
		return (PF_CALL("%s%c%d%x", pc.s, pc.i, pc.j, pc.u));
	case 7:
		return (PF_CALL("%.*s", pc.i, pc.s));
	case 8:
		return (PF_CALL("%*d|%ld", pc.j, pc.i, pc.l));
	case 9:
		// Format text taken straight from the data (no '%' in it).
		return (PF_CALL(pc.s2));
	case 10:
		return (PF_CALL("%s%s%s", pc.s, "", pc.s2));
	default:
		return (PF_CALL("%u/%o/%X/%c%c", pc.u, pc.u, pc.u, pc.i,
		    pc.j));
	}
}

#undef PF_CALL

static void
printf_case(const PrintfCase &pc)
{
	char what[64];

	st_printf.cases++;
	cap.trim();
	long a = cap.pos();
	int ra = printf_invoke(true, pc);
	long b = cap.pos();
	int rb = printf_invoke(false, pc);
	long d = cap.pos();
	snprintf(what, sizeof(what), "kind=%d i=%d j=%d", pc.kind, pc.i,
	    pc.j);
	compare_output(st_printf, what, ra, rb, a, b, d);
}

// ---------------------------------------------------------------------------
// Hand-written edge cases
// ---------------------------------------------------------------------------

static const int edge_chars[] = {
	0, 1, 2, 7, '\n', '\r', 0x1f, ' ', '0', 'A', 'a', 0x7e, 0x7f,
	0x80, 0x81, 0xa0, 0xbf, 0xc0, 0xfe, 0xff,
	0x100, 0x101, 0x17f, 0x180, 0x1ff, 0x200,
	-1, -2, -128, -129, -255, -256, -257,
	INT_MIN, INT_MIN + 1, INT_MAX, INT_MAX - 1,
	0x7fffff00, (int)0x80000001u, 0xffff, 0x10000,
};

enum { N_EDGE_CHARS = sizeof(edge_chars) / sizeof(edge_chars[0]) };

static void
edge_putchar(void)
{

	for (size_t i = 0; i < N_EDGE_CHARS; i++) {
		putchar_case(edge_chars[i]);
		putchar_unlocked_case(edge_chars[i]);
	}
	// Every single byte value, both entry points.
	for (int c = 0; c < 256; c++) {
		putchar_case(c);
		putchar_unlocked_case(c);
	}
}

static void
edge_arginfo(void)
{
	struct printf_info pz, pr;
	static const size_t ns[] = {
		1, 2, 3, 4, 7, 8, 16, 64, 65535,
		(size_t)INT_MAX, (size_t)INT_MAX + 1,
		SIZE_MAX / 2, SIZE_MAX - 1, SIZE_MAX,
	};

	pi_zero(pz);
	rng_seed(0x0f1e2d3c4b5a6978ULL);
	pi_random(pr);

	// n == 1 is the assert()'s boundary.
	for (size_t i = 0; i < sizeof(ns) / sizeof(ns[0]); i++) {
		arginfo_case(pz, ns[i]);
		arginfo_case(pr, ns[i]);
	}
	// n == 0 must abort; only the forked variant can see that.
	arginfo_fork_case(pz, 0);
	arginfo_fork_case(pz, 1);
	arginfo_fork_case(pz, 2);
	arginfo_fork_case(pr, 0);
	arginfo_fork_case(pr, 1);
	arginfo_fork_case(pr, SIZE_MAX);
}

static void
edge_render(void)
{
	const int nerr = __hidden_sys_nerr;
	int errs[32];
	size_t n = 0;

	errs[n++] = INT_MIN;
	errs[n++] = INT_MIN + 1;
	errs[n++] = -1000;
	errs[n++] = -2;
	errs[n++] = -1;		// -1 >= 0 false but -1 < nerr true: && vs ||
	errs[n++] = 0;		// the >= boundary
	errs[n++] = 1;
	errs[n++] = 2;
	errs[n++] = 3;
	errs[n++] = 4;
	errs[n++] = 34;
	errs[n++] = 95;
	errs[n++] = nerr - 2;
	errs[n++] = nerr - 1;	// last value in range
	errs[n++] = nerr;	// the < boundary
	errs[n++] = nerr + 1;
	errs[n++] = nerr + 2;
	errs[n++] = 1000;
	errs[n++] = 0xffff;
	errs[n++] = INT_MAX - 1;
	errs[n++] = INT_MAX;

	struct printf_info pz, pw, pl, pbig;
	pi_zero(pz);
	pi_zero(pw);
	pw.width = 40;
	pw.pad = (wchar_t)'*';
	pi_zero(pl);
	pl.width = 40;
	pl.left = 1;
	pl.pad = (wchar_t)'.';
	pi_zero(pbig);
	pbig.width = INT_MAX;
	pbig.prec = INT_MIN;
	pbig.spec = (wchar_t)'m';
	pbig.pad = (wchar_t)'#';

	const int others[] = { 0, 1, -1, 7, nerr, 12345, INT_MIN, INT_MAX };

	for (size_t i = 0; i < n; i++) {
		for (size_t k = 0; k < sizeof(others) / sizeof(others[0]);
		    k++) {
			int other = others[k];

			if (other == errs[i])
				other = ~other;
			render_case(errs[i], other, pz);
			render_case(errs[i], other, pw);
			render_case(errs[i], other, pl);
			render_case(errs[i], other, pbig);
		}
	}
}

static void
edge_printf(void)
{
	static const char *strs[] = {
		"",
		"a",
		"ab",
		"abcd",
		"0123456789",
		"\x7f",
		"\x80",
		"\x80\x81\xfe\xff",
		"\xff\xff\xff\xff\xff\xff\xff\xff",
		"high\xc3\xa9utf8",
		"a very long string that runs well past any small buffer "
		    "boundary in the implementation, more than a hundred "
		    "characters long, so buffering is exercised too",
		"tab\there",
		"nl\nhere",
	};
	static const int ints[] = {
		0, 1, -1, 2, -2, 9, 10, 99, 100, 255, 256, -255, -256,
		INT_MIN, INT_MIN + 1, INT_MAX, INT_MAX - 1, 0x7f, 0x80, 0xff,
	};
	size_t nstr = sizeof(strs) / sizeof(strs[0]);
	size_t nint = sizeof(ints) / sizeof(ints[0]);

	for (int kind = 0; kind < PRINTF_KINDS; kind++) {
		for (size_t si = 0; si < nstr; si++) {
			for (size_t ii = 0; ii < nint; ii++) {
				PrintfCase pc;

				pc.kind = kind;
				pc.i = ints[ii];
				pc.j = ints[(ii + 3) % nint];
				pc.u = (unsigned)ints[(ii + 5) % nint];
				pc.l = (long)ints[ii] * 1000003L;
				pc.s = strs[si];
				pc.s2 = strs[(si + 1) % nstr];
				if (kind == 7)		// %.*s precision
					pc.i = (int)(ii % 12) - 2;
				if (kind == 8)		// %*d field width
					pc.j = (int)(ii % 21) - 10;
				if (kind == 9 && strchr(pc.s2, '%') != NULL)
					continue;
				printf_case(pc);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Randomised sweeps
// ---------------------------------------------------------------------------

enum { ITERS = 200000 };

static int
rng_char(void)
{

	switch (rng_u32() % 4u) {
	case 0:
		return ((int)(rng_u32() & 0xffu));		// plain byte
	case 1:
		return ((int)(rng_u32() % 512u) - 256);		// around 0
	case 2:
		return ((int)rng_u32());			// anything
	default:
		return (edge_chars[rng_u32() % N_EDGE_CHARS]);
	}
}

static void
sweep_putchar(void)
{

	rng_seed(0x1a2b3c4d5e6f7081ULL);
	for (long i = 0; i < ITERS; i++)
		putchar_case(rng_char());
}

static void
sweep_putchar_unlocked(void)
{

	rng_seed(0x2b3c4d5e6f708192ULL);
	for (long i = 0; i < ITERS; i++)
		putchar_unlocked_case(rng_char());
}

static void
sweep_arginfo(void)
{
	struct printf_info pi;

	rng_seed(0x3c4d5e6f708192a3ULL);
	for (long i = 0; i < ITERS; i++) {
		size_t n;

		pi_random(pi);
		switch (rng_u32() % 4u) {
		case 0:
			n = 1;				// the boundary itself
			break;
		case 1:
			n = (size_t)rng_range(1, 8);
			break;
		case 2:
			n = (size_t)rng_u32() | 1u;
			break;
		default:
			n = (size_t)rng_next();
			if (n == 0)
				n = 1;
			break;
		}
		arginfo_case(pi, n);
	}
}

static void
sweep_render(void)
{
	const int nerr = __hidden_sys_nerr;
	struct printf_info pi;

	rng_seed(0x4d5e6f708192a3b4ULL);
	for (long i = 0; i < ITERS; i++) {
		int error, other;

		switch (rng_u32() % 6u) {
		case 0:
			error = rng_range(-4, 4);		// around 0
			break;
		case 1:
			error = nerr + rng_range(-4, 4);	// around nerr
			break;
		case 2:
			error = rng_range(0, nerr - 1);		// in range
			break;
		case 3:
			error = rng_range(-100000, 100000);
			break;
		case 4:
			error = (int)rng_u32();			// anything
			break;
		default:
			error = (int)(rng_u32() & 0x7fffffffu);
			break;
		}
		other = (int)rng_u32();
		if (other == error)
			other = ~other;
		pi_random(pi);
		render_case(error, other, pi);
	}
}

static void
sweep_printf(void)
{
	char sbuf[40];
	char s2buf[40];

	rng_seed(0x5e6f708192a3b4c5ULL);
	for (long i = 0; i < ITERS; i++) {
		PrintfCase pc;
		size_t n1 = (size_t)rng_range(0, 24);
		size_t n2 = (size_t)rng_range(0, 24);

		// Every byte value but NUL, high-bit bytes included.
		for (size_t k = 0; k < n1; k++)
			sbuf[k] = (char)rng_range(1, 255);
		sbuf[n1] = '\0';
		for (size_t k = 0; k < n2; k++) {
			int b;

			do {
				b = rng_range(1, 255);
			} while (b == '%');	// s2 doubles as a format
			s2buf[k] = (char)b;
		}
		s2buf[n2] = '\0';

		pc.kind = (int)(rng_u32() % (unsigned)PRINTF_KINDS);
		pc.i = (int)rng_u32();
		pc.j = (int)rng_u32();
		pc.u = rng_u32();
		pc.l = (long)rng_next();
		pc.s = sbuf;
		pc.s2 = s2buf;
		if (pc.kind == 7)
			pc.i = rng_range(-4, 30);	// %.*s precision
		if (pc.kind == 8)
			pc.j = rng_range(-12, 12);	// %*d field width
		if ((pc.kind == 3 || pc.kind == 6 || pc.kind == 11) &&
		    (rng_u32() & 1u) != 0)
			pc.i = rng_range(-300, 300);	// %c, near the edges
		printf_case(pc);
	}
}

// ---------------------------------------------------------------------------

static void
row(const Stats &s)
{

	printf("  %-24s %10ld %10ld  %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	int dupfd = dup(2);

	if (dupfd >= 0)
		report_fd = dupfd;

	// Everything that writes to stdout runs with stdout captured.
	cap.begin();
	edge_putchar();
	edge_printf();
	sweep_putchar();
	sweep_putchar_unlocked();
	sweep_printf();
	cap.end();

	edge_arginfo();
	edge_render();
	sweep_arginfo();
	sweep_render();

	printf("b0140 differential test (port vs. ref_ oracle)\n");
	printf("  %-24s %10s %10s\n", "function", "cases", "failures");
	row(st_putchar);
	row(st_putchar_u);
	row(st_arginfo);
	row(st_render);
	row(st_printf);

	long fails = st_putchar.fails + st_putchar_u.fails +
	    st_arginfo.fails + st_render.fails + st_printf.fails;
	long cases = st_putchar.cases + st_putchar_u.cases +
	    st_arginfo.cases + st_render.cases + st_printf.cases;

	printf("  %-24s %10ld %10ld  %s\n", "TOTAL", cases, fails,
	    fails == 0 ? "PASS" : "FAIL");
	fflush(stdout);
	return (fails == 0 ? 0 : 1);
}
