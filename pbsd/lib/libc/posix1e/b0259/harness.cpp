/*
 * harness.cpp -- differential test for PBSD batch b0259.
 *
 * acl_free, mac_execve and the __oldacl_* compatibility wrappers are compared
 * side by side against the C oracle.  free(3) is wrapped to observe whether
 * acl_free releases memory; __mac_execve and the acl_* helpers are defined
 * once here so both sides drive identical instrumentation.
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>

import pbsd.lib.libc.posix1e.b0259;

namespace P = pbsd::lib_libc_posix1e::b0259;

using MAC = P::mac;

extern "C" {
int ref_acl_free(void *obj_p);
int ref_mac_execve(char *fname, char **argv, char **envv, MAC *label);
int ref___oldacl_get_perm_np(uint32_t *permset_d, mode_t perm);
int ref___oldacl_add_perm(uint32_t *permset_d, mode_t perm);
int ref___oldacl_delete_perm(uint32_t *permset_d, mode_t perm);
void __real_free(void *ptr);
}

#define	GUARD		0x7f
#define	SWEEP_ITERS	200000L
#define	MAX_PRINT	12

enum {
	F_ACL_FREE,
	F_MAC_EXECVE,
	F_OLDACL_GET,
	F_OLDACL_ADD,
	F_OLDACL_DEL,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"acl_free",
	"mac_execve",
	"__oldacl_get_perm_np",
	"__oldacl_add_perm",
	"__oldacl_delete_perm",
};

static long long ncases[NFUNC];
static long long nfails[NFUNC];
static int nprinted;

/* ------------------------------------------------------------------ PRNG */

static uint64_t rng_state = 0xb0259decafbadULL;

static uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static int
rndbit(void)
{
	return ((int)(rnd32() & 1u));
}

/* ------------------------------------------------------------------ stats */

static void
fail(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nprinted < MAX_PRINT) {
		nprinted++;
		std::printf("  FAIL %-22s %-24s %s\n", fname[f], ctx, detail);
	} else if (nprinted == MAX_PRINT) {
		nprinted++;
		std::printf("  ... further failures suppressed\n");
	}
}

static void
case_bump(int f)
{
	ncases[f]++;
}

/* ---------------------------------------------------------- free tracking */

static int g_free_calls;
static void *g_freed_ptr;

extern "C" void
__wrap_free(void *ptr)
{
	g_freed_ptr = ptr;
	g_free_calls++;
	__real_free(ptr);
}

static void
free_track_reset(void)
{
	g_free_calls = 0;
	g_freed_ptr = NULL;
}

/* -------------------------------------------------------- helper mocks */

struct AclCall {
	int		which;
	uint32_t	*permset;
	long long	permset_off;
	mode_t		perm;
	int		ret;
	int		err;
};

static uint32_t g_perm_storage;
static uint32_t *g_perm_base = &g_perm_storage;

static AclCall g_acl_last;
static int g_acl_calls;

static int
acl_mix(uint32_t *permset, mode_t perm, int tag)
{
	uint64_t h = (uint64_t)(permset == NULL ? 0 :
	    (long long)(permset - g_perm_base));

	h ^= (uint64_t)perm * 0x9e3779b97f4a7c15ULL;
	h ^= (uint64_t)tag * 0xd6e8feb86659fd93ULL;
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdULL;
	h ^= h >> 33;
	return ((int)(h & 0x7fffffffu) % 5) - 2;
}

static void
acl_record(int which, uint32_t *permset, mode_t perm, int ret, int err)
{
	g_acl_calls++;
	g_acl_last.which = which;
	g_acl_last.permset = permset;
	g_acl_last.permset_off = permset == NULL ? -1 :
	    (long long)(permset - g_perm_base);
	g_acl_last.perm = perm;
	g_acl_last.ret = ret;
	g_acl_last.err = err;
}

extern "C" int
acl_get_perm_np(uint32_t *permset_d, uint32_t perm)
{
	int ret;

	if (permset_d == NULL) {
		errno = EINVAL;
		acl_record(1, permset_d, (mode_t)perm, -1, EINVAL);
		return (-1);
	}
	ret = (*permset_d & perm) ? 1 : 0;
	acl_record(1, permset_d, (mode_t)perm, ret, 0);
	return (ret);
}

