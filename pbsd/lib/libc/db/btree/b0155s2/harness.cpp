/*
 * Differential test harness for batch b0155s2
 * (hbsd/src/lib/libc/db/btree/bt_open.c).
 *
 * Each ported function is called side by side with its ref_ oracle and the
 * whole observable result is compared:
 *
 *	byteorder()	the return value.
 *
 *	tmp()		the return value; errno when the call fails; the
 *			FD_CLOEXEC flag of the descriptor handed back; the
 *			process signal mask left behind by the call; the net
 *			change in the contents of the temporary directory,
 *			which is how the unlink() of the file mkostemp() just
 *			created is observed; and whether a signal that was
 *			blocked and pending on entry got delivered inside the
 *			call, which is how the SIG_BLOCK / SIG_SETMASK pair is
 *			observed.
 *
 * Neither function writes through a caller-supplied buffer and neither is a
 * stateful iterator, so the guard-byte buffer protocol and the
 * drive-to-exhaustion protocol have no subject here.  tmp()'s hidden state is
 * its internal MAXPATHLEN path buffer; it is observed indirectly through the
 * returned errno (which distinguishes its own ENAMETOOLONG from every kernel
 * error the assembled path can produce) and through the directory contents.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

import pbsd.lib.libc.db.btree.b0155s2;

extern "C" int ref_tmp(void);
extern "C" int ref_byteorder(void);

namespace bport = pbsd::lib_libc_db_btree::b0155s2;

static int port_tmp(void) { return bport::tmp(); }
static int port_byteorder(void) { return bport::byteorder(); }

/* ------------------------------------------------------------------ */
/* bookkeeping								*/
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	long cases;
	long fail;
	long reported;
};

static const long MAX_REPORT = 12;

