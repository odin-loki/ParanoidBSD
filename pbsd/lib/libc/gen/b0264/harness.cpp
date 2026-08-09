/*
 * Differential harness for batch b0264.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <unistd.h>

import pbsd.lib.libc.gen.b0264;

namespace P = pbsd::lib_libc_gen::b0264;

extern "C" {
int ref___getcontextx_size(void);
int ref___fillcontextx2(char *ctx);
int ref___fillcontextx(char *ctx);
ucontext_t *ref___getcontextx(void);
int ref___pause(void);
int ref___ssp_real_gethostname(char *name, size_t namelen);
}

#ifndef CTL_KERN
#define CTL_KERN 1
#endif
#ifndef KERN_HOSTNAME
#define KERN_HOSTNAME 10
#endif

enum {
	F_GETCONTEXTX_SIZE,
	F_FILLCONTEXTX2,
	F_FILLCONTEXTX,
	F_GETCONTEXTX,
	F_PAUSE,
	F_SSP_REAL_GETHOSTNAME,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"__getcontextx_size",
	"__fillcontextx2",
	"__fillcontextx",
	"__getcontextx",
	"__pause",
	"__ssp_real_gethostname",
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static constexpr unsigned char GUARD = 0x7fu;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-24s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

static int
randint(void)
{
	return (int)randu32();
}

static void
fill_hibyte_str(char *buf, std::size_t bufsz, std::uint32_t pat, int len)
{
	std::size_t i;

	if (len < 0)
		len = 0;
	if ((std::size_t)len >= bufsz)
		len = (int)bufsz - 1;
	for (i = 0; i < (std::size_t)len; i++)
		buf[i] = (char)(0x80u | ((pat + (std::uint32_t)i) & 0x7fu));
	buf[len] = '\0';
}

static void
fill_guard(char *buf, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		buf[i] = (char)GUARD;
}

/* ------------------------------------------------------------------ */
/* malloc / free mock                                                 */
/* ------------------------------------------------------------------ */

static constexpr std::size_t MALLOC_POOL_SIZE = 4u * 1024u * 1024u;
static char malloc_pool[MALLOC_POOL_SIZE];
static std::size_t malloc_next;
static int malloc_fail;
static int malloc_err;
static long malloc_calls;
static long free_calls;

static void
malloc_mock_reset(void)
{
	malloc_next = 0;
	malloc_fail = 0;
	malloc_err = ENOMEM;
	malloc_calls = 0;
	free_calls = 0;
}

static void
malloc_mock_configure(int fail, int err)
{
	malloc_fail = fail;
	malloc_err = err;
}

extern "C" void *
malloc(std::size_t sz)
{
	void *p;

	malloc_calls++;
	if (malloc_fail) {
		errno = malloc_err;
		return (NULL);
	}
	if (malloc_next + sz > MALLOC_POOL_SIZE) {
		errno = ENOMEM;
		return (NULL);
	}
	p = malloc_pool + malloc_next;
	malloc_next = (malloc_next + sz + 15u) & ~15u;
	return (p);
}

extern "C" void
free(void *ptr)
{
	free_calls++;
	(void)ptr;
}

static long
ptr_offset(const void *p, const void *base)
{
	if (p == NULL)
		return (-1);
	return (long)((const char *)p - (const char *)base);
}

/* ------------------------------------------------------------------ */
/* getcontext mock                                                    */
/* ------------------------------------------------------------------ */

static int gc_fail;
static int gc_err;
static int gc_ret;
static unsigned char gc_byte;
static long gc_calls;
static ucontext_t *gc_last;

static void
getcontext_mock_reset(void)
{
	gc_fail = 0;
	gc_err = EINVAL;
	gc_ret = 0;
	gc_byte = 0xa5u;
	gc_calls = 0;
	gc_last = NULL;
}

static void
getcontext_mock_configure(int fail, int err, int ret, unsigned char byte)
{
	gc_fail = fail;
	gc_err = err;
	gc_ret = ret;
	gc_byte = byte;
}

