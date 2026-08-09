/*
 * harness.cpp -- differential test for PBSD batch b0320.
 *
 * mac_get_* wrappers and acl_init/acl_dup are compared side by side against
 * the C oracle.  External helpers (__mac_get_*, getsockopt) and posix_memalign
 * are instrumented once here so both sides drive identical behaviour.
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>

import pbsd.lib.libc.posix1e.b0320;

namespace P = pbsd::lib_libc_posix1e::b0320;

using MAC = P::mac;
using ACL = P::acl_t_struct;

#ifndef SOL_SOCKET
#define SOL_SOCKET	1
#endif
#ifndef SO_PEERLABEL
#define SO_PEERLABEL	0x1010
#endif

extern "C" {
int ref_mac_get_fd(int fd, MAC *label);
int ref_mac_get_file(const char *path, MAC *label);
int ref_mac_get_link(const char *path, MAC *label);
int ref_mac_get_peer(int fd, MAC *label);
int ref_mac_get_pid(pid_t pid, MAC *label);
int ref_mac_get_proc(MAC *label);
P::acl_t ref_acl_init(int count);
P::acl_t ref_acl_dup(P::acl_t acl);
int __real_posix_memalign(void **memptr, size_t alignment, size_t size);
}

#define	GUARD		0x7f
#define	SWEEP_ITERS	50000L
#define	MAX_PRINT	12
#define	ACL_ALIGN	(1u << P::_ACL_T_ALIGNMENT_BITS)

enum {
	F_MAC_GET_FD,
	F_MAC_GET_FILE,
	F_MAC_GET_LINK,
	F_MAC_GET_PEER,
	F_MAC_GET_PID,
	F_MAC_GET_PROC,
	F_ACL_INIT,
	F_ACL_DUP,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"mac_get_fd",
	"mac_get_file",
	"mac_get_link",
	"mac_get_peer",
	"mac_get_pid",
	"mac_get_proc",
	"acl_init",
	"acl_dup",
};

static long long ncases[NFUNC];
static long long nfails[NFUNC];
static int nprinted;

/* ------------------------------------------------------------------ PRNG */

static uint64_t rng_state = 0xb0320decafbadULL;

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

/* ---------------------------------------------------------- guarded buf */

static const size_t REG = 512;
static const size_t OFF = 64;

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

static void
fill_path(unsigned char *dst, size_t cap, uint32_t seed, int allow_nul)
{
	size_t i, len;

	if (cap == 0)
		return;
	len = seed % cap;
	if (len == 0 && (seed & 3u) == 0)
		len = 1;
	for (i = 0; i < cap; i++)
		dst[i] = GUARD;
	for (i = 0; i < len; i++) {
		unsigned char c = (unsigned char)((seed + (uint32_t)i * 23u) & 0xffu);
		if (!allow_nul && c == 0)
			c = 0x80;
		dst[i] = c;
	}
	dst[len % cap] = '\0';
}

/* -------------------------------------------------------- helper mocks */

static const unsigned char *g_r1;
static const unsigned char *g_r2;

static void
set_regions(const unsigned char *r1, const unsigned char *r2)
{
	g_r1 = r1;
	g_r2 = r2;
}

static long long
buf_off(const void *p)
{
	const unsigned char *q = (const unsigned char *)p;

	if (p == NULL)
		return (-1);
	if (g_r1 != NULL && q >= g_r1 && q < g_r1 + REG)
		return ((long long)(q - g_r1));
	if (g_r2 != NULL && q >= g_r2 && q < g_r2 + REG)
		return ((long long)(q - g_r2));
	return (-2);
}

struct MacCall {
	int		which;
	int		fd;
	pid_t		pid;
	long long	path_off;
	long long	label_off;
	int		ret;
	int		err;
};

static MacCall g_mac_last;
static int g_mac_calls;

