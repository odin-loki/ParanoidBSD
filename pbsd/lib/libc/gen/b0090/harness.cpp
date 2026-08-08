/*
 * harness.cpp -- differential test for batch b0090.
 *
 * getgrouplist(), getbootfile(), and getdomainname() bottom out in
 * __getgroupmembership() and sysctl().  Those symbols are defined here as
 * recording test doubles shared by BOTH the port and the oracle: each
 * records exactly what it was handed and returns a value that is an
 * injective function of every argument, so any divergence in the arguments
 * the port computes shows up both in the recorded state and in the return
 * value.
 */

import pbsd.lib.libc.gen.b0090;

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>

namespace port = pbsd::lib_libc_gen::b0090;

extern "C" {
int ref_getgrouplist(const char *uname, gid_t agroup, gid_t *groups,
    int *grpcnt);
const char *ref_getbootfile(void);
int ref_getdomainname(char *name, int namelen);
}

#ifndef CTL_KERN
#define CTL_KERN 1
#endif
#ifndef KERN_NISDOMAINNAME
#define KERN_NISDOMAINNAME 22
#endif
#ifndef KERN_BOOTFILE
#define KERN_BOOTFILE 26
#endif

/* ------------------------------------------------------------------ doubles */

#define	MOCK_PATHCOPY	128u
#define	MOCK_DOMAINCOPY 256u

struct mock_state {
	int		force_fail;
	char		bootfile_path[MOCK_PATHCOPY];
	char		domain_value[MOCK_DOMAINCOPY];

	long		ggm_calls;
	const char     *ggm_uname;
	gid_t		ggm_agroup;
	gid_t	       *ggm_groups;
	int		ggm_grpcnt_in;
	int	       *ggm_grpcnt_ptr;

	long		sysctl_calls;
	int		sysctl_mib0;
	int		sysctl_mib1;
	unsigned int	sysctl_namelen;
	void	       *sysctl_oldp;
	size_t		sysctl_oldlen_in;
	size_t		sysctl_oldlen_out;
	const void     *sysctl_newp;
	size_t		sysctl_newlen;
};

static mock_state g;

static void
mock_reset(void)
{
	char boot[MOCK_PATHCOPY];
	char dom[MOCK_DOMAINCOPY];

	memcpy(boot, g.bootfile_path, MOCK_PATHCOPY);
	memcpy(dom, g.domain_value, MOCK_DOMAINCOPY);
	memset(&g, 0, sizeof(g));
	memcpy(g.bootfile_path, boot, MOCK_PATHCOPY);
	memcpy(g.domain_value, dom, MOCK_DOMAINCOPY);
}

static void
mock_set_bootfile(const char *path)
{
	size_t n;

	n = strlen(path);
	if (n >= MOCK_PATHCOPY)
		n = MOCK_PATHCOPY - 1;
	memcpy(g.bootfile_path, path, n);
	g.bootfile_path[n] = '\0';
}

static void
mock_set_domain(const char *dom)
{
	size_t n;

	n = strlen(dom);
	if (n >= MOCK_DOMAINCOPY)
		n = MOCK_DOMAINCOPY - 1;
	memcpy(g.domain_value, dom, n);
	g.domain_value[n] = '\0';
}

static inline uint32_t
fnv_step(uint32_t h, uint32_t v)
{
	return (h ^ v) * 16777619u;
}

static uint32_t
hash_str(const char *s)
{
	uint32_t h = 0x811c9dc5u;

	if (s == nullptr)
		return (fnv_step(h, 0xdeadbeefu));
	for (; *s != '\0'; s++)
		h = fnv_step(h, (uint32_t)(unsigned char)*s);
	return (h);
}