extern "C" int
acl_add_perm(uint32_t *permset_d, uint32_t perm)
{
	int ret;

	if (permset_d == NULL) {
		errno = EINVAL;
		acl_record(2, permset_d, (mode_t)perm, -1, EINVAL);
		return (-1);
	}
	*permset_d |= perm;
	ret = acl_mix(permset_d, (mode_t)perm, 2);
	acl_record(2, permset_d, (mode_t)perm, ret, 0);
	return (ret);
}

extern "C" int
acl_delete_perm(uint32_t *permset_d, uint32_t perm)
{
	int ret;

	if (permset_d == NULL) {
		errno = EINVAL;
		acl_record(3, permset_d, (mode_t)perm, -1, EINVAL);
		return (-1);
	}
	*permset_d &= ~perm;
	ret = acl_mix(permset_d, (mode_t)perm, 3);
	acl_record(3, permset_d, (mode_t)perm, ret, 0);
	return (ret);
}

struct MacExecSnap {
	int		calls;
	char		*fname;
	long long	fname_off;
	char		**argv;
	long long	argv_off;
	char		**envv;
	long long	envv_off;
	MAC		*label;
	long long	label_off;
	int		ret;
	int		err;
};

static MacExecSnap g_mac_last;
static const unsigned char *g_mac_r1;
static const unsigned char *g_mac_r2;

static void
mac_regions(const unsigned char *r1, const unsigned char *r2)
{
	g_mac_r1 = r1;
	g_mac_r2 = r2;
}

static long long
mac_off(const void *p)
{
	const unsigned char *q = (const unsigned char *)p;

	if (p == NULL)
		return (-1);
	if (g_mac_r1 != NULL && q >= g_mac_r1 && q < g_mac_r1 + 512)
		return ((long long)(q - g_mac_r1));
	if (g_mac_r2 != NULL && q >= g_mac_r2 && q < g_mac_r2 + 512)
		return ((long long)(q - g_mac_r2));
	return (-2);
}

static int
mac_mix(char *fname, char **argv, char **envv, MAC *label)
{
	uint64_t h = 0;

	if (fname != NULL)
		h ^= (unsigned char)fname[0];
	if (argv != NULL && argv[0] != NULL)
		h ^= (unsigned char)argv[0][0] << 8;
	if (envv != NULL && envv[0] != NULL)
		h ^= (unsigned char)envv[0][0] << 16;
	if (label != NULL)
		h ^= (uint64_t)label->m_buflen;
	h ^= (uint64_t)mac_off(fname);
	h ^= (uint64_t)mac_off(argv) << 16;
	h ^= (uint64_t)mac_off(envv) << 32;
	h ^= (uint64_t)mac_off(label) << 48;
	h ^= h >> 17;
	h *= 0xed5ad4bbULL;
	return ((int)(h % 7) - 3);
}

extern "C" int
__mac_execve(char *fname, char **argv, char **envv, MAC *mac_p)
{
	int ret;

	g_mac_last.calls++;
	g_mac_last.fname = fname;
	g_mac_last.fname_off = mac_off(fname);
	g_mac_last.argv = argv;
	g_mac_last.argv_off = mac_off(argv);
	g_mac_last.envv = envv;
	g_mac_last.envv_off = mac_off(envv);
	g_mac_last.label = mac_p;
	g_mac_last.label_off = mac_off(mac_p);

	if (fname == NULL) {
		errno = EFAULT;
		g_mac_last.ret = -1;
		g_mac_last.err = EFAULT;
		return (-1);
	}

	ret = mac_mix(fname, argv, envv, mac_p);
	g_mac_last.ret = ret;
	g_mac_last.err = 0;
	return (ret);
}

static void
acl_track_reset(void)
{
	std::memset(&g_acl_last, 0, sizeof(g_acl_last));
	g_acl_calls = 0;
	errno = 0;
}

static void
mac_track_reset(void)
{
	std::memset(&g_mac_last, 0, sizeof(g_mac_last));
	errno = 0;
}

static bool
acl_snap_eq(const AclCall &a, const AclCall &b)
{
	return (a.which == b.which &&
	    a.permset_off == b.permset_off &&
	    a.perm == b.perm &&
	    a.ret == b.ret &&
	    a.err == b.err);
}

