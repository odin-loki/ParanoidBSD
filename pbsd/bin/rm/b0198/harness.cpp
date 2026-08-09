// Differential test harness for PBSD batch b0198 (hbsd/src/bin/rm/rm.c).
//
// Every retained function is exercised against the ref_ oracle in oracle.c:
// return value, the caller-visible argv buffer (compared in full, including
// guard slots past the nominal write window), the shared file-scope flags the
// function mutates, the bytes the function emitted on stderr, and - for the
// stdin-consuming check2 - the stream position left behind.

import pbsd.bin.rm.b0198;

#include <sys/stat.h>
#include <sys/types.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <string>
#include <vector>
#include <unistd.h>

namespace P = pbsd::bin_rm::b0198;

extern "C" {
extern int eval;
extern int rflag;
extern volatile sig_atomic_t info;
int  ref_check2(char **);
void ref_checkdot(char **);
void ref_checkslash(char **);
void ref_siginfo(int);
}

// ---------------------------------------------------------------- utilities

namespace {

struct Rng {
	std::uint64_t s;
	explicit Rng(std::uint64_t seed) : s(seed ? seed : 0x9e3779b97f4a7c15ull) {}
	std::uint32_t next()
	{
		s ^= s << 13;
		s ^= s >> 7;
		s ^= s << 17;
		return static_cast<std::uint32_t>(s >> 32);
	}
	std::uint32_t below(std::uint32_t n) { return n ? next() % n : 0u; }
};

struct Capture {
	char *buf = nullptr;
	size_t len = 0;
	FILE *saved = nullptr;
};

void
cap_begin(Capture &c)
{
	c.saved = stderr;
	c.buf = nullptr;
	c.len = 0;
	stderr = open_memstream(&c.buf, &c.len);
	if (stderr == nullptr) {
		stderr = c.saved;
		std::printf("FATAL: open_memstream failed\n");
		std::exit(2);
	}
}

std::string
cap_end(Capture &c)
{
	std::fflush(stderr);
	FILE *ms = stderr;
	stderr = c.saved;
	std::fclose(ms);
	std::string out(c.buf ? c.buf : "", c.len);
	std::free(c.buf);
	c.buf = nullptr;
	c.len = 0;
	return out;
}

std::string
escape(const char *p, size_t n)
{
	std::string r;
	char tmp[8];
	for (size_t i = 0; i < n; ++i) {
		unsigned char u = static_cast<unsigned char>(p[i]);
		if (u == '\\') {
			r += "\\\\";
		} else if (u == '\n') {
			r += "\\n";
		} else if (u >= 0x20 && u < 0x7f) {
			r += static_cast<char>(u);
		} else {
			std::snprintf(tmp, sizeof(tmp), "\\x%02x", u);
			r += tmp;
		}
	}
	return r;
}

std::string
escape(const std::string &s)
{
	return escape(s.data(), s.size());
}

struct Tally {
	const char *name;
	long cases = 0;
	long fails = 0;
	long reported = 0;

	explicit Tally(const char *n) : name(n) {}