extern "C" int
__getgroupmembership(const char *uname, gid_t agroup, gid_t *groups,
    int grpcnt_in, int *grpcnt_ptr)
{
	uint32_t h;
	int ngroups, ret, i, cap;

	g.ggm_calls++;
	g.ggm_uname = uname;
	g.ggm_agroup = agroup;
	g.ggm_groups = groups;
	g.ggm_grpcnt_in = grpcnt_in;
	g.ggm_grpcnt_ptr = grpcnt_ptr;

	h = 0x811c9dc5u;
	h = fnv_step(h, hash_str(uname));
	h = fnv_step(h, (uint32_t)agroup);
	h = fnv_step(h, (uint32_t)grpcnt_in);
	h = fnv_step(h, groups == nullptr ? 0xbadc0deu : 0x600dfeedu);
	h = fnv_step(h, grpcnt_ptr == nullptr ? 0xcafebabeu : 0x0ddba11u);

	ngroups = (int)((h % 17u) + 1u);
	ret = (int)(h & 0x7fffffffu);
	if ((h & 0x100u) != 0)
		ret = -1;

	if (grpcnt_ptr != nullptr)
		*grpcnt_ptr = ngroups;

	if (groups != nullptr && grpcnt_in > 0) {
		cap = grpcnt_in < ngroups ? grpcnt_in : ngroups;
		for (i = 0; i < cap; i++)
			groups[i] = agroup + (gid_t)(i + 1);
		for (; i < grpcnt_in; i++)
			groups[i] = (gid_t)0x7f7f7f7fu;
	}

	return (ret);
}

extern "C" int
sysctl(const int *name, unsigned int namelen, void *oldp, size_t *oldlenp,
    const void *newp, size_t newlen)
{
	const char *src;
	size_t slen, copy, avail;

	g.sysctl_calls++;
	g.sysctl_namelen = namelen;
	g.sysctl_oldp = oldp;
	g.sysctl_newp = newp;
	g.sysctl_newlen = newlen;
	if (oldlenp != nullptr)
		g.sysctl_oldlen_in = *oldlenp;
	else
		g.sysctl_oldlen_in = 0;

	if (name != nullptr && namelen >= 1u) {
		g.sysctl_mib0 = name[0];
		if (namelen >= 2u)
			g.sysctl_mib1 = name[1];
	}

	if (g.force_fail)
		return (-1);

	if (name == nullptr || namelen < 2u)
		return (-1);
	if (name[0] != CTL_KERN)
		return (-1);

	if (name[1] == KERN_BOOTFILE) {
		src = g.bootfile_path;
		slen = strlen(src);
		if (oldp == nullptr || oldlenp == nullptr) {
			g.sysctl_oldlen_out = slen;
			return (0);
		}
		avail = *oldlenp;
		copy = slen;
		if (copy >= avail)
			copy = avail > 0 ? avail - 1 : 0;
		if (copy > 0)
			memcpy(oldp, src, copy);
		if (avail > 0)
			((char *)oldp)[copy] = '\0';
		*oldlenp = copy;
		g.sysctl_oldlen_out = copy;
		return (0);
	}

	if (name[1] == KERN_NISDOMAINNAME) {
		src = g.domain_value;
		slen = strlen(src);
		if (oldp == nullptr || oldlenp == nullptr) {
			g.sysctl_oldlen_out = slen;
			return (0);
		}
		avail = *oldlenp;
		copy = slen;
		if (copy >= avail)
			copy = avail > 0 ? avail - 1 : 0;
		if (copy > 0)
			memcpy(oldp, src, copy);
		if (avail > 0)
			((char *)oldp)[copy] = '\0';
		*oldlenp = copy;
		g.sysctl_oldlen_out = copy;
		return (0);
	}

	return (-1);
}

/* ------------------------------------------------------------------ capture */

static const long OFF_NULL = -999999L;

struct ggm_capture {
	int		rv;
	long		calls;
	long		uname_off;
	gid_t		agroup;
	long		groups_off;
	int		grpcnt_in;
	int		grpcnt_out;
};

struct boot_capture {
	const char     *ret;
	int		fallback;
	long		sysctl_calls;
	int		mib0;
	int		mib1;
	size_t		oldlen_in;
	size_t		oldlen_out;
	char		path[MOCK_PATHCOPY];
};

struct dom_capture {
	int		rv;
	long		sysctl_calls;
	int		mib0;
	int		mib1;
	size_t		oldlen_in;
	size_t		oldlen_out;
};

struct stat_row {
	const char     *name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "getgrouplist", 0, 0, 0 },
	{ "getbootfile", 0, 0, 0 },
	{ "getdomainname", 0, 0, 0 },
};

#define	R_GROUPLIST	0
#define	R_BOOTFILE	1
#define	R_DOMAINNAME	2
#define	MAX_PRINT	12

#define	BUFSZ		2048u
#define	GUARD		0x7f
#define	GID_GUARD	(gid_t)0x7f7f7f7fU
#define	GRPCNT_GUARD	0x7f7f7f7f