static bool
mac_snap_eq(const MacExecSnap &a, const MacExecSnap &b)
{
	return (a.calls == b.calls &&
	    a.fname_off == b.fname_off &&
	    a.argv_off == b.argv_off &&
	    a.envv_off == b.envv_off &&
	    a.label_off == b.label_off &&
	    a.ret == b.ret &&
	    a.err == b.err);
}

/* ---------------------------------------------------------- guarded buf */

static const size_t REG = 256;
static const size_t OFF = 32;

struct Region {
	unsigned char b[REG];
};

static void
reg_init(Region &r)
{
	std::memset(r.b, GUARD, REG);
}

static bool
reg_eq(const Region &a, const Region &b)
{
	return (std::memcmp(a.b, b.b, REG) == 0);
}

static bool
reg_unchanged(const Region &after, const Region &before)
{
	return (std::memcmp(after.b, before.b, REG) == 0);
}

/* ------------------------------------------------------------- acl_free */

static void
test_acl_free_once(void *p_port, void *p_ref, int expect_free)
{
	int rp, rr;
	int fc_p, fc_r;
	void *fp_p, *fp_r;

	free_track_reset();
	rp = P::acl_free(p_port);
	fc_p = g_free_calls;
	fp_p = g_freed_ptr;

	free_track_reset();
	rr = ref_acl_free(p_ref);
	fc_r = g_free_calls;
	fp_r = g_freed_ptr;

	case_bump(F_ACL_FREE);

	if (rp != rr)
		fail(F_ACL_FREE, "ret", "return mismatch");
	if (fc_p != fc_r || fc_p != expect_free)
		fail(F_ACL_FREE, "free_calls", "free call count mismatch");
	if (expect_free) {
		if (fp_p != p_port || fp_r != p_ref)
			fail(F_ACL_FREE, "freed_ptr", "freed pointer mismatch");
	} else if (fp_p != NULL || fp_r != NULL)
		fail(F_ACL_FREE, "freed_ptr", "unexpected free target");
}

static void
edge_acl_free(void)
{
	test_acl_free_once(NULL, NULL, 0);

	void *p1 = std::malloc(1);
	void *p2 = std::malloc(16);
	if (p1 != NULL && p2 != NULL) {
		test_acl_free_once(p1, p2, 1);
	} else {
		std::free(p1);
		std::free(p2);
	}

	void *p3 = std::malloc(0);
	void *p4 = std::malloc(0);
	if (p3 != NULL && p4 != NULL)
		test_acl_free_once(p3, p4, 1);
	else {
		std::free(p3);
		std::free(p4);
	}
}

static void
sweep_acl_free(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		if (rndbit() == 0) {
			test_acl_free_once(NULL, NULL, 0);
		} else {
			size_t sz = (size_t)(rnd32() & 0xffu);
			void *p1 = std::malloc(sz == 0 ? 1 : sz);
			void *p2 = std::malloc(sz == 0 ? 1 : sz);
			if (p1 == NULL || p2 == NULL) {
				std::free(p1);
				std::free(p2);
				continue;
			}
			test_acl_free_once(p1, p2, 1);
		}
	}
}

/* ----------------------------------------------------------- mac_execve */

static void
fill_str(unsigned char *dst, size_t cap, uint32_t seed, int allow_nul)
{
	size_t i, len;

	if (cap == 0)
		return;
	len = seed % cap;
	if (len == 0 && (seed & 1u) == 0)
		len = 1;
	for (i = 0; i < cap; i++)
		dst[i] = GUARD;
	for (i = 0; i < len; i++) {
		unsigned char c = (unsigned char)((seed + (uint32_t)i * 17u) & 0xffu);
		if (!allow_nul && c == 0)
			c = 0x80;
		dst[i] = c;
	}
	dst[len % cap] = '\0';
}