static void
note(Stat &st, const char *what, const char *detail)
{
	st.fail++;
	if (st.reported < MAX_REPORT) {
		st.reported++;
		std::fprintf(stderr, "FAIL %s: %s differs [%s]\n", st.name,
		    what, detail);
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64)						*/
/* ------------------------------------------------------------------ */

static std::uint64_t rngstate;

static std::uint64_t
nextrand(void)
{
	rngstate += 0x9E3779B97F4A7C15ULL;
	std::uint64_t z = rngstate;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

/* Any byte a TMPDIR value can hold: 0x01 .. 0xff, high bit included. */
static char
rand_byte(void)
{
	return (char)(unsigned char)(1u + (unsigned)(nextrand() % 255u));
}

/* ------------------------------------------------------------------ */
/* scratch state							*/
/* ------------------------------------------------------------------ */

static volatile sig_atomic_t g_sigcount;

extern "C" void
usr1_handler(int)
{
	g_sigcount = (sig_atomic_t)(g_sigcount + 1);
}

static char g_dir[] = "/tmp/pbsd_b0155s2_XXXXXX";
static std::string g_plainfile;	/* exists, but is a regular file */
static std::string g_nowrite;	/* exists, is a directory, not writable */
static std::string g_missing;	/* does not exist */

static long
dircount(const char *dir)
{
	DIR *d = opendir(dir);
	if (d == nullptr)
		return -1;
	long n = 0;
	struct dirent *e;
	while ((e = readdir(d)) != nullptr) {
		if (std::strcmp(e->d_name, ".") == 0 ||
		    std::strcmp(e->d_name, "..") == 0)
			continue;
		n++;
	}
	closedir(d);
	return n;
}

/*
 * Drop any bt.* file a broken port left behind so the directory cannot grow
 * without bound over the sweep.  Only ever called between measurements.
 */
static void
cleanup_bt(const char *dir)
{
	DIR *d = opendir(dir);
	if (d == nullptr)
		return;
	std::vector<std::string> names;
	struct dirent *e;
	while ((e = readdir(d)) != nullptr) {
		if (std::strncmp(e->d_name, "bt.", 3) == 0)
			names.push_back(e->d_name);
	}
	closedir(d);
	for (std::size_t i = 0; i < names.size(); i++) {
		std::string p = std::string(dir) + "/" + names[i];
		unlink(p.c_str());
	}
}

/*
 * A path of exactly n bytes made of short components below a directory that
 * does not exist.  No component reaches NAME_MAX, so the kernel's answer for
 * such a path is ENOENT; that keeps ENAMETOOLONG the exclusive property of
 * tmp()'s own length test and makes the two sides of that test tell apart.
 */
static std::string
make_long_path(std::size_t n)
{
	std::string s = "/tmp/pbsd_nx_b0155s2";
	if (n <= s.size())
		return s.substr(0, n);
	while (s.size() < n) {
		if (n - s.size() <= 9) {
			s.push_back('/');
			while (s.size() < n)
				s.push_back('a');
			break;
		}
		s += "/aaaaaaaa";
	}
	if (s.size() > n)
		s.resize(n);
	return s;
}

/* ------------------------------------------------------------------ */
/* tmp() observation							*/
/* ------------------------------------------------------------------ */

struct Obs {
	int ret;
	int err;
	int cloexec;
	int delivered;
	long dirdelta;
	unsigned char maskbits[64];
};

/*
 * Run one implementation.  SIGUSR1 is blocked and made pending beforehand, so
 * a call that widens rather than narrows the mask -- or that fails to put the
 * old mask back -- lets the handler run and is caught by `delivered'.
 */
static Obs
run_tmp(int (*fn)(void), const char *scan)
{
	Obs o;
	std::memset(&o, 0, sizeof(o));

	long before = -1;
	if (scan != nullptr)
		before = dircount(scan);

	sigset_t usr1, prevmask;
	sigemptyset(&usr1);
	sigaddset(&usr1, SIGUSR1);
	sigemptyset(&prevmask);
	sigprocmask(SIG_BLOCK, &usr1, &prevmask);
	g_sigcount = 0;
	raise(SIGUSR1);

	errno = 0;
	int r = fn();
	int e = errno;

	o.delivered = (int)g_sigcount;
	o.ret = r;
	o.err = e;
	o.cloexec = -2;
	if (r >= 0)
		o.cloexec = fcntl(r, F_GETFD);

	sigset_t cur;
	sigemptyset(&cur);
	if (sigprocmask(SIG_SETMASK, nullptr, &cur) != 0)
		sigemptyset(&cur);
	for (int s = 1; s <= 64; s++)
		o.maskbits[s - 1] = (unsigned char)(sigismember(&cur, s) == 1);

	long after = -1;
	if (scan != nullptr)
		after = dircount(scan);
	o.dirdelta = (scan != nullptr) ? after - before : 0;

	if (r >= 0)
		close(r);

	/* Restoring the mask drains the pending SIGUSR1 if it is still there. */
	sigprocmask(SIG_SETMASK, &prevmask, nullptr);
	return o;
}

static const char *
obs_diff(const Obs &a, const Obs &b)
{
	if (a.ret != b.ret)
		return "return value";
	if ((a.ret < 0 || b.ret < 0) && a.err != b.err)
		return "errno";
	if (a.cloexec != b.cloexec)
		return "descriptor flags";
	if (a.delivered != b.delivered)
		return "signal delivery inside the call";
	if (a.dirdelta != b.dirdelta)
		return "temporary directory residue";
	if (std::memcmp(a.maskbits, b.maskbits, sizeof(a.maskbits)) != 0)
		return "signal mask after the call";
	return nullptr;
}

static void
set_tmpdir(const char *v)
{
	if (v == nullptr)
		unsetenv("TMPDIR");
	else
		setenv("TMPDIR", v, 1);
}

static void
describe(char *buf, std::size_t bufsz, const char *v, const Obs &a,
    const Obs &b)
{
	char shown[96];
	if (v == nullptr) {
		std::snprintf(shown, sizeof(shown), "<unset>");
	} else {
		std::size_t n = std::strlen(v);
		std::size_t k = 0;
		for (std::size_t i = 0; i < n && k + 6 < sizeof(shown) - 16;
		    i++) {
			unsigned char c = (unsigned char)v[i];
			if (c >= 0x20 && c < 0x7f)
				shown[k++] = (char)c;
			else
				k += (std::size_t)std::snprintf(shown + k,
				    sizeof(shown) - k, "\\x%02x", c);
		}
		shown[k] = '\0';
		std::snprintf(shown + k, sizeof(shown) - k, "|len=%zu", n);
	}
	std::snprintf(buf, bufsz,
	    "TMPDIR=%s ref{ret=%d errno=%d fdfl=%d sig=%d dirdelta=%ld} "
	    "port{ret=%d errno=%d fdfl=%d sig=%d dirdelta=%ld}",
	    shown, a.ret, a.err, a.cloexec, a.delivered, a.dirdelta, b.ret,
	    b.err, b.cloexec, b.delivered, b.dirdelta);
}

static void
tmp_compare(Stat &st, const char *v, const Obs &a, const Obs &b)
{
	st.cases++;
	const char *d = obs_diff(a, b);
	if (d != nullptr) {
		char buf[512];
		describe(buf, sizeof(buf), v, a, b);
		note(st, d, buf);
	}
}

static void
tmp_case(Stat &st, const char *v, bool scan)
{
	set_tmpdir(v);
	const char *sc = scan ? g_dir : nullptr;

	Obs a = run_tmp(ref_tmp, sc);
	Obs b = run_tmp(port_tmp, sc);
	tmp_compare(st, v, a, b);
}

/*
 * The same comparison with descriptor 0 closed first, so that mkostemp()
 * hands back 0.  This is what separates `fd != -1' from a mutated `fd != 0':
 * under the mutation the file is not unlinked and the directory residue
 * differs.
 */
static void
tmp_case_fd_zero(Stat &st, const char *v)
{
	set_tmpdir(v);

	int saved = dup(0);
	if (saved < 0)
		return;

	/* run_tmp() closes the descriptor it observed, freeing 0 again. */
	close(0);
	Obs a = run_tmp(ref_tmp, g_dir);
	close(0);
	Obs b = run_tmp(port_tmp, g_dir);

	dup2(saved, 0);
	close(saved);

	tmp_compare(st, v, a, b);

	if (a.ret != 0) {
		char buf[512];
		describe(buf, sizeof(buf), v, a, b);
		note(st, "fd-zero precondition (oracle did not return fd 0)",
		    buf);
	}
}

/* ------------------------------------------------------------------ */
/* tmp() hand-written edge cases					*/
/* ------------------------------------------------------------------ */

static void
tmp_edges(Stat &st)
{
	const std::size_t suffix = std::strlen("/bt.XXXXXXXXXX");	/* 14 */
	const std::size_t lim = (std::size_t)MAXPATHLEN;

	/* Absent value: the "/tmp" arm of the ternary. */
	tmp_case(st, nullptr, false);

	/* Empty value: present, so the envtmp arm is taken, and length 0. */
	tmp_case(st, "", false);

	/* Single-character values, including high-bit bytes. */
	tmp_case(st, "/", false);
	tmp_case(st, "x", false);
	tmp_case(st, "\x01", false);
	tmp_case(st, "\x7f", false);
	tmp_case(st, "\x80", false);
	tmp_case(st, "\xfe", false);
	tmp_case(st, "\xff", false);

	/* The success path, with the created file observed and unlinked. */
	tmp_case(st, g_dir, true);
	tmp_case(st, g_dir, true);
	tmp_case_fd_zero(st, g_dir);

	/* Trailing separators, still naming the same writable directory. */
	{
		std::string s = g_dir;
		tmp_case(st, (s + "/").c_str(), true);
		tmp_case(st, (s + "//").c_str(), true);
		tmp_case(st, (s + "/.").c_str(), true);
		tmp_case(st, (s + "/./").c_str(), true);
	}

	/* Existing name that is not a directory -> ENOTDIR. */
	tmp_case(st, g_plainfile.c_str(), false);
	/* Existing directory without the write bit -> EACCES unless root. */
	tmp_case(st, g_nowrite.c_str(), false);
	/* Missing intermediate component -> ENOENT. */
	tmp_case(st, g_missing.c_str(), false);
	/* Writable directory plus one missing component -> ENOENT. */
	tmp_case(st, (std::string(g_dir) + "/absent").c_str(), false);

	/* Every byte value a value can carry, and the high half alone. */
	{
		std::string s;
		for (int i = 1; i < 256; i++)
			if (i != '/')
				s.push_back((char)(unsigned char)i);
		tmp_case(st, s.c_str(), false);
	}
	{
		std::string s;
		for (int i = 0x80; i < 0x100; i++)
			s.push_back((char)(unsigned char)i);
		tmp_case(st, s.c_str(), false);
	}

	/*
	 * Both sides of tmp()'s own length test.  len == strlen(TMPDIR) + 14
	 * and the test is `len >= (int)sizeof(path)' with sizeof(path) ==
	 * MAXPATHLEN:
	 *
	 *	strlen == lim - suffix - 1  ->  len == lim - 1	accepted
	 *	strlen == lim - suffix	    ->  len == lim	rejected
	 */
	for (std::size_t d = 4; d >= 1; d--)
		tmp_case(st, make_long_path(lim - suffix - d).c_str(), false);
	for (std::size_t d = 0; d <= 4; d++)
		tmp_case(st, make_long_path(lim - suffix + d).c_str(), false);

	/* Well past the boundary, and a spread of shorter lengths. */
	tmp_case(st, make_long_path(lim).c_str(), false);
	tmp_case(st, make_long_path(lim + 1).c_str(), false);
	tmp_case(st, make_long_path(lim + 500).c_str(), false);
	tmp_case(st, make_long_path(2 * lim).c_str(), false);
	tmp_case(st, make_long_path(4 * lim).c_str(), false);
	tmp_case(st, make_long_path(lim / 2).c_str(), false);
	tmp_case(st, make_long_path(21).c_str(), false);
	tmp_case(st, make_long_path(64).c_str(), false);
	tmp_case(st, make_long_path(255).c_str(), false);
	tmp_case(st, make_long_path(256).c_str(), false);
	tmp_case(st, make_long_path(257).c_str(), false);

	cleanup_bt(g_dir);
}

/* ------------------------------------------------------------------ */
/* tmp() randomised sweep						*/
/* ------------------------------------------------------------------ */

static void
tmp_sweep(Stat &st, long iters)
{
	const std::size_t suffix = std::strlen("/bt.XXXXXXXXXX");
	const std::size_t lim = (std::size_t)MAXPATHLEN;

	rngstate = 0x0B0155525EED01ULL;		/* fixed seed */

	std::string s;
	for (long i = 0; i < iters; i++) {
		std::uint64_t r = nextrand();
		int mode = (int)(r & 7u);
		bool scan = false;
		bool unset = false;

		s.clear();
		switch (mode) {
		case 0:
			/* the directory that works: drives the success path */
			s = g_dir;
			scan = ((i % 64) == 0);
			break;
		case 1: {
			/* writable directory plus a random missing component */
			s = g_dir;
			s.push_back('/');
			int n = 1 + (int)((r >> 3) % 8u);
			for (int k = 0; k < n; k++)
				s.push_back(rand_byte());
			break;
		}
		case 2: {
			/* short random garbage, any byte value */
			int n = 1 + (int)((r >> 3) % 32u);
			for (int k = 0; k < n; k++)
				s.push_back(rand_byte());
			break;
		}
		case 3:
			/* lengths straddling tmp()'s MAXPATHLEN test */
			s = make_long_path(lim - suffix - 20 +
			    (std::size_t)((r >> 3) % 41u));
			break;
		case 4:
			unset = true;
			break;
		case 5:
			/* present but empty */
			break;
		case 6: {
			/* the writable directory with random trailing slashes */
			s = g_dir;
			int n = (int)((r >> 3) % 4u);
			for (int k = 0; k < n; k++)
				s.push_back('/');
			scan = ((i % 64) == 0);
			break;
		}
		default:
			/* exactly at, and exactly one below, the boundary */
			s = make_long_path(((r >> 3) & 1u) ?
			    lim - suffix - 1 : lim - suffix);
			break;
		}

		tmp_case(st, unset ? nullptr : s.c_str(), scan);

		if ((i % 4096) == 0)
			cleanup_bt(g_dir);
	}
	cleanup_bt(g_dir);
}

/* ------------------------------------------------------------------ */
/* byteorder()								*/
/* ------------------------------------------------------------------ */

static void
byteorder_edges(Stat &st)
{
	/*
	 * byteorder() takes no argument, so its whole input space is the one
	 * call; the sweep below re-checks it under a hot cache and after the
	 * heap and signal traffic the tmp() cases generate.
	 */
	int a = ref_byteorder();
	int b = port_byteorder();
	st.cases++;
	if (a != b) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "ref=%d port=%d", a, b);
		note(st, "return value", buf);
	}
}

