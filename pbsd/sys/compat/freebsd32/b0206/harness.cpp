/*
 * PBSD batch b0206 -- differential test: port.cppm vs oracle.c.
 *
 * Every ported function is called twice per case, once in the module and once
 * in the C reference, against a mocked kernel environment that records
 * everything the function does: the return value, td_retval[], every
 * fueword32()/suword32()/copyin()/malloc()/free()/AUDIT_ARG_FD()/filedesc lock
 * call (with its argument, its result and, for the user-space accessors, the
 * *offset* from the buffer base rather than the address), and the arguments the
 * kern_* helpers were handed, including the contents of the arrays passed to
 * them.  User buffers are duplicated: two allocations per case, both painted
 * with the guard byte 0x7f, the same input copied into each, and the whole
 * buffer -- including the bytes past the window the function is supposed to
 * write -- compared afterwards.
 *
 * Exit status is 0 only if every case matched.
 */

#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sys.compat.freebsd32.b0206;

namespace P = pbsd::sys_compat_freebsd32::b0206;

extern "C" {
int	ref_freebsd32_abort2(P::thread *td, P::freebsd32_abort2_args *uap);
int	ref_freebsd32_cap_ioctls_limit(P::thread *td,
	    P::freebsd32_cap_ioctls_limit_args *uap);
int	ref_freebsd32_cap_ioctls_get(P::thread *td,
	    P::freebsd32_cap_ioctls_get_args *uap);
extern const char *ref_freebsd32_syscallnames[];
extern const P::size_t ref_freebsd32_syscallnames_count;
}

/* ------------------------------------------------------------------ */
/* recorded effects						      */
/* ------------------------------------------------------------------ */

enum {
	FU_CAP = 48,		/* fueword32 calls logged		*/
	SU_CAP = 160,		/* suword32 calls logged		*/
	CMD_CAP = 300,		/* kern_cap_ioctls_limit cmds logged	*/
	MISC_CAP = 12,
	UARG_CAP = 20
};

struct KLog {
	int		rv;
	long		retval0, retval1;

	int		n_fu;
	long		fu_idx[FU_CAP];
	int		fu_ret[FU_CAP];
	unsigned int	fu_val[FU_CAP];

	int		n_su;
	long		su_idx[SU_CAP];
	int		su_ret[SU_CAP];
	unsigned int	su_val[SU_CAP];

	int		n_alloc;
	unsigned long long alloc_size[MISC_CAP];
	int		alloc_type_ok[MISC_CAP];
	int		alloc_flags[MISC_CAP];

	int		n_free;
	int		free_type_ok[MISC_CAP];

	int		n_copyin;
	unsigned long long copyin_len[MISC_CAP];
	long		copyin_off[MISC_CAP];
	int		copyin_ret[MISC_CAP];

	int		n_audit;
	int		audit_fd[MISC_CAP];

	int		slock, sunlock;

	int		n_fget;
	int		fget_fd[MISC_CAP];
	int		fget_null[MISC_CAP];

	int		a2_calls, a2_td_ok, a2_why_ok, a2_nargs, a2_uargsp_null;
	int		a2_nlogged;
	unsigned long long a2_uargs[UARG_CAP];

	int		cl_calls, cl_td_ok, cl_fd, cl_cmds_null, cl_nlogged;
	unsigned long long cl_ncmds;
	unsigned long long cl_cmds[CMD_CAP];
};

static KLog	g_logP, g_logR;
static KLog	*KL = &g_logP;

/* ------------------------------------------------------------------ */
/* mock configuration						      */
/* ------------------------------------------------------------------ */

static const P::uint32_t	*fu_base;	/* fueword32 window	*/
static long			 fu_count;
static unsigned long long	 fu_mask;	/* per-index fault	*/

static P::uint32_t		*su_base;	/* suword32 window	*/
static long			 su_count;
static unsigned long long	 su_mask;

static int			 copyin_fail;
static P::size_t		 copyin_max;
static const void		*copyin_base;

static const P::thread		*expect_td;
static const char		*expect_why;

static int			 m_filecaps_storage;
extern "C" {
void				*M_FILECAPS = &m_filecaps_storage;
}

static void
bail(const char *what)
{

	std::fprintf(stderr, "harness: %s\n", what);
	std::exit(1);
}

/* ------------------------------------------------------------------ */
/* mocked kernel primitives					      */
/* ------------------------------------------------------------------ */

extern "C" int
fueword32(const void *base, P::uint32_t *val)
{
	long idx;
	unsigned int got;
	int ret;

	idx = (const P::uint32_t *)base - fu_base;
	got = 0;
	if (idx < 0 || idx >= fu_count ||
	    (idx < 64 && ((fu_mask >> idx) & 1) != 0))
		ret = -1;
	else {
		got = fu_base[idx];
		*val = got;
		ret = 0;
	}
	if (KL->n_fu < FU_CAP) {
		KL->fu_idx[KL->n_fu] = idx;
		KL->fu_ret[KL->n_fu] = ret;
		KL->fu_val[KL->n_fu] = got;
	}
	KL->n_fu++;
	return (ret);
}