	void note_fail() { ++fails; ++reported; }
};

// Guard slots trail the NULL terminator of every argv array; both copies are
// pre-filled with 0x7f bytes so any write past the window is visible.
constexpr size_t GUARD_SLOTS = 4;
char *const GUARD_PTR = reinterpret_cast<char *>(static_cast<std::uintptr_t>(
    sizeof(char *) == 8 ? 0x7f7f7f7f7f7f7f7full : 0x7f7f7f7ful));

// ------------------------------------------------------- argv array plumbing

struct ArgvPair {
	std::vector<char *> a;	// fed to the oracle
	std::vector<char *> b;	// fed to the port
	size_t cap = 0;
};

void
build_pair(ArgvPair &ap, char *const *elems, size_t n)
{
	ap.cap = n + 1 + GUARD_SLOTS;
	ap.a.assign(ap.cap, nullptr);
	ap.b.assign(ap.cap, nullptr);
	std::memset(ap.a.data(), 0x7f, ap.cap * sizeof(char *));
	std::memset(ap.b.data(), 0x7f, ap.cap * sizeof(char *));
	for (size_t i = 0; i < n; ++i) {
		ap.a[i] = elems[i];
		ap.b[i] = elems[i];
	}
	ap.a[n] = nullptr;
	ap.b[n] = nullptr;
}

// Slot identity expressed as an index into the shared string table, never a
// raw address: >=0 pool index, -1 NULL, -2 untouched guard, -3 foreign.
long
slot_id(char *p, char *const *pool, size_t pool_n)
{
	if (p == nullptr)
		return -1;
	if (p == GUARD_PTR)
		return -2;
	for (size_t i = 0; i < pool_n; ++i)
		if (p == pool[i])
			return static_cast<long>(i);
	return -3;
}

std::string
describe(char *const *arr, size_t cap, char *const *pool, size_t pool_n)
{
	std::string r = "[";
	char tmp[32];
	for (size_t i = 0; i < cap; ++i) {
		if (i)
			r += ",";
		std::snprintf(tmp, sizeof(tmp), "%ld", slot_id(arr[i], pool, pool_n));
		r += tmp;
	}
	r += "]";
	return r;
}

std::string
describe_input(char *const *elems, size_t n)
{
	std::string r = "{";
	for (size_t i = 0; i < n; ++i) {
		if (i)
			r += ", ";
		r += '"';
		r += escape(elems[i], std::strlen(elems[i]));
		r += '"';
	}
	r += "}";
	return r;
}

// ------------------------------------------------ checkslash / checkdot pool

char POOL[][16] = {
	"/",		// ISSLASH true
	"//",		// ISSLASH false on the second byte
	"///",
	"/a",
	"a/",
	"",		// empty string
	".",		// ISDOT true via !(a)[1]
	"..",		// ISDOT true via (a)[1]=='.' && !(a)[2]
	"...",		// ISDOT false on (a)[2]
	".a",		// ISDOT false on (a)[1]
	"a.",
	"a",
	"/.",
	"/..",
	"/...",
	"a/.",
	"a/..",
	"a/...",
	"b/c/.",
	"b/c/..",
	"./",		// basename after the last slash is empty
	"../",
	"\x80",		// high-bit single byte
	"\xff",
	"\xfe\xfd\xfc",
	"/\x80",
	"\x80/",
	"\x80/.",
	"\x80/..",
	".\x80",
	"..\x80",
	"..a",
	"a..b",
	"//.",
	"//..",
	"x/y/",
	"-",
	"--",
	"/tmp",
	"\x7f",
};
constexpr size_t POOL_N = sizeof(POOL) / sizeof(POOL[0]);

char *
pool_ptr(size_t i)
{
	return POOL[i];
}

std::vector<char *> &
string_pool()
{
	static std::vector<char *> p = [] {
		std::vector<char *> v;
		for (size_t i = 0; i < POOL_N; ++i)
			v.push_back(pool_ptr(i));
		return v;
	}();
	return p;
}

// Small exhaustively-enumerated core: hits both sides of every byte test in
// ISSLASH and ISDOT plus the strrchr-hit and strrchr-miss paths.
const size_t CORE[] = { 0, 1, 5, 6, 7, 8, 11, 16 };
constexpr size_t CORE_N = sizeof(CORE) / sizeof(CORE[0]);

// Entries that make the removal branches fire often under random generation.
const size_t HOT[] = { 0, 6, 7, 12, 13, 15, 16, 20, 26, 28 };
constexpr size_t HOT_N = sizeof(HOT) / sizeof(HOT[0]);

size_t
pick_index(Rng &r)
{
	if (r.below(3) == 0)
		return HOT[r.below(HOT_N)];
	return r.below(static_cast<std::uint32_t>(POOL_N));
}

// ------------------------------------------------------- generic comparator

bool
compare_case(Tally &t, char *const *elems, size_t n, int eval_init,
    void (*ref_fn)(char **), void (*port_fn)(char **), const char *label)
{
	ArgvPair ap;
	build_pair(ap, elems, n);

	Capture c1;
	eval = eval_init;
	cap_begin(c1);
	ref_fn(ap.a.data());
	std::string err_ref = cap_end(c1);
	int eval_ref = eval;

	Capture c2;
	P::eval = eval_init;
	cap_begin(c2);
	port_fn(ap.b.data());
	std::string err_port = cap_end(c2);
	int eval_port = P::eval;

	++t.cases;

	std::vector<char *> &pool = string_pool();
	bool bytes_ok = std::memcmp(ap.a.data(), ap.b.data(),
	    ap.cap * sizeof(char *)) == 0;
	bool ids_ok = true;
	for (size_t i = 0; i < ap.cap; ++i) {
		if (slot_id(ap.a[i], pool.data(), POOL_N) !=
		    slot_id(ap.b[i], pool.data(), POOL_N)) {
			ids_ok = false;
			break;
		}
	}

	if (bytes_ok && ids_ok && eval_ref == eval_port && err_ref == err_port)
		return true;

	t.note_fail();
	if (t.reported <= 8) {
		std::printf("  FAIL %s %s eval_init=%d\n", label,
		    describe_input(elems, n).c_str(), eval_init);
		std::printf("    ref  arr=%s eval=%d err=\"%s\"\n",
		    describe(ap.a.data(), ap.cap, pool.data(), POOL_N).c_str(),
		    eval_ref, escape(err_ref).c_str());
		std::printf("    port arr=%s eval=%d err=\"%s\"\n",
		    describe(ap.b.data(), ap.cap, pool.data(), POOL_N).c_str(),
		    eval_port, escape(err_port).c_str());
	}
	return false;
}

// -------------------------------------------- checkslash / checkdot drivers

void
enumerate_arrays(Tally &t, void (*ref_fn)(char **), void (*port_fn)(char **),
    const char *label)
{
	char *elems[8];

	// Exhaustive over the core alphabet for lengths 0..4.
	for (size_t n = 0; n <= 4; ++n) {
		size_t total = 1;
		for (size_t k = 0; k < n; ++k)
			total *= CORE_N;
		for (size_t code = 0; code < total; ++code) {
			size_t c = code;
			for (size_t k = 0; k < n; ++k) {
				elems[k] = pool_ptr(CORE[c % CORE_N]);
				c /= CORE_N;
			}
			compare_case(t, elems, n, 0, ref_fn, port_fn, label);
		}
	}

	// Non-zero initial eval so a mutated "eval = 1" cannot hide behind an
	// already-zero flag.
	static const size_t specials[] = { 0, 1, 5, 6, 7, 22, 23, 26 };
	for (size_t i = 0; i < sizeof(specials) / sizeof(specials[0]); ++i) {
		elems[0] = pool_ptr(specials[i]);
		compare_case(t, elems, 1, 0, ref_fn, port_fn, label);
		compare_case(t, elems, 1, 1, ref_fn, port_fn, label);
		compare_case(t, elems, 1, 7, ref_fn, port_fn, label);
	}

	// Every single pool entry on its own, and every ordered pair.
	for (size_t i = 0; i < POOL_N; ++i) {
		elems[0] = pool_ptr(i);
		compare_case(t, elems, 1, 0, ref_fn, port_fn, label);
		compare_case(t, elems, 1, 3, ref_fn, port_fn, label);
		for (size_t j = 0; j < POOL_N; ++j) {
			elems[1] = pool_ptr(j);
			compare_case(t, elems, 2, 0, ref_fn, port_fn, label);
		}
	}

	// Long runs of removable entries: drives the shifting loop well past
	// its first iteration and the "complained" latch past its first hit.
	for (size_t n = 1; n <= 8; ++n) {
		for (size_t which = 0; which < HOT_N; ++which) {
			for (size_t k = 0; k < n; ++k)
				elems[k] = pool_ptr(HOT[which]);
			compare_case(t, elems, n, 0, ref_fn, port_fn, label);
		}
		for (size_t k = 0; k < n; ++k)
			elems[k] = pool_ptr((k % 2) ? 11 : 0);
		compare_case(t, elems, n, 0, ref_fn, port_fn, label);
		for (size_t k = 0; k < n; ++k)
			elems[k] = pool_ptr((k % 2) ? 6 : 11);
		compare_case(t, elems, n, 0, ref_fn, port_fn, label);
		for (size_t k = 0; k < n; ++k)
			elems[k] = pool_ptr((k % 3) ? 7 : 8);
		compare_case(t, elems, n, 0, ref_fn, port_fn, label);
	}

	// Fixed-seed randomised sweep.
	Rng r(0xb0198u ^ static_cast<std::uint32_t>(label[5]));
	for (long it = 0; it < 200000; ++it) {
		size_t n = r.below(8);
		for (size_t k = 0; k < n; ++k)
			elems[k] = pool_ptr(pick_index(r));
		int ev = static_cast<int>(r.below(3));
		compare_case(t, elems, n, ev, ref_fn, port_fn, label);
	}
}

// ------------------------------------------------------------- check2 setup

std::vector<std::string> c2_names;	// stable storage for the path strings
std::vector<char *> c2_pool;
std::string c2_dir;

void
c2_fixture()
{
	char tmpl[] = "/tmp/pbsd_b0198_XXXXXX";
	const char *d = mkdtemp(tmpl);
	if (d == nullptr) {
		std::printf("FATAL: mkdtemp failed\n");
		std::exit(2);
	}
	c2_dir = d;

	const char *files[] = { "f0", "f1", "f2", "f3", "f4", "\x80" "f" };
	for (const char *f : files) {
		std::string p = c2_dir + "/" + f;
		FILE *fp = std::fopen(p.c_str(), "w");
		if (fp == nullptr) {
			std::printf("FATAL: cannot create %s\n", p.c_str());
			std::exit(2);
		}
		std::fputc('x', fp);
		std::fclose(fp);
		c2_names.push_back(p);
	}
	const char *dirs[] = { "d0", "d1", "d2", "d3", "\xff" "d" };
	for (const char *dn : dirs) {
		std::string p = c2_dir + "/" + dn;
		if (mkdir(p.c_str(), 0700) != 0) {
			std::printf("FATAL: cannot create dir %s\n", p.c_str());
			std::exit(2);
		}
		c2_names.push_back(p);
	}
	// lstat() sees the link itself, so these count as files even when the
	// target is a directory.
	std::string l0 = c2_dir + "/l0";
	::symlink((c2_dir + "/f0").c_str(), l0.c_str());
	c2_names.push_back(l0);
	std::string ld = c2_dir + "/ld";
	::symlink((c2_dir + "/d0").c_str(), ld.c_str());
	c2_names.push_back(ld);

	// Names lstat() cannot resolve: neither counter moves.
	c2_names.push_back(c2_dir + "/nx0");
	c2_names.push_back(c2_dir + "/nx1");
	c2_names.push_back(c2_dir + "/nx2");
	c2_names.push_back("");
	c2_names.push_back("/nonexistent-pbsd-b0198");
	// Directories outside the fixture.
	c2_names.push_back(".");
	c2_names.push_back("/");

	c2_pool.clear();
	for (std::string &s : c2_names)
		c2_pool.push_back(&s[0]);
}

void
c2_cleanup()
{
	if (c2_dir.empty())
		return;
	for (std::string &s : c2_names) {
		if (s.size() > c2_dir.size() &&
		    s.compare(0, c2_dir.size(), c2_dir) == 0) {
			::unlink(s.c_str());
			::rmdir(s.c_str());
		}
	}
	::rmdir(c2_dir.c_str());
}

// Pool indices: plain file, directory, symlink, missing name.
size_t C2_CORE[4];

struct C2Out {
	int ret;
	long pos;
	std::string err;
};

C2Out
call_check2(int which, char **argv, int rf, const std::string &in)
{
	std::vector<char> ib(in.begin(), in.end());
	ib.push_back('\0');	// backing store only; the stream size is in.size()

	FILE *saved_in = stdin;
	FILE *ms = fmemopen(ib.data(), in.size(), "r");
	if (ms == nullptr) {
		std::printf("FATAL: fmemopen failed\n");
		std::exit(2);
	}
	stdin = ms;

	Capture c;
	cap_begin(c);
	int r;
	if (which == 0) {
		rflag = rf;
		r = ref_check2(argv);
	} else {
		P::rflag = rf;
		r = P::check2(argv);
	}
	std::string err = cap_end(c);

	long pos = std::ftell(stdin);
	std::fclose(stdin);
	stdin = saved_in;

	C2Out o;
	o.ret = r;
	o.pos = pos;
	o.err = err;
	return o;
}

bool
c2_case(Tally &t, char *const *elems, size_t n, int rf, const std::string &in)
{
	ArgvPair ap;
	build_pair(ap, elems, n);

	C2Out a = call_check2(0, ap.a.data(), rf, in);
	C2Out b = call_check2(1, ap.b.data(), rf, in);

	++t.cases;

	bool bytes_ok = std::memcmp(ap.a.data(), ap.b.data(),
	    ap.cap * sizeof(char *)) == 0;
	bool ids_ok = true;
	for (size_t i = 0; i < ap.cap; ++i) {
		if (slot_id(ap.a[i], c2_pool.data(), c2_pool.size()) !=
		    slot_id(ap.b[i], c2_pool.data(), c2_pool.size())) {
			ids_ok = false;
			break;
		}
	}

	if (a.ret == b.ret && a.pos == b.pos && a.err == b.err && bytes_ok &&
	    ids_ok)
		return true;

	t.note_fail();
	if (t.reported <= 8) {
		std::printf("  FAIL check2 n=%zu rflag=%d stdin=\"%s\"\n", n, rf,
		    escape(in).c_str());
		for (size_t i = 0; i < n; ++i)
			std::printf("      argv[%zu]=\"%s\"\n", i,
			    escape(elems[i], std::strlen(elems[i])).c_str());
		std::printf("    ref  ret=%d pos=%ld err=\"%s\" arr=%s\n", a.ret,
		    a.pos, escape(a.err).c_str(),
		    describe(ap.a.data(), ap.cap, c2_pool.data(),
			c2_pool.size()).c_str());
		std::printf("    port ret=%d pos=%ld err=\"%s\" arr=%s\n", b.ret,
		    b.pos, escape(b.err).c_str(),
		    describe(ap.b.data(), ap.cap, c2_pool.data(),
			c2_pool.size()).c_str());
	}
	return false;
}

const char *C2_REPLIES[] = {
	"",		// immediate EOF
	"\n",		// blank line, then EOF on the retry
	"y",		// accepted without a terminating newline
	"y\n",
	"Y\n",
	"n\n",
	"N\n",
	"N",
	"yes\n",	// 'y' first, trailing junk drained
	"nope\n",
	"a\nY\n",	// two prompts before the answer
	"a\nb\nn\n",
	"\n\n\ny\n",	// four prompts
	"q",
	"\x80\ny\n",	// high-bit reply byte
	"\xff\xfe\nN\n",
	"z\n",
	"Yn\n",		// 'Y' first wins, the rest is drained
	"nY\n",		// 'n' first wins
	" y\n",		// the space is the answer, so another prompt follows
};
constexpr size_t C2_REPLIES_N = sizeof(C2_REPLIES) / sizeof(C2_REPLIES[0]);

std::string
c2_random_reply(Rng &r)
{
	if (r.below(4) == 0)
		return std::string(C2_REPLIES[r.below(C2_REPLIES_N)]);
	size_t len = r.below(9);
	std::string s;
	for (size_t i = 0; i < len; ++i) {
		switch (r.below(10)) {
		case 0: s += '\n'; break;
		case 1: s += 'y'; break;
		case 2: s += 'Y'; break;
		case 3: s += 'n'; break;
		case 4: s += 'N'; break;
		case 5: s += '\0'; break;
		case 6: s += static_cast<char>(0x80 + r.below(0x80)); break;
		default: s += static_cast<char>(0x20 + r.below(95)); break;
		}
	}
	return s;
}

void
run_check2(Tally &t)
{
	char *elems[10];

	// Exhaustive over {file, dir, symlink, missing} for lengths 0..4
	// against both rflag settings and every canned reply.  Lengths 3 and 4
	// straddle the "dcount + fcount > 3" boundary from both sides.
	for (size_t n = 0; n <= 4; ++n) {
		size_t total = 1;
		for (size_t k = 0; k < n; ++k)
			total *= 4;
		for (size_t code = 0; code < total; ++code) {
			size_t c = code;
			for (size_t k = 0; k < n; ++k) {
				elems[k] = c2_pool[C2_CORE[c % 4]];
				c /= 4;
			}
			for (int rf = 0; rf <= 1; ++rf)
				for (size_t rep = 0; rep < C2_REPLIES_N; ++rep)
					c2_case(t, elems, n, rf, C2_REPLIES[rep]);
		}
	}

	// Explicit count boundaries: 0..6 files crossed with 0..3 directories.
	for (size_t nf = 0; nf <= 6; ++nf) {
		for (size_t nd = 0; nd <= 3; ++nd) {
			size_t n = 0;
			for (size_t k = 0; k < nf; ++k)
				elems[n++] = c2_pool[k];	// f0..f4 plus the high-bit file
			for (size_t k = 0; k < nd; ++k)
				elems[n++] = c2_pool[6 + k];	// d0..d2
			for (int rf = 0; rf <= 1; ++rf)
				for (size_t rep = 0; rep < C2_REPLIES_N; ++rep)
					c2_case(t, elems, n, rf, C2_REPLIES[rep]);
		}
	}

	// Every pool entry on its own, and each one padded out to four names so
	// the dcount/fcount totals cross the prompt threshold.
	for (size_t i = 0; i < c2_pool.size(); ++i) {
		elems[0] = c2_pool[i];
		for (int rf = 0; rf <= 1; ++rf) {
			c2_case(t, elems, 1, rf, "y\n");
			c2_case(t, elems, 1, rf, "");
		}
		elems[1] = c2_pool[6];
		elems[2] = c2_pool[7];
		elems[3] = c2_pool[8];
		for (int rf = 0; rf <= 1; ++rf) {
			c2_case(t, elems, 4, rf, "a\nY\n");
			c2_case(t, elems, 4, rf, "n\n");
		}
	}

	Rng r(0xc2b0198u);
	for (long it = 0; it < 200000; ++it) {
		size_t n = r.below(9);
		for (size_t k = 0; k < n; ++k)
			elems[k] = c2_pool[r.below(
			    static_cast<std::uint32_t>(c2_pool.size()))];
		int rf = static_cast<int>(r.below(2));
		c2_case(t, elems, n, rf, c2_random_reply(r));
	}
}

// ------------------------------------------------------------------ siginfo

void
run_siginfo(Tally &t)
{
	static const int presets[] = { 0, 1, -1, 2, 127, -128, 255, 0x7fffffff,
	    -0x7fffffff - 1 };
	static const int sigs[] = { 0, 1, 29, -1, 65535, 0x7fffffff };

	for (int pre : presets) {
		for (int sg : sigs) {
			info = static_cast<sig_atomic_t>(pre);
			P::info = static_cast<sig_atomic_t>(pre);
			Capture c1;
			cap_begin(c1);
			ref_siginfo(sg);
			std::string e1 = cap_end(c1);
			Capture c2;
			cap_begin(c2);
			P::siginfo(sg);
			std::string e2 = cap_end(c2);
			int ra = static_cast<int>(info);
			int rb = static_cast<int>(P::info);
			++t.cases;
			if (ra != rb || e1 != e2) {
				t.note_fail();
				if (t.reported <= 8)
					std::printf("  FAIL siginfo pre=%d sig=%d"
					    " ref=%d port=%d\n", pre, sg, ra, rb);
			}
		}
	}

	Rng r(0x516e0f0u);
	for (long it = 0; it < 200000; ++it) {
		int pre = static_cast<int>(r.next());
		int sg = static_cast<int>(r.next());
		info = static_cast<sig_atomic_t>(pre);
		P::info = static_cast<sig_atomic_t>(pre);
		ref_siginfo(sg);
		int ra = static_cast<int>(info);
		P::siginfo(sg);
		int rb = static_cast<int>(P::info);
		++t.cases;
		if (ra != rb) {
			t.note_fail();
			if (t.reported <= 8)
				std::printf("  FAIL siginfo pre=%d sig=%d ref=%d"
				    " port=%d\n", pre, sg, ra, rb);
		}
	}
}

} // namespace