static unsigned char *bufa;
static unsigned char *bufb;
static gid_t *groups_a;
static gid_t *groups_b;
static int grpcnt_a;
static int grpcnt_b;

static void
dump_buf_diff(const unsigned char *a, const unsigned char *b, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			printf("    buffer diverges at [%zu]: port=%#02x "
			    "oracle=%#02x\n", i, a[i], b[i]);
			return;
		}
	}
}

static void
dump_gid_diff(const gid_t *a, const gid_t *b, int n)
{
	for (int i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			printf("    groups diverge at [%d]: port=%u "
			    "oracle=%u\n", i, (unsigned)a[i], (unsigned)b[i]);
			return;
		}
	}
}

static ggm_capture
snap_ggm(int rv, const unsigned char *base, const gid_t *groups_base)
{
	ggm_capture c;

	memset(&c, 0, sizeof(c));
	c.rv = rv;
	c.calls = g.ggm_calls;
	c.uname_off = g.ggm_uname == nullptr ? OFF_NULL :
	    (long)(g.ggm_uname - (const char *)base);
	c.agroup = g.ggm_agroup;
	c.groups_off = g.ggm_groups == nullptr ? OFF_NULL :
	    (long)(g.ggm_groups - groups_base);
	c.grpcnt_in = g.ggm_grpcnt_in;
	if (g.ggm_grpcnt_ptr != nullptr)
		c.grpcnt_out = *g.ggm_grpcnt_ptr;
	else
		c.grpcnt_out = GRPCNT_GUARD;
	return (c);
}

static boot_capture
snap_boot(const char *ret)
{
	boot_capture c;
	size_t n;

	memset(&c, 0, sizeof(c));
	c.ret = ret;
	c.fallback = (ret != nullptr &&
	    strcmp(ret, "/boot/kernel/kernel") == 0) ? 1 : 0;
	c.sysctl_calls = g.sysctl_calls;
	c.mib0 = g.sysctl_mib0;
	c.mib1 = g.sysctl_mib1;
	c.oldlen_in = g.sysctl_oldlen_in;
	c.oldlen_out = g.sysctl_oldlen_out;
	if (ret != nullptr) {
		n = strlen(ret);
		if (n >= MOCK_PATHCOPY)
			n = MOCK_PATHCOPY - 1;
		memcpy(c.path, ret, n);
		c.path[n] = '\0';
	}
	return (c);
}

static dom_capture
snap_dom(int rv)
{
	dom_capture c;

	memset(&c, 0, sizeof(c));
	c.rv = rv;
	c.sysctl_calls = g.sysctl_calls;
	c.mib0 = g.sysctl_mib0;
	c.mib1 = g.sysctl_mib1;
	c.oldlen_in = g.sysctl_oldlen_in;
	c.oldlen_out = g.sysctl_oldlen_out;
	return (c);
}

static bool
ggm_eq(const ggm_capture &a, const ggm_capture &b)
{
	return (a.rv == b.rv && a.calls == b.calls &&
	    a.uname_off == b.uname_off && a.agroup == b.agroup &&
	    a.groups_off == b.groups_off && a.grpcnt_in == b.grpcnt_in &&
	    a.grpcnt_out == b.grpcnt_out);
}

static bool
boot_eq(const boot_capture &a, const boot_capture &b)
{
	return (a.fallback == b.fallback && a.sysctl_calls == b.sysctl_calls &&
	    a.mib0 == b.mib0 && a.mib1 == b.mib1 &&
	    a.oldlen_in == b.oldlen_in && a.oldlen_out == b.oldlen_out &&
	    strcmp(a.path, b.path) == 0);
}

static bool
dom_eq(const dom_capture &a, const dom_capture &b)
{
	return (a.rv == b.rv && a.sysctl_calls == b.sysctl_calls &&
	    a.mib0 == b.mib0 && a.mib1 == b.mib1 &&
	    a.oldlen_in == b.oldlen_in && a.oldlen_out == b.oldlen_out);
}

/* --------------------------------------------------------------- test cases */