static void
setup_mac_exec(Region &r1, Region &r2, uint32_t seed, int null_fname,
    int null_argv, int null_envv, int null_label)
{
	char *fname_p, *fname_r;
	char **argv_p, **argv_r;
	char **envv_p, **envv_r;
	MAC *lab_p, *lab_r;
	unsigned char *s1, *s2;
	size_t base;
	Region snap1, snap2;

	reg_init(r1);
	reg_init(r2);

	base = OFF;
	s1 = r1.b + base;
	s2 = r2.b + base;
	fill_str(s1, 64, seed, 1);
	fill_str(s2, 64, seed, 1);
	fname_p = null_fname ? NULL : (char *)s1;
	fname_r = null_fname ? NULL : (char *)s2;

	s1 = r1.b + base + 72;
	s2 = r2.b + base + 72;
	fill_str(s1, 32, seed ^ 0xa5a5a5a5u, 1);
	fill_str(s2, 32, seed ^ 0xa5a5a5a5u, 1);

	s1 = r1.b + base + 112;
	s2 = r2.b + base + 112;
	fill_str(s1, 32, seed ^ 0x5a5a5a5au, 1);
	fill_str(s2, 32, seed ^ 0x5a5a5a5au, 1);

	argv_p = null_argv ? NULL : (char **)(r1.b + base + 160);
	argv_r = null_argv ? NULL : (char **)(r2.b + base + 160);
	if (!null_argv) {
		argv_p[0] = (char *)(r1.b + base + 72);
		argv_p[1] = NULL;
		argv_r[0] = (char *)(r2.b + base + 72);
		argv_r[1] = NULL;
	}

	envv_p = null_envv ? NULL : (char **)(r1.b + base + 176);
	envv_r = null_envv ? NULL : (char **)(r2.b + base + 176);
	if (!null_envv) {
		envv_p[0] = (char *)(r1.b + base + 112);
		envv_p[1] = NULL;
		envv_r[0] = (char *)(r2.b + base + 112);
		envv_r[1] = NULL;
	}

	lab_p = null_label ? NULL : (MAC *)(r1.b + base + 192);
	lab_r = null_label ? NULL : (MAC *)(r2.b + base + 192);
	if (!null_label) {
		lab_p->m_buflen = (size_t)(seed & 0xffffu);
		lab_p->m_string = (char *)(r1.b + base + 72);
		lab_r->m_buflen = (size_t)(seed & 0xffffu);
		lab_r->m_string = (char *)(r2.b + base + 72);
	}

	snap1 = r1;
	snap2 = r2;
	mac_regions(r1.b, r2.b);
	mac_track_reset();
	int rp = P::mac_execve(fname_p, argv_p, envv_p, lab_p);
	MacExecSnap sp = g_mac_last;
	int ep = errno;

	mac_track_reset();
	int rr = ref_mac_execve(fname_r, argv_r, envv_r, lab_r);
	MacExecSnap sr = g_mac_last;
	int er = errno;

	case_bump(F_MAC_EXECVE);

	if (seed == 0 && null_fname == 0 && null_argv == 0 && null_envv == 0 &&
	    null_label == 0) {
		std::fprintf(stderr,
		    "DBG rp=%d rr=%d fo %lld/%lld ao %lld/%lld eo %lld/%lld lo %lld/%lld ret %d/%d\n",
		    rp, rr, sp.fname_off, sr.fname_off, sp.argv_off, sr.argv_off,
		    sp.envv_off, sr.envv_off, sp.label_off, sr.label_off,
		    sp.ret, sr.ret);
	}

	if (rp != rr)
		fail(F_MAC_EXECVE, "ret", "return mismatch");
	if (ep != er)
		fail(F_MAC_EXECVE, "errno", "errno mismatch");
	if (!mac_snap_eq(sp, sr))
		fail(F_MAC_EXECVE, "mock", "helper call mismatch");
	if (!reg_unchanged(r1, snap1) || !reg_unchanged(r2, snap2))
		fail(F_MAC_EXECVE, "buffer", "region mutated");
}

static void
edge_mac_execve(void)
{
	Region r1, r2;

	setup_mac_exec(r1, r2, 0, 0, 0, 0, 0);
	setup_mac_exec(r1, r2, 1, 1, 0, 0, 0);
	setup_mac_exec(r1, r2, 0xff, 0, 1, 0, 0);
	setup_mac_exec(r1, r2, 0x80808080u, 0, 0, 1, 0);
	setup_mac_exec(r1, r2, 0xdeadbeefu, 0, 0, 0, 1);
	setup_mac_exec(r1, r2, 0x7f7f7f7fu, 0, 1, 1, 1);
}

static void
sweep_mac_execve(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		Region r1, r2;
		uint32_t seed = rnd32();

		setup_mac_exec(r1, r2, seed,
		    rndbit(), rndbit(), rndbit(), rndbit());
	}
}

/* -------------------------------------------------------- oldacl helpers */