static int
mac_mix(int which, int fd, pid_t pid, long long path_off, long long label_off)
{
	uint64_t h = (uint64_t)which;

	h ^= (uint64_t)(unsigned int)fd * 0x9e3779b97f4a7c15ULL;
	h ^= (uint64_t)(unsigned int)pid << 16;
	h ^= (uint64_t)path_off << 24;
	h ^= (uint64_t)label_off << 32;
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdULL;
	h ^= h >> 33;
	return ((int)(h & 0x7fffffffu) % 7) - 3;
}

static void
mac_record(int which, int fd, pid_t pid, const char *path, MAC *label,
    int ret, int err)
{
	g_mac_calls++;
	g_mac_last.which = which;
	g_mac_last.fd = fd;
	g_mac_last.pid = pid;
	g_mac_last.path_off = buf_off(path);
	g_mac_last.label_off = buf_off(label);
	g_mac_last.ret = ret;
	g_mac_last.err = err;
}

static void
mac_touch(MAC *label, int tag)
{
	if (label == NULL)
		return;
	label->m_buflen = (size_t)(0x100 + tag);
	label->m_string = (char *)(uintptr_t)(0x2000 + tag);
}

extern "C" int
__mac_get_fd(int fd, MAC *mac_p)
{
	int ret;

	if (mac_p == NULL) {
		errno = EINVAL;
		mac_record(1, fd, 0, NULL, mac_p, -1, EINVAL);
		return (-1);
	}
	mac_touch(mac_p, fd & 0xff);
	ret = mac_mix(1, fd, 0, -1, buf_off(mac_p));
	mac_record(1, fd, 0, NULL, mac_p, ret, 0);
	return (ret);
}

extern "C" int
__mac_get_file(const char *path_p, MAC *mac_p)
{
	int ret;

	if (mac_p == NULL) {
		errno = EINVAL;
		mac_record(2, 0, 0, path_p, mac_p, -1, EINVAL);
		return (-1);
	}
	if (path_p == NULL) {
		errno = EFAULT;
		mac_record(2, 0, 0, path_p, mac_p, -1, EFAULT);
		return (-1);
	}
	mac_touch(mac_p, (int)(unsigned char)path_p[0]);
	ret = mac_mix(2, 0, 0, buf_off(path_p), buf_off(mac_p));
	mac_record(2, 0, 0, path_p, mac_p, ret, 0);
	return (ret);
}

extern "C" int
__mac_get_link(const char *path_p, MAC *mac_p)
{
	int ret;

	if (mac_p == NULL) {
		errno = EINVAL;
		mac_record(3, 0, 0, path_p, mac_p, -1, EINVAL);
		return (-1);
	}
	if (path_p == NULL) {
		errno = EFAULT;
		mac_record(3, 0, 0, path_p, mac_p, -1, EFAULT);
		return (-1);
	}
	mac_touch(mac_p, (int)(unsigned char)path_p[0] ^ 0x55);
	ret = mac_mix(3, 0, 0, buf_off(path_p), buf_off(mac_p));
	mac_record(3, 0, 0, path_p, mac_p, ret, 0);
	return (ret);
}

extern "C" int
__mac_get_pid(pid_t pid, MAC *mac_p)
{
	int ret;

	if (mac_p == NULL) {
		errno = EINVAL;
		mac_record(4, 0, pid, NULL, mac_p, -1, EINVAL);
		return (-1);
	}
	mac_touch(mac_p, (int)(pid & 0xff));
	ret = mac_mix(4, 0, pid, -1, buf_off(mac_p));
	mac_record(4, 0, pid, NULL, mac_p, ret, 0);
	return (ret);
}

extern "C" int
__mac_get_proc(MAC *mac_p)
{
	int ret;

	if (mac_p == NULL) {
		errno = EINVAL;
		mac_record(5, 0, 0, NULL, mac_p, -1, EINVAL);
		return (-1);
	}
	mac_touch(mac_p, 0x42);
	ret = mac_mix(5, 0, 0, -1, buf_off(mac_p));
	mac_record(5, 0, 0, NULL, mac_p, ret, 0);
	return (ret);
}