extern "C" int
getcontext(ucontext_t *ucp)
{
	gc_calls++;
	gc_last = ucp;
	if (gc_fail) {
		errno = gc_err;
		return (-1);
	}
	if (ucp != NULL)
		std::memset(ucp, gc_byte, sizeof(ucontext_t));
	return (gc_ret);
}

/* ------------------------------------------------------------------ */
/* sigprocmask / sigsuspend mock                                      */
/* ------------------------------------------------------------------ */

static int spm_fail;
static int spm_err;
static unsigned char spm_oset_byte;
static long spm_calls;
static int spm_how;

static int ss_fail;
static int ss_err;
static int ss_ret;
static long ss_calls;
static sigset_t ss_set_copy;
static int ss_have_set;

static void
pause_mock_reset(void)
{
	spm_fail = 0;
	spm_err = EINVAL;
	spm_oset_byte = 0xc3u;
	spm_calls = 0;
	spm_how = 0;
	ss_fail = 0;
	ss_err = EINTR;
	ss_ret = -1;
	ss_calls = 0;
	ss_have_set = 0;
	std::memset(&ss_set_copy, 0, sizeof ss_set_copy);
}

static void
pause_mock_configure(int spm_f, int spm_e, unsigned char oset_b,
    int ss_f, int ss_e, int ss_r)
{
	spm_fail = spm_f;
	spm_err = spm_e;
	spm_oset_byte = oset_b;
	ss_fail = ss_f;
	ss_err = ss_e;
	ss_ret = ss_r;
}

extern "C" int
sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	spm_calls++;
	spm_how = how;
	(void)set;
	if (spm_fail) {
		errno = spm_err;
		return (-1);
	}
	if (oset != NULL)
		std::memset(oset, spm_oset_byte, sizeof(sigset_t));
	return (0);
}

extern "C" int
sigsuspend(const sigset_t *set)
{
	ss_calls++;
	ss_have_set = 0;
	if (set != NULL) {
		std::memcpy(&ss_set_copy, set, sizeof(sigset_t));
		ss_have_set = 1;
	}
	if (ss_fail) {
		errno = ss_err;
		return (-1);
	}
	return (ss_ret);
}

/* ------------------------------------------------------------------ */
/* sysctl mock                                                        */
/* ------------------------------------------------------------------ */

static constexpr std::size_t HOSTNAME_MAX = 256u;

static char hostname_value[HOSTNAME_MAX];
static int sysctl_fail_errno;
static long sysctl_calls;
static int sysctl_mib0;
static int sysctl_mib1;
static unsigned int sysctl_namelen;
static size_t sysctl_oldlen_in;
static size_t sysctl_oldlen_out;

static void
sysctl_mock_reset(void)
{
	std::memset(hostname_value, 0, sizeof hostname_value);
	sysctl_fail_errno = 0;
	sysctl_calls = 0;
	sysctl_mib0 = 0;
	sysctl_mib1 = 0;
	sysctl_namelen = 0;
	sysctl_oldlen_in = 0;
	sysctl_oldlen_out = 0;
}

static void
sysctl_mock_set_hostname(const char *s)
{
	std::size_t n;

	n = std::strlen(s);
	if (n >= HOSTNAME_MAX)
		n = HOSTNAME_MAX - 1;
	std::memcpy(hostname_value, s, n);
	hostname_value[n] = '\0';
}

static void
sysctl_mock_configure_fail(int err)
{
	sysctl_fail_errno = err;
}

extern "C" int
sysctl(const int *name, unsigned int namelen, void *oldp, size_t *oldlenp,
    const void *newp, size_t newlen)
{
	const char *src;
	std::size_t slen, copy, avail;

	sysctl_calls++;
	sysctl_namelen = namelen;
	if (oldlenp != NULL)
		sysctl_oldlen_in = *oldlenp;
	else
		sysctl_oldlen_in = 0;
	(void)newp;
	(void)newlen;

	if (name != NULL && namelen >= 1u) {
		sysctl_mib0 = name[0];
		if (namelen >= 2u)
			sysctl_mib1 = name[1];
	}

	if (sysctl_fail_errno != 0) {
		errno = sysctl_fail_errno;
		return (-1);
	}

	if (name == NULL || namelen < 2u)
		return (-1);
	if (name[0] != CTL_KERN || name[1] != KERN_HOSTNAME)
		return (-1);

	src = hostname_value;
	slen = std::strlen(src);
	if (oldp == NULL || oldlenp == NULL) {
		sysctl_oldlen_out = slen;
		return (0);
	}

	avail = *oldlenp;
	copy = slen;
	if (copy >= avail)
		copy = avail > 0 ? avail - 1 : 0;
	if (copy > 0)
		std::memcpy(oldp, src, copy);
	if (avail > 0)
		((char *)oldp)[copy] = '\0';
	*oldlenp = copy;
	sysctl_oldlen_out = copy;
	return (0);
}

