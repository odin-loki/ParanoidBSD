/*
 * harness.cpp -- differential test for PBSD batch b0110.
 *
 * FreeBSD-private libc internals are exercised through recording test doubles
 * defined here and linked into both the port and the oracle.  Every observable
 * (return value, errno, buffer contents including guard bytes, and mock
 * records) is compared side by side.
 */

import pbsd.lib.libc.posix1e.b0110;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

namespace port = pbsd::lib_libc_posix1e::b0110;

#define	GUARD			0x7f
#define	SWEEP_ITERS		200000L
#define	MAX_PRINT		10
#define	_ACL_T_ALIGNMENT_BITS	13
#define	ACL_MAX_ENTRIES		254
#define	ACL_BRAND_UNKNOWN	0
#define	ACL_BRAND_POSIX		1
#define	ACL_BRAND_NFS4		2

#define	ACL_TYPE_ACCESS_OLD	0x00000000
#define	ACL_TYPE_DEFAULT_OLD	0x00000001
#define	ACL_TYPE_ACCESS		0x00000002
#define	ACL_TYPE_DEFAULT	0x00000003
#define	ACL_TYPE_NFS4		0x00000004

#define	EXTATTR_NAMESPACE_USER		0x00000001
#define	EXTATTR_NAMESPACE_USER_STRING	"user"
#define	EXTATTR_NAMESPACE_SYSTEM	0x00000002
#define	EXTATTR_NAMESPACE_SYSTEM_STRING	"system"

struct acl_t_struct {
	struct {
		unsigned int		acl_maxcnt;
		unsigned int		acl_cnt;
		int			acl_spare[4];
		acl_entry		entries[ACL_MAX_ENTRIES];
	}			ats_acl;
	int			ats_cur_entry;
	int			ats_brand;
};
typedef struct acl_t_struct	*acl_t_full;

extern "C" {
int ref_mac_set_fd(int, mac *);
int ref_mac_set_file(const char *, mac *);
int ref_mac_set_link(const char *, mac *);
int ref_mac_set_proc(mac *);
int ref_acl_delete_def_file(const char *);
int ref_acl_delete_def_link_np(const char *);
int ref_acl_delete_file_np(const char *, acl_type_t);
int ref_acl_delete_link_np(const char *, acl_type_t);
int ref_acl_delete_fd_np(int, acl_type_t);
int ref_extattr_namespace_to_string(int, char **);
int ref_extattr_string_to_namespace(const char *, int *);
int ref_acl_copy_entry(acl_entry_t, acl_entry_t);
ssize_t ref_acl_copy_ext(void *, acl_t, ssize_t);
acl_t ref_acl_copy_int(const void *);
void mock_reset(void);
struct mock_snap {
	int		set;
	int		kind;
	int		fd;
	const char	*path;
	mac		*label;
	acl_type_t	type;
	int		ret;
};
struct mock_snap mock_capture(void);
}

/* ------------------------------------------------------------------ stats */

struct Stat {
	const char *name;
	long		cases;
	long		fails;
	long		printed;
};

static Stat stats[] = {
	{ "mac_set_fd", 0, 0, 0 },
	{ "mac_set_file", 0, 0, 0 },
	{ "mac_set_link", 0, 0, 0 },
	{ "mac_set_proc", 0, 0, 0 },
	{ "acl_delete_def_file", 0, 0, 0 },
	{ "acl_delete_def_link_np", 0, 0, 0 },
	{ "acl_delete_file_np", 0, 0, 0 },
	{ "acl_delete_link_np", 0, 0, 0 },
	{ "acl_delete_fd_np", 0, 0, 0 },
	{ "extattr_namespace_to_string", 0, 0, 0 },
	{ "extattr_string_to_namespace", 0, 0, 0 },
	{ "acl_copy_entry", 0, 0, 0 },
	{ "acl_copy_ext", 0, 0, 0 },
	{ "acl_copy_int", 0, 0, 0 },
};