static void
byteorder_sweep(Stat &st, long iters)
{
	for (long i = 0; i < iters; i++) {
		int a = ref_byteorder();
		int b = port_byteorder();
		st.cases++;
		if (a != b) {
			char buf[128];
			std::snprintf(buf, sizeof(buf),
			    "iter=%ld ref=%d port=%d", i, a, b);
			note(st, "return value", buf);
		}
	}
}

int
main(void)
{
	if (mkdtemp(g_dir) == nullptr) {
		std::fprintf(stderr, "harness: mkdtemp failed: %s\n",
		    std::strerror(errno));
		return 1;
	}

	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = usr1_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGUSR1, &sa, nullptr) != 0) {
		std::fprintf(stderr, "harness: sigaction failed: %s\n",
		    std::strerror(errno));
		return 1;
	}

	g_plainfile = std::string(g_dir) + "/plain";
	int pf = open(g_plainfile.c_str(), O_CREAT | O_WRONLY, 0600);
	if (pf >= 0)
		close(pf);
	g_nowrite = std::string(g_dir) + "/nowr";
	mkdir(g_nowrite.c_str(), 0500);
	g_missing = std::string(g_dir) + "/absent/deeper";

	Stat s_byteorder = { "byteorder", 0, 0, 0 };
	Stat s_tmp = { "tmp", 0, 0, 0 };

	byteorder_edges(s_byteorder);

	tmp_edges(s_tmp);
	tmp_sweep(s_tmp, 200000);

	byteorder_sweep(s_byteorder, 200000);

	cleanup_bt(g_dir);
	rmdir(g_nowrite.c_str());
	unlink(g_plainfile.c_str());
	rmdir(g_dir);

	const Stat *all[] = { &s_byteorder, &s_tmp };
	long fails = 0;
	std::printf("\n%-14s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-14s %12s %12s  %s\n", "--------------", "------------",
	    "------------", "------");
	for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		std::printf("%-14s %12ld %12ld  %s\n", all[i]->name,
		    all[i]->cases, all[i]->fail,
		    all[i]->fail == 0 ? "ok" : "FAIL");
		fails += all[i]->fail;
	}
	std::printf("\n%s: %ld failure(s)\n", fails == 0 ? "PASS" : "FAIL",
	    fails);
	return fails == 0 ? 0 : 1;
}