extern "C" int
suword32(void *base, int word)
{
	long idx;
	int ret;

	idx = (P::uint32_t *)base - su_base;
	if (idx < 0 || idx >= su_count ||
	    (idx < 64 && ((su_mask >> idx) & 1) != 0))
		ret = -1;
	else {
		su_base[idx] = (P::uint32_t)word;
		ret = 0;
	}
	if (KL->n_su < SU_CAP) {
		KL->su_idx[KL->n_su] = idx;
		KL->su_ret[KL->n_su] = ret;
		KL->su_val[KL->n_su] = (unsigned int)word;
	}
	KL->n_su++;
	return (ret);
}

extern "C" int
copyin(const void *uaddr, void *kaddr, P::size_t len)
{
	int ret;

	ret = 0;
	if (copyin_fail != 0 || len > copyin_max)
		ret = 14;			/* EFAULT */
	else if (len != 0)
		std::memcpy(kaddr, uaddr, len);
	if (KL->n_copyin < MISC_CAP) {
		KL->copyin_len[KL->n_copyin] = len;
		KL->copyin_off[KL->n_copyin] =
		    (const char *)uaddr - (const char *)copyin_base;
		KL->copyin_ret[KL->n_copyin] = ret;
	}
	KL->n_copyin++;
	return (ret);
}

extern "C" void *
kmock_malloc(P::size_t size, void *type, int flags)
{
	P::size_t n;
	void *p;

	n = size != 0 ? size : 1;
	if (n > (64u << 20))
		bail("absurd allocation request from the code under test");
	p = std::malloc(n);
	if (p == nullptr)
		bail("out of memory");
	std::memset(p, 0xaa, n);
	if (KL->n_alloc < MISC_CAP) {
		KL->alloc_size[KL->n_alloc] = size;
		KL->alloc_type_ok[KL->n_alloc] = (type == M_FILECAPS);
		KL->alloc_flags[KL->n_alloc] = flags;
	}
	KL->n_alloc++;
	return (p);
}

extern "C" void
kmock_free(void *addr, void *type)
{

	if (KL->n_free < MISC_CAP)
		KL->free_type_ok[KL->n_free] = (type == M_FILECAPS);
	KL->n_free++;
	std::free(addr);
}

extern "C" void
kmock_audit_arg_fd(int fd)
{

	if (KL->n_audit < MISC_CAP)
		KL->audit_fd[KL->n_audit] = fd;
	KL->n_audit++;
}

extern "C" void
kmock_filedesc_slock(P::filedesc *fdp)
{

	(void)fdp;
	KL->slock++;
}

extern "C" void
kmock_filedesc_sunlock(P::filedesc *fdp)
{

	(void)fdp;
	KL->sunlock++;
}

extern "C" P::file *
fget_noref(P::filedesc *fdp, int fd)
{
	P::file *fp;

	if (fd < 0 || fd >= fdp->fd_nfiles)
		fp = nullptr;
	else
		fp = fdp->fd_ofiles[fd].fde_file;
	if (KL->n_fget < MISC_CAP) {
		KL->fget_fd[KL->n_fget] = fd;
		KL->fget_null[KL->n_fget] = (fp == nullptr);
	}
	KL->n_fget++;
	return (fp);
}

extern "C" int
kern_abort2(P::thread *td, const char *why, int nargs, void **uargs)
{
	int i, n;

	KL->a2_calls++;
	KL->a2_td_ok = (td == expect_td);
	KL->a2_why_ok = (why == expect_why);
	KL->a2_nargs = nargs;
	KL->a2_uargsp_null = (uargs == nullptr);
	n = 0;
	if (uargs != nullptr && nargs > 0) {
		n = nargs > UARG_CAP ? UARG_CAP : nargs;
		for (i = 0; i < n; i++)
			KL->a2_uargs[i] =
			    (unsigned long long)(P::uintptr_t)uargs[i];
	}
	KL->a2_nlogged = n;
	return ((int)(900u + (unsigned)nargs));
}

extern "C" int
kern_cap_ioctls_limit(P::thread *td, int fd, P::u_long *cmds, P::size_t ncmds)
{
	P::size_t i, n;

	KL->cl_calls++;
	KL->cl_td_ok = (td == expect_td);
	KL->cl_fd = fd;
	KL->cl_ncmds = ncmds;
	KL->cl_cmds_null = (cmds == nullptr);
	n = 0;
	if (cmds != nullptr) {
		n = ncmds > (P::size_t)CMD_CAP ? (P::size_t)CMD_CAP : ncmds;
		for (i = 0; i < n; i++)
			KL->cl_cmds[i] = cmds[i];
	}
	KL->cl_nlogged = (int)n;
	std::free(cmds);		/* the kernel takes ownership */
	return ((int)(1000u + (unsigned)ncmds * 7u + (unsigned)fd));
}