struct SockCall {
	int		fd;
	int		level;
	int		optname;
	long long	optval_off;
	socklen_t	len_in;
	socklen_t	len_out;
	int		ret;
	int		err;
};

static SockCall g_sock_last;
static int g_sock_calls;

extern "C" int
getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
	int ret;
	MAC *label;
	socklen_t inlen, outlen;

	g_sock_calls++;
	g_sock_last.fd = fd;
	g_sock_last.level = level;
	g_sock_last.optname = optname;
	g_sock_last.optval_off = buf_off(optval);
	inlen = optlen == NULL ? 0 : *optlen;
	g_sock_last.len_in = inlen;

	if (optval == NULL || optlen == NULL) {
		errno = EFAULT;
		g_sock_last.len_out = inlen;
		g_sock_last.ret = -1;
		g_sock_last.err = EFAULT;
		return (-1);
	}
	if (level != SOL_SOCKET) {
		errno = ENOPROTOOPT;
		g_sock_last.len_out = inlen;
		g_sock_last.ret = -1;
		g_sock_last.err = ENOPROTOOPT;
		return (-1);
	}
	if (optname != SO_PEERLABEL) {
		errno = ENOPROTOOPT;
		g_sock_last.len_out = inlen;
		g_sock_last.ret = -1;
		g_sock_last.err = ENOPROTOOPT;
		return (-1);
	}
	if (inlen < sizeof(MAC)) {
		errno = EINVAL;
		g_sock_last.len_out = inlen;
		g_sock_last.ret = -1;
		g_sock_last.err = EINVAL;
		return (-1);
	}

	label = (MAC *)optval;
	label->m_buflen = (size_t)(0x300 + (unsigned)fd);
	label->m_string = (char *)(uintptr_t)(0x4000 + (unsigned)fd);
	outlen = sizeof(MAC);
	*optlen = outlen;
	g_sock_last.len_out = outlen;
	ret = mac_mix(6, fd, 0, -1, buf_off(optval));
	g_sock_last.ret = ret;
	g_sock_last.err = 0;
	return (ret);
}

static void
mac_track_reset(void)
{
	std::memset(&g_mac_last, 0, sizeof(g_mac_last));
	g_mac_calls = 0;
	errno = 0;
}

static void
sock_track_reset(void)
{
	std::memset(&g_sock_last, 0, sizeof(g_sock_last));
	g_sock_calls = 0;
	errno = 0;
}

static bool
mac_snap_eq(const MacCall &a, const MacCall &b)
{
	return (a.which == b.which &&
	    a.fd == b.fd &&
	    a.pid == b.pid &&
	    a.path_off == b.path_off &&
	    a.label_off == b.label_off &&
	    a.ret == b.ret &&
	    a.err == b.err);
}

static bool
sock_snap_eq(const SockCall &a, const SockCall &b)
{
	return (a.fd == b.fd &&
	    a.level == b.level &&
	    a.optname == b.optname &&
	    a.optval_off == b.optval_off &&
	    a.len_in == b.len_in &&
	    a.len_out == b.len_out &&
	    a.ret == b.ret &&
	    a.err == b.err);
}

/* ---------------------------------------------------- posix_memalign wrap */

static int g_pma_fail;

extern "C" int
__wrap_posix_memalign(void **memptr, size_t alignment, size_t size)
{
	if (g_pma_fail) {
		g_pma_fail = 0;
		return (ENOMEM);
	}
	return (__real_posix_memalign(memptr, alignment, size));
}

static void
pma_fail_next(void)
{
	g_pma_fail = 1;
}

/* -------------------------------------------------------- mac_get tests */