static void
do_grouplist(const char *uname, gid_t agroup, size_t uname_off,
    size_t uname_len, int grpcnt_in, bool use_null_name, bool use_null_groups)
{
	stat_row &r = rows[R_GROUPLIST];
	const int ng = 64;
	int i;

	if (uname_off + uname_len + 1 > BUFSZ) {
		fprintf(stderr, "internal: uname does not fit buffer\n");
		abort();
	}

	r.cases++;

	memset(bufa, GUARD, BUFSZ);
	memset(bufb, GUARD, BUFSZ);
	for (i = 0; i < ng; i++) {
		groups_a[i] = GID_GUARD;
		groups_b[i] = GID_GUARD;
	}
	grpcnt_a = GRPCNT_GUARD;
	grpcnt_b = GRPCNT_GUARD;

	if (!use_null_name) {
		memcpy(bufa + uname_off, uname, uname_len);
		bufa[uname_off + uname_len] = '\0';
		memcpy(bufb + uname_off, uname, uname_len);
		bufb[uname_off + uname_len] = '\0';
	}

	const char *pa = use_null_name ? nullptr :
	    (const char *)(bufa + uname_off);
	const char *pb = use_null_name ? nullptr :
	    (const char *)(bufb + uname_off);
	gid_t *ga = use_null_groups ? nullptr : groups_a;
	gid_t *gb = use_null_groups ? nullptr : groups_b;

	grpcnt_a = grpcnt_in;
	grpcnt_b = grpcnt_in;

	mock_reset();
	int rva = port::getgrouplist(pa, agroup, ga, &grpcnt_a);
	ggm_capture ca = snap_ggm(rva, bufa, groups_a);

	mock_reset();
	int rvb = ref_getgrouplist(pb, agroup, gb, &grpcnt_b);
	ggm_capture cb = snap_ggm(rvb, bufb, groups_b);

	bool same_buf = memcmp(bufa, bufb, BUFSZ) == 0;
	bool same_gid = true;
	for (i = 0; i < ng; i++) {
		if (groups_a[i] != groups_b[i]) {
			same_gid = false;
			break;
		}
	}
	bool same_grpcnt = (grpcnt_a == grpcnt_b);

	if (!ggm_eq(ca, cb) || !same_buf || !same_gid || !same_grpcnt) {
		r.failures++;
		if (r.printed < MAX_PRINT) {
			r.printed++;
			printf("FAIL getgrouplist agroup=%u grpcnt=%d "
			    "null{name=%d groups=%d}\n",
			    (unsigned)agroup, grpcnt_in, (int)use_null_name,
			    (int)use_null_groups);
			printf("    port rv=%d oracle rv=%d\n", ca.rv, cb.rv);
			if (!same_buf)
				dump_buf_diff(bufa, bufb, BUFSZ);
			if (!same_gid)
				dump_gid_diff(groups_a, groups_b, ng);
			if (!same_grpcnt)
				printf("    grpcnt port=%d oracle=%d\n",
				    grpcnt_a, grpcnt_b);
		}
	}
}

static void
do_bootfile(bool fail)
{
	stat_row &r = rows[R_BOOTFILE];

	r.cases++;

	mock_reset();
	g.force_fail = fail ? 1 : 0;
	const char *rva = port::getbootfile();
	boot_capture ca = snap_boot(rva);

	mock_reset();
	g.force_fail = fail ? 1 : 0;
	const char *rvb = ref_getbootfile();
	boot_capture cb = snap_boot(rvb);

	if (!boot_eq(ca, cb)) {
		r.failures++;
		if (r.printed < MAX_PRINT) {
			r.printed++;
			printf("FAIL getbootfile fail=%d\n", (int)fail);
			printf("    port path=%s fallback=%d\n", ca.path,
			    ca.fallback);
			printf("    oracle path=%s fallback=%d\n", cb.path,
			    cb.fallback);
		}
	}
}

static void
do_domainname(int namelen, bool use_null, bool fail)
{
	stat_row &r = rows[R_DOMAINNAME];

	r.cases++;

	memset(bufa, GUARD, BUFSZ);
	memset(bufb, GUARD, BUFSZ);

	mock_reset();
	g.force_fail = fail ? 1 : 0;
	int rva = use_null ? port::getdomainname(nullptr, namelen) :
	    port::getdomainname((char *)bufa, namelen);
	dom_capture ca = snap_dom(rva);

	mock_reset();
	g.force_fail = fail ? 1 : 0;
	int rvb = use_null ? ref_getdomainname(nullptr, namelen) :
	    ref_getdomainname((char *)bufb, namelen);
	dom_capture cb = snap_dom(rvb);

	bool same_buf = use_null || memcmp(bufa, bufb, BUFSZ) == 0;
	if (!dom_eq(ca, cb) || !same_buf) {
		r.failures++;
		if (r.printed < MAX_PRINT) {
			r.printed++;
			printf("FAIL getdomainname namelen=%d null=%d\n",
			    namelen, (int)use_null);
			printf("    port rv=%d oracle rv=%d\n", ca.rv, cb.rv);
			if (!same_buf)
				dump_buf_diff(bufa, bufb, BUFSZ);
		}
	}
}