/* ------------------------------------------------------------------ */
/* __getcontextx_size                                                 */
/* ------------------------------------------------------------------ */

static void
check_getcontextx_size(int f, const char *ctx)
{
	int ref_ret;
	int port_ret;

	ncases[f]++;
	ref_ret = ref___getcontextx_size();
	port_ret = P::__getcontextx_size();
	if (ref_ret != port_ret) {
		char detail[128];
		std::snprintf(detail, sizeof detail, "ref=%d port=%d", ref_ret,
		    port_ret);
		report(f, ctx, detail);
	}
}

/* ------------------------------------------------------------------ */
/* buffer helpers                                                     */
/* ------------------------------------------------------------------ */

struct BufObs {
	int		ret;
	int		err;
	long		gc_calls;
	ucontext_t     *gc_last;
	unsigned char	gc_last_byte;
	char		buf[sizeof(ucontext_t) + 64];
};

static BufObs
run_ref_fillcontextx2(char *ctx)
{
	BufObs obs{};

	getcontext_mock_reset();
	errno = 0;
	obs.ret = ref___fillcontextx2(ctx);
	obs.err = errno;
	obs.gc_calls = gc_calls;
	obs.gc_last = gc_last;
	if (ctx != NULL)
		obs.gc_last_byte = (unsigned char)ctx[0];
	std::memcpy(obs.buf, ctx - 32,
	    sizeof(ucontext_t) + 64);
	return (obs);
}

static BufObs
run_port_fillcontextx2(char *ctx)
{
	BufObs obs{};

	getcontext_mock_reset();
	errno = 0;
	obs.ret = P::__fillcontextx2(ctx);
	obs.err = errno;
	obs.gc_calls = gc_calls;
	obs.gc_last = gc_last;
	if (ctx != NULL)
		obs.gc_last_byte = (unsigned char)ctx[0];
	std::memcpy(obs.buf, ctx - 32,
	    sizeof(ucontext_t) + 64);
	return (obs);
}

static int
buf_obs_equal(const BufObs &a, const BufObs &b)
{
	if (a.ret != b.ret || a.err != b.err || a.gc_calls != b.gc_calls)
		return (0);
	if (a.gc_last_byte != b.gc_last_byte)
		return (0);
	if (std::memcmp(a.buf, b.buf, sizeof a.buf) != 0)
		return (0);
	return (1);
}

static void
check_fillcontextx2(int f, const char *ctx, unsigned char pre_byte)
{
	char storage[sizeof(ucontext_t) + 64];
	char *ctxp;
	BufObs ref_obs;
	BufObs port_obs;
	char detail[128];

	ncases[f]++;
	fill_guard(storage, sizeof storage);
	ctxp = storage + 32;
	if (pre_byte != 0)
		std::memset(ctxp, pre_byte, sizeof(ucontext_t));

	ref_obs = run_ref_fillcontextx2(ctxp);
	port_obs = run_port_fillcontextx2(ctxp);

	if (!buf_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "ref={ret=%d err=%d gc=%ld} port={ret=%d err=%d gc=%ld}",
		    ref_obs.ret, ref_obs.err, ref_obs.gc_calls, port_obs.ret,
		    port_obs.err, port_obs.gc_calls);
		report(f, ctx, detail);
	}
}