#define	S_MAC_FD	0
#define	S_MAC_FILE	1
#define	S_MAC_LINK	2
#define	S_MAC_PROC	3
#define	S_ADF		4
#define	S_ADL		5
#define	S_AFN		6
#define	S_ALN		7
#define	S_AFD		8
#define	S_ENS		9
#define	S_ESN		10
#define	S_ACE		11
#define	S_ACX		12
#define	S_ACI		13

static std::uint64_t rng_state = 0x00b0110feedULL;

static inline std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::uint32_t
rnd32(void)
{
	return (std::uint32_t)(rnd64() >> 32);
}

static inline int
rnd_i(void)
{
	return (int)rnd32();
}

static void
fail_stat(int idx, const char *tag, const char *detail)
{
	Stat &st = stats[idx];

	st.fails++;
	if (st.printed < MAX_PRINT) {
		st.printed++;
		std::printf("  FAIL %-28s %-20s %s\n", st.name, tag, detail);
	}
}

/* -------------------------------------------------------- shared internals */

static bool
mock_eq(const mock_snap &a, const mock_snap &b)
{

	return (a.set == b.set && a.kind == b.kind && a.fd == b.fd &&
	    a.path == b.path && a.label == b.label && a.type == b.type &&
	    a.ret == b.ret);
}

/* ---------------------------------------------------------- ACL allocation */

static acl_t_full
alloc_acl(void)
{
	acl_t_full acl;
	int error;

	error = posix_memalign((void **)&acl, 1 << _ACL_T_ALIGNMENT_BITS,
	    sizeof(struct acl_t_struct));
	if (error != 0)
		return (nullptr);
	std::memset(acl, 0, sizeof(*acl));
	acl->ats_brand = ACL_BRAND_UNKNOWN;
	acl->ats_acl.acl_maxcnt = ACL_MAX_ENTRIES;
	return (acl);
}

static void
free_acl(acl_t_full acl)
{
	std::free(acl);
}

/* -------------------------------------------------------------- MAC tests */

static void
fill_mac(mac *m, size_t buflen, char *buf, size_t bufsz,
    const char *s)
{
	std::memset(buf, GUARD, bufsz);
	if (s != nullptr)
		std::strncpy(buf, s, bufsz - 1);
	buf[bufsz - 1] = '\0';
	m->m_buflen = buflen;
	m->m_string = buf;
}

static bool
mac_run(int idx, int (*port_fn)(int, mac *),
    int (*ref_fn)(int, mac *), int fd, mac *label,
    const char *tag)
{
	int rp, rr, ep, er;
	mock_snap mp, mr;

	mock_reset();
	errno = 0;
	rp = port_fn(fd, label);
	ep = errno;
	mp = mock_capture();

	mock_reset();
	errno = 0;
	rr = ref_fn(fd, label);
	er = errno;
	mr = mock_capture();

	stats[idx].cases++;
	if (rp != rr || ep != er || !mock_eq(mp, mr)) {
		fail_stat(idx, tag, "rv/errno/mock");
		return (false);
	}
	return (true);
}

static bool
mac_run_path(int idx, int (*port_fn)(const char *, mac *),
    int (*ref_fn)(const char *, mac *), const char *path,
    mac *label, const char *tag)
{
	int rp, rr, ep, er;
	mock_snap mp, mr;

	mock_reset();
	errno = 0;
	rp = port_fn(path, label);
	ep = errno;
	mp = mock_capture();

	mock_reset();
	errno = 0;
	rr = ref_fn(path, label);
	er = errno;
	mr = mock_capture();

	stats[idx].cases++;
	if (rp != rr || ep != er || !mock_eq(mp, mr)) {
		fail_stat(idx, tag, "rv/errno/mock");
		return (false);
	}
	return (true);
}