static void
setup_mac(Region &rp, Region &rr, uint32_t seed, int null_label)
{
	reg_init(rp);
	reg_init(rr);
	if (!null_label) {
		MAC *lp = (MAC *)(rp.b + OFF);
		MAC *lr = (MAC *)(rr.b + OFF);
		lp->m_buflen = (size_t)(seed & 0xffffu);
		lp->m_string = (char *)(rp.b + OFF + 32);
		lr->m_buflen = (size_t)(seed & 0xffffu);
		lr->m_string = (char *)(rr.b + OFF + 32);
	}
}

static void
test_mac_once(int which, int fd, pid_t pid, const char *path_p,
    const char *path_r, MAC *lab_p, MAC *lab_r, Region &rp, Region &rr)
{
	int ret_p, ret_r;
	MacCall snap_p, snap_r;
	SockCall ss_p, ss_r;
	Region snap_rp, snap_rr;
	int ep, er;

	snap_rp = rp;
	snap_rr = rr;

	set_regions(rp.b, rp.b);
	if (which == F_MAC_GET_FD) {
		mac_track_reset();
		ret_p = P::mac_get_fd(fd, lab_p);
		snap_p = g_mac_last;
		ep = errno;
	} else if (which == F_MAC_GET_FILE) {
		mac_track_reset();
		ret_p = P::mac_get_file(path_p, lab_p);
		snap_p = g_mac_last;
		ep = errno;
	} else if (which == F_MAC_GET_LINK) {
		mac_track_reset();
		ret_p = P::mac_get_link(path_p, lab_p);
		snap_p = g_mac_last;
		ep = errno;
	} else if (which == F_MAC_GET_PEER) {
		sock_track_reset();
		ret_p = P::mac_get_peer(fd, lab_p);
		ss_p = g_sock_last;
		ep = errno;
	} else if (which == F_MAC_GET_PID) {
		mac_track_reset();
		ret_p = P::mac_get_pid(pid, lab_p);
		snap_p = g_mac_last;
		ep = errno;
	} else {
		mac_track_reset();
		ret_p = P::mac_get_proc(lab_p);
		snap_p = g_mac_last;
		ep = errno;
	}

	set_regions(rr.b, rr.b);
	if (which == F_MAC_GET_FD) {
		mac_track_reset();
		ret_r = ref_mac_get_fd(fd, lab_r);
		snap_r = g_mac_last;
		er = errno;
	} else if (which == F_MAC_GET_FILE) {
		mac_track_reset();
		ret_r = ref_mac_get_file(path_r, lab_r);
		snap_r = g_mac_last;
		er = errno;
	} else if (which == F_MAC_GET_LINK) {
		mac_track_reset();
		ret_r = ref_mac_get_link(path_r, lab_r);
		snap_r = g_mac_last;
		er = errno;
	} else if (which == F_MAC_GET_PEER) {
		sock_track_reset();
		ret_r = ref_mac_get_peer(fd, lab_r);
		ss_r = g_sock_last;
		er = errno;
	} else if (which == F_MAC_GET_PID) {
		mac_track_reset();
		ret_r = ref_mac_get_pid(pid, lab_r);
		snap_r = g_mac_last;
		er = errno;
	} else {
		mac_track_reset();
		ret_r = ref_mac_get_proc(lab_r);
		snap_r = g_mac_last;
		er = errno;
	}

	case_bump(which);

	if (ret_p != ret_r)
		fail(which, "ret", "return mismatch");
	if (ep != er)
		fail(which, "errno", "errno mismatch");
	if (which == F_MAC_GET_PEER) {
		if (!sock_snap_eq(ss_p, ss_r))
			fail(which, "mock", "getsockopt mismatch");
	} else if (!mac_snap_eq(snap_p, snap_r)) {
		fail(which, "mock", "helper call mismatch");
	}
	if (!reg_eq(rp, rr))
		fail(which, "buffer", "region mismatch");
	if (!reg_unchanged(rp, snap_rp) || !reg_unchanged(rr, snap_rr))
		fail(which, "guard", "guard bytes corrupted");
}

