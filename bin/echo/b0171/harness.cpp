/*
 * PBSD b0171 -- differential test: port.cppm vs oracle.c (bin/echo).
 *
 * echo(1) is a single function whose entire contract is side effects, so the
 * comparison is done on a full trace of everything the function does:
 *
 *	- the capsicum probe sequence (which of the two probes ran, and the
 *	  value each returned -- short-circuit behaviour is observable);
 *	- the exact byte count handed to malloc();
 *	- every writev() call: the fd, the iovcnt, and for every iovec the
 *	  base (as an offset into the argv arena, or "l" for a pointer to one
 *	  of main()'s locals), the length, and the bytes;
 *	- err() calls, with code and format string;
 *	- the return value, or the fact that the function did not return;
 *	- 256 guard bytes of 0x7f past the end of every malloc() block;
 *	- the entire argv arena, which is padded with 0x7f guard bytes on both
 *	  sides of every argument string and compared in full, byte for byte,
 *	  between the two runs.
 *
 * Pointers are never compared as addresses: argv-arena pointers are reduced
 * to offsets from the arena base, and the two runs use separate arenas built
 * at identical offsets.
 */

#include <sys/types.h>
#include <sys/uio.h>

#include <cerrno>
#include <climits>
#include <csetjmp>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

import pbsd.bin.echo.b0171;

extern "C" int ref_main(int argc, char *argv[]);

#ifndef IOV_MAX
#define	IOV_MAX	1024
#endif

/* ------------------------------------------------------------------ trace */

static std::string g_trace;

static inline void
t_str(const char *s)
{
	g_trace += s;
}

static inline void
t_num(long long v)
{
	char b[24];
	int i = 24;
	bool neg = v < 0;
	unsigned long long u = neg ?
	    (unsigned long long)(-(v + 1)) + 1ULL : (unsigned long long)v;

	do {
		b[--i] = (char)('0' + (int)(u % 10));
		u /= 10;
	} while (u != 0);
	if (neg)
		b[--i] = '-';
	g_trace.append(b + i, (size_t)(24 - i));
}

static inline void
t_hex(const unsigned char *p, size_t n)
{
	static const char H[] = "0123456789abcdef";

	for (size_t i = 0; i < n; i++) {
		g_trace += H[p[i] >> 4];
		g_trace += H[p[i] & 15];
	}
}

/* ------------------------------------------------------------- shim state */

static const unsigned char GUARD_BYTE = 0x7f;
static const size_t MALLOC_GUARD = 256;
static const size_t MALLOC_CAP = (size_t)1 << 28;

struct Blk {
	unsigned char *p;
	size_t n;
};

static unsigned char *g_arena = nullptr;
static const unsigned char *g_arena_init = nullptr;
static size_t g_arena_size = 0;

static int g_caph1 = 0;
static int g_caph2 = 0;

static long g_wfail = -1;
static long g_wcalls = 0;
static long g_mfail = -1;
static long g_mcalls = 0;

static std::vector<Blk> g_blocks;

static std::jmp_buf g_jb;

/* -------------------------------------------------------------- the shims */

extern "C" int
pbsd_shim_caph_limit_stdio(void)
{
	t_str("caph_limit_stdio -> ");
	t_num(g_caph1);
	t_str("\n");
	return g_caph1;
}

extern "C" int
pbsd_shim_caph_enter(void)
{
	t_str("caph_enter -> ");
	t_num(g_caph2);
	t_str("\n");
	return g_caph2;
}

extern "C" void *
pbsd_shim_malloc(size_t n)
{
	long idx = g_mcalls++;

	t_str("malloc ");
	t_num((long long)n);
	if (idx == g_mfail) {
		t_str(" -> NULL(injected)\n");
		return nullptr;
	}
	if (n > MALLOC_CAP) {
		t_str(" -> NULL(too big)\n");
		return nullptr;
	}

	unsigned char *p = (unsigned char *)std::malloc(n + MALLOC_GUARD);
	if (p == nullptr) {
		t_str(" -> NULL(oom)\n");
		return nullptr;
	}
	std::memset(p, GUARD_BYTE, n + MALLOC_GUARD);
	g_blocks.push_back(Blk{p, n});
	t_str(" -> ok\n");
	return p;
}