static BufObs
run_ref_fillcontextx(char *ctx, int gc_f, int gc_e, int gc_r,
    unsigned char gc_b)
{
	BufObs obs{};

	getcontext_mock_reset();
	getcontext_mock_configure(gc_f, gc_e, gc_r, gc_b);
	errno = 0;
	obs.ret = ref___fillcontextx(ctx);
	obs.err = errno;
	obs.gc_calls = gc_calls;
	obs.gc_last = gc_last;
	if (gc_last != NULL)
		obs.gc_last_byte = (unsigned char)gc_last[0];
	std::memcpy(obs.buf, ctx - 32, sizeof(ucontext_t) + 64);
	return (obs);
}

static BufObs
run_port_fillcontextx(char *ctx, int gc_f, int gc_e, int gc_r,
    unsigned char gc_b)
{
	BufObs obs{};

	getcontext_mock_reset();
	getcontext_mock_configure(gc_f, gc_e, gc_r, gc_b);
	errno = 0;
	obs.ret = P::__fillcontextx(ctx);
	obs.err = errno;
	obs.gc_calls = gc_calls;
	obs.gc_last = gc_last;
	if (gc_last != NULL)
		obs.gc_last_byte = (unsigned char)gc_last[0];
	std::memcpy(obs.buf, ctx - 32, sizeof(ucontext_t) + 64);
	return (obs);
}

static void
check_fillcontextx(int f, const char *ctx, unsigned char pre_byte, int gc_f,
    int gc_e, int gc_r, unsigned char gc_b)
{
	char storage[sizeof(ucontext_t) + 64];
	char *ctxp;
	BufObs ref_obs;
	BufObs port_obs;
	char detail[192];

	ncases[f]++;
	fill_guard(storage, sizeof storage);
	ctxp = storage + 32;
	if (pre_byte != 0)
		std::memset(ctxp, pre_byte, sizeof(ucontext_t));

	ref_obs = run_ref_fillcontextx(ctxp, gc_f, gc_e, gc_r, gc_b);
	port_obs = run_port_fillcontextx(ctxp, gc_f, gc_e, gc_r, gc_b);

	if (!buf_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "ref={ret=%d err=%d gc=%ld b=0x%02x} "
		    "port={ret=%d err=%d gc=%ld b=0x%02x}",
		    ref_obs.ret, ref_obs.err, ref_obs.gc_calls,
		    ref_obs.gc_last_byte, port_obs.ret, port_obs.err,
		    port_obs.gc_calls, port_obs.gc_last_byte);
		report(f, ctx, detail);
	}
}

/* ------------------------------------------------------------------ */
/* __getcontextx                                                      */
/* ------------------------------------------------------------------ */

struct GetCtxObs {
	ucontext_t     *ptr;
	long		ptr_off;
	int		err;
	long		malloc_calls;
	long		free_calls;
	long		gc_calls;
	unsigned char	first_byte;
};

static GetCtxObs
run_ref_getcontextx(int mal_f, int mal_e, int gc_f, int gc_e, int gc_r,
    unsigned char gc_b)
{
	GetCtxObs obs{};

	malloc_mock_reset();
	malloc_mock_configure(mal_f, mal_e);
	getcontext_mock_reset();
	getcontext_mock_configure(gc_f, gc_e, gc_r, gc_b);
	errno = 0;
	obs.ptr = ref___getcontextx();
	obs.err = errno;
	obs.malloc_calls = malloc_calls;
	obs.free_calls = free_calls;
	obs.gc_calls = gc_calls;
	obs.ptr_off = ptr_offset(obs.ptr, malloc_pool);
	if (obs.ptr != NULL)
		obs.first_byte = (unsigned char)obs.ptr[0];
	return (obs);
}

static GetCtxObs
run_port_getcontextx(int mal_f, int mal_e, int gc_f, int gc_e, int gc_r,
    unsigned char gc_b)
{
	GetCtxObs obs{};

	malloc_mock_reset();
	malloc_mock_configure(mal_f, mal_e);
	getcontext_mock_reset();
	getcontext_mock_configure(gc_f, gc_e, gc_r, gc_b);
	errno = 0;
	obs.ptr = P::__getcontextx();
	obs.err = errno;
	obs.malloc_calls = malloc_calls;
	obs.free_calls = free_calls;
	obs.gc_calls = gc_calls;
	obs.ptr_off = ptr_offset(obs.ptr, malloc_pool);
	if (obs.ptr != NULL)
		obs.first_byte = (unsigned char)obs.ptr[0];
	return (obs);
}