static void
edge_mac(int which)
{
	Region rp, rr;
	int fds[] = { -1, 0, 1, 7, 255, 0x7fff, INT_MAX };
	pid_t pids[] = { 0, 1, 42, (pid_t)INT_MAX };
	size_t i;

	for (i = 0; i < sizeof(fds) / sizeof(fds[0]); i++) {
		setup_mac(rp, rr, (uint32_t)i, 0);
		test_mac_once(which, fds[i], 0, NULL, NULL,
		    (MAC *)(rp.b + OFF), (MAC *)(rr.b + OFF), rp, rr);
	}

	setup_mac(rp, rr, 0, 1);
	test_mac_once(which, 3, 0, NULL, NULL, NULL, NULL, rp, rr);

	if (which == F_MAC_GET_FILE || which == F_MAC_GET_LINK) {
		const char *paths[] = { "", "a", "\x80\xff", "/tmp/x" };
		for (i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
			setup_mac(rp, rr, (uint32_t)i * 0x1111u, 0);
			fill_path(rp.b + OFF + 128, 96, (uint32_t)i, 1);
			fill_path(rr.b + OFF + 128, 96, (uint32_t)i, 1);
			test_mac_once(which, 0, 0,
			    (const char *)(rp.b + OFF + 128),
			    (const char *)(rr.b + OFF + 128),
			    (MAC *)(rp.b + OFF), (MAC *)(rr.b + OFF), rp, rr);
		}
		setup_mac(rp, rr, 0, 0);
		test_mac_once(which, 0, 0, NULL, NULL,
		    (MAC *)(rp.b + OFF), (MAC *)(rr.b + OFF), rp, rr);
	}

	if (which == F_MAC_GET_PID) {
		for (i = 0; i < sizeof(pids) / sizeof(pids[0]); i++) {
			setup_mac(rp, rr, (uint32_t)i, 0);
			test_mac_once(which, 0, pids[i],
			    NULL, NULL,
			    (MAC *)(rp.b + OFF), (MAC *)(rr.b + OFF), rp, rr);
		}
	}
}

static void
sweep_mac(int which)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		Region rp, rr;
		uint32_t seed = rnd32();
		int null_label = rndbit();
		int fd = (int)(seed & 0xffffu);
		pid_t pid = (pid_t)(seed >> 16);

		setup_mac(rp, rr, seed, null_label);
		fill_path(rp.b + OFF + 128, 96, seed ^ 0xa5a5a5a5u, 1);
		fill_path(rr.b + OFF + 128, 96, seed ^ 0xa5a5a5a5u, 1);

		if (which == F_MAC_GET_FILE || which == F_MAC_GET_LINK) {
			int null_path = rndbit();
			test_mac_once(which, fd, pid,
			    null_path ? NULL : (const char *)(rp.b + OFF + 128),
			    null_path ? NULL : (const char *)(rr.b + OFF + 128),
			    null_label ? NULL : (MAC *)(rp.b + OFF),
			    null_label ? NULL : (MAC *)(rr.b + OFF),
			    rp, rr);
		} else if (which == F_MAC_GET_PID) {
			test_mac_once(which, fd, pid, NULL, NULL,
			    null_label ? NULL : (MAC *)(rp.b + OFF),
			    null_label ? NULL : (MAC *)(rr.b + OFF),
			    rp, rr);
		} else {
			test_mac_once(which, fd, pid, NULL, NULL,
			    null_label ? NULL : (MAC *)(rp.b + OFF),
			    null_label ? NULL : (MAC *)(rr.b + OFF),
			    rp, rr);
		}
	}
}

/* -------------------------------------------------------- acl_init tests */

static bool
acl_struct_eq(const ACL *a, const ACL *b)
{
	return (std::memcmp(a, b, sizeof(ACL)) == 0);
}