extern "C" ssize_t
pbsd_shim_writev(int fd, const struct iovec *iov, int cnt)
{
	long idx = g_wcalls++;

	t_str("writev fd=");
	t_num(fd);
	t_str(" cnt=");
	t_num(cnt);
	t_str("\n");

	/*
	 * Clamp the number of iovecs actually dereferenced to whatever we
	 * handed out, so that a mutated port producing a bogus iovcnt is
	 * reported as a trace difference rather than a segfault.  A correct
	 * port never trips this.
	 */
	long lim = cnt;
	const unsigned char *q = (const unsigned char *)iov;
	for (size_t i = 0; i < g_blocks.size(); i++) {
		const unsigned char *base = g_blocks[i].p;
		if (q >= base && q < base + g_blocks[i].n) {
			long avail = (long)((size_t)(base + g_blocks[i].n - q) /
			    sizeof(struct iovec));
			if (lim > avail) {
				lim = avail;
				t_str("  (clamped)\n");
			}
			break;
		}
	}
	if (lim < 0)
		lim = 0;
	if (lim > 1 << 20)
		lim = 1 << 20;

	ssize_t total = 0;
	for (long i = 0; i < lim; i++) {
		const unsigned char *p =
		    (const unsigned char *)iov[i].iov_base;
		size_t l = iov[i].iov_len;

		t_str("  b=");
		if (g_arena != nullptr && p >= g_arena &&
		    p < g_arena + g_arena_size) {
			t_str("a");
			t_num((long long)(p - g_arena));
		} else {
			t_str("l");
		}
		t_str(" len=");
		t_num((long long)l);
		t_str(" d=");
		t_hex(p, l > 4096 ? 4096 : l);
		t_str("\n");
		total += (ssize_t)l;
	}

	if (idx == g_wfail) {
		t_str("writev -> -1\n");
		errno = EIO;
		return -1;
	}
	t_str("writev -> ");
	t_num((long long)total);
	t_str("\n");
	return total;
}

extern "C" {
[[noreturn]] void
pbsd_shim_err(int code, const char *fmt)
{
	t_str("err ");
	t_num(code);
	t_str(" \"");
	t_str(fmt);
	t_str("\"\n");
	std::longjmp(g_jb, 1);
}
}

/* ---------------------------------------------------------------- harness */

struct Env {
	std::vector<unsigned char> arena;
	std::vector<unsigned char> init;
	std::vector<char *> argv;
};

struct Case {
	std::vector<std::string> args;
	/*
	 * Bytes laid down immediately *before* every argument string.  The
	 * port indexes argv[0][len - 2] and argv[0][len - 1], so an off-by-one
	 * in the `len >= 2' guard reads underneath the string; seeding that
	 * memory with '\\' and 'c' makes such a read observable instead of
	 * silently landing on a byte that happens not to match.
	 */
	std::string pre;
	int c1 = 0;
	int c2 = 0;
	long wfail = -1;
	long mfail = -1;
};

static const size_t PAD = 32;

static void
build_env(const std::vector<std::string> &args, const std::string &pre, Env &e)
{
	size_t need = 0;
	for (size_t i = 0; i < args.size(); i++)
		need += args[i].size() + 1 + PAD;

	e.arena.assign(need + PAD * 2, GUARD_BYTE);
	e.argv.clear();

	size_t off = PAD;
	for (size_t i = 0; i < args.size(); i++) {
		for (size_t j = 0; j < pre.size(); j++)
			e.arena[off - pre.size() + j] =
			    (unsigned char)pre[j];
		if (!args[i].empty())
			std::memcpy(&e.arena[off], args[i].data(),
			    args[i].size());
		e.arena[off + args[i].size()] = 0;
		e.argv.push_back((char *)&e.arena[off]);
		off += args[i].size() + 1 + PAD;
	}
	e.argv.push_back(nullptr);
	e.init = e.arena;
}

static int g_which = 0;
static int g_argc = 0;
static char **g_argvp = nullptr;