/* ----------------------------------------------------------------- edge set */

static void
edge_grouplist(void)
{
	static const gid_t agroups[] = {
		0, 1, 2, 10, 100, 1000, 65535, 65536,
		(gid_t)0x7ffffffeU, (gid_t)0x7fffffffU,
		(gid_t)0x80000000U, (gid_t)0xffffffffU,
	};
	static const int grpcnts[] = {
		INT_MIN, -2, -1, 0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64,
		127, 128, 255, 256, 1023, 1024, INT_MAX,
	};
	static const size_t offs[] = { 0, 1, 2, 7, 8, 63, 64, 128, 512, 1024 };
	unsigned char u[256];

	do_grouplist("", 0, 0, 0, 0, false, false);
	do_grouplist("a", 1, 0, 1, 1, false, false);
	do_grouplist("root", 0, 0, 4, 16, false, false);

	for (unsigned v = 0; v < 256u; v++) {
		u[0] = (unsigned char)v;
		do_grouplist((const char *)u, (gid_t)v, 0, 1, (int)(v % 32u) + 1,
		    false, false);
	}

	for (unsigned v = 0x80u; v <= 0xffu; v++) {
		for (unsigned i = 0; i < 8u; i++)
			u[i] = (unsigned char)v;
		u[8] = '\0';
		do_grouplist((const char *)u, (gid_t)v, 0, 8, 8, false, false);
	}

	{
		static const unsigned char nulheavy[] = {
			'u', 0x00, 's', 0x00, 'e', 0xff, 0x00, 0x80, 'r', 0x00,
		};
		for (size_t n = 0; n <= sizeof(nulheavy); n++)
			do_grouplist((const char *)nulheavy, 42, 3, n, 4, false,
			    false);
	}

	for (unsigned a = 0; a < sizeof(agroups) / sizeof(agroups[0]); a++)
		for (unsigned c = 0; c < sizeof(grpcnts) / sizeof(grpcnts[0]);
		    c++)
			do_grouplist("testuser", agroups[a], 0, 8, grpcnts[c],
			    false, false);

	for (unsigned o = 0; o < sizeof(offs) / sizeof(offs[0]); o++)
		do_grouplist("x", 5, offs[o], 1, 4, false, false);

	do_grouplist(nullptr, 0, 0, 0, 0, true, false);
	do_grouplist("n", 1, 0, 1, 1, false, true);
}

static void
edge_bootfile(void)
{
	mock_set_bootfile("");
	do_bootfile(false);

	mock_set_bootfile("a");
	do_bootfile(false);

	mock_set_bootfile("/boot/kernel/kernel");
	do_bootfile(false);

	mock_set_bootfile("/boot/kernel/custom");
	do_bootfile(false);

	{
		char longpath[MOCK_PATHCOPY];
		for (unsigned i = 0; i < MOCK_PATHCOPY - 1u; i++)
			longpath[i] = (char)('A' + (i % 26u));
		longpath[MOCK_PATHCOPY - 1u] = '\0';
		mock_set_bootfile(longpath);
		do_bootfile(false);
	}

	for (unsigned v = 0x80u; v <= 0xffu; v++) {
		unsigned char p[4] = { '/', (unsigned char)v, 'k', '\0' };
		mock_set_bootfile((const char *)p);
		do_bootfile(false);
	}

	do_bootfile(true);
	do_bootfile(true);
}