static void
test_acl_init_once(int count, int force_pma_fail)
{
	P::acl_t ap;
	P::acl_t ar;
	int ep, er;

	if (force_pma_fail)
		pma_fail_next();
	errno = 0;
	ap = P::acl_init(count);
	ep = errno;

	if (force_pma_fail)
		pma_fail_next();
	errno = 0;
	ar = ref_acl_init(count);
	er = errno;

	case_bump(F_ACL_INIT);

	if ((ap == NULL) != (ar == NULL))
		fail(F_ACL_INIT, "null", "NULL mismatch");
	if (ep != er)
		fail(F_ACL_INIT, "errno", "errno mismatch");

	if (ap == NULL && ar == NULL)
		return;

	if (((uintptr_t)ap % ACL_ALIGN) != 0)
		fail(F_ACL_INIT, "align", "port alignment wrong");
	if (((uintptr_t)ar % ACL_ALIGN) != 0)
		fail(F_ACL_INIT, "align", "ref alignment wrong");
	if (!acl_struct_eq(ap, ar))
		fail(F_ACL_INIT, "struct", "acl content mismatch");
	if (ap->ats_brand != P::ACL_BRAND_UNKNOWN)
		fail(F_ACL_INIT, "brand", "port brand wrong");
	if (ar->ats_brand != P::ACL_BRAND_UNKNOWN)
		fail(F_ACL_INIT, "brand", "ref brand wrong");
	if (ap->ats_acl.acl_maxcnt != P::ACL_MAX_ENTRIES)
		fail(F_ACL_INIT, "maxcnt", "port maxcnt wrong");
	if (ar->ats_acl.acl_maxcnt != P::ACL_MAX_ENTRIES)
		fail(F_ACL_INIT, "maxcnt", "ref maxcnt wrong");

	std::free(ap);
	std::free(ar);
}

static void
edge_acl_init(void)
{
	int counts[] = {
		INT_MIN, -1, 0, 1, 2,
		(int)P::ACL_MAX_ENTRIES - 1,
		(int)P::ACL_MAX_ENTRIES,
		(int)P::ACL_MAX_ENTRIES + 1,
		INT_MAX
	};
	size_t i;

	for (i = 0; i < sizeof(counts) / sizeof(counts[0]); i++)
		test_acl_init_once(counts[i], 0);

	test_acl_init_once(0, 1);
	test_acl_init_once(10, 1);
	test_acl_init_once((int)P::ACL_MAX_ENTRIES, 1);
}

static void
sweep_acl_init(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		uint32_t seed = rnd32();
		int count;

		if ((seed & 0xfu) == 0)
			count = (int)P::ACL_MAX_ENTRIES + 1 + (int)(seed & 0xffu);
		else if ((seed & 0xfu) == 1)
			count = -(int)(seed & 0xffu) - 1;
		else if ((seed & 0xfu) == 2)
			count = (int)P::ACL_MAX_ENTRIES;
		else if ((seed & 0xfu) == 3)
			count = (int)P::ACL_MAX_ENTRIES - 1;
		else
			count = (int)(seed % (P::ACL_MAX_ENTRIES + 1));

		test_acl_init_once(count, (seed & 0x1000u) != 0);
	}
}

/* --------------------------------------------------------- acl_dup tests */

static void
fill_acl(ACL *a, uint32_t seed)
{
	size_t i;
	unsigned char *p = (unsigned char *)a;

	std::memset(a, 0, sizeof(ACL));
	a->ats_cur_entry = (int)(seed & 0xffu);
	a->ats_brand = (int)((seed >> 8) & 3u);
	a->ats_acl.acl_cnt = seed & 0x7fu;
	a->ats_acl.acl_maxcnt = P::ACL_MAX_ENTRIES;
	for (i = 0; i < sizeof(ACL); i++)
		p[i] = (unsigned char)((seed + (uint32_t)i * 31u) & 0xffu);
	a->ats_acl.acl_maxcnt = P::ACL_MAX_ENTRIES;
}