static int
getctx_obs_equal(const GetCtxObs &a, const GetCtxObs &b)
{
	return (a.ptr_off == b.ptr_off && a.err == b.err &&
	    a.malloc_calls == b.malloc_calls && a.free_calls == b.free_calls &&
	    a.gc_calls == b.gc_calls && a.first_byte == b.first_byte);
}

static void
check_getcontextx(int f, const char *ctx, int mal_f, int mal_e, int gc_f,
    int gc_e, int gc_r, unsigned char gc_b)
{
	GetCtxObs ref_obs;
	GetCtxObs port_obs;
	char detail[256];

	ncases[f]++;
	ref_obs = run_ref_getcontextx(mal_f, mal_e, gc_f, gc_e, gc_r, gc_b);
	port_obs = run_port_getcontextx(mal_f, mal_e, gc_f, gc_e, gc_r, gc_b);

	if (!getctx_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "ref={off=%ld err=%d mal=%ld free=%ld gc=%ld b=0x%02x} "
		    "port={off=%ld err=%d mal=%ld free=%ld gc=%ld b=0x%02x}",
		    ref_obs.ptr_off, ref_obs.err, ref_obs.malloc_calls,
		    ref_obs.free_calls, ref_obs.gc_calls, ref_obs.first_byte,
		    port_obs.ptr_off, port_obs.err, port_obs.malloc_calls,
		    port_obs.free_calls, port_obs.gc_calls, port_obs.first_byte);
		report(f, ctx, detail);
	}
}

/* ------------------------------------------------------------------ */
/* __pause                                                            */
/* ------------------------------------------------------------------ */

struct PauseObs {
	int		ret;
	int		err;
	long		spm_calls;
	int		spm_how;
	long		ss_calls;
	sigset_t	ss_set;
	int		ss_have_set;
};

static PauseObs
run_ref_pause(int spm_f, int spm_e, unsigned char oset_b, int ss_f, int ss_e,
    int ss_r)
{
	PauseObs obs{};

	pause_mock_reset();
	pause_mock_configure(spm_f, spm_e, oset_b, ss_f, ss_e, ss_r);
	errno = 0;
	obs.ret = ref___pause();
	obs.err = errno;
	obs.spm_calls = spm_calls;
	obs.spm_how = spm_how;
	obs.ss_calls = ss_calls;
	obs.ss_have_set = ss_have_set;
	if (ss_have_set)
		obs.ss_set = ss_set_copy;
	return (obs);
}

static PauseObs
run_port_pause(int spm_f, int spm_e, unsigned char oset_b, int ss_f, int ss_e,
    int ss_r)
{
	PauseObs obs{};

	pause_mock_reset();
	pause_mock_configure(spm_f, spm_e, oset_b, ss_f, ss_e, ss_r);
	errno = 0;
	obs.ret = P::__pause();
	obs.err = errno;
	obs.spm_calls = spm_calls;
	obs.spm_how = spm_how;
	obs.ss_calls = ss_calls;
	obs.ss_have_set = ss_have_set;
	if (ss_have_set)
		obs.ss_set = ss_set_copy;
	return (obs);
}

static int
pause_obs_equal(const PauseObs &a, const PauseObs &b)
{
	if (a.ret != b.ret || a.err != b.err || a.spm_calls != b.spm_calls ||
	    a.spm_how != b.spm_how || a.ss_calls != b.ss_calls ||
	    a.ss_have_set != b.ss_have_set)
		return (0);
	if (a.ss_have_set &&
	    std::memcmp(&a.ss_set, &b.ss_set, sizeof(sigset_t)) != 0)
		return (0);
	return (1);
}

