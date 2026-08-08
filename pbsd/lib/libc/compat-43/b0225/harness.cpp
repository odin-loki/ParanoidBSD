/*
 * harness.cpp -- differential test for batch b0225.
 *
 * Compares pbsd::lib_libc_compat_43::b0225 against ref_* from oracle.c.
 * Signal compat functions are exercised through the harness-controlled mock
 * libc_private layer in oracle.c (non-blocking, deterministic).
 */

#define _DEFAULT_SOURCE

#include <cstddef>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.compat_43.b0225;

namespace port = pbsd::lib_libc_compat_43::b0225;

#ifdef __GLIBC__
#define __bits __val
#endif

#ifndef SV_INTERRUPT
#define SV_INTERRUPT 0x00000002
#endif

#ifndef SIG_HOLD
#define SIG_HOLD ((void (*)(int))2)
#endif

extern "C" {
char *ref_getwd(char *buf);
int ref_sigvec(int signo, struct sigvec *sv, struct sigvec *osv);
int ref_sigsetmask(int mask);
int ref_sigblock(int mask);
int ref_sigpause(int mask);
int ref_xsi_sigpause(int sig);
int ref_sighold(int sig);
int ref_sigignore(int sig);
int ref_sigrelse(int sig);
void (*ref_sigset(int sig, void (*disp)(int)))(int);

void pbsd_mock_reset(void);
void pbsd_mock_set_sigaction_ret(int v);
void pbsd_mock_set_sigprocmask_ret(int v);
void pbsd_mock_set_sigsuspend_ret(int v);
void pbsd_mock_set_mask_bits(int mask);
void pbsd_mock_set_handler(int sig, void (*h)(int));
}

/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	long cases;
	long fails;
};

static Stat stats[] = {
	{ "getwd", 0, 0 },
	{ "sigvec", 0, 0 },
	{ "sigsetmask", 0, 0 },
	{ "sigblock", 0, 0 },
	{ "sigpause", 0, 0 },
	{ "xsi_sigpause", 0, 0 },
	{ "sighold", 0, 0 },
	{ "sigignore", 0, 0 },
	{ "sigrelse", 0, 0 },
	{ "sigset", 0, 0 },
};

static constexpr unsigned char GUARD = 0x7f;
static constexpr int RANDOM_ITERS = 200000;
static uint32_t rng_state = 0xB0225U;
static long mismatch_reports = 0;

static uint32_t rng_next(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 17;
	rng_state ^= rng_state << 5;
	return (rng_state);
}

static int rng_int(int lo, int hi)
{
	uint32_t span = (uint32_t)(hi - lo + 1);
	return lo + (int)(rng_next() % span);
}

static void h1(int) {}
static void h2(int) {}
static void h3(int) {}

static void fill_guard(unsigned char *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		b[i] = GUARD;
}