static void
test_acl_dup_once(uint32_t seed)
{
	P::acl_t src_p, src_r, dup_p, dup_r;
	ACL snap_sp, snap_sr;
	int ep, er;

	src_p = P::acl_init(10);
	src_r = ref_acl_init(10);
	if (src_p == NULL || src_r == NULL) {
		std::free(src_p);
		std::free(src_r);
		return;
	}

	fill_acl(src_p, seed);
	fill_acl(src_r, seed);
	snap_sp = *src_p;
	snap_sr = *src_r;

	errno = 0;
	dup_p = P::acl_dup(src_p);
	ep = errno;

	errno = 0;
	dup_r = ref_acl_dup(src_r);
	er = errno;

	case_bump(F_ACL_DUP);

	if ((dup_p == NULL) != (dup_r == NULL))
		fail(F_ACL_DUP, "null", "NULL mismatch");
	if (ep != er)
		fail(F_ACL_DUP, "errno", "errno mismatch");

	if (dup_p != NULL && dup_r != NULL) {
		ACL expect = snap_sp;
		expect.ats_cur_entry = 0;
		if (!acl_struct_eq(dup_p, &expect))
			fail(F_ACL_DUP, "dup_p", "port dup content mismatch");
		expect = snap_sr;
		expect.ats_cur_entry = 0;
		if (!acl_struct_eq(dup_r, &expect))
			fail(F_ACL_DUP, "dup_r", "ref dup content mismatch");
		if (!acl_struct_eq(dup_p, dup_r))
			fail(F_ACL_DUP, "struct", "dup cross mismatch");
		std::free(dup_p);
		std::free(dup_r);
	}

	if (src_p->ats_cur_entry != 0)
		fail(F_ACL_DUP, "src_p", "port source cur_entry not cleared");
	if (src_r->ats_cur_entry != 0)
		fail(F_ACL_DUP, "src_r", "ref source cur_entry not cleared");

	snap_sp.ats_cur_entry = 0;
	snap_sr.ats_cur_entry = 0;
	if (!acl_struct_eq(src_p, &snap_sp))
		fail(F_ACL_DUP, "src_p", "port source mutated unexpectedly");
	if (!acl_struct_eq(src_r, &snap_sr))
		fail(F_ACL_DUP, "src_r", "ref source mutated unexpectedly");

	std::free(src_p);
	std::free(src_r);
}

static void
edge_acl_dup(void)
{
	test_acl_dup_once(0);
	test_acl_dup_once(1);
	test_acl_dup_once(0xffffffffu);
	test_acl_dup_once(0x80808080u);
	test_acl_dup_once(0x7f7f7f7fu);

	pma_fail_next();
	test_acl_dup_once(42);
}

static void
sweep_acl_dup(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++)
		test_acl_dup_once(rnd32());
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	int f;

	edge_mac(F_MAC_GET_FD);
	sweep_mac(F_MAC_GET_FD);

	edge_mac(F_MAC_GET_FILE);
	sweep_mac(F_MAC_GET_FILE);

	edge_mac(F_MAC_GET_LINK);
	sweep_mac(F_MAC_GET_LINK);

	edge_mac(F_MAC_GET_PEER);
	sweep_mac(F_MAC_GET_PEER);

	edge_mac(F_MAC_GET_PID);
	sweep_mac(F_MAC_GET_PID);

	edge_mac(F_MAC_GET_PROC);
	sweep_mac(F_MAC_GET_PROC);

	edge_acl_init();
	sweep_acl_init();

	edge_acl_dup();
	sweep_acl_dup();

	std::printf("\n%-26s %12s %12s\n", "function", "cases", "failures");
	for (f = 0; f < NFUNC; f++)
		std::printf("%-26s %12lld %12lld\n", fname[f], ncases[f],
		    nfails[f]);

	long long total_fail = 0;
	for (f = 0; f < NFUNC; f++)
		total_fail += nfails[f];

	return (total_fail == 0 ? 0 : 1);
}
