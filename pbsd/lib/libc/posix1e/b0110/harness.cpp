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

typedef std::uint32_t	acl_tag_t;
typedef std::uint32_t	acl_perm_t;
typedef std::uint16_t	acl_entry_type_t;
typedef std::uint16_t	acl_flag_t;
typedef int		acl_type_t;

struct mac {
	size_t		 m_buflen;
	char		*m_string;
};

struct acl_entry {
	acl_tag_t		ae_tag;
	std::uint32_t		ae_id;
	acl_perm_t		ae_perm;
	acl_entry_type_t	ae_entry_type;
	acl_flag_t		ae_flags;
};
typedef struct acl_entry	*acl_entry_t;

struct acl {
	unsigned int		acl_maxcnt;
	unsigned int		acl_cnt;
	int			acl_spare[4];
	struct acl_entry	acl_entry[ACL_MAX_ENTRIES];
};

struct acl_t_struct {
	struct acl		ats_acl;
	int			ats_cur_entry;
	int			ats_brand;
};
typedef struct acl_t_struct	*acl_t;

extern "C" {
int ref_mac_set_fd(int, struct mac *);
int ref_mac_set_file(const char *, struct mac *);
int ref_mac_set_link(const char *, struct mac *);
int ref_mac_set_proc(struct mac *);
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

static acl_t
entry2acl(acl_entry_t entry)
{
	return ((acl_t)(((long)entry >> _ACL_T_ALIGNMENT_BITS)
	    << _ACL_T_ALIGNMENT_BITS));
}

extern "C" int
_acl_type_unold(acl_type_t type)
{

	switch (type) {
	case ACL_TYPE_ACCESS_OLD:
		return (ACL_TYPE_ACCESS);
	case ACL_TYPE_DEFAULT_OLD:
		return (ACL_TYPE_DEFAULT);
	default:
		return (type);
	}
}

extern "C" int
_acl_brand(const acl_t acl)
{

	return (acl->ats_brand);
}

extern "C" int
_entry_brand(const acl_entry_t entry)
{

	return (_acl_brand(entry2acl(entry)));
}

extern "C" int
_acl_brand_may_be(const acl_t acl, int brand)
{

	if (_acl_brand(acl) == ACL_BRAND_UNKNOWN)
		return (1);

	if (_acl_brand(acl) == brand)
		return (1);

	return (0);
}

extern "C" int
_entry_brand_may_be(const acl_entry_t entry, int brand)
{

	return (_acl_brand_may_be(entry2acl(entry), brand));
}

extern "C" void
_acl_brand_as(acl_t acl, int brand)
{

	acl->ats_brand = brand;
}

extern "C" void
_entry_brand_as(const acl_entry_t entry, int brand)
{

	_acl_brand_as(entry2acl(entry), brand);
}

/* ----------------------------------------------------------- syscall mocks */

enum mock_kind {
	MOCK_MAC_FD = 1,
	MOCK_MAC_FILE,
	MOCK_MAC_LINK,
	MOCK_MAC_PROC,
	MOCK_ACL_FILE,
	MOCK_ACL_LINK,
	MOCK_ACL_FD,
};

struct mock_rec {
	int		kind;
	int		fd;
	const char	*path;
	struct mac	*label;
	acl_type_t	type;
	int		ret;
};

static mock_rec g_mock;
static int g_mock_set;

static void
mock_reset(void)
{
	std::memset(&g_mock, 0, sizeof(g_mock));
	g_mock_set = 0;
}

static int
mac_ret(int fd, struct mac *label)
{
	int v = fd * 31;

	if (label != nullptr)
		v ^= (int)label->m_buflen ^ (int)(uintptr_t)label->m_string;
	return (v ^ 0x5a5a);
}

static int
acl_ret(const char *path, acl_type_t type)
{
	int v = (int)type * 17;

	if (path != nullptr)
		v ^= (int)std::strlen(path);
	return (v ^ 0x1234);
}

static int
acl_fd_ret(int fd, acl_type_t type)
{

	return ((fd * 23) ^ ((int)type * 41) ^ 0xbeef);
}

extern "C" int
__mac_set_fd(int fd, struct mac *mac_p)
{

	g_mock.kind = MOCK_MAC_FD;
	g_mock.fd = fd;
	g_mock.label = mac_p;
	g_mock.ret = mac_ret(fd, mac_p);
	g_mock_set = 1;
	return (g_mock.ret);
}

extern "C" int
__mac_set_file(const char *path_p, struct mac *mac_p)
{

	g_mock.kind = MOCK_MAC_FILE;
	g_mock.path = path_p;
	g_mock.label = mac_p;
	g_mock.ret = mac_ret(1, mac_p) ^ (int)(uintptr_t)path_p;
	g_mock_set = 1;
	return (g_mock.ret);
}

extern "C" int
__mac_set_link(const char *path_p, struct mac *mac_p)
{

	g_mock.kind = MOCK_MAC_LINK;
	g_mock.path = path_p;
	g_mock.label = mac_p;
	g_mock.ret = mac_ret(2, mac_p) ^ (int)(uintptr_t)path_p;
	g_mock_set = 1;
	return (g_mock.ret);
}

extern "C" int
__mac_set_proc(struct mac *mac_p)
{

	g_mock.kind = MOCK_MAC_PROC;
	g_mock.label = mac_p;
	g_mock.ret = mac_ret(3, mac_p);
	g_mock_set = 1;
	return (g_mock.ret);
}

extern "C" int
__acl_delete_file(const char *path_p, acl_type_t type)
{

	g_mock.kind = MOCK_ACL_FILE;
	g_mock.path = path_p;
	g_mock.type = type;
	g_mock.ret = acl_ret(path_p, type);
	g_mock_set = 1;
	return (g_mock.ret);
}

extern "C" int
__acl_delete_link(const char *path_p, acl_type_t type)
{

	g_mock.kind = MOCK_ACL_LINK;
	g_mock.path = path_p;
	g_mock.type = type;
	g_mock.ret = acl_ret(path_p, type) ^ 0x100;
	g_mock_set = 1;
	return (g_mock.ret);
}

extern "C" int
___acl_delete_fd(int filedes, acl_type_t type)
{

	g_mock.kind = MOCK_ACL_FD;
	g_mock.fd = filedes;
	g_mock.type = type;
	g_mock.ret = acl_fd_ret(filedes, type);
	g_mock_set = 1;
	return (g_mock.ret);
}

struct mock_snap {
	int		set;
	int		kind;
	int		fd;
	const char	*path;
	struct mac	*label;
	acl_type_t	type;
	int		ret;
};

static mock_snap
mock_capture(void)
{
	mock_snap s;

	s.set = g_mock_set;
	s.kind = g_mock.kind;
	s.fd = g_mock.fd;
	s.path = g_mock.path;
	s.label = g_mock.label;
	s.type = g_mock.type;
	s.ret = g_mock.ret;
	return (s);
}

static bool
mock_eq(const mock_snap &a, const mock_snap &b)
{

	return (a.set == b.set && a.kind == b.kind && a.fd == b.fd &&
	    a.path == b.path && a.label == b.label && a.type == b.type &&
	    a.ret == b.ret);
}

/* ---------------------------------------------------------- ACL allocation */

static acl_t
alloc_acl(void)
{
	acl_t acl;
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
free_acl(acl_t acl)
{
	std::free(acl);
}

/* -------------------------------------------------------------- MAC tests */

static void
fill_mac(struct mac *m, size_t buflen, char *buf, size_t bufsz,
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
mac_run(int idx, int (*port_fn)(int, struct mac *),
    int (*ref_fn)(int, struct mac *), int fd, struct mac *label,
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
mac_run_path(int idx, int (*port_fn)(const char *, struct mac *),
    int (*ref_fn)(const char *, struct mac *), const char *path,
    struct mac *label, const char *tag)
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
mac_run_proc(int idx, int (*port_fn)(struct mac *),
    int (*ref_fn)(struct mac *), struct mac *label, const char *tag)
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
	struct mac m;

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
	struct mac m;

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
	0x80000000,
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

struct entry_blob {
	unsigned char		pre[32];
	struct acl_entry	entry;
	unsigned char		post[32];
};

static void
init_entry_blob(entry_blob &b, acl_tag_t tag, std::uint32_t id,
    acl_perm_t perm, acl_entry_type_t etype, acl_flag_t flags)
{
	std::memset(&b, GUARD, sizeof(b));
	b.entry.ae_tag = tag;
	b.entry.ae_id = id;
	b.entry.ae_perm = perm;
	b.entry.ae_entry_type = etype;
	b.entry.ae_flags = flags;
}

static bool
ace_run(acl_entry_t dest, acl_entry_t src, const char *tag)
{
	int rp, rr, ep, er;
	unsigned char dp_pre[32], dp_post[32], dr_pre[32], dr_post[32];
	unsigned char sp_pre[32], sp_post[32], sr_pre[32], sr_post[32];
	entry_blob db, sb, dbr, sbr;

	if (dest != nullptr) {
		std::memcpy(&db.entry, dest, sizeof(db.entry));
		std::memcpy(dp_pre, (unsigned char *)dest - 32, 32);
		std::memcpy(dp_post, (unsigned char *)dest + sizeof(*dest),
		    32);
	}
	if (src != nullptr) {
		std::memcpy(&sb.entry, src, sizeof(sb.entry));
		std::memcpy(sp_pre, (unsigned char *)src - 32, 32);
		std::memcpy(sp_post, (unsigned char *)src + sizeof(*src), 32);
	}

	mock_reset();
	errno = 0;
	rp = port::acl_copy_entry(dest, src);
	ep = errno;

	mock_reset();
	errno = 0;
	rr = ref_acl_copy_entry(dest, src);
	er = errno;

	if (dest != nullptr) {
		std::memcpy(&dbr.entry, dest, sizeof(dbr.entry));
		std::memcpy(dr_pre, (unsigned char *)dest - 32, 32);
		std::memcpy(dr_post, (unsigned char *)dest + sizeof(*dest),
		    32);
	}
	if (src != nullptr) {
		std::memcpy(&sbr.entry, src, sizeof(sbr.entry));
		std::memcpy(sr_pre, (unsigned char *)src - 32, 32);
		std::memcpy(sr_post, (unsigned char *)src + sizeof(*src), 32);
	}

	stats[S_ACE].cases++;
	bool bad = (rp != rr || ep != er);
	if (!bad && dest != nullptr) {
		bad = (std::memcmp(&db.entry, &dbr.entry, sizeof(db.entry)) !=
		    0 || std::memcmp(dp_pre, dr_pre, 32) != 0 ||
		    std::memcmp(dp_post, dr_post, 32) != 0);
	}
	if (!bad && src != nullptr) {
		bad = (std::memcmp(&sb.entry, &sbr.entry, sizeof(sb.entry)) !=
		    0 || std::memcmp(sp_pre, sr_pre, 32) != 0 ||
		    std::memcmp(sp_post, sr_post, 32) != 0);
	}
	if (bad)
		fail_stat(S_ACE, tag, "rv/errno/entry");
	return (!bad);
}

static bool
ace_run_pair(acl_t dest_acl, int dest_idx, acl_t src_acl, int src_idx,
    const char *tag)
{
	acl_entry_t dest = &dest_acl->ats_acl.acl_entry[dest_idx];
	acl_entry_t src = &src_acl->ats_acl.acl_entry[src_idx];

	return (ace_run(dest, src, tag));
}

static void
test_acl_copy_entry_edge(void)
{
	acl_t da = alloc_acl();
	acl_t sa = alloc_acl();

	ace_run(nullptr, &da->ats_acl.acl_entry[0], "null-dest");
	ace_run(&da->ats_acl.acl_entry[0], nullptr, "null-src");
	ace_run(&da->ats_acl.acl_entry[0], &da->ats_acl.acl_entry[0],
	    "same");

	da->ats_brand = ACL_BRAND_POSIX;
	sa->ats_brand = ACL_BRAND_NFS4;
	init_entry_blob(*(entry_blob *)&sa->ats_acl.acl_entry[0], 1, 2, 3, 4, 5);
	sa->ats_acl.acl_entry[0] =
	    ((entry_blob *)&sa->ats_acl.acl_entry[0])->entry;
	ace_run_pair(da, 0, sa, 0, "brand-mismatch");

	da->ats_brand = ACL_BRAND_UNKNOWN;
	sa->ats_brand = ACL_BRAND_POSIX;
	sa->ats_acl.acl_entry[1].ae_tag = 10;
	sa->ats_acl.acl_entry[1].ae_id = 20;
	sa->ats_acl.acl_entry[1].ae_perm = 0x80ff;
	sa->ats_acl.acl_entry[1].ae_entry_type = 0xff;
	sa->ats_acl.acl_entry[1].ae_flags = 0x7f00;
	ace_run_pair(da, 0, sa, 1, "ok-unknown-dest");

	da->ats_brand = ACL_BRAND_POSIX;
	sa->ats_brand = ACL_BRAND_POSIX;
	sa->ats_acl.acl_entry[2].ae_tag = 0xffffffff;
	sa->ats_acl.acl_entry[2].ae_id = 0;
	sa->ats_acl.acl_entry[2].ae_perm = 0;
	sa->ats_acl.acl_entry[2].ae_entry_type = 0;
	sa->ats_acl.acl_entry[2].ae_flags = 0;
	ace_run_pair(da, 1, sa, 2, "ok-posix");

	da->ats_brand = ACL_BRAND_NFS4;
	sa->ats_brand = ACL_BRAND_NFS4;
	ace_run_pair(da, 2, sa, 0, "ok-nfs4");

	free_acl(da);
	free_acl(sa);
}

static void
test_acl_copy_entry_random(void)
{

	for (long it = 0; it < SWEEP_ITERS; it++) {
		acl_t da = alloc_acl();
		acl_t sa = alloc_acl();
		int di = (int)(rnd32() % 8);
		int si = (int)(rnd32() % 8);

		da->ats_brand = (int)(rnd32() % 3);
		sa->ats_brand = (int)(rnd32() % 3);
		sa->ats_acl.acl_entry[si].ae_tag = rnd32();
		sa->ats_acl.acl_entry[si].ae_id = rnd32();
		sa->ats_acl.acl_entry[si].ae_perm = rnd32();
		sa->ats_acl.acl_entry[si].ae_entry_type =
		    (acl_entry_type_t)(rnd32() & 0xffff);
		sa->ats_acl.acl_entry[si].ae_flags =
		    (acl_flag_t)(rnd32() & 0xffff);
		ace_run_pair(da, di, sa, si, "rand");
		free_acl(da);
		free_acl(sa);
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
	    (acl_t)0x1234, size);
	ep = errno;

	mock_reset();
	errno = 0;
	rr = (int)ref_acl_copy_ext(buf != nullptr ? bb : nullptr,
	    (acl_t)0x1234, size);
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