static void
test_oldacl_once(int which, uint32_t *perm_p, uint32_t *perm_r, mode_t perm,
    Region &rp, Region &rr)
{
	int ret_p, ret_r;
	AclCall snap_p, snap_r;

	g_perm_base = perm_p;
	acl_track_reset();
	switch (which) {
	case F_OLDACL_GET:
		ret_p = P::__oldacl_get_perm_np(perm_p, perm);
		break;
	case F_OLDACL_ADD:
		ret_p = P::__oldacl_add_perm(perm_p, perm);
		break;
	default:
		ret_p = P::__oldacl_delete_perm(perm_p, perm);
		break;
	}
	snap_p = g_acl_last;
	int ep = errno;

	g_perm_base = perm_r;
	acl_track_reset();
	switch (which) {
	case F_OLDACL_GET:
		ret_r = ref___oldacl_get_perm_np(perm_r, perm);
		break;
	case F_OLDACL_ADD:
		ret_r = ref___oldacl_add_perm(perm_r, perm);
		break;
	default:
		ret_r = ref___oldacl_delete_perm(perm_r, perm);
		break;
	}
	snap_r = g_acl_last;
	int er = errno;

	case_bump(which);

	if (ret_p != ret_r)
		fail(which, "ret", "return mismatch");
	if (ep != er)
		fail(which, "errno", "errno mismatch");
	if (!acl_snap_eq(snap_p, snap_r))
		fail(which, "mock", "helper call mismatch");
	if (!reg_eq(rp, rr))
		fail(which, "buffer", "region mismatch");
}

static void
setup_perm_regions(Region &rp, Region &rr, uint32_t seed, int null_permset)
{
	reg_init(rp);
	reg_init(rr);
	if (!null_permset) {
		uint32_t *pp = (uint32_t *)(rp.b + OFF);
		uint32_t *pr = (uint32_t *)(rr.b + OFF);
		*pp = seed;
		*pr = seed;
	}
}

static void
edge_oldacl(int which)
{
	Region rp, rr;
	mode_t perms[] = { 0, 1, 0400, 0200, 0100, 0040, 0020, 0010,
	    0004, 0002, 0001, 0777, 01777, (mode_t)0x80000000u,
	    (mode_t)0xffffffffu, (mode_t)0x80, (mode_t)0xff };
	size_t i;

	for (i = 0; i < sizeof(perms) / sizeof(perms[0]); i++) {
		setup_perm_regions(rp, rr, (uint32_t)i * 0x11111111u, 0);
		test_oldacl_once(which, (uint32_t *)(rp.b + OFF),
		    (uint32_t *)(rr.b + OFF), perms[i], rp, rr);
	}

	setup_perm_regions(rp, rr, 0, 1);
	test_oldacl_once(which, NULL, NULL, 0, rp, rr);
	setup_perm_regions(rp, rr, 0, 1);
	test_oldacl_once(which, NULL, NULL, (mode_t)0xff, rp, rr);
}

static void
sweep_oldacl(int which)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		Region rp, rr;
		uint32_t seed = rnd32();
		int nullset = (seed & 0x1000u) == 0;
		mode_t perm = (mode_t)(seed ^ (seed >> 16));

		setup_perm_regions(rp, rr, seed, nullset);
		test_oldacl_once(which,
		    nullset ? NULL : (uint32_t *)(rp.b + OFF),
		    nullset ? NULL : (uint32_t *)(rr.b + OFF),
		    perm, rp, rr);
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	edge_acl_free();
	sweep_acl_free();

	edge_mac_execve();
	sweep_mac_execve();

	edge_oldacl(F_OLDACL_GET);
	sweep_oldacl(F_OLDACL_GET);

	edge_oldacl(F_OLDACL_ADD);
	sweep_oldacl(F_OLDACL_ADD);

	edge_oldacl(F_OLDACL_DEL);
	sweep_oldacl(F_OLDACL_DEL);

	std::printf("\n%-26s %12s %12s\n", "function", "cases", "failures");
	for (int f = 0; f < NFUNC; f++)
		std::printf("%-26s %12lld %12lld\n", fname[f], ncases[f],
		    nfails[f]);

	long long total_fail = 0;
	for (int f = 0; f < NFUNC; f++)
		total_fail += nfails[f];

	return (total_fail == 0 ? 0 : 1);
}