static bool
mac_run_proc(int idx, int (*port_fn)(mac *),
    int (*ref_fn)(mac *), mac *label, const char *tag)
{
	int rp, rr, ep, er;
	mock_snap mp, mr;

	mock_reset();
	errno = 0;
	rp = port_fn(label);
	ep = errno;
	mp = mock_capture();

	mock_reset();
	errno = 0;
	rr = ref_fn(label);
	er = errno;
	mr = mock_capture();

	stats[idx].cases++;
	if (rp != rr || ep != er || !mock_eq(mp, mr)) {
		fail_stat(idx, tag, "rv/errno/mock");
		return (false);
	}
	return (true);
}

static void
test_mac_edge(void)
{
	char mbuf[64];
	mac m;

	fill_mac(&m, 0, mbuf, sizeof(mbuf), "");
	mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, 0, &m, "fd0");
	mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, -1, &m, "fd-1");
	mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, 255, &m, "fd255");
	mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, 0x7fffffff, &m,
	    "fdmax");
	mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, 3, nullptr,
	    "fd-null-label");

	fill_mac(&m, 4, mbuf, sizeof(mbuf), "test");
	mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, 7, &m, "fd7");

	mac_run_path(S_MAC_FILE, port::mac_set_file, ref_mac_set_file, "",
	    &m, "file-empty");
	mac_run_path(S_MAC_FILE, port::mac_set_file, ref_mac_set_file,
	    "/tmp/\x80\xff", &m, "file-hibit");
	mac_run_path(S_MAC_FILE, port::mac_set_file, ref_mac_set_file,
	    nullptr, &m, "file-null");
	mac_run_path(S_MAC_FILE, port::mac_set_file, ref_mac_set_file,
	    "a", nullptr, "file-null-mac");

	mac_run_path(S_MAC_LINK, port::mac_set_link, ref_mac_set_link,
	    "/link", &m, "link");
	mac_run_path(S_MAC_LINK, port::mac_set_link, ref_mac_set_link,
	    "\xff", nullptr, "link-null-mac");

	mac_run_proc(S_MAC_PROC, port::mac_set_proc, ref_mac_set_proc, &m,
	    "proc");
	mac_run_proc(S_MAC_PROC, port::mac_set_proc, ref_mac_set_proc,
	    nullptr, "proc-null");
}

static void
test_mac_random(void)
{
	char mbuf[96];
	mac m;

	for (long it = 0; it < SWEEP_ITERS; it++) {
		size_t blen = (size_t)(rnd32() % 80);
		size_t slen = (size_t)(rnd32() % 48);
		char sbuf[49];

		for (size_t i = 0; i < slen; i++)
			sbuf[i] = (char)(rnd32() & 0xff);
		sbuf[slen] = '\0';
		fill_mac(&m, blen, mbuf, sizeof(mbuf), sbuf);

		int fd = rnd_i();
		mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd, fd, &m,
		    "rand");
		if ((rnd32() & 3) == 0)
			mac_run(S_MAC_FD, port::mac_set_fd, ref_mac_set_fd,
			    fd, nullptr, "rand-null");

		char pbuf[64];
		for (size_t i = 0; i < sizeof(pbuf) - 1; i++)
			pbuf[i] = (char)(rnd32() & 0xff);
		pbuf[sizeof(pbuf) - 1] = '\0';
		mac_run_path(S_MAC_FILE, port::mac_set_file, ref_mac_set_file,
		    pbuf, &m, "rand");
		mac_run_path(S_MAC_LINK, port::mac_set_link, ref_mac_set_link,
		    pbuf, &m, "rand");
		mac_run_proc(S_MAC_PROC, port::mac_set_proc, ref_mac_set_proc,
		    &m, "rand");
	}
}

/* --------------------------------------------------------- ACL delete tests */