static void
invoke(void)
{
	if (setjmp(g_jb) == 0) {
		int r;

		if (g_which != 0)
			r = pbsd::bin_echo::b0171::echo_main(g_argc, g_argvp);
		else
			r = ref_main(g_argc, g_argvp);
		t_str("return ");
		t_num(r);
		t_str("\n");
	} else {
		t_str("did-not-return\n");
	}
}

static std::string
execute(int which, const Case &c, Env &e)
{
	g_trace.clear();
	g_arena = e.arena.data();
	g_arena_init = e.init.data();
	g_arena_size = e.arena.size();
	g_caph1 = c.c1;
	g_caph2 = c.c2;
	g_wfail = c.wfail;
	g_mfail = c.mfail;
	g_wcalls = 0;
	g_mcalls = 0;
	g_blocks.clear();
	g_which = which;
	g_argc = (int)c.args.size();
	g_argvp = e.argv.data();

	invoke();

	for (size_t i = 0; i < g_blocks.size(); i++) {
		size_t bad = 0;
		unsigned char *p = g_blocks[i].p;
		size_t n = g_blocks[i].n;

		for (size_t j = n; j < n + MALLOC_GUARD; j++)
			if (p[j] != GUARD_BYTE)
				bad++;
		t_str("mallocguard n=");
		t_num((long long)n);
		t_str(" bad=");
		t_num((long long)bad);
		t_str("\n");
		std::free(p);
	}
	g_blocks.clear();

	size_t mod = 0;
	for (size_t i = 0; i < g_arena_size; i++) {
		if (g_arena[i] != g_arena_init[i]) {
			if (mod < 64) {
				t_str("arenamod ");
				t_num((long long)i);
				t_str("=");
				t_hex(&g_arena[i], 1);
				t_str("\n");
			}
			mod++;
		}
	}
	t_str("arenamod_count ");
	t_num((long long)mod);
	t_str("\n");

	g_arena = nullptr;
	g_arena_init = nullptr;
	g_arena_size = 0;
	return g_trace;
}

static std::string
describe(const Case &c)
{
	std::string s = "argc=";
	char b[32];

	std::snprintf(b, sizeof b, "%d", (int)c.args.size());
	s += b;
	s += " argv=[";
	for (size_t i = 0; i < c.args.size(); i++) {
		if (i != 0)
			s += ", ";
		s += "\"";
		for (size_t j = 0; j < c.args[i].size(); j++) {
			unsigned char ch = (unsigned char)c.args[i][j];
			if (ch >= 0x20 && ch < 0x7f) {
				if (ch == '"' || ch == '\\')
					s += '\\';
				s += (char)ch;
			} else {
				std::snprintf(b, sizeof b, "\\x%02x", ch);
				s += b;
			}
		}
		s += "\"";
	}
	s += "] pre=\"";
	for (size_t j = 0; j < c.pre.size(); j++) {
		unsigned char ch = (unsigned char)c.pre[j];
		if (ch >= 0x20 && ch < 0x7f) {
			if (ch == '"' || ch == '\\')
				s += '\\';
			s += (char)ch;
		} else {
			std::snprintf(b, sizeof b, "\\x%02x", ch);
			s += b;
		}
	}
	s += "\"";
	std::snprintf(b, sizeof b, " caph=(%d,%d)", c.c1, c.c2);
	s += b;
	std::snprintf(b, sizeof b, " wfail=%ld mfail=%ld", c.wfail, c.mfail);
	s += b;
	return s;
}

struct Group {
	const char *name;
	long cases;
	long failures;
};

static Group g_groups[8];
static int g_ngroups = 0;
static long g_reported = 0;

static int
group(const char *name)
{
	for (int i = 0; i < g_ngroups; i++)
		if (std::strcmp(g_groups[i].name, name) == 0)
			return i;
	g_groups[g_ngroups].name = name;
	g_groups[g_ngroups].cases = 0;
	g_groups[g_ngroups].failures = 0;
	return g_ngroups++;
}

static void
first_diff(const std::string &a, const std::string &b)
{
	size_t i = 0;
	while (i < a.size() && i < b.size() && a[i] == b[i])
		i++;
	size_t start = i > 60 ? i - 60 : 0;

	std::printf("      port  : ...%s\n",
	    a.substr(start, 160).c_str());
	std::printf("      oracle: ...%s\n",
	    b.substr(start, 160).c_str());
}