static void
check_pause(int f, const char *ctx, int spm_f, int spm_e, unsigned char oset_b,
    int ss_f, int ss_e, int ss_r)
{
	PauseObs ref_obs;
	PauseObs port_obs;
	char detail[256];

	ncases[f]++;
	ref_obs = run_ref_pause(spm_f, spm_e, oset_b, ss_f, ss_e, ss_r);
	port_obs = run_port_pause(spm_f, spm_e, oset_b, ss_f, ss_e, ss_r);

	if (!pause_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "ref={ret=%d err=%d spm=%ld ss=%ld} "
		    "port={ret=%d err=%d spm=%ld ss=%ld}",
		    ref_obs.ret, ref_obs.err, ref_obs.spm_calls,
		    ref_obs.ss_calls, port_obs.ret, port_obs.err,
		    port_obs.spm_calls, port_obs.ss_calls);
		report(f, ctx, detail);
	}
}

/* ------------------------------------------------------------------ */
/* __ssp_real_gethostname                                             */
/* ------------------------------------------------------------------ */

struct HostObs {
	int		ret;
	int		err;
	size_t		namelen_out;
	long		sysctl_calls;
	int		mib0;
	int		mib1;
	size_t		oldlen_in;
	size_t		oldlen_out;
	char		buf[512];
};

static HostObs
run_ref_gethostname(char *name, size_t namelen_in, int sysctl_err,
    const char *host)
{
	HostObs obs{};

	sysctl_mock_reset();
	sysctl_mock_set_hostname(host);
	if (sysctl_err != 0)
		sysctl_mock_configure_fail(sysctl_err);
	std::memcpy(obs.buf, name - 64, 512);
	errno = 0;
	obs.ret = ref___ssp_real_gethostname(name, namelen_in);
	obs.err = errno;
	obs.namelen_out = namelen_in;
	obs.sysctl_calls = sysctl_calls;
	obs.mib0 = sysctl_mib0;
	obs.mib1 = sysctl_mib1;
	obs.oldlen_in = sysctl_oldlen_in;
	obs.oldlen_out = sysctl_oldlen_out;
	std::memcpy(obs.buf, name - 64, 512);
	return (obs);
}

static HostObs
run_port_gethostname(char *name, size_t namelen_in, int sysctl_err,
    const char *host)
{
	HostObs obs{};

	sysctl_mock_reset();
	sysctl_mock_set_hostname(host);
	if (sysctl_err != 0)
		sysctl_mock_configure_fail(sysctl_err);
	std::memcpy(obs.buf, name - 64, 512);
	errno = 0;
	obs.ret = P::__ssp_real_gethostname(name, namelen_in);
	obs.err = errno;
	obs.namelen_out = namelen_in;
	obs.sysctl_calls = sysctl_calls;
	obs.mib0 = sysctl_mib0;
	obs.mib1 = sysctl_mib1;
	obs.oldlen_in = sysctl_oldlen_in;
	obs.oldlen_out = sysctl_oldlen_out;
	std::memcpy(obs.buf, name - 64, 512);
	return (obs);
}

static int
host_obs_equal(const HostObs &a, const HostObs &b)
{
	if (a.ret != b.ret || a.err != b.err || a.namelen_out != b.namelen_out ||
	    a.sysctl_calls != b.sysctl_calls || a.mib0 != b.mib0 ||
	    a.mib1 != b.mib1 || a.oldlen_in != b.oldlen_in ||
	    a.oldlen_out != b.oldlen_out)
		return (0);
	if (std::memcmp(a.buf, b.buf, sizeof a.buf) != 0)
		return (0);
	return (1);
}

static void
check_gethostname(int f, const char *ctx, size_t namelen_in, int sysctl_err,
    const char *host, unsigned char pre_byte)
{
	char storage[512];
	char *name;
	HostObs ref_obs;
	HostObs port_obs;
	char detail[256];

	ncases[f]++;
	fill_guard(storage, sizeof storage);
	name = storage + 64;
	if (pre_byte != 0)
		std::memset(name, pre_byte, 256);

	ref_obs = run_ref_gethostname(name, namelen_in, sysctl_err, host);
	port_obs = run_port_gethostname(name, namelen_in, sysctl_err, host);

	if (!host_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "ref={ret=%d err=%d len=%zu sc=%ld} "
		    "port={ret=%d err=%d len=%zu sc=%ld}",
		    ref_obs.ret, ref_obs.err, ref_obs.namelen_out,
		    ref_obs.sysctl_calls, port_obs.ret, port_obs.err,
		    port_obs.namelen_out, port_obs.sysctl_calls);
		report(f, ctx, detail);
	}
}