static bool
acl_del_run(int idx, int (*port_fn)(const char *),
    int (*ref_fn)(const char *), const char *path, const char *tag)
{
	int rp, rr, ep, er;
	mock_snap mp, mr;

	mock_reset();
	errno = 0;
	rp = port_fn(path);
	ep = errno;
	mp = mock_capture();

	mock_reset();
	errno = 0;
	rr = ref_fn(path);
	er = errno;
	mr = mock_capture();

	stats[idx].cases++;
	if (rp != rr || ep != er || !mock_eq(mp, mr)) {
		fail_stat(idx, tag, "rv/errno/mock");
		return (false);
	}
	return (true);
}

static bool
acl_del_type_run(int idx,
    int (*port_fn)(const char *, acl_type_t),
    int (*ref_fn)(const char *, acl_type_t), const char *path,
    acl_type_t type, const char *tag)
{
	int rp, rr, ep, er;
	mock_snap mp, mr;

	mock_reset();
	errno = 0;
	rp = port_fn(path, type);
	ep = errno;
	mp = mock_capture();

	mock_reset();
	errno = 0;
	rr = ref_fn(path, type);
	er = errno;
	mr = mock_capture();

	stats[idx].cases++;
	if (rp != rr || ep != er || !mock_eq(mp, mr)) {
		fail_stat(idx, tag, "rv/errno/mock");
		return (false);
	}
	return (true);
}

static bool
acl_del_fd_run(int idx, int (*port_fn)(int, acl_type_t),
    int (*ref_fn)(int, acl_type_t), int fd, acl_type_t type,
    const char *tag)
{
	int rp, rr, ep, er;
	mock_snap mp, mr;

	mock_reset();
	errno = 0;
	rp = port_fn(fd, type);
	ep = errno;
	mp = mock_capture();

	mock_reset();
	errno = 0;
	rr = ref_fn(fd, type);
	er = errno;
	mr = mock_capture();

	stats[idx].cases++;
	if (rp != rr || ep != er || !mock_eq(mp, mr)) {
		fail_stat(idx, tag, "rv/errno/mock");
		return (false);
	}
	return (true);
}

static const acl_type_t acl_types[] = {
	ACL_TYPE_ACCESS_OLD,
	ACL_TYPE_DEFAULT_OLD,
	ACL_TYPE_ACCESS,
	ACL_TYPE_DEFAULT,
	ACL_TYPE_NFS4,
	-1,
	0,
	1,
	2,
	4,
	5,
	0x7fffffff,
	(int)0x80000000,
};

static void
test_acl_delete_edge(void)
{
	acl_del_run(S_ADF, port::acl_delete_def_file, ref_acl_delete_def_file,
	    "", "empty");
	acl_del_run(S_ADF, port::acl_delete_def_file, ref_acl_delete_def_file,
	    "/file", "path");
	acl_del_run(S_ADF, port::acl_delete_def_file, ref_acl_delete_def_file,
	    "\x80\xff", "hibit");

	acl_del_run(S_ADL, port::acl_delete_def_link_np,
	    ref_acl_delete_def_link_np, "/link", "link");

	for (acl_type_t t : acl_types) {
		acl_del_type_run(S_AFN, port::acl_delete_file_np,
		    ref_acl_delete_file_np, "/f", t, "type");
		acl_del_type_run(S_ALN, port::acl_delete_link_np,
		    ref_acl_delete_link_np, "/l", t, "type");
		acl_del_fd_run(S_AFD, port::acl_delete_fd_np,
		    ref_acl_delete_fd_np, 0, t, "type");
		acl_del_fd_run(S_AFD, port::acl_delete_fd_np,
		    ref_acl_delete_fd_np, -1, t, "type-neg");
	}
}