static bool buf_same(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

static void report_mismatch(const char *fn, const char *detail)
{
	if (mismatch_reports++ < 25)
		std::printf("  MISMATCH [%s] %s\n", fn, detail);
}

static void bump_fail(Stat &st)
{
	st.fails++;
}

static void bump_case(Stat &st)
{
	st.cases++;
}

/* ------------------------------------------------------------------ */
/* getwd                                                             */
/* ------------------------------------------------------------------ */

struct GetwdResult {
	char *ret;
	unsigned char *base;
	size_t total;
	int err;
};

static GetwdResult run_getwd(bool use_port, bool want_fail)
{
	const size_t pre = 32;
	const size_t post = 32;
	const size_t total = pre + (size_t)MAXPATHLEN + post;

	GetwdResult gr;
	gr.base = (unsigned char *)std::malloc(total);
	gr.total = total;
	gr.err = 0;
	if (gr.base == nullptr) {
		gr.ret = nullptr;
		gr.err = ENOMEM;
		return gr;
	}
	fill_guard(gr.base, total);
	unsigned char *work = gr.base + pre;

	if (want_fail) {
		char tmpl[] = "/tmp/pbsd_getwdXXXXXX";
		char *dir = mkdtemp(tmpl);
		if (dir == nullptr) {
			gr.ret = nullptr;
			gr.err = errno;
			return gr;
		}
		if (chdir(dir) != 0) {
			gr.ret = nullptr;
			gr.err = errno;
			return gr;
		}
		char parent[sizeof(tmpl)];
		std::strcpy(parent, dir);
		char *leaf = std::strrchr(parent, '/');
		if (leaf != nullptr)
			*leaf = '\0';
		if (chdir(parent) != 0) {
			gr.ret = nullptr;
			gr.err = errno;
			return gr;
		}
		if (rmdir(dir) != 0) {
			gr.ret = nullptr;
			gr.err = errno;
			return gr;
		}
	}

	errno = 0;
	gr.ret = use_port ? port::getwd((char *)work) : ref_getwd((char *)work);
	gr.err = errno;
	return gr;
}

static void free_getwd(GetwdResult &gr)
{
	std::free(gr.base);
	gr.base = nullptr;
}

static void test_getwd_case(Stat &st, bool want_fail)
{
	GetwdResult p = run_getwd(true, want_fail);
	GetwdResult r = run_getwd(false, want_fail);
	bump_case(st);

	bool ok = true;
	if ((p.ret == nullptr) != (r.ret == nullptr))
		ok = false;
	if (p.ret != nullptr && r.ret != nullptr) {
		std::ptrdiff_t po = p.ret - (char *)(p.base + 32);
		std::ptrdiff_t ro = r.ret - (char *)(r.base + 32);
		if (po != ro)
			ok = false;
	}
	if (p.base != nullptr && r.base != nullptr &&
	    !buf_same(p.base, r.base, p.total))
		ok = false;
	if (p.err != r.err)
		ok = false;

	if (!ok) {
		bump_fail(st);
		report_mismatch("getwd", want_fail ? "deleted-cwd" : "success");
	}

	free_getwd(p);
	free_getwd(r);
}

static void test_getwd_all(Stat &st)
{
	test_getwd_case(st, false);
	test_getwd_case(st, true);

	for (int i = 0; i < RANDOM_ITERS; i++)
		test_getwd_case(st, (rng_next() & 3U) == 0U);
}

/* ------------------------------------------------------------------ */
/* sigvec                                                            */
/* ------------------------------------------------------------------ */

struct SigvecOut {
	int ret;
	struct sigvec osv;
	bool osv_valid;
};

static SigvecOut call_sigvec(bool use_port, int signo, struct sigvec *sv,
    struct sigvec *osv_in)
{
	struct sigvec osv_buf;
	struct sigvec *osv = osv_in;
	if (osv != nullptr)
		osv_buf = *osv_in;

	SigvecOut out{};
	errno = 0;
	out.ret = use_port ? port::sigvec(signo, sv, osv) :
	    ref_sigvec(signo, sv, osv);
	if (osv != nullptr) {
		out.osv = *osv;
		out.osv_valid = true;
	}
	(void)out.osv_valid;
	return out;
}

static void test_sigvec_one(Stat &st, int signo, struct sigvec *sv,
    bool with_osv, int mock_ret)
{
	pbsd_mock_reset();
	if (mock_ret != 0)
		pbsd_mock_set_sigaction_ret(mock_ret);
	pbsd_mock_set_handler(signo > 0 ? signo : SIGUSR1, h2);

	struct sigvec osv_pre{};
	if (with_osv) {
		osv_pre.sv_handler = h1;
		osv_pre.sv_mask = 0x55;
		osv_pre.sv_flags = SV_INTERRUPT;
	}

	SigvecOut p = call_sigvec(true, signo, sv, with_osv ? &osv_pre : nullptr);
	SigvecOut r = call_sigvec(false, signo, sv, with_osv ? &osv_pre : nullptr);
	bump_case(st);

	bool ok = (p.ret == r.ret);
	if (with_osv && p.ret == 0 && r.ret == 0) {
		if (p.osv.sv_handler != r.osv.sv_handler ||
		    p.osv.sv_mask != r.osv.sv_mask ||
		    p.osv.sv_flags != r.osv.sv_flags)
			ok = false;
	}
	if (!ok) {
		bump_fail(st);
		report_mismatch("sigvec", "fields");
	}
}

static void test_sigvec_all(Stat &st)
{
	struct sigvec sv{};

	sv.sv_handler = h1;
	sv.sv_mask = 0;
	sv.sv_flags = 0;
	test_sigvec_one(st, SIGUSR1, &sv, true, 0);

	sv.sv_flags = SV_INTERRUPT;
	sv.sv_mask = 0x80;
	test_sigvec_one(st, SIGUSR2, &sv, true, 0);

	sv.sv_mask = (int)0xFF00FF00U;
	sv.sv_flags = 0;
	test_sigvec_one(st, SIGTERM, &sv, true, 0);

	test_sigvec_one(st, SIGUSR1, nullptr, true, 0);
	test_sigvec_one(st, SIGUSR1, &sv, false, 0);
	test_sigvec_one(st, 0, &sv, true, 0);
	test_sigvec_one(st, -1, &sv, true, 0);
	test_sigvec_one(st, _NSIG, &sv, true, 0);
	test_sigvec_one(st, SIGUSR1, &sv, true, -1);
	test_sigvec_one(st, SIGUSR1, &sv, true, 1);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		sv.sv_handler = (rng_next() & 1U) ? h1 : h2;
		sv.sv_mask = (int)rng_next();
		sv.sv_flags = (int)(rng_next() & 0xFFU);
		int signo = rng_int(-2, _NSIG + 2);
		bool osv = (rng_next() & 1U) != 0U;
		bool null_sv = (rng_next() & 7U) == 0U;
		int mret = (rng_next() & 15U) == 0U ? -1 : 0;
		test_sigvec_one(st, signo, null_sv ? nullptr : &sv, osv, mret);
	}
}