/* ------------------------------------------------------------------ */
/* hand-written tests                                                 */
/* ------------------------------------------------------------------ */

static void
test_getcontextx_size_hand(void)
{
	const int f = F_GETCONTEXTX_SIZE;

	check_getcontextx_size(f, "basic");
}

static void
test_fillcontextx2_hand(void)
{
	const int f = F_FILLCONTEXTX2;

	check_fillcontextx2(f, "guard_only", 0);
	check_fillcontextx2(f, "pre_0x80", 0x80u);
	check_fillcontextx2(f, "pre_0xff", 0xffu);
	check_fillcontextx2(f, "pre_0x7f", GUARD);
}

static void
test_fillcontextx_hand(void)
{
	const int f = F_FILLCONTEXTX;

	check_fillcontextx(f, "gc_ok", 0x80u, 0, 0, 0, 0x5bu);
	check_fillcontextx(f, "gc_fail", GUARD, 1, EAGAIN, -1, 0xa5u);
	check_fillcontextx(f, "gc_ok_zero", 0, 0, 0, 0, 0xc3u);
	check_fillcontextx(f, "gc_fail_enomem", 0xffu, 1, ENOMEM, -1, 0x91u);
}

static void
test_getcontextx_hand(void)
{
	const int f = F_GETCONTEXTX;

	check_getcontextx(f, "ok", 0, 0, 0, 0, 0, 0x42u);
	check_getcontextx(f, "malloc_fail", 1, ENOMEM, 0, 0, 0, 0);
	check_getcontextx(f, "gc_fail", 0, 0, 1, EINVAL, -1, 0xabu);
	check_getcontextx(f, "gc_fail_eagain", 0, 0, 1, EAGAIN, -1, 0xcdu);
}

static void
test_pause_hand(void)
{
	const int f = F_PAUSE;

	check_pause(f, "spm_fail", 1, EINVAL, 0xc3u, 0, 0, 0);
	check_pause(f, "spm_ok_ss_fail", 0, 0, 0x80u, 1, EINTR, -1);
	check_pause(f, "spm_ok_ss_ok", 0, 0, 0xffu, 0, 0, 0);
	check_pause(f, "oset_hibyte", 0, 0, 0xabu, 1, EFAULT, -1);
}

static void
test_gethostname_hand(void)
{
	const int f = F_SSP_REAL_GETHOSTNAME;
	char hibyte[64];

	check_gethostname(f, "empty_host", 64, 0, "", 0);
	check_gethostname(f, "short", 32, 0, "host", GUARD);
	check_gethostname(f, "exact_fit", 5, 0, "abcd", 0x80u);
	check_gethostname(f, "trunc", 4, 0, "longname", 0xffu);
	check_gethostname(f, "size_zero", 0, 0, "x", GUARD);
	check_gethostname(f, "size_one", 1, 0, "y", 0);
	check_gethostname(f, "sysctl_enomem", 16, ENOMEM, "host", 0x7fu);
	check_gethostname(f, "sysctl_einval", 16, EINVAL, "host", 0);
	check_gethostname(f, "sysctl_enomem_empty", 8, ENOMEM, "", GUARD);

	fill_hibyte_str(hibyte, sizeof hibyte, 0xd7u, 40);
	check_gethostname(f, "hibyte_host", 128, 0, hibyte, 0x80u);
	check_gethostname(f, "nul_heavy", 64, 0, "a\0b\0c", 0xffu);
}

/* ------------------------------------------------------------------ */
/* random tests                                                       */
/* ------------------------------------------------------------------ */

static void
test_getcontextx_size_random(void)
{
	const int f = F_GETCONTEXTX_SIZE;
	char ctx[64];

	for (int i = 0; i < 200000; i++) {
		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_getcontextx_size(f, ctx);
	}
}