/* ------------------------------------------------------------------ */
/* comparison / bookkeeping					      */
/* ------------------------------------------------------------------ */

static const char *
log_diff(const KLog &a, const KLog &b)
{
	int i, n;

#define	DS(f)	if (a.f != b.f) return (#f);
#define	DA(arr, cnt, cap)						\
	n = a.cnt < (cap) ? a.cnt : (cap);				\
	for (i = 0; i < n; i++)						\
		if (a.arr[i] != b.arr[i])				\
			return (#arr);

	DS(rv) DS(retval0) DS(retval1)
	DS(n_fu) DS(n_su) DS(n_alloc) DS(n_free) DS(n_copyin) DS(n_audit)
	DS(slock) DS(sunlock) DS(n_fget)
	DS(a2_calls) DS(a2_td_ok) DS(a2_why_ok) DS(a2_nargs)
	DS(a2_uargsp_null) DS(a2_nlogged)
	DS(cl_calls) DS(cl_td_ok) DS(cl_fd) DS(cl_ncmds) DS(cl_cmds_null)
	DS(cl_nlogged)

	DA(fu_idx, n_fu, FU_CAP)
	DA(fu_ret, n_fu, FU_CAP)
	DA(fu_val, n_fu, FU_CAP)
	DA(su_idx, n_su, SU_CAP)
	DA(su_ret, n_su, SU_CAP)
	DA(su_val, n_su, SU_CAP)
	DA(alloc_size, n_alloc, MISC_CAP)
	DA(alloc_type_ok, n_alloc, MISC_CAP)
	DA(alloc_flags, n_alloc, MISC_CAP)
	DA(free_type_ok, n_free, MISC_CAP)
	DA(copyin_len, n_copyin, MISC_CAP)
	DA(copyin_off, n_copyin, MISC_CAP)
	DA(copyin_ret, n_copyin, MISC_CAP)
	DA(audit_fd, n_audit, MISC_CAP)
	DA(fget_fd, n_fget, MISC_CAP)
	DA(fget_null, n_fget, MISC_CAP)
	DA(a2_uargs, a2_nlogged, UARG_CAP)
	DA(cl_cmds, cl_nlogged, CMD_CAP)

#undef DA
#undef DS
	return (nullptr);
}

struct Stat {
	const char	*name;
	long long	 cases;
	long long	 fails;
	char		 first[512];
};

static Stat st[4] = {
	{ "freebsd32_abort2", 0, 0, "" },
	{ "freebsd32_cap_ioctls_limit", 0, 0, "" },
	{ "freebsd32_cap_ioctls_get", 0, 0, "" },
	{ "freebsd32_syscallnames", 0, 0, "" }
};

static void
fail(Stat &s, const char *what, const char *fmt, ...)
{
	va_list ap;
	char buf[384];

	va_start(ap, fmt);
	std::vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (s.fails == 0)
		std::snprintf(s.first, sizeof(s.first), "%s: %s", what, buf);
	s.fails++;
}

static const long SENT0 = 0x5a5a5a5a5a5a5a5aL;
static const long SENT1 = 0x3c3c3c3c3c3c3c3cL;

static unsigned long long
mix(unsigned long long x)
{

	x ^= x >> 33;
	x *= 0xff51afd7ed558ccdULL;
	x ^= x >> 33;
	x *= 0xc4ceb9fe1a85ec53ULL;
	x ^= x >> 33;
	return (x);
}

/* ------------------------------------------------------------------ */
/* freebsd32_abort2						      */
/* ------------------------------------------------------------------ */

enum { A_WORDS = 40, A_DATA = 32 };
static P::uint32_t a2_pri[A_WORDS], a2_bufP[A_WORDS], a2_bufR[A_WORDS];

static void
run_abort2(int nargs, int args_null, long fu_cnt, unsigned long long mask,
    const P::uint32_t *data, const char *why)
{
	P::proc prP, prR;
	P::thread tdP, tdR;
	P::freebsd32_abort2_args uapP, uapR;
	const char *d;
	int i;

	std::memset(a2_pri, 0x7f, sizeof(a2_pri));
	for (i = 0; i < A_DATA; i++)
		a2_pri[i] = data[i];
	std::memcpy(a2_bufP, a2_pri, sizeof(a2_pri));
	std::memcpy(a2_bufR, a2_pri, sizeof(a2_pri));

	prP.p_fd = nullptr;
	prR.p_fd = nullptr;
	tdP.td_proc = &prP;
	tdR.td_proc = &prR;
	tdP.td_retval[0] = tdR.td_retval[0] = SENT0;
	tdP.td_retval[1] = tdR.td_retval[1] = SENT1;

	uapP.why = why;
	uapP.nargs = nargs;
	uapP.args = args_null != 0 ? nullptr : a2_bufP;
	uapR.why = why;
	uapR.nargs = nargs;
	uapR.args = args_null != 0 ? nullptr : a2_bufR;

	su_base = nullptr;
	su_count = 0;
	su_mask = 0;
	fu_count = fu_cnt;
	fu_mask = mask;
	expect_why = why;

	KL = &g_logP;
	std::memset(KL, 0, sizeof(*KL));
	fu_base = a2_bufP;
	expect_td = &tdP;
	g_logP.rv = P::freebsd32_abort2(&tdP, &uapP);
	g_logP.retval0 = tdP.td_retval[0];
	g_logP.retval1 = tdP.td_retval[1];

	KL = &g_logR;
	std::memset(KL, 0, sizeof(*KL));
	fu_base = a2_bufR;
	expect_td = &tdR;
	g_logR.rv = ref_freebsd32_abort2(&tdR, &uapR);
	g_logR.retval0 = tdR.td_retval[0];
	g_logR.retval1 = tdR.td_retval[1];

	st[0].cases++;
	d = log_diff(g_logP, g_logR);
	if (d == nullptr && std::memcmp(a2_bufP, a2_bufR, sizeof(a2_bufP)) != 0)
		d = "args buffer differs";
	if (d == nullptr && std::memcmp(a2_bufP, a2_pri, sizeof(a2_pri)) != 0)
		d = "args buffer was written";
	if (d != nullptr)
		fail(st[0], d,
		    "nargs=%d args_null=%d fu_count=%ld mask=0x%llx "
		    "rv %d/%d nargs %d/%d nfu %d/%d",
		    nargs, args_null, fu_cnt, mask, g_logP.rv, g_logR.rv,
		    g_logP.a2_nargs, g_logR.a2_nargs, g_logP.n_fu,
		    g_logR.n_fu);
}

/* ------------------------------------------------------------------ */
/* freebsd32_cap_ioctls_limit					      */
/* ------------------------------------------------------------------ */

enum { U_WORDS = 320, U_DATA = 304 };
static P::uint32_t cl_pri[U_WORDS], cl_bufP[U_WORDS], cl_bufR[U_WORDS];

static void
run_climit(int fd, P::uint32_t ncmds, int cifail, const P::uint32_t *data)
{
	P::proc prP, prR;
	P::thread tdP, tdR;
	P::freebsd32_cap_ioctls_limit_args uapP, uapR;
	const char *d;
	int i;

	std::memset(cl_pri, 0x7f, sizeof(cl_pri));
	for (i = 0; i < U_DATA; i++)
		cl_pri[i] = data[i % A_WORDS] ^ (P::uint32_t)(i << 24);
	std::memcpy(cl_bufP, cl_pri, sizeof(cl_pri));
	std::memcpy(cl_bufR, cl_pri, sizeof(cl_pri));

	prP.p_fd = nullptr;
	prR.p_fd = nullptr;
	tdP.td_proc = &prP;
	tdR.td_proc = &prR;
	tdP.td_retval[0] = tdR.td_retval[0] = SENT0;
	tdP.td_retval[1] = tdR.td_retval[1] = SENT1;

	uapP.fd = fd;
	uapP.cmds = cl_bufP;
	uapP.ncmds = ncmds;
	uapR.fd = fd;
	uapR.cmds = cl_bufR;
	uapR.ncmds = ncmds;

	fu_base = nullptr;
	fu_count = 0;
	fu_mask = 0;
	su_base = nullptr;
	su_count = 0;
	su_mask = 0;
	copyin_fail = cifail;
	copyin_max = sizeof(cl_bufP);

	KL = &g_logP;
	std::memset(KL, 0, sizeof(*KL));
	copyin_base = cl_bufP;
	expect_td = &tdP;
	g_logP.rv = P::freebsd32_cap_ioctls_limit(&tdP, &uapP);
	g_logP.retval0 = tdP.td_retval[0];
	g_logP.retval1 = tdP.td_retval[1];

	KL = &g_logR;
	std::memset(KL, 0, sizeof(*KL));
	copyin_base = cl_bufR;
	expect_td = &tdR;
	g_logR.rv = ref_freebsd32_cap_ioctls_limit(&tdR, &uapR);
	g_logR.retval0 = tdR.td_retval[0];
	g_logR.retval1 = tdR.td_retval[1];

	st[1].cases++;
	d = log_diff(g_logP, g_logR);
	if (d == nullptr && std::memcmp(cl_bufP, cl_bufR, sizeof(cl_bufP)) != 0)
		d = "user cmds buffer differs";
	if (d == nullptr && std::memcmp(cl_bufP, cl_pri, sizeof(cl_pri)) != 0)
		d = "user cmds buffer was written";
	if (d != nullptr)
		fail(st[1], d,
		    "fd=%d ncmds=%u copyin_fail=%d rv %d/%d ncmds %llu/%llu "
		    "cmds_null %d/%d nalloc %d/%d",
		    fd, ncmds, cifail, g_logP.rv, g_logR.rv,
		    g_logP.cl_ncmds, g_logR.cl_ncmds, g_logP.cl_cmds_null,
		    g_logR.cl_cmds_null, g_logP.n_alloc, g_logR.n_alloc);
}

/* ------------------------------------------------------------------ */
/* freebsd32_cap_ioctls_get					      */
/* ------------------------------------------------------------------ */

enum {
	OUT_WORDS = 96,		/* words suword32 will accept		*/
	OUT_TOTAL = 104,	/* words allocated (hard guard past end)	*/
	IOC_N = 384,		/* fde_ioctls capacity			*/
	IOC_FILL = 200,		/* entries refreshed per case		*/
	NFD = 8
};

static P::uint32_t out_pri[OUT_TOTAL], out_bufP[OUT_TOTAL], out_bufR[OUT_TOTAL];
static P::u_long iocP[IOC_N], iocR[IOC_N];
static P::filedescent fdeP[NFD], fdeR[NFD];
static P::file fobjP, fobjR;

static void
fill_ioctls(unsigned long long seed)
{
	static const P::u_long fixed[8] = {
		0UL, 0xffffffffffffffffUL, 0x80000000UL, 0x7fffffffUL,
		0xffffffff00000000UL, 0x8080808080808080UL, 0xffUL, 0x80UL
	};
	P::u_long v;
	int i;

	for (i = 0; i < 8; i++) {
		iocP[i] = fixed[i];
		iocR[i] = fixed[i];
	}
	for (i = 8; i < IOC_FILL; i++) {
		v = (P::u_long)mix(seed + (unsigned long long)i);
		switch (i % 5) {
		case 0:
			v |= 0x8000000080000000UL;
			break;
		case 1:
			v &= 0xffUL;
			break;
		case 2:
			v = (v & 0xffffffffUL) | 0x80000000UL;
			break;
		default:
			break;
		}
		iocP[i] = v;
		iocR[i] = v;
	}
}

static void
run_cget(int fd, int file_present, int ioctls_present, int nioctls,
    int cmds32_null, P::uint32_t maxcmds, unsigned long long mask,
    unsigned long long seed)
{
	P::filedesc fdpP, fdpR;
	P::proc prP, prR;
	P::thread tdP, tdR;
	P::freebsd32_cap_ioctls_get_args uapP, uapR;
	const char *d;
	int i;

	fill_ioctls(seed);
	std::memset(out_pri, 0x7f, sizeof(out_pri));
	std::memcpy(out_bufP, out_pri, sizeof(out_pri));
	std::memcpy(out_bufR, out_pri, sizeof(out_pri));

	for (i = 0; i < NFD; i++) {
		fdeP[i].fde_file = nullptr;
		fdeP[i].fde_ioctls = nullptr;
		fdeP[i].fde_nioctls = 0;
		fdeR[i].fde_file = nullptr;
		fdeR[i].fde_ioctls = nullptr;
		fdeR[i].fde_nioctls = 0;
	}
	if (fd >= 0 && fd < NFD) {
		fdeP[fd].fde_file = file_present != 0 ? &fobjP : nullptr;
		fdeP[fd].fde_ioctls = ioctls_present != 0 ? iocP : nullptr;
		fdeP[fd].fde_nioctls = (P::int16_t)nioctls;
		fdeR[fd].fde_file = file_present != 0 ? &fobjR : nullptr;
		fdeR[fd].fde_ioctls = ioctls_present != 0 ? iocR : nullptr;
		fdeR[fd].fde_nioctls = (P::int16_t)nioctls;
	}
	fdpP.fd_ofiles = fdeP;
	fdpP.fd_nfiles = NFD;
	fdpR.fd_ofiles = fdeR;
	fdpR.fd_nfiles = NFD;
	prP.p_fd = &fdpP;
	prR.p_fd = &fdpR;
	tdP.td_proc = &prP;
	tdR.td_proc = &prR;
	tdP.td_retval[0] = tdR.td_retval[0] = SENT0;
	tdP.td_retval[1] = tdR.td_retval[1] = SENT1;

	uapP.fd = fd;
	uapP.cmds = cmds32_null != 0 ? nullptr : out_bufP;
	uapP.maxcmds = maxcmds;
	uapR.fd = fd;
	uapR.cmds = cmds32_null != 0 ? nullptr : out_bufR;
	uapR.maxcmds = maxcmds;

	fu_base = nullptr;
	fu_count = 0;
	fu_mask = 0;
	su_count = OUT_WORDS;
	su_mask = mask;

	KL = &g_logP;
	std::memset(KL, 0, sizeof(*KL));
	su_base = out_bufP;
	expect_td = &tdP;
	g_logP.rv = P::freebsd32_cap_ioctls_get(&tdP, &uapP);
	g_logP.retval0 = tdP.td_retval[0];
	g_logP.retval1 = tdP.td_retval[1];

	KL = &g_logR;
	std::memset(KL, 0, sizeof(*KL));
	su_base = out_bufR;
	expect_td = &tdR;
	g_logR.rv = ref_freebsd32_cap_ioctls_get(&tdR, &uapR);
	g_logR.retval0 = tdR.td_retval[0];
	g_logR.retval1 = tdR.td_retval[1];

	st[2].cases++;
	d = log_diff(g_logP, g_logR);
	if (d == nullptr &&
	    std::memcmp(out_bufP, out_bufR, sizeof(out_bufP)) != 0)
		d = "output buffer differs";
	if (d == nullptr && std::memcmp(iocP, iocR, sizeof(iocP)) != 0)
		d = "fde_ioctls differs";
	if (d == nullptr && std::memcmp(&out_bufP[OUT_WORDS],
	    &out_pri[OUT_WORDS],
	    (OUT_TOTAL - OUT_WORDS) * sizeof(out_pri[0])) != 0)
		d = "guard words past the accessible window were written";
	if (d != nullptr)
		fail(st[2], d,
		    "fd=%d file=%d ioctls=%d nioctls=%d cmds32_null=%d "
		    "maxcmds=%u mask=0x%llx rv %d/%d retval0 %ld/%ld "
		    "nsu %d/%d",
		    fd, file_present, ioctls_present, nioctls, cmds32_null,
		    maxcmds, mask, g_logP.rv, g_logR.rv, g_logP.retval0,
		    g_logR.retval0, g_logP.n_su, g_logR.n_su);
}

/* ------------------------------------------------------------------ */
/* data patterns						      */
/* ------------------------------------------------------------------ */

static P::uint32_t pat_zero[A_WORDS];
static P::uint32_t pat_ones[A_WORDS];
static P::uint32_t pat_high[A_WORDS];
static P::uint32_t pat_rand[A_WORDS];

static void
init_patterns(void)
{
	int i;

	for (i = 0; i < A_WORDS; i++) {
		pat_zero[i] = 0;
		pat_ones[i] = 0xffffffffu;
		switch (i % 6) {
		case 0:
			pat_high[i] = 0x80000000u;
			break;
		case 1:
			pat_high[i] = 0x000000ffu;
			break;
		case 2:
			pat_high[i] = 0xff80ff80u;
			break;
		case 3:
			pat_high[i] = 0x00000080u;
			break;
		case 4:
			pat_high[i] = 0x7f7f7f7fu;
			break;
		default:
			pat_high[i] = 0x00000001u;
			break;
		}
		pat_rand[i] = (P::uint32_t)mix(0x9e3779b97f4a7c15ULL +
		    (unsigned long long)i);
	}
}

/* xorshift64, fixed seed */
static unsigned long long rng_state = 0x0206b0206b0206b0ULL;

static unsigned long long
rnd(void)
{

	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (rng_state);
}

static void
fill_words(P::uint32_t *out, int n)
{
	unsigned long long q;
	int i;

	for (i = 0; i < n; i++) {
		q = rnd();
		switch (q % 5) {
		case 0:
			out[i] = 0;
			break;
		case 1:
			out[i] = 0xffffffffu;
			break;
		case 2:
			out[i] = 0x80000000u | (P::uint32_t)(q >> 8);
			break;
		case 3:
			out[i] = (P::uint32_t)((q >> 11) & 0xffu);
			break;
		default:
			out[i] = (P::uint32_t)(q >> 13);
			break;
		}
	}
}

/* ------------------------------------------------------------------ */
/* hand written edge cases					      */
/* ------------------------------------------------------------------ */

static void
edge_abort2(void)
{
	static const int nargs_v[] = {
		INT_MIN, INT_MIN + 1, -1000, -17, -2, -1, 0, 1, 2, 3, 8, 15,
		16, 17, 18, 33, INT_MAX - 1, INT_MAX
	};
	static const long fucnt_v[] = { 0, 1, 2, 8, 15, 16, 17, 40 };
	static const unsigned long long mask_v[] = {
		0ULL, 1ULL, 2ULL, 1ULL << 7, 1ULL << 15, ~0ULL
	};
	static const char *why_v[] = { nullptr, "", "x", "\x80\xff\xfe\x01" };
	const P::uint32_t *pat_v[3];
	unsigned int a, b, c, e, f;

	pat_v[0] = pat_zero;
	pat_v[1] = pat_ones;
	pat_v[2] = pat_high;
	for (a = 0; a < sizeof(nargs_v) / sizeof(nargs_v[0]); a++)
		for (b = 0; b < 2; b++)
			for (c = 0; c < sizeof(fucnt_v) / sizeof(fucnt_v[0]);
			    c++)
				for (e = 0;
				    e < sizeof(mask_v) / sizeof(mask_v[0]); e++)
					for (f = 0; f < 3; f++)
						run_abort2(nargs_v[a], (int)b,
						    fucnt_v[c], mask_v[e],
						    pat_v[f],
						    why_v[(a + e + f) % 4]);
}

static void
edge_climit(void)
{
	static const P::uint32_t ncmds_v[] = {
		0, 1, 2, 3, 4, 15, 16, 17, 63, 64, 65, 127, 128, 254, 255,
		256, 257, 258, 1000, 0x7fffffffu, 0x80000000u, 0xffffffffu
	};
	static const int fd_v[] = { -1, 0, 1, 7, 1000 };
	const P::uint32_t *pat_v[4];
	unsigned int a, b, c, e;

	pat_v[0] = pat_zero;
	pat_v[1] = pat_ones;
	pat_v[2] = pat_high;
	pat_v[3] = pat_rand;
	for (a = 0; a < sizeof(ncmds_v) / sizeof(ncmds_v[0]); a++)
		for (b = 0; b < sizeof(fd_v) / sizeof(fd_v[0]); b++)
			for (c = 0; c < 2; c++)
				for (e = 0; e < 4; e++)
					run_climit(fd_v[b], ncmds_v[a], (int)c,
					    pat_v[e]);
}

static void
edge_cget(void)
{
	static const int fd_v[] = { -1, 0, 1, 7, 8, 100 };
	static const int nioctls_v[] = {
		-32768, -100, -2, -1, 0, 1, 2, 3, 15, 16, 31, 32, 33, 95, 96,
		97, 300, 32767
	};
	static const P::uint32_t maxcmds_v[] = {
		0, 1, 2, 3, 15, 16, 31, 32, 33, 96, 97, 0xffffffffu
	};
	static const unsigned long long mask_v[] = { 0ULL, 1ULL, 1ULL << 3 };
	unsigned int a, b, c, e, f, g, h;

	for (a = 0; a < sizeof(fd_v) / sizeof(fd_v[0]); a++)
	    for (b = 0; b < 2; b++)
		for (c = 0; c < 2; c++)
		    for (e = 0; e < sizeof(nioctls_v) / sizeof(nioctls_v[0]);
			e++)
			for (f = 0; f < 2; f++)
			    for (g = 0;
				g < sizeof(maxcmds_v) / sizeof(maxcmds_v[0]);
				g++)
				for (h = 0;
				    h < sizeof(mask_v) / sizeof(mask_v[0]); h++)
					run_cget(fd_v[a], (int)b, (int)c,
					    nioctls_v[e], (int)f, maxcmds_v[g],
					    mask_v[h], 0x1000ULL + a + e + g);
}

/* ------------------------------------------------------------------ */
/* randomised sweeps						      */
/* ------------------------------------------------------------------ */

enum { SWEEP = 200000 };

static void
sweep_abort2(void)
{
	static const char *why_v[] = { nullptr, "", "y", "\xff\x80 why" };
	P::uint32_t data[A_WORDS];
	unsigned long long r, q, mask;
	long it, fu_cnt;
	int nargs, args_null;

	for (it = 0; it < SWEEP; it++) {
		r = rnd();
		switch (r % 10) {
		case 0:
			nargs = -(int)((rnd() % 5) + 1);
			break;
		case 1:
			nargs = 16;
			break;
		case 2:
			nargs = 17;
			break;
		case 3:
			nargs = (int)(rnd() % 40) - 4;
			break;
		case 4:
			nargs = INT_MIN;
			break;
		case 5:
			nargs = INT_MAX;
			break;
		case 6:
			nargs = 0;
			break;
		case 7:
			nargs = 1;
			break;
		default:
			nargs = (int)(rnd() % 18);
			break;
		}
		args_null = (rnd() % 4) == 0;
		fu_cnt = (long)(rnd() % 22);
		mask = 0;
		q = rnd();
		if (q % 3 == 0)
			mask = 1ULL << (rnd() % 20);
		else if (q % 11 == 0)
			mask = ~0ULL;
		fill_words(data, A_WORDS);
		run_abort2(nargs, args_null, fu_cnt, mask, data,
		    why_v[rnd() % 4]);
	}
}

static void
sweep_climit(void)
{
	P::uint32_t data[A_WORDS];
	unsigned long long r, q;
	long it;
	int fd, cifail;
	P::uint32_t ncmds;

	for (it = 0; it < SWEEP; it++) {
		r = rnd();
		switch (r % 12) {
		case 0:
			ncmds = 0;
			break;
		case 1:
			ncmds = 1;
			break;
		case 2:
			ncmds = 255;
			break;
		case 3:
			ncmds = 256;
			break;
		case 4:
			ncmds = 257;
			break;
		case 5:
			ncmds = 0xffffffffu;
			break;
		case 6:
			ncmds = (P::uint32_t)(rnd() % 300);
			break;
		case 7:
			ncmds = (P::uint32_t)(rnd() % 3);
			break;
		default:
			ncmds = (P::uint32_t)(rnd() % 40);
			break;
		}
		q = rnd();
		switch (q % 6) {
		case 0:
			fd = -1;
			break;
		case 1:
			fd = 0;
			break;
		case 2:
			fd = (int)(rnd() % 8);
			break;
		case 3:
			fd = -(int)(rnd() % 1000);
			break;
		default:
			fd = (int)(rnd() % 100000);
			break;
		}
		cifail = (rnd() % 5) == 0;
		fill_words(data, A_WORDS);
		run_climit(fd, ncmds, cifail, data);
	}
}

static void
sweep_cget(void)
{
	unsigned long long mask;
	long it;
	int fd, file_present, ioctls_present, nioctls, cmds32_null;
	P::uint32_t maxcmds;

	for (it = 0; it < SWEEP; it++) {
		switch (rnd() % 8) {
		case 0:
			fd = -1;
			break;
		case 1:
			fd = 8;
			break;
		case 2:
			fd = 100;
			break;
		case 3:
			fd = -(int)(rnd() % 100);
			break;
		default:
			fd = (int)(rnd() % 8);
			break;
		}
		file_present = (rnd() % 4) != 0;
		ioctls_present = (rnd() % 4) != 0;
		switch (rnd() % 10) {
		case 0:
			nioctls = -1;
			break;
		case 1:
			nioctls = 0;
			break;
		case 2:
			nioctls = -(int)(rnd() % 32768);
			break;
		case 3:
			nioctls = (int)(rnd() % 32768);
			break;
		case 4:
			nioctls = 96;
			break;
		case 5:
			nioctls = 97;
			break;
		default:
			nioctls = (int)(rnd() % 40);
			break;
		}
		cmds32_null = (rnd() % 6) == 0;
		switch (rnd() % 8) {
		case 0:
			maxcmds = 0;
			break;
		case 1:
			maxcmds = 0xffffffffu;
			break;
		case 2:
			maxcmds = 96;
			break;
		case 3:
			maxcmds = 97;
			break;
		default:
			maxcmds = (P::uint32_t)(rnd() % 40);
			break;
		}
		mask = 0;
		switch (rnd() % 6) {
		case 0:
			mask = 1ULL << (rnd() % 40);
			break;
		case 1:
			mask = ~0ULL;
			break;
		case 2:
			mask = 1ULL;
			break;
		default:
			break;
		}
		run_cget(fd, file_present, ioctls_present, nioctls,
		    cmds32_null, maxcmds, mask, rnd());
	}
}

/* ------------------------------------------------------------------ */
/* freebsd32_syscallnames					      */
/* ------------------------------------------------------------------ */

static void
check_syscallnames(void)
{
	P::size_t np, nr, i, n;

	np = sizeof(P::freebsd32_syscallnames) /
	    sizeof(P::freebsd32_syscallnames[0]);
	nr = ref_freebsd32_syscallnames_count;

	st[3].cases++;
	if (np != nr)
		fail(st[3], "table size differs", "port=%zu ref=%zu", np, nr);

	n = np < nr ? np : nr;
	for (i = 0; i < n; i++) {
		st[3].cases++;
		if (P::freebsd32_syscallnames[i] == nullptr ||
		    ref_freebsd32_syscallnames[i] == nullptr) {
			fail(st[3], "null entry", "index %zu", i);
			continue;
		}
		if (std::strcmp(P::freebsd32_syscallnames[i],
		    ref_freebsd32_syscallnames[i]) != 0)
			fail(st[3], "entry differs",
			    "index %zu: \"%s\" vs \"%s\"", i,
			    P::freebsd32_syscallnames[i],
			    ref_freebsd32_syscallnames[i]);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long long cases, fails;
	int i;

	init_patterns();

	edge_abort2();
	edge_climit();
	edge_cget();

	sweep_abort2();
	sweep_climit();
	sweep_cget();

	check_syscallnames();

	std::printf("%-30s %10s %10s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-30s %10s %10s  %s\n", "------------------------------",
	    "----------", "----------", "------");
	cases = 0;
	fails = 0;
	for (i = 0; i < 4; i++) {
		std::printf("%-30s %10lld %10lld  %s\n", st[i].name,
		    st[i].cases, st[i].fails,
		    st[i].fails == 0 ? "ok" : "FAIL");
		if (st[i].fails != 0)
			std::printf("    first failure: %s\n", st[i].first);
		cases += st[i].cases;
		fails += st[i].fails;
	}
	std::printf("%-30s %10lld %10lld  %s\n", "TOTAL", cases, fails,
	    fails == 0 ? "ok" : "FAIL");
	return (fails != 0 ? 1 : 0);
}