/* ------------------------------------------------------------------ */
/* sigsetmask / sigblock / sigpause                                  */
/* ------------------------------------------------------------------ */

static void test_sigsetmask_one(Stat &st, int mask, int mock_ret,
    int initial_mask)
{
	pbsd_mock_reset();
	pbsd_mock_set_mask_bits(initial_mask);
	if (mock_ret != 0)
		pbsd_mock_set_sigprocmask_ret(mock_ret);

	errno = 0;
	int pr = port::sigsetmask(mask);
	int pe = errno;
	errno = 0;
	int rr = ref_sigsetmask(mask);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("sigsetmask", "ret/errno");
	}
}

static void test_sigsetmask_all(Stat &st)
{
	test_sigsetmask_one(st, 0, 0, 0);
	test_sigsetmask_one(st, -1, 0, 0x33);
	test_sigsetmask_one(st, 0x7FFFFFFF, 0, 0x80);
	test_sigsetmask_one(st, (int)0x80000000U, 0, 0xFF);
	test_sigsetmask_one(st, 0x55, -1, 0);
	test_sigsetmask_one(st, 0xAA, 1, 0x0F);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int mask = (int)rng_next();
		int init = (int)rng_next();
		int mret = (rng_next() & 31U) == 0U ? -1 : 0;
		test_sigsetmask_one(st, mask, mret, init);
	}
}

static void test_sigblock_one(Stat &st, int mask, int mock_ret,
    int initial_mask)
{
	pbsd_mock_reset();
	pbsd_mock_set_mask_bits(initial_mask);
	if (mock_ret != 0)
		pbsd_mock_set_sigprocmask_ret(mock_ret);

	errno = 0;
	int pr = port::sigblock(mask);
	int pe = errno;
	errno = 0;
	int rr = ref_sigblock(mask);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("sigblock", "ret/errno");
	}
}

static void test_sigblock_all(Stat &st)
{
	test_sigblock_one(st, 0, 0, 0);
	test_sigblock_one(st, 0xFF, 0, 0x10);
	test_sigblock_one(st, (int)0x80808080U, 0, 0x01);
	test_sigblock_one(st, 1, -1, 0);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int mask = (int)rng_next();
		int init = (int)rng_next();
		int mret = (rng_next() & 31U) == 0U ? -1 : 0;
		test_sigblock_one(st, mask, mret, init);
	}
}

static void test_sigpause_one(Stat &st, int mask, int suspend_ret)
{
	pbsd_mock_reset();
	pbsd_mock_set_sigsuspend_ret(suspend_ret);

	errno = 0;
	int pr = port::sigpause(mask);
	int pe = errno;
	errno = 0;
	int rr = ref_sigpause(mask);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("sigpause", "ret/errno");
	}
}

static void test_sigpause_all(Stat &st)
{
	test_sigpause_one(st, 0, 0);
	test_sigpause_one(st, 0xFF, -1);
	test_sigpause_one(st, (int)0x80000000U, 0);
	test_sigpause_one(st, -1, 1);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int mask = (int)rng_next();
		int sret = (rng_next() & 15U) == 0U ? -1 : 0;
		test_sigpause_one(st, mask, sret);
	}
}