static void
test_acl_delete_random(void)
{
	char pbuf[80];

	for (long it = 0; it < SWEEP_ITERS; it++) {
		size_t n = (size_t)(rnd32() % (sizeof(pbuf) - 1));

		for (size_t i = 0; i < n; i++)
			pbuf[i] = (char)(rnd32() & 0xff);
		pbuf[n] = '\0';

		acl_del_run(S_ADF, port::acl_delete_def_file,
		    ref_acl_delete_def_file, pbuf, "rand");
		acl_del_run(S_ADL, port::acl_delete_def_link_np,
		    ref_acl_delete_def_link_np, pbuf, "rand");

		acl_type_t t = (acl_type_t)rnd_i();
		acl_del_type_run(S_AFN, port::acl_delete_file_np,
		    ref_acl_delete_file_np, pbuf, t, "rand");
		acl_del_type_run(S_ALN, port::acl_delete_link_np,
		    ref_acl_delete_link_np, pbuf, t, "rand");
		acl_del_fd_run(S_AFD, port::acl_delete_fd_np,
		    ref_acl_delete_fd_np, rnd_i(), t, "rand");
	}
}

/* ----------------------------------------------------------- extattr tests */

static bool
ens_run(int ns, bool out_ptr, const char *tag)
{
	char *out_p = nullptr;
	char *out_r = nullptr;
	unsigned char slot_p[sizeof(char *)];
	unsigned char slot_r[sizeof(char *)];
	int rp, rr, ep, er;

	std::memset(slot_p, GUARD, sizeof(slot_p));
	std::memset(slot_r, GUARD, sizeof(slot_r));

	mock_reset();
	errno = 0;
	if (out_ptr)
		rp = port::extattr_namespace_to_string(ns, (char **)&slot_p);
	else
		rp = port::extattr_namespace_to_string(ns, nullptr);
	ep = errno;
	if (out_ptr && rp == 0)
		out_p = *(char **)slot_p;

	mock_reset();
	errno = 0;
	if (out_ptr)
		rr = ref_extattr_namespace_to_string(ns, (char **)&slot_r);
	else
		rr = ref_extattr_namespace_to_string(ns, nullptr);
	er = errno;
	if (out_ptr && rr == 0)
		out_r = *(char **)slot_r;

	stats[S_ENS].cases++;
	bool bad = (rp != rr || ep != er ||
	    std::memcmp(slot_p, slot_r, sizeof(slot_p)) != 0);
	if (!bad && rp == 0 && out_ptr) {
		if ((out_p == nullptr) != (out_r == nullptr))
			bad = true;
		else if (out_p != nullptr &&
		    std::strcmp(out_p, out_r) != 0)
			bad = true;
	}
	if (bad)
		fail_stat(S_ENS, tag, "rv/errno/out");
	std::free(out_p);
	std::free(out_r);
	return (!bad);
}

static bool
esn_run(const char *s, bool out_ptr, const char *tag)
{
	unsigned char outp[sizeof(int)];
	unsigned char outr[sizeof(int)];
	int rp, rr, ep, er;

	std::memset(outp, GUARD, sizeof(outp));
	std::memset(outr, GUARD, sizeof(outr));

	mock_reset();
	errno = 0;
	rp = port::extattr_string_to_namespace(s,
	    out_ptr ? (int *)outp : nullptr);
	ep = errno;

	mock_reset();
	errno = 0;
	rr = ref_extattr_string_to_namespace(s,
	    out_ptr ? (int *)outr : nullptr);
	er = errno;

	stats[S_ESN].cases++;
	if (rp != rr || ep != er ||
	    std::memcmp(outp, outr, sizeof(outp)) != 0) {
		fail_stat(S_ESN, tag, "rv/errno/out");
		return (false);
	}
	return (true);
}