static void
check(int gi, const Case &c)
{
	Env ea, eb;

	build_env(c.args, c.pre, ea);
	build_env(c.args, c.pre, eb);

	std::string ta = execute(1, c, ea);
	std::string tb = execute(0, c, eb);

	bool ok = (ta == tb);

	/* Full byte-for-byte comparison of the two argv arenas. */
	if (ea.arena.size() != eb.arena.size() ||
	    std::memcmp(ea.arena.data(), eb.arena.data(), ea.arena.size()) != 0)
		ok = false;

	g_groups[gi].cases++;
	if (!ok) {
		g_groups[gi].failures++;
		if (g_reported < 12) {
			g_reported++;
			std::printf("  FAIL [%s] %s\n", g_groups[gi].name,
			    describe(c).c_str());
			first_diff(ta, tb);
		}
	}
}

/* ------------------------------------------------------------- generators */

static unsigned long long g_seed = 0x9e3779b97f4a7c15ULL;

static unsigned long long
rnd(void)
{
	unsigned long long z = (g_seed += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static unsigned
rnd_n(unsigned n)
{
	return (unsigned)(rnd() % n);
}

/*
 * Strings chosen to sit on both sides of every comparison in the port:
 * lengths 0/1/2/3 around the `len >= 2' test, a trailing 'c' without the
 * backslash and a trailing backslash without the 'c' around the two halves
 * of the && chain, exact and near misses for "-n", and high-bit bytes so a
 * signed/unsigned slip in the char comparisons shows up.
 */
static const char *const SPECIAL[] = {
	"", "a", "c", "\\", "n", "-",
	"\\c", "c\\", "cc", "ac", "\\\\", "\\n", "\\d", "xc", "-n",
	"a\\c", "\\\\c", "\\cc", "\\c\\c", "x\\c", "ab\\c", "-nc", "-n\\c",
	"-N", "-nn", "-n ", " -n", "--n", "n-", "--",
	"\x80", "\xff", "\x7f", "\x01", "\x80\\c", "\\\x80", "\\\xff",
	"\xff\xff", "\x80" "c", "\\c\x80", "hello", "  ", "\t", "\x1b[0m",
	"\xc3\xa9", "\\\x63", "\x5c\x63", "\x5c\x64", "z\\c",
};
static const int NSPECIAL = (int)(sizeof SPECIAL / sizeof SPECIAL[0]);

static const char *const SMALLPOOL[] = {
	"", "a", "c", "\\", "\\c", "x\\c", "c\\", "-n", "\xff", "\x80\\c",
	"ac", "\\d",
};
static const int NSMALL = (int)(sizeof SMALLPOOL / sizeof SMALLPOOL[0]);

static const unsigned char ALPHA[] = {
	'a', 'b', 'c', 'n', '-', ' ', '\\', 'x', 'C', '\\',
	0x01, 0x7f, 0x80, 0xfe, 0xff, 0xc3,
};
static const int NALPHA = (int)(sizeof ALPHA / sizeof ALPHA[0]);

static std::string
rand_arg(void)
{
	unsigned r = rnd_n(100);

	if (r < 35)
		return std::string(SPECIAL[rnd_n((unsigned)NSPECIAL)]);

	unsigned len = rnd_n(6);
	std::string s;
	for (unsigned i = 0; i < len; i++)
		s += (char)ALPHA[rnd_n((unsigned)NALPHA)];
	/* Bias the tail towards the \c shapes the port actually tests for. */
	if (rnd_n(4) == 0) {
		unsigned k = rnd_n(4);
		if (k == 0)
			s += "\\c";
		else if (k == 1)
			s += "\\";
		else if (k == 2)
			s += "c";
		else
			s += "\\d";
	}
	return s;
}

/*
 * Build an argv that makes the final veclen exactly `target', so that the
 * `veclen > IOV_MAX' clamp can be exercised on both sides of the boundary.
 *
 * With no -n and no trailing \c: veclen == 2*N - 2 (N = argc, N >= 3).
 * With a trailing \c:            veclen == 2*N - 3.
 */
static Case
make_big(long target)
{
	Case c;
	long n;
	bool chop = (target % 2) != 0;

	if (chop)
		n = (target + 3) / 2;
	else
		n = (target + 2) / 2;

	c.args.push_back("echo");
	for (long i = 1; i < n; i++)
		c.args.push_back("a");
	if (chop)
		c.args[(size_t)(n - 1)] = "x\\c";
	return c;
}

int
main(void)
{
	int g_edge = group("edge");
	int g_under = group("underrun");
	int g_pair = group("pairs");
	int g_trip = group("triples");
	int g_cfg = group("configs");
	int g_big = group("iov_max");
	int g_rand = group("random");

	/* ---- hand-written edge cases -------------------------------- */
	{
		Case c;
		c.args.push_back("echo");
		check(g_edge, c);
	}
	for (int i = 0; i < NSPECIAL; i++) {
		Case c;
		c.args.push_back("echo");
		c.args.push_back(SPECIAL[i]);
		check(g_edge, c);
	}
	/* "-n" in first position followed by each special. */
	for (int i = 0; i < NSPECIAL; i++) {
		Case c;
		c.args.push_back("echo");
		c.args.push_back("-n");
		c.args.push_back(SPECIAL[i]);
		check(g_edge, c);
	}
	/* "-n" in a non-first position: must NOT be treated as a flag. */
	for (int i = 0; i < NSPECIAL; i++) {
		Case c;
		c.args.push_back("echo");
		c.args.push_back(SPECIAL[i]);
		c.args.push_back("-n");
		check(g_edge, c);
	}
	/* Two leading -n: only the first is a flag. */
	{
		Case c;
		c.args.push_back("echo");
		c.args.push_back("-n");
		c.args.push_back("-n");
		check(g_edge, c);
		c.args.push_back("-n");
		check(g_edge, c);
	}
	/* Deliberately long runs of empty strings and NUL-adjacent shapes. */
	for (int n = 1; n <= 9; n++) {
		Case c;
		c.args.push_back("echo");
		for (int i = 0; i < n; i++)
			c.args.push_back("");
		check(g_edge, c);
		c.args.push_back("\\c");
		check(g_edge, c);
	}
	/* Every special as the sole, final argument after some filler. */
	for (int i = 0; i < NSPECIAL; i++) {
		Case c;
		c.args.push_back("echo");
		c.args.push_back("filler");
		c.args.push_back("more");
		c.args.push_back(SPECIAL[i]);
		check(g_edge, c);
	}

	/*
	 * The same shapes with the bytes immediately below each argument
	 * seeded so that a short read under the string looks like a "\c":
	 * "c\\" puts a backslash at argv[i][-1] (caught by a length-1 final
	 * argument of "c"), "\\c" puts "\c" at argv[i][-2..-1] (caught by an
	 * empty final argument).
	 */
	{
		static const char *const PRE[] = {
			"c\\", "\\c", "\\\\", "cc", "\x80\\", "\\\x63",
		};
		for (size_t p = 0; p < sizeof PRE / sizeof PRE[0]; p++)
			for (int i = 0; i < NSPECIAL; i++) {
				Case c;
				c.pre = PRE[p];
				c.args.push_back("echo");
				c.args.push_back(SPECIAL[i]);
				check(g_under, c);

				Case d;
				d.pre = PRE[p];
				d.args.push_back("echo");
				d.args.push_back("q");
				d.args.push_back(SPECIAL[i]);
				check(g_under, d);

				Case f;
				f.pre = PRE[p];
				f.args.push_back("echo");
				f.args.push_back("-n");
				f.args.push_back(SPECIAL[i]);
				check(g_under, f);
			}
	}

	/* ---- exhaustive small cross products ------------------------ */
	for (int i = 0; i < NSPECIAL; i++)
		for (int j = 0; j < NSMALL; j++) {
			Case c;
			c.args.push_back("echo");
			c.args.push_back(SPECIAL[i]);
			c.args.push_back(SMALLPOOL[j]);
			check(g_pair, c);
		}
	for (int i = 0; i < NSMALL; i++)
		for (int j = 0; j < NSMALL; j++)
			for (int k = 0; k < NSMALL; k++) {
				Case c;
				c.args.push_back("echo");
				c.args.push_back(SMALLPOOL[i]);
				c.args.push_back(SMALLPOOL[j]);
				c.args.push_back(SMALLPOOL[k]);
				check(g_trip, c);
			}

	/* ---- capsicum / malloc / writev failure matrix -------------- */
	{
		static const int CAPH[][2] = {
			{ 0, 0 }, { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 },
			{ -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 },
		};
		std::vector<std::vector<std::string> > shapes;
		{
			std::vector<std::string> v;
			v.push_back("echo");
			shapes.push_back(v);
			v.push_back("a");
			shapes.push_back(v);
			v.push_back("b");
			shapes.push_back(v);
			v.push_back("c\\c");
			shapes.push_back(v);
		}
		{
			std::vector<std::string> v;
			v.push_back("echo");
			v.push_back("-n");
			shapes.push_back(v);
			v.push_back("q");
			shapes.push_back(v);
		}
		for (size_t s = 0; s < shapes.size(); s++)
			for (int ci = 0; ci < 9; ci++)
				for (long mf = -1; mf <= 0; mf++)
					for (long wf = -1; wf <= 2; wf++) {
						Case c;
						c.args = shapes[s];
						c.c1 = CAPH[ci][0];
						c.c2 = CAPH[ci][1];
						c.mfail = mf;
						c.wfail = wf;
						check(g_cfg, c);
					}
	}

	/* ---- the IOV_MAX clamp, both sides of the boundary ---------- */
	{
		long targets[] = {
			6, 7, 8, 9,
			IOV_MAX - 3, IOV_MAX - 2, IOV_MAX - 1, IOV_MAX,
			IOV_MAX + 1, IOV_MAX + 2, IOV_MAX + 3,
			2 * IOV_MAX - 1, 2 * IOV_MAX, 2 * IOV_MAX + 1,
			3 * IOV_MAX, 3 * IOV_MAX + 7,
		};
		for (size_t i = 0; i < sizeof targets / sizeof targets[0]; i++) {
			Case c = make_big(targets[i]);
			check(g_big, c);
			/* Same shape, but fail the second writev. */
			Case d = c;
			d.wfail = 1;
			check(g_big, d);
			/* And the first. */
			Case e = c;
			e.wfail = 0;
			check(g_big, e);
		}
	}

	/* ---- fixed-seed randomised sweep ---------------------------- */
	{
		const long ITER = 200000;
		for (long it = 0; it < ITER; it++) {
			Case c;
			unsigned n = 1 + rnd_n(6);

			c.args.push_back("echo");
			for (unsigned i = 1; i < n; i++)
				c.args.push_back(rand_arg());

			static const char *const RPRE[] = {
				"", "", "", "c\\", "\\c", "\\\\", "cc",
				"\x80\\",
			};
			c.pre = RPRE[rnd_n(8)];

			unsigned r = rnd_n(100);
			if (r < 92) {
				c.c1 = 0;
				c.c2 = 0;
			} else {
				static const int V[] = { 0, -1, 1 };
				c.c1 = V[rnd_n(3)];
				c.c2 = V[rnd_n(3)];
			}
			if (rnd_n(100) < 6)
				c.wfail = (long)rnd_n(3);
			if (rnd_n(100) < 3)
				c.mfail = 0;
			check(g_rand, c);
		}
	}

	/* ---- report -------------------------------------------------- */
	long tc = 0, tf = 0;

	std::printf("\n%-22s %-12s %10s %10s\n", "function", "group",
	    "cases", "failures");
	std::printf("%-22s %-12s %10s %10s\n", "----------------------",
	    "------------", "----------", "----------");
	for (int i = 0; i < g_ngroups; i++) {
		std::printf("%-22s %-12s %10ld %10ld\n", "echo_main",
		    g_groups[i].name, g_groups[i].cases, g_groups[i].failures);
		tc += g_groups[i].cases;
		tf += g_groups[i].failures;
	}
	std::printf("%-22s %-12s %10s %10s\n", "----------------------",
	    "------------", "----------", "----------");
	std::printf("%-22s %-12s %10ld %10ld\n", "TOTAL", "", tc, tf);
	std::printf("\n%s\n", tf == 0 ? "PASS" : "FAIL");

	return tf == 0 ? 0 : 1;
}