static void
test_fillcontextx2_random(void)
{
	const int f = F_FILLCONTEXTX2;
	char ctx[64];

	for (int i = 0; i < 200000; i++) {
		unsigned char pre = (unsigned char)(randu32() & 0xffu);

		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_fillcontextx2(f, ctx, pre);
	}
}

static void
test_fillcontextx_random(void)
{
	const int f = F_FILLCONTEXTX;
	char ctx[64];

	for (int i = 0; i < 200000; i++) {
		unsigned char pre = (unsigned char)(randu32() & 0xffu);
		int gc_f = (int)(randu32() & 1u);
		int gc_e = (int)(randu32() % 64u);
		int gc_r = (int)(randu32() & 1u) ? -1 : 0;
		unsigned char gc_b = (unsigned char)(randu32() & 0xffu);

		if (gc_e == 0)
			gc_e = EINVAL;
		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_fillcontextx(f, ctx, pre, gc_f, gc_e, gc_r, gc_b);
	}
}

static void
test_getcontextx_random(void)
{
	const int f = F_GETCONTEXTX;
	char ctx[64];

	for (int i = 0; i < 200000; i++) {
		int mal_f = (int)(randu32() & 1u);
		int mal_e = (int)(randu32() % 64u);
		int gc_f = (int)(randu32() & 1u);
		int gc_e = (int)(randu32() % 64u);
		int gc_r = (int)(randu32() & 1u) ? -1 : 0;
		unsigned char gc_b = (unsigned char)(randu32() & 0xffu);

		if (mal_e == 0)
			mal_e = ENOMEM;
		if (gc_e == 0)
			gc_e = EINVAL;
		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_getcontextx(f, ctx, mal_f, mal_e, gc_f, gc_e, gc_r, gc_b);
	}
}

static void
test_pause_random(void)
{
	const int f = F_PAUSE;
	char ctx[64];

	for (int i = 0; i < 200000; i++) {
		int spm_f = (int)(randu32() & 1u);
		int spm_e = (int)(randu32() % 64u);
		unsigned char oset_b = (unsigned char)(randu32() & 0xffu);
		int ss_f = (int)(randu32() & 1u);
		int ss_e = (int)(randu32() % 64u);
		int ss_r = (int)(randu32() & 1u) ? -1 : 0;

		if (spm_e == 0)
			spm_e = EINVAL;
		if (ss_e == 0)
			ss_e = EINTR;
		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_pause(f, ctx, spm_f, spm_e, oset_b, ss_f, ss_e, ss_r);
	}
}

static void
test_gethostname_random(void)
{
	const int f = F_SSP_REAL_GETHOSTNAME;
	char ctx[64];
	char host[96];
	static const char *const hosts[] = {
		"", "a", "localhost", "host\0hidden", "x"
	};

	for (int i = 0; i < 200000; i++) {
		size_t namelen = (size_t)(randu32() % 256u);
		int sysctl_err = 0;
		unsigned char pre = (unsigned char)(randu32() & 0xffu);
		int pick = (int)(randu32() % 6u);

		if ((randu32() & 7u) == 0u) {
			sysctl_err = (int)(randu32() % 64u);
			if (sysctl_err == 0)
				sysctl_err = ENOMEM;
		}

		if ((randu32() & 1u) != 0u)
			fill_hibyte_str(host, sizeof host, randu32(),
			    (int)(randu32() % 80u) + 1);
		else
			std::strncpy(host, hosts[pick % 5], sizeof host - 1);
		host[sizeof host - 1] = '\0';

		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_gethostname(f, ctx, namelen, sysctl_err, host, pre);
	}
}

int
main(void)
{
	test_getcontextx_size_hand();
	test_getcontextx_size_random();
	test_fillcontextx2_hand();
	test_fillcontextx2_random();
	test_fillcontextx_hand();
	test_fillcontextx_random();
	test_getcontextx_hand();
	test_getcontextx_random();
	test_pause_hand();
	test_pause_random();
	test_gethostname_hand();
	test_gethostname_random();

	std::printf("\n%-28s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-28s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	for (int i = 0; i < NFUNC; i++) {
		if (nfails[i] != 0)
			return (1);
	}
	return (0);
}