static void
test_extattr_edge(void)
{
	ens_run(EXTATTR_NAMESPACE_USER, true, "user");
	ens_run(EXTATTR_NAMESPACE_USER, false, "user-no-out");
	ens_run(EXTATTR_NAMESPACE_SYSTEM, true, "system");
	ens_run(0, true, "zero");
	ens_run(-1, true, "neg");
	ens_run(2, true, "two");
	ens_run(3, true, "three");
	ens_run(0x80, true, "0x80");
	ens_run(0xff, true, "0xff");
	ens_run(0x7fffffff, true, "max");

	esn_run(EXTATTR_NAMESPACE_USER_STRING, true, "user");
	esn_run(EXTATTR_NAMESPACE_SYSTEM_STRING, true, "system");
	esn_run("", true, "empty");
	esn_run("User", true, "case");
	esn_run("user\x80", true, "suffix");
	esn_run("\x80user", true, "prefix");
	esn_run("system\xff", true, "sys-suffix");
	esn_run("userr", true, "userr");
	esn_run("use", true, "use");
	esn_run("syste", true, "syste");
	esn_run(EXTATTR_NAMESPACE_USER_STRING, false, "user-no-out");
}

static void
test_extattr_random(void)
{
	char sbuf[64];

	for (long it = 0; it < SWEEP_ITERS; it++) {
		int ns = rnd_i();
		ens_run(ns, (rnd32() & 1) != 0, "rand");

		size_t n = (size_t)(rnd32() % (sizeof(sbuf) - 1));
		for (size_t i = 0; i < n; i++)
			sbuf[i] = (char)(rnd32() & 0xff);
		sbuf[n] = '\0';
		esn_run(sbuf, (rnd32() & 1) != 0, "rand");
	}
}

/* -------------------------------------------------------- acl_copy_entry */

static void
acl_clone(acl_t_full dst, const acl_t_full src)
{

	std::memcpy(dst, src, sizeof(*dst));
}

static bool
ace_run_ptrs(acl_entry_t dest_p, acl_entry_t src_p, acl_entry_t dest_r,
    acl_entry_t src_r, const char *tag)
{
	int rp, rr, ep, er;
	unsigned char snap_dp[sizeof(struct acl_t_struct)];
	unsigned char snap_dr[sizeof(struct acl_t_struct)];
	unsigned char snap_sp[sizeof(struct acl_t_struct)];
	unsigned char snap_sr[sizeof(struct acl_t_struct)];
	acl_t_full adp, adr, asp, asr;

	adp = (dest_p != nullptr) ?
	    (acl_t_full)(((long)dest_p >> _ACL_T_ALIGNMENT_BITS)
	    << _ACL_T_ALIGNMENT_BITS) : nullptr;
	adr = (dest_r != nullptr) ?
	    (acl_t_full)(((long)dest_r >> _ACL_T_ALIGNMENT_BITS)
	    << _ACL_T_ALIGNMENT_BITS) : nullptr;
	asp = (src_p != nullptr) ?
	    (acl_t_full)(((long)src_p >> _ACL_T_ALIGNMENT_BITS)
	    << _ACL_T_ALIGNMENT_BITS) : nullptr;
	asr = (src_r != nullptr) ?
	    (acl_t_full)(((long)src_r >> _ACL_T_ALIGNMENT_BITS)
	    << _ACL_T_ALIGNMENT_BITS) : nullptr;

	if (adp != nullptr)
		std::memcpy(snap_dp, adp, sizeof(snap_dp));
	if (adr != nullptr)
		std::memcpy(snap_dr, adr, sizeof(snap_dr));
	if (asp != nullptr)
		std::memcpy(snap_sp, asp, sizeof(snap_sp));
	if (asr != nullptr)
		std::memcpy(snap_sr, asr, sizeof(snap_sr));

	mock_reset();
	errno = 0;
	rp = port::acl_copy_entry(dest_p, src_p);
	ep = errno;

	mock_reset();
	errno = 0;
	rr = ref_acl_copy_entry(dest_r, src_r);
	er = errno;

	stats[S_ACE].cases++;
	bool bad = (rp != rr || ep != er);
	if (!bad && adp != nullptr && adr != nullptr)
		bad = (std::memcmp(adp, adr, sizeof(*adp)) != 0);
	if (!bad && asp != nullptr && asr != nullptr)
		bad = (std::memcmp(asp, asr, sizeof(*asp)) != 0);
	if (bad)
		fail_stat(S_ACE, tag, "rv/errno/acl");
	return (!bad);
}