/* ------------------------------------------------------------------ */
/* xsi_sigpause, sighold, sigignore, sigrelse                        */
/* ------------------------------------------------------------------ */

static void test_xsi_sigpause_one(Stat &st, int sig, int procmask_ret,
    int suspend_ret)
{
	pbsd_mock_reset();
	pbsd_mock_set_mask_bits(0);
	if (procmask_ret != 0)
		pbsd_mock_set_sigprocmask_ret(procmask_ret);
	pbsd_mock_set_sigsuspend_ret(suspend_ret);

	errno = 0;
	int pr = port::xsi_sigpause(sig);
	int pe = errno;
	errno = 0;
	int rr = ref_xsi_sigpause(sig);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("xsi_sigpause", "ret/errno");
	}
}

static void test_xsi_sigpause_all(Stat &st)
{
	test_xsi_sigpause_one(st, SIGUSR1, 0, 0);
	test_xsi_sigpause_one(st, SIGUSR2, -1, 0);
	test_xsi_sigpause_one(st, 0, 0, 0);
	test_xsi_sigpause_one(st, -1, 0, 0);
	test_xsi_sigpause_one(st, _NSIG, 0, 0);
	test_xsi_sigpause_one(st, SIGTERM, 0, -1);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int sig = rng_int(-2, _NSIG + 2);
		int pret = (rng_next() & 31U) == 0U ? -1 : 0;
		int sret = (rng_next() & 31U) == 0U ? -1 : 0;
		test_xsi_sigpause_one(st, sig, pret, sret);
	}
}

static void test_sighold_one(Stat &st, int sig, int procmask_ret)
{
	pbsd_mock_reset();
	if (procmask_ret != 0)
		pbsd_mock_set_sigprocmask_ret(procmask_ret);

	errno = 0;
	int pr = port::sighold(sig);
	int pe = errno;
	errno = 0;
	int rr = ref_sighold(sig);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("sighold", "ret/errno");
	}
}

static void test_sighold_all(Stat &st)
{
	test_sighold_one(st, SIGUSR1, 0);
	test_sighold_one(st, 0, 0);
	test_sighold_one(st, -1, 0);
	test_sighold_one(st, SIGUSR2, -1);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int sig = rng_int(-2, _NSIG + 2);
		int pret = (rng_next() & 31U) == 0U ? -1 : 0;
		test_sighold_one(st, sig, pret);
	}
}

static void test_sigignore_one(Stat &st, int sig, int action_ret)
{
	pbsd_mock_reset();
	if (action_ret != 0)
		pbsd_mock_set_sigaction_ret(action_ret);

	errno = 0;
	int pr = port::sigignore(sig);
	int pe = errno;
	errno = 0;
	int rr = ref_sigignore(sig);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("sigignore", "ret/errno");
	}
}

static void test_sigignore_all(Stat &st)
{
	test_sigignore_one(st, SIGUSR1, 0);
	test_sigignore_one(st, 0, 0);
	test_sigignore_one(st, -1, 0);
	test_sigignore_one(st, SIGTERM, -1);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int sig = rng_int(-2, _NSIG + 2);
		int aret = (rng_next() & 31U) == 0U ? -1 : 0;
		test_sigignore_one(st, sig, aret);
	}
}

static void test_sigrelse_one(Stat &st, int sig, int procmask_ret)
{
	pbsd_mock_reset();
	if (procmask_ret != 0)
		pbsd_mock_set_sigprocmask_ret(procmask_ret);

	errno = 0;
	int pr = port::sigrelse(sig);
	int pe = errno;
	errno = 0;
	int rr = ref_sigrelse(sig);
	int re = errno;
	bump_case(st);

	if (pr != rr || pe != re) {
		bump_fail(st);
		report_mismatch("sigrelse", "ret/errno");
	}
}

static void test_sigrelse_all(Stat &st)
{
	test_sigrelse_one(st, SIGUSR1, 0);
	test_sigrelse_one(st, 0, 0);
	test_sigrelse_one(st, -1, 0);
	test_sigrelse_one(st, SIGUSR2, -1);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int sig = rng_int(-2, _NSIG + 2);
		int pret = (rng_next() & 31U) == 0U ? -1 : 0;
		test_sigrelse_one(st, sig, pret);
	}
}