// --------------------------------------------------------------------- main

int
main()
{
	std::setvbuf(stdout, nullptr, _IOLBF, 0);

	Tally t_slash("checkslash");
	Tally t_dot("checkdot");
	Tally t_check2("check2");
	Tally t_siginfo("siginfo");

	std::printf("pbsd batch b0198 -- differential test vs oracle\n\n");

	enumerate_arrays(t_slash, ref_checkslash, P::checkslash, "checkslash");
	enumerate_arrays(t_dot, ref_checkdot, P::checkdot, "checkdot");

	c2_fixture();
	C2_CORE[0] = 0;		// regular file
	C2_CORE[1] = 6;		// directory
	C2_CORE[2] = 11;	// symlink to a regular file
	C2_CORE[3] = 13;	// non-existent name
	run_check2(t_check2);
	c2_cleanup();

	run_siginfo(t_siginfo);

	Tally *all[] = { &t_slash, &t_dot, &t_check2, &t_siginfo };
	long total_cases = 0, total_fails = 0;

	std::printf("\n%-16s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("--------------------------------------------------------\n");
	for (Tally *t : all) {
		total_cases += t->cases;
		total_fails += t->fails;
		std::printf("%-16s %12ld %12ld  %s\n", t->name, t->cases, t->fails,
		    t->fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("--------------------------------------------------------\n");
	std::printf("%-16s %12ld %12ld  %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "PASS" : "FAIL");

	return total_fails == 0 ? 0 : 1;
}