static bool
ace_run_pair(int dest_idx, int src_idx, const acl_t_full dest_tpl,
    const acl_t_full src_tpl, const char *tag)
{
	acl_t_full da = alloc_acl();
	acl_t_full db = alloc_acl();
	acl_t_full sa = alloc_acl();
	acl_t_full sb = alloc_acl();
	bool ok;

	acl_clone(da, dest_tpl);
	acl_clone(db, dest_tpl);
	acl_clone(sa, src_tpl);
	acl_clone(sb, src_tpl);

	ok = ace_run_ptrs(&da->ats_acl.entries[dest_idx],
	    &sa->ats_acl.entries[src_idx], &db->ats_acl.entries[dest_idx],
	    &sb->ats_acl.entries[src_idx], tag);

	free_acl(da);
	free_acl(db);
	free_acl(sa);
	free_acl(sb);
	return (ok);
}

static void
test_acl_copy_entry_edge(void)
{
	acl_t_full dest = alloc_acl();
	acl_t_full src = alloc_acl();

	ace_run_ptrs(nullptr, &src->ats_acl.entries[0], nullptr,
	    &src->ats_acl.entries[0], "null-dest");
	ace_run_ptrs(&dest->ats_acl.entries[0], nullptr,
	    &dest->ats_acl.entries[0], nullptr, "null-src");
	{
		acl_t_full same_a = alloc_acl();
		acl_t_full same_b = alloc_acl();

		acl_clone(same_b, same_a);
		ace_run_ptrs(&same_a->ats_acl.entries[0],
		    &same_a->ats_acl.entries[0],
		    &same_b->ats_acl.entries[0],
		    &same_b->ats_acl.entries[0], "same");
		free_acl(same_a);
		free_acl(same_b);
	}

	dest->ats_brand = ACL_BRAND_POSIX;
	src->ats_brand = ACL_BRAND_NFS4;
	src->ats_acl.entries[0].ae_tag = 1;
	ace_run_pair(0, 0, dest, src, "brand-mismatch");

	dest->ats_brand = ACL_BRAND_UNKNOWN;
	src->ats_brand = ACL_BRAND_POSIX;
	src->ats_acl.entries[1].ae_tag = 10;
	src->ats_acl.entries[1].ae_id = 20;
	src->ats_acl.entries[1].ae_perm = 0x80ff;
	src->ats_acl.entries[1].ae_entry_type = 0xff;
	src->ats_acl.entries[1].ae_flags = 0x7f00;
	ace_run_pair(0, 1, dest, src, "ok-unknown-dest");

	dest->ats_brand = ACL_BRAND_POSIX;
	src->ats_brand = ACL_BRAND_POSIX;
	src->ats_acl.entries[2].ae_tag = 0xffffffff;
	src->ats_acl.entries[2].ae_id = 0;
	src->ats_acl.entries[2].ae_perm = 0;
	src->ats_acl.entries[2].ae_entry_type = 0;
	src->ats_acl.entries[2].ae_flags = 0;
	ace_run_pair(1, 2, dest, src, "ok-posix");

	dest->ats_brand = ACL_BRAND_NFS4;
	src->ats_brand = ACL_BRAND_NFS4;
	ace_run_pair(2, 0, dest, src, "ok-nfs4");

	free_acl(dest);
	free_acl(src);
}

static void
test_acl_copy_entry_random(void)
{

	for (long it = 0; it < SWEEP_ITERS; it++) {
		acl_t_full dest = alloc_acl();
		acl_t_full src = alloc_acl();
		int di = (int)(rnd32() % 8);
		int si = (int)(rnd32() % 8);

		dest->ats_brand = (int)(rnd32() % 3);
		src->ats_brand = (int)(rnd32() % 3);
		src->ats_acl.entries[si].ae_tag = rnd32();
		src->ats_acl.entries[si].ae_id = rnd32();
		src->ats_acl.entries[si].ae_perm = rnd32();
		src->ats_acl.entries[si].ae_entry_type =
		    (acl_entry_type_t)(rnd32() & 0xffff);
		src->ats_acl.entries[si].ae_flags =
		    (acl_flag_t)(rnd32() & 0xffff);
		ace_run_pair(di, si, dest, src, "rand");
		free_acl(dest);
		free_acl(src);
	}
}