/* ------------------------------------------------------------------ */
/* sigset                                                            */
/* ------------------------------------------------------------------ */

static uintptr_t handler_tag(void (*h)(int))
{
	return (uintptr_t)(void *)h;
}

static void test_sigset_one(Stat &st, int sig, void (*disp)(int),
    int initial_mask, int procmask_ret, int action_ret)
{
	pbsd_mock_reset();
	pbsd_mock_set_mask_bits(initial_mask);
	if (procmask_ret != 0)
		pbsd_mock_set_sigprocmask_ret(procmask_ret);
	if (action_ret != 0)
		pbsd_mock_set_sigaction_ret(action_ret);
	if (sig > 0 && sig < _NSIG)
		pbsd_mock_set_handler(sig, h3);

	errno = 0;
	void (*pr)(int) = port::sigset(sig, disp);
	int pe = errno;
	errno = 0;
	void (*rr)(int) = ref_sigset(sig, disp);
	int re = errno;
	bump_case(st);

	if (handler_tag(pr) != handler_tag(rr) || pe != re) {
		bump_fail(st);
		report_mismatch("sigset", "handler/errno");
	}
}

static int mask_bits_for_sig(int sig)
{
	sigset_t set;

	sigemptyset(&set);
	if (sig > 0 && sig < _NSIG)
		sigaddset(&set, sig);
	return ((int)set.__bits[0]);
}

static void test_sigset_all(Stat &st)
{
	test_sigset_one(st, SIGUSR1, SIG_HOLD, 0, 0, 0);
	test_sigset_one(st, SIGUSR1, SIG_HOLD, 0, -1, 0);
	test_sigset_one(st, SIGUSR1, SIG_HOLD, 0, 0, -1);
	test_sigset_one(st, SIGUSR1, SIG_HOLD, mask_bits_for_sig(SIGUSR1), 0, 0);

	test_sigset_one(st, SIGUSR2, h1, 0, 0, 0);
	test_sigset_one(st, SIGUSR2, SIG_IGN, 0, 0, 0);
	test_sigset_one(st, SIGUSR2, SIG_DFL, 0, 0, 0);
	test_sigset_one(st, SIGUSR2, h2, 0, -1, 0);
	test_sigset_one(st, SIGUSR2, h2, 0, 0, -1);
	test_sigset_one(st, SIGUSR2, h1, mask_bits_for_sig(SIGUSR2), 0, 0);

	test_sigset_one(st, 0, h1, 0, 0, 0);
	test_sigset_one(st, -1, h1, 0, 0, 0);
	test_sigset_one(st, _NSIG, h1, 0, 0, 0);

	for (int i = 0; i < RANDOM_ITERS; i++) {
		int sig = rng_int(-2, _NSIG + 2);
		void (*disp)(int);
		switch (rng_next() % 6U) {
		case 0: disp = SIG_HOLD; break;
		case 1: disp = SIG_IGN; break;
		case 2: disp = SIG_DFL; break;
		case 3: disp = h1; break;
		case 4: disp = h2; break;
		default: disp = h3; break;
		}
		int init = (int)rng_next();
		int pret = (rng_next() & 63U) == 0U ? -1 : 0;
		int aret = (rng_next() & 63U) == 0U ? -1 : 0;
		test_sigset_one(st, sig, disp, init, pret, aret);
	}
}

/* ------------------------------------------------------------------ */

int main(void)
{
	test_getwd_all(stats[0]);
	test_sigvec_all(stats[1]);
	test_sigsetmask_all(stats[2]);
	test_sigblock_all(stats[3]);
	test_sigpause_all(stats[4]);
	test_xsi_sigpause_all(stats[5]);
	test_sighold_all(stats[6]);
	test_sigignore_all(stats[7]);
	test_sigrelse_all(stats[8]);
	test_sigset_all(stats[9]);

	long total_cases = 0;
	long total_fails = 0;
	std::printf("\n%-14s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-14s %10s %10s\n", "--------", "-----", "--------");
	for (auto &st : stats) {
		std::printf("%-14s %10ld %10ld\n", st.name, st.cases, st.fails);
		total_cases += st.cases;
		total_fails += st.fails;
	}
	std::printf("%-14s %10ld %10ld\n", "TOTAL", total_cases, total_fails);
	return total_fails == 0 ? 0 : 1;
}