static void
edge_domainname(void)
{
	static const int lens[] = {
		INT_MIN, -2, -1, 0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64,
		127, 128, 255, 256, 511, 512, 1023, 1024, 2047, 2048,
		INT_MAX,
	};

	mock_set_domain("");
	for (unsigned i = 0; i < sizeof(lens) / sizeof(lens[0]); i++)
		do_domainname(lens[i], false, false);

	mock_set_domain("a");
	do_domainname(1, false, false);
	do_domainname(2, false, false);

	mock_set_domain("example.com");
	for (unsigned i = 0; i < sizeof(lens) / sizeof(lens[0]); i++)
		do_domainname(lens[i], false, false);

	mock_set_domain("long.domain.name.test");
	do_domainname(64, false, false);
	do_domainname(256, false, false);
	do_domainname(1024, false, false);

	{
		char dom[300];
		for (unsigned i = 0; i < sizeof(dom) - 1u; i++)
			dom[i] = (char)(0x80 + (i % 0x80u));
		dom[sizeof(dom) - 1u] = '\0';
		mock_set_domain(dom);
		do_domainname(128, false, false);
		do_domainname(512, false, false);
	}

	g.force_fail = 1;
	for (unsigned i = 0; i < sizeof(lens) / sizeof(lens[0]); i++)
		do_domainname(lens[i], false, true);
	g.force_fail = 0;

	do_domainname(64, true, false);
}

/* ------------------------------------------------------------------- random */

static uint64_t rng_state = 0x00b0090ba11adULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static unsigned char
rnd_byte(void)
{
	switch (rnd32() % 4u) {
	case 0:
		return (0u);
	case 1:
		return ((unsigned char)(0x80u + rnd32() % 0x80u));
	case 2:
		return ((unsigned char)(0x20u + rnd32() % 0x5fu));
	default:
		return ((unsigned char)(rnd32() & 0xffu));
	}
}

#define	SWEEP_ITERS	250000L

static void
sweep(void)
{
	unsigned char u[128];
	char boot[MOCK_PATHCOPY];
	char dom[MOCK_DOMAINCOPY];

	for (long it = 0; it < SWEEP_ITERS; it++) {
		size_t ulen = (size_t)(rnd32() % 80u);
		size_t uoff = (size_t)(rnd32() % (BUFSZ - ulen - 1u));
		for (size_t i = 0; i < ulen; i++)
			u[i] = rnd_byte();
		do_grouplist((const char *)u, (gid_t)rnd32(),
		    uoff, ulen, (int)(rnd32() % 65u), false, false);

		if ((rnd32() % 16u) == 0u)
			do_grouplist(nullptr, (gid_t)rnd32(), 0, 0,
			    (int)(rnd32() % 8u), true,
			    (rnd32() & 1u) != 0);

		size_t blen = (size_t)(rnd32() % (MOCK_PATHCOPY - 1u));
		for (size_t i = 0; i < blen; i++)
			boot[i] = (char)rnd_byte();
		boot[blen] = '\0';
		mock_set_bootfile(boot);
		do_bootfile((rnd32() & 3u) == 0u);

		size_t dlen = (size_t)(rnd32() % (MOCK_DOMAINCOPY - 1u));
		for (size_t i = 0; i < dlen; i++)
			dom[i] = (char)rnd_byte();
		dom[dlen] = '\0';
		mock_set_domain(dom);
		g.force_fail = (rnd32() & 7u) == 0u ? 1 : 0;
		do_domainname((int)rnd32(), (rnd32() % 128u) == 0u,
		    g.force_fail != 0);
		g.force_fail = 0;
	}
}

/* --------------------------------------------------------------------- main */

int
main(void)
{
	bufa = (unsigned char *)malloc(BUFSZ);
	bufb = (unsigned char *)malloc(BUFSZ);
	groups_a = (gid_t *)malloc(64 * sizeof(gid_t));
	groups_b = (gid_t *)malloc(64 * sizeof(gid_t));
	if (bufa == nullptr || bufb == nullptr || groups_a == nullptr ||
	    groups_b == nullptr) {
		fprintf(stderr, "out of memory\n");
		return (1);
	}

	mock_set_bootfile("/boot/kernel/test");
	mock_set_domain("localdomain");

	edge_grouplist();
	edge_bootfile();
	edge_domainname();
	sweep();

	long total_cases = 0, total_failures = 0;

	printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	printf("--------------------------------------------------\n");
	for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
		printf("%-24s %10ld %10ld\n", rows[i].name, rows[i].cases,
		    rows[i].failures);
		total_cases += rows[i].cases;
		total_failures += rows[i].failures;
	}
	printf("--------------------------------------------------\n");
	printf("%-24s %10ld %10ld\n", "TOTAL", total_cases, total_failures);

	if (total_failures != 0) {
		printf("\nRESULT: FAIL (%ld mismatching cases)\n",
		    total_failures);
		return (1);
	}
	printf("\nRESULT: PASS\n");
	return (0);
}