/* --------------------------------------------------------- acl_copy stubs */

static bool
acx_run(void *buf, ssize_t size, const char *tag)
{
	unsigned char ba[64], bb[64];
	int rp, rr, ep, er;

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	if (buf != nullptr && size > 0) {
		size_t n = (size_t)size;
		if (n > sizeof(ba))
			n = sizeof(ba);
		for (size_t i = 0; i < n; i++)
			ba[i] = bb[i] = (unsigned char)(i ^ 0xa5);
	}

	mock_reset();
	errno = 0;
	rp = (int)port::acl_copy_ext(buf != nullptr ? ba : nullptr,
	    reinterpret_cast<acl_t>((void *)(uintptr_t)0x1234), size);
	ep = errno;

	mock_reset();
	errno = 0;
	rr = (int)ref_acl_copy_ext(buf != nullptr ? bb : nullptr,
	    (acl_t)(uintptr_t)0x1234, size);
	er = errno;

	stats[S_ACX].cases++;
	if (rp != rr || ep != er ||
	    std::memcmp(ba, bb, sizeof(ba)) != 0) {
		fail_stat(S_ACX, tag, "rv/errno/buf");
		return (false);
	}
	return (true);
}

static bool
aci_run(const void *buf, const char *tag)
{
	unsigned char ba[32], bb[32];
	acl_t rp, rr;
	int ep, er;

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	if (buf != nullptr)
		std::memcpy(ba, buf, sizeof(ba));

	mock_reset();
	errno = 0;
	rp = port::acl_copy_int(buf != nullptr ? ba : nullptr);
	ep = errno;

	mock_reset();
	errno = 0;
	rr = ref_acl_copy_int(buf != nullptr ? bb : nullptr);
	er = errno;

	stats[S_ACI].cases++;
	if ((rp != rr) || ep != er ||
	    std::memcmp(ba, bb, sizeof(ba)) != 0) {
		fail_stat(S_ACI, tag, "rv/errno/buf");
		return (false);
	}
	return (true);
}

static void
test_acl_copy_stubs_edge(void)
{
	acx_run(nullptr, 0, "null-zero");
	acx_run(nullptr, -1, "null-neg");
	acx_run((void *)1, 16, "buf16");
	acx_run((void *)1, 1024, "buf-big");
	acx_run((void *)1, -100, "buf-neg-size");

	aci_run(nullptr, "null");
	aci_run((void *)1, "buf");
}

static void
test_acl_copy_stubs_random(void)
{
	unsigned char buf[48];

	for (long it = 0; it < SWEEP_ITERS; it++) {
		for (size_t i = 0; i < sizeof(buf); i++)
			buf[i] = (unsigned char)(rnd32() & 0xff);
		acx_run(buf, (ssize_t)rnd_i(), "rand");
		aci_run(buf, "rand");
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	test_mac_edge();
	test_mac_random();
	test_acl_delete_edge();
	test_acl_delete_random();
	test_extattr_edge();
	test_extattr_random();
	test_acl_copy_entry_edge();
	test_acl_copy_entry_random();
	test_acl_copy_stubs_edge();
	test_acl_copy_stubs_random();

	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-28s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-28s %10s %10s\n", "--------", "-----", "--------");
	for (Stat &st : stats) {
		std::printf("%-28s %10ld %10ld\n", st.name, st.cases,
		    st.fails);
		total_cases += st.cases;
		total_fails += st.fails;
	}
	std::printf("%-28s %10ld %10ld\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
