/*
 * harness.cpp -- differential test for PBSD batch b0110.
 *
 * Every function of the batch is called twice with byte-identical inputs:
 * once in the C++23 port (module pbsd.lib.libc.posix1e.b0110) and once in the
 * reference oracle built from the untouched HardenedBSD C.  Compared after
 * every single call:
 *
 *   - the return value (as an offset from the buffer base for pointers,
 *     never as a raw address);
 *   - errno;
 *   - the entire output buffer, including the guard bytes outside the
 *     nominal write window;
 *   - the entire input buffer, so that a spurious write is caught too;
 *   - the sequence of calls made into the external helpers
 *     (__mac_set_*, __acl_delete_*, _acl_type_unold, _entry_brand*),
 *     including every argument and the classification/offset of every
 *     pointer handed to them.  The helpers are defined once here, so the
 *     port and the oracle drive the exact same code and get the exact same
 *     deterministic results.
 */

#include <errno.h>
#include <limits.h>
#include <new>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

import pbsd.lib.libc.posix1e.b0110;

namespace P = pbsd::lib_libc_posix1e::b0110;

using AE = P::acl_entry;
using MAC = P::mac;
typedef char *charp;

extern "C" {
int	ref_mac_set_fd(int fd, MAC *label);
int	ref_mac_set_file(const char *path, MAC *label);
int	ref_mac_set_link(const char *path, MAC *label);
int	ref_mac_set_proc(MAC *label);
int	ref_acl_delete_def_file(const char *path_p);
int	ref_acl_delete_def_link_np(const char *path_p);
int	ref_acl_delete_file_np(const char *path_p, P::acl_type_t type);
int	ref_acl_delete_link_np(const char *path_p, P::acl_type_t type);
int	ref_acl_delete_fd_np(int filedes, P::acl_type_t type);
int	ref_extattr_namespace_to_string(int attrnamespace, char **string);
int	ref_extattr_string_to_namespace(const char *string, int *attrnamespace);
int	ref_acl_copy_entry(AE *dest_d, AE *src_d);
P::ssize_t ref_acl_copy_ext(void *buf_p, P::acl_t acl, P::ssize_t size);
P::acl_t   ref_acl_copy_int(const void *buf_p);
}

/* ------------------------------------------------------------------ */
/* Guarded buffers						      */
/* ------------------------------------------------------------------ */

static const size_t RSZ = 96;		/* region size			*/
static const size_t OFF = 32;		/* object offset in region	*/
static const unsigned char GUARD = 0x7f;

struct Reg {
	alignas(16) unsigned char b[RSZ];
};

static void
reg_init(Reg &r)
{

	memset(r.b, GUARD, RSZ);
}

static bool
reg_eq(const Reg &x, const Reg &y)
{

	return (memcmp(x.b, y.b, RSZ) == 0);
}

/* Compares everything outside [skip, skip + len). */
static bool
reg_eq_except(const Reg &x, const Reg &y, size_t skip, size_t len)
{

	if (memcmp(x.b, y.b, skip) != 0)
		return (false);
	return (memcmp(x.b + skip + len, y.b + skip + len,
	    RSZ - skip - len) == 0);
}

/* ------------------------------------------------------------------ */
/* External-helper instrumentation				      */
/* ------------------------------------------------------------------ */

enum {
	S_MAC_FD = 1, S_MAC_FILE, S_MAC_LINK, S_MAC_PROC,
	S_DEL_FILE, S_DEL_LINK, S_DEL_FD, S_UNOLD,
	S_BRAND, S_BRAND_MAY_BE, S_BRAND_AS
};

static const int MAXLOG = 8;
static const size_t STRCAP = 32;

struct Call {
	int		id;
	int		pcls;	/* 0 NULL, 1 region1, 2 region2, 9 foreign */
	long long	poff;	/* offset inside its region, -1 NULL	   */
	long long	i1;
	long long	i2;
	long long	ret;
	int		has_str;
	unsigned int	slen;
	unsigned char	str[STRCAP];
};

static Call		g_log[MAXLOG];
static int		g_n;
static const unsigned char *g_r1;
static const unsigned char *g_r2;

static void
begin_side(const unsigned char *r1, const unsigned char *r2)
{

	g_r1 = r1;
	g_r2 = r2;
	g_n = 0;
	errno = 0;
}

static int
pcls(const void *p)
{
	const unsigned char *q = (const unsigned char *)p;

	if (p == NULL)
		return (0);
	if (g_r1 != NULL && q >= g_r1 && q < g_r1 + RSZ)
		return (1);
	if (g_r2 != NULL && q >= g_r2 && q < g_r2 + RSZ)
		return (2);
	return (9);
}

static long long
poff(const void *p)
{
	const unsigned char *q = (const unsigned char *)p;

	if (p == NULL)
		return (-1);
	if (g_r1 != NULL && q >= g_r1 && q < g_r1 + RSZ)
		return ((long long)(q - g_r1));
	if (g_r2 != NULL && q >= g_r2 && q < g_r2 + RSZ)
		return ((long long)(q - g_r2));
	return (-2);
}

static void
grab_str(Call &c, const char *s)
{
	size_t n;

	if (s == NULL) {
		c.has_str = 0;
		c.slen = 0;
		return;
	}
	c.has_str = 1;
	n = strlen(s);
	c.slen = (unsigned int)n;
	if (n > STRCAP)
		n = STRCAP;
	memcpy(c.str, s, n);
}

static void
logpush(const Call &c)
{

	if (g_n < MAXLOG)
		g_log[g_n] = c;
	g_n++;
}

/*
 * Deterministic mixing, so that a helper's result depends on every argument
 * it was handed: a mutation that perturbs an argument changes the return
 * value as well as the log.
 */
static long long
mix(long long a, long long b)
{

	return ((long long)(((unsigned long long)a * 0x9E3779B97F4A7C15ULL) ^
	    ((unsigned long long)b * 0xC2B2AE3D27D4EB4FULL)));
}

static int
mac_stub(int id, int fd, const char *path, MAC *mac_p)
{
	Call c;
	long long h;

	memset(&c, 0, sizeof(c));
	c.id = id;
	c.pcls = pcls(mac_p);
	c.poff = poff(mac_p);
	c.i1 = fd;
	grab_str(c, path);
	if (mac_p != NULL)
		c.i2 = mix((long long)mac_p->m_buflen,
		    mac_p->m_string != NULL ? 1 : 0);
	h = mix(mix(id, fd), mix(c.pcls * 131 + (long long)c.slen, c.i2));
	c.ret = (int)(h % 4096) - 2048;
	logpush(c);
	errno = (int)(((h % 37) + 37) % 37) + 1;
	return ((int)c.ret);
}

extern "C" int
__mac_set_fd(int fd, MAC *mac_p)
{

	return (mac_stub(S_MAC_FD, fd, NULL, mac_p));
}

extern "C" int
__mac_set_file(const char *path_p, MAC *mac_p)
{

	return (mac_stub(S_MAC_FILE, 0, path_p, mac_p));
}

extern "C" int
__mac_set_link(const char *path_p, MAC *mac_p)
{

	return (mac_stub(S_MAC_LINK, 0, path_p, mac_p));
}

extern "C" int
__mac_set_proc(MAC *mac_p)
{

	return (mac_stub(S_MAC_PROC, 0, NULL, mac_p));
}

static int
del_stub(int id, const char *path, int fd, P::acl_type_t type)
{
	Call c;
	long long h;

	memset(&c, 0, sizeof(c));
	c.id = id;
	c.pcls = pcls(path);
	c.poff = poff(path);
	c.i1 = fd;
	c.i2 = type;
	grab_str(c, path);
	h = mix(mix(id, type), mix(fd, c.pcls * 131 + (long long)c.slen));
	c.ret = (int)(h % 4096) - 2048;
	logpush(c);
	errno = (int)(((h % 37) + 37) % 37) + 1;
	return ((int)c.ret);
}

extern "C" int
__acl_delete_file(const char *path_p, P::acl_type_t type)
{

	return (del_stub(S_DEL_FILE, path_p, 0, type));
}

extern "C" int
__acl_delete_link(const char *path_p, P::acl_type_t type)
{

	return (del_stub(S_DEL_LINK, path_p, 0, type));
}

extern "C" int
___acl_delete_fd(int filedes, P::acl_type_t type)
{

	return (del_stub(S_DEL_FD, NULL, filedes, type));
}

/* The acl_support.c implementation. */
extern "C" P::acl_type_t
_acl_type_unold(P::acl_type_t type)
{
	Call c;
	P::acl_type_t out;

	switch (type) {
	case P::ACL_TYPE_ACCESS_OLD:
		out = P::ACL_TYPE_ACCESS;
		break;
	case P::ACL_TYPE_DEFAULT_OLD:
		out = P::ACL_TYPE_DEFAULT;
		break;
	default:
		out = type;
		break;
	}
	memset(&c, 0, sizeof(c));
	c.id = S_UNOLD;
	c.poff = -1;
	c.i1 = type;
	c.ret = out;
	logpush(c);
	return (out);
}

/*
 * acl_branding.c-alike branding, made a pure function of the entry so that
 * both sides agree.  NULL-tolerant on purpose: a mutated port that reaches a
 * helper it should never have reached is then caught by the call log instead
 * of dying on a segfault.
 */
static int
brand_of(const AE *e)
{

	if (e == NULL)
		return (P::ACL_BRAND_UNKNOWN);
	return ((int)(e->ae_tag % 3u));
}

extern "C" int
_entry_brand(AE *entry)
{
	Call c;

	memset(&c, 0, sizeof(c));
	c.id = S_BRAND;
	c.pcls = pcls(entry);
	c.poff = poff(entry);
	c.i1 = entry != NULL ? (long long)entry->ae_tag : -1;
	c.ret = brand_of(entry);
	logpush(c);
	return ((int)c.ret);
}

extern "C" int
_entry_brand_may_be(AE *entry, int brand)
{
	Call c;
	int b = brand_of(entry);
	int ok;

	if (b == brand)
		ok = 1;
	else if (b == P::ACL_BRAND_UNKNOWN || brand == P::ACL_BRAND_UNKNOWN)
		ok = 1;
	else
		ok = 0;
	memset(&c, 0, sizeof(c));
	c.id = S_BRAND_MAY_BE;
	c.pcls = pcls(entry);
	c.poff = poff(entry);
	c.i1 = entry != NULL ? (long long)entry->ae_tag : -1;
	c.i2 = brand;
	c.ret = ok;
	logpush(c);
	return (ok);
}

extern "C" void
_entry_brand_as(AE *entry, int brand)
{
	Call c;

	memset(&c, 0, sizeof(c));
	c.id = S_BRAND_AS;
	c.pcls = pcls(entry);
	c.poff = poff(entry);
	c.i1 = entry != NULL ? (long long)entry->ae_tag : -1;
	c.i2 = brand;
	c.ret = 0;
	logpush(c);
}

/* ------------------------------------------------------------------ */
/* Per-call capture and comparison				      */
/* ------------------------------------------------------------------ */

struct Side {
	long long	ret;
	int		err;
	int		n;
	Call		log[MAXLOG];
};

static void
end_side(Side &s, long long ret)
{
	int m;

	s.err = errno;
	s.ret = ret;
	s.n = g_n;
	m = g_n < MAXLOG ? g_n : MAXLOG;
	for (int i = 0; i < m; i++)
		s.log[i] = g_log[i];
}

enum {
	F_MAC_SET_FD, F_MAC_SET_FILE, F_MAC_SET_LINK, F_MAC_SET_PROC,
	F_ACL_DELETE_DEF_FILE, F_ACL_DELETE_DEF_LINK_NP,
	F_ACL_DELETE_FILE_NP, F_ACL_DELETE_LINK_NP, F_ACL_DELETE_FD_NP,
	F_EXTATTR_NS_TO_STR, F_EXTATTR_STR_TO_NS,
	F_ACL_COPY_ENTRY, F_ACL_COPY_EXT, F_ACL_COPY_INT,
	F_COUNT
};

static const char *g_fname[F_COUNT] = {
	"mac_set_fd", "mac_set_file", "mac_set_link", "mac_set_proc",
	"acl_delete_def_file", "acl_delete_def_link_np",
	"acl_delete_file_np", "acl_delete_link_np", "acl_delete_fd_np",
	"extattr_namespace_to_string", "extattr_string_to_namespace",
	"acl_copy_entry", "acl_copy_ext", "acl_copy_int"
};

static unsigned long long g_cases[F_COUNT];
static unsigned long long g_fails[F_COUNT];
static int g_printed;
static volatile int g_curfn = -1;

static unsigned long long
print_table(void)
{
	unsigned long long tc = 0, tf = 0;
	int i;

	printf("%-30s %14s %10s\n", "function", "cases", "failures");
	printf("---------------------------------------------------------\n");
	for (i = 0; i < F_COUNT; i++) {
		printf("%-30s %14llu %10llu\n", g_fname[i], g_cases[i],
		    g_fails[i]);
		tc += g_cases[i];
		tf += g_fails[i];
	}
	printf("---------------------------------------------------------\n");
	printf("%-30s %14llu %10llu\n", "TOTAL", tc, tf);
	return (tf);
}

/*
 * A port that faults on an input the original handles is a failure like any
 * other; report it with the table instead of dying silently.
 */
static void
crash_handler(int sig)
{

	printf("\nFAIL: the port faulted (signal %d) while exercising %s\n",
	    sig, (g_curfn >= 0 && g_curfn < F_COUNT) ? g_fname[g_curfn] :
	    "<unknown>");
	g_fails[(g_curfn >= 0 && g_curfn < F_COUNT) ? g_curfn : 0]++;
	print_table();
	printf("\nFAIL: port diverges from the oracle\n");
	fflush(NULL);
	_exit(1);
}

static void
fail(int f, const char *why, const Side &p, const Side &r)
{
	int m;

	g_fails[f]++;
	if (g_printed >= 25)
		return;
	g_printed++;
	fprintf(stderr, "FAIL %s: %s\n", g_fname[f], why);
	fprintf(stderr, "     port: ret=%lld errno=%d helper-calls=%d\n",
	    p.ret, p.err, p.n);
	fprintf(stderr, "     ref : ret=%lld errno=%d helper-calls=%d\n",
	    r.ret, r.err, r.n);
	m = p.n < r.n ? r.n : p.n;
	if (m > MAXLOG)
		m = MAXLOG;
	for (int i = 0; i < m; i++) {
		if (i < p.n)
			fprintf(stderr, "     port[%d] id=%d pcls=%d off=%lld "
			    "i1=%lld i2=%lld ret=%lld str=%d/%u\n", i,
			    p.log[i].id, p.log[i].pcls, p.log[i].poff,
			    p.log[i].i1, p.log[i].i2, p.log[i].ret,
			    p.log[i].has_str, p.log[i].slen);
		if (i < r.n)
			fprintf(stderr, "     ref [%d] id=%d pcls=%d off=%lld "
			    "i1=%lld i2=%lld ret=%lld str=%d/%u\n", i,
			    r.log[i].id, r.log[i].pcls, r.log[i].poff,
			    r.log[i].i1, r.log[i].i2, r.log[i].ret,
			    r.log[i].has_str, r.log[i].slen);
	}
}

static const char *
sides_diff(const Side &p, const Side &r)
{
	int m;

	if (p.ret != r.ret)
		return ("return value differs");
	if (p.err != r.err)
		return ("errno differs");
	if (p.n != r.n)
		return ("number of external helper calls differs");
	m = p.n < MAXLOG ? p.n : MAXLOG;
	for (int i = 0; i < m; i++) {
		const Call &a = p.log[i];
		const Call &b = r.log[i];

		if (a.id != b.id)
			return ("wrong external helper called");
		if (a.pcls != b.pcls || a.poff != b.poff)
			return ("pointer passed to helper differs");
		if (a.i1 != b.i1 || a.i2 != b.i2)
			return ("argument passed to helper differs");
		if (a.ret != b.ret)
			return ("helper result differs");
		if (a.has_str != b.has_str || a.slen != b.slen)
			return ("string passed to helper differs");
		if (a.has_str) {
			size_t n = a.slen < STRCAP ? a.slen : STRCAP;

			if (memcmp(a.str, b.str, n) != 0)
				return ("string passed to helper differs");
		}
	}
	return (NULL);
}

static void
judge(int f, const Side &p, const Side &r, const char *extra)
{
	const char *why;

	g_cases[f]++;
	why = sides_diff(p, r);
	if (why == NULL)
		why = extra;
	if (why != NULL)
		fail(f, why, p, r);
}

/* ------------------------------------------------------------------ */
/* Byte-string inputs						      */
/* ------------------------------------------------------------------ */

struct Bytes {
	unsigned char	b[40];
	size_t		n;
};

static Bytes
mk(const char *s)
{
	Bytes x;

	x.n = strlen(s);
	memcpy(x.b, s, x.n);
	return (x);
}

static Bytes
mkn(const void *s, size_t n)
{
	Bytes x;

	x.n = n;
	memcpy(x.b, s, n);
	return (x);
}

/* Writes s into the region at OFF and NUL-terminates it. */
static char *
put_str(Reg &r, const Bytes &s)
{

	memcpy(r.b + OFF, s.b, s.n);
	r.b[OFF + s.n] = '\0';
	return ((char *)(r.b + OFF));
}

/* ------------------------------------------------------------------ */
/* mac_set.c							      */
/* ------------------------------------------------------------------ */

static char g_shared[8] = { 'x', 'y', 0, 0, 0, 0, 0, 0 };

static void
build_label(Reg &r, int labelmode, unsigned long buflen, MAC **out)
{
	MAC *m;

	if (labelmode == 0) {
		*out = NULL;
		return;
	}
	m = new (r.b + OFF) MAC();
	m->m_buflen = (size_t)buflen;
	m->m_string = (labelmode == 2) ? g_shared : NULL;
	*out = m;
}

static void
run_mac_set_fd(int fd, int labelmode, unsigned long buflen)
{
	g_curfn = F_MAC_SET_FD;
	Reg a, b;
	MAC *la, *lb;
	Side sp, sr;

	reg_init(a);
	reg_init(b);
	build_label(a, labelmode, buflen, &la);
	build_label(b, labelmode, buflen, &lb);

	begin_side(a.b, NULL);
	long long rp = P::mac_set_fd(fd, la);
	end_side(sp, rp);

	begin_side(b.b, NULL);
	long long rr = ref_mac_set_fd(fd, lb);
	end_side(sr, rr);

	judge(F_MAC_SET_FD, sp, sr,
	    reg_eq(a, b) ? NULL : "label buffer differs");
}

static void
run_mac_set_proc(int labelmode, unsigned long buflen)
{
	g_curfn = F_MAC_SET_PROC;
	Reg a, b;
	MAC *la, *lb;
	Side sp, sr;

	reg_init(a);
	reg_init(b);
	build_label(a, labelmode, buflen, &la);
	build_label(b, labelmode, buflen, &lb);

	begin_side(a.b, NULL);
	long long rp = P::mac_set_proc(la);
	end_side(sp, rp);

	begin_side(b.b, NULL);
	long long rr = ref_mac_set_proc(lb);
	end_side(sr, rr);

	judge(F_MAC_SET_PROC, sp, sr,
	    reg_eq(a, b) ? NULL : "label buffer differs");
}

/* which: 0 = mac_set_file, 1 = mac_set_link */
static void
run_mac_set_path(int which, int pathnull, const Bytes &path, int labelmode,
    unsigned long buflen)
{
	Reg pa, pb, qa, qb;
	MAC *la, *lb;
	char *ppa, *ppb;
	Side sp, sr;
	const char *extra;
	int f = which == 0 ? F_MAC_SET_FILE : F_MAC_SET_LINK;

	g_curfn = f;
	reg_init(pa);
	reg_init(pb);
	reg_init(qa);
	reg_init(qb);
	ppa = pathnull ? NULL : put_str(pa, path);
	ppb = pathnull ? NULL : put_str(pb, path);
	build_label(qa, labelmode, buflen, &la);
	build_label(qb, labelmode, buflen, &lb);

	begin_side(qa.b, pa.b);
	long long rp = which == 0 ? P::mac_set_file(ppa, la)
				  : P::mac_set_link(ppa, la);
	end_side(sp, rp);

	begin_side(qb.b, pb.b);
	long long rr = which == 0 ? ref_mac_set_file(ppb, lb)
				  : ref_mac_set_link(ppb, lb);
	end_side(sr, rr);

	extra = NULL;
	if (!reg_eq(qa, qb))
		extra = "label buffer differs";
	else if (!reg_eq(pa, pb))
		extra = "path buffer differs";
	judge(f, sp, sr, extra);
}

/* ------------------------------------------------------------------ */
/* acl_delete.c							      */
/* ------------------------------------------------------------------ */

/*
 * which: 0 acl_delete_def_file, 1 acl_delete_def_link_np,
 *        2 acl_delete_file_np,  3 acl_delete_link_np
 */
static void
run_acl_delete_path(int which, int pathnull, const Bytes &path,
    P::acl_type_t type)
{
	Reg a, b;
	char *pa, *pb;
	Side sp, sr;
	long long rp, rr;
	static const int fmap[4] = {
		F_ACL_DELETE_DEF_FILE, F_ACL_DELETE_DEF_LINK_NP,
		F_ACL_DELETE_FILE_NP, F_ACL_DELETE_LINK_NP
	};

	g_curfn = fmap[which];
	reg_init(a);
	reg_init(b);
	pa = pathnull ? NULL : put_str(a, path);
	pb = pathnull ? NULL : put_str(b, path);

	begin_side(a.b, NULL);
	switch (which) {
	case 0: rp = P::acl_delete_def_file(pa); break;
	case 1: rp = P::acl_delete_def_link_np(pa); break;
	case 2: rp = P::acl_delete_file_np(pa, type); break;
	default: rp = P::acl_delete_link_np(pa, type); break;
	}
	end_side(sp, rp);

	begin_side(b.b, NULL);
	switch (which) {
	case 0: rr = ref_acl_delete_def_file(pb); break;
	case 1: rr = ref_acl_delete_def_link_np(pb); break;
	case 2: rr = ref_acl_delete_file_np(pb, type); break;
	default: rr = ref_acl_delete_link_np(pb, type); break;
	}
	end_side(sr, rr);

	judge(fmap[which], sp, sr,
	    reg_eq(a, b) ? NULL : "path buffer differs");
}

static void
run_acl_delete_fd(int filedes, P::acl_type_t type)
{
	g_curfn = F_ACL_DELETE_FD_NP;
	Side sp, sr;

	begin_side(NULL, NULL);
	long long rp = P::acl_delete_fd_np(filedes, type);
	end_side(sp, rp);

	begin_side(NULL, NULL);
	long long rr = ref_acl_delete_fd_np(filedes, type);
	end_side(sr, rr);

	judge(F_ACL_DELETE_FD_NP, sp, sr, NULL);
}

/* ------------------------------------------------------------------ */
/* extattr.c							      */
/* ------------------------------------------------------------------ */

static char g_canary_obj[1] = { 0 };
static char *const CANARY = g_canary_obj;

static void
run_ns_to_str(int attrnamespace, int strnull)
{
	g_curfn = F_EXTATTR_NS_TO_STR;
	Reg a, b;
	char **pa = NULL, **pb = NULL;
	Side sp, sr;
	const char *extra;

	reg_init(a);
	reg_init(b);
	if (!strnull) {
		pa = new (a.b + OFF) charp(CANARY);
		pb = new (b.b + OFF) charp(CANARY);
	}

	begin_side(a.b, NULL);
	long long rp = P::extattr_namespace_to_string(attrnamespace, pa);
	end_side(sp, rp);

	begin_side(b.b, NULL);
	long long rr = ref_extattr_namespace_to_string(attrnamespace, pb);
	end_side(sr, rr);

	extra = NULL;
	if (strnull) {
		if (!reg_eq(a, b))
			extra = "buffer differs";
	} else {
		/*
		 * Everything except the returned pointer slot has to match
		 * byte for byte; the slot itself is compared by content.
		 */
		char *va = *pa;
		char *vb = *pb;
		bool wa = (va != CANARY);
		bool wb = (vb != CANARY);

		if (!reg_eq_except(a, b, OFF, sizeof(char *)))
			extra = "buffer outside the pointer slot differs";
		if (extra == NULL && wa != wb)
			extra = wa ? "port wrote *string, oracle did not"
				   : "oracle wrote *string, port did not";
		if (extra == NULL && wa) {
			if ((va == NULL) != (vb == NULL))
				extra = "one *string is NULL";
			else if (va != NULL && strcmp(va, vb) != 0)
				extra = "*string contents differ";
		}
		if (wa && va != NULL)
			free(va);
		if (wb && vb != NULL)
			free(vb);
	}
	judge(F_EXTATTR_NS_TO_STR, sp, sr, extra);
}

static void
run_str_to_ns(const Bytes &s, int nsnull)
{
	g_curfn = F_EXTATTR_STR_TO_NS;
	Reg ia, ib, oa, ob;
	int *na = NULL, *nb = NULL;
	char *sa, *sb;
	Side sp, sr;
	const char *extra;

	reg_init(ia);
	reg_init(ib);
	reg_init(oa);
	reg_init(ob);
	sa = put_str(ia, s);
	sb = put_str(ib, s);
	if (!nsnull) {
		na = new (oa.b + OFF) int(0x7f7f7f7f);
		nb = new (ob.b + OFF) int(0x7f7f7f7f);
	}

	begin_side(oa.b, ia.b);
	long long rp = P::extattr_string_to_namespace(sa, na);
	end_side(sp, rp);

	begin_side(ob.b, ib.b);
	long long rr = ref_extattr_string_to_namespace(sb, nb);
	end_side(sr, rr);

	extra = NULL;
	if (!reg_eq(oa, ob))
		extra = "output buffer differs";
	else if (!reg_eq(ia, ib))
		extra = "input buffer differs";
	judge(F_EXTATTR_STR_TO_NS, sp, sr, extra);
}

/* ------------------------------------------------------------------ */
/* acl_copy.c							      */
/* ------------------------------------------------------------------ */

/*
 * ptrmode: 0 both NULL, 1 src NULL, 2 dest NULL, 3 src == dest,
 *          4 two distinct entries
 */
static void
run_copy_entry(int ptrmode, const AE &dv, const AE &sv)
{
	g_curfn = F_ACL_COPY_ENTRY;
	Reg da, db, sa, sb;
	Side sp, sr;
	AE *dpa, *dpb, *spa, *spb;
	const char *extra;

	reg_init(da);
	reg_init(db);
	reg_init(sa);
	reg_init(sb);
	AE *oda = new (da.b + OFF) AE(dv);
	AE *odb = new (db.b + OFF) AE(dv);
	AE *osa = new (sa.b + OFF) AE(sv);
	AE *osb = new (sb.b + OFF) AE(sv);

	switch (ptrmode) {
	case 0:
		dpa = NULL; dpb = NULL; spa = NULL; spb = NULL;
		break;
	case 1:
		dpa = oda; dpb = odb; spa = NULL; spb = NULL;
		break;
	case 2:
		dpa = NULL; dpb = NULL; spa = osa; spb = osb;
		break;
	case 3:
		dpa = oda; dpb = odb; spa = oda; spb = odb;
		break;
	default:
		dpa = oda; dpb = odb; spa = osa; spb = osb;
		break;
	}

	begin_side(da.b, sa.b);
	long long rp = P::acl_copy_entry(dpa, spa);
	end_side(sp, rp);

	begin_side(db.b, sb.b);
	long long rr = ref_acl_copy_entry(dpb, spb);
	end_side(sr, rr);

	extra = NULL;
	if (!reg_eq(da, db))
		extra = "destination buffer differs";
	else if (!reg_eq(sa, sb))
		extra = "source buffer differs";
	judge(F_ACL_COPY_ENTRY, sp, sr, extra);
}

static void
run_copy_ext(int bufnull, long size)
{
	g_curfn = F_ACL_COPY_EXT;
	Reg a, b;
	void *pa, *pb;
	Side sp, sr;

	reg_init(a);
	reg_init(b);
	pa = bufnull ? NULL : (void *)(a.b + OFF);
	pb = bufnull ? NULL : (void *)(b.b + OFF);

	begin_side(a.b, NULL);
	long long rp = P::acl_copy_ext(pa, NULL, (P::ssize_t)size);
	end_side(sp, rp);

	begin_side(b.b, NULL);
	long long rr = ref_acl_copy_ext(pb, NULL, (P::ssize_t)size);
	end_side(sr, rr);

	judge(F_ACL_COPY_EXT, sp, sr,
	    reg_eq(a, b) ? NULL : "buffer differs");
}

/* Offset of p inside r; -1 for NULL, -2 for a foreign pointer. */
static long long
ret_off(const void *p, const Reg &r)
{
	const unsigned char *q = (const unsigned char *)p;

	if (p == NULL)
		return (-1);
	if (q >= r.b && q < r.b + RSZ)
		return ((long long)(q - r.b));
	return (-2);
}

static void
run_copy_int(int bufnull)
{
	g_curfn = F_ACL_COPY_INT;
	Reg a, b;
	const void *pa, *pb;
	Side sp, sr;

	reg_init(a);
	reg_init(b);
	pa = bufnull ? NULL : (const void *)(a.b + OFF);
	pb = bufnull ? NULL : (const void *)(b.b + OFF);

	begin_side(a.b, NULL);
	P::acl_t vp = P::acl_copy_int(pa);
	end_side(sp, ret_off(vp, a));

	begin_side(b.b, NULL);
	P::acl_t vr = ref_acl_copy_int(pb);
	end_side(sr, ret_off(vr, b));

	judge(F_ACL_COPY_INT, sp, sr,
	    reg_eq(a, b) ? NULL : "buffer differs");
}

/* ------------------------------------------------------------------ */
/* Random generators (fixed seed)				      */
/* ------------------------------------------------------------------ */

static uint64_t g_state = 0x243F6A8885A308D3ULL;

static uint64_t
nrand(void)
{
	uint64_t z;

	g_state += 0x9E3779B97F4A7C15ULL;
	z = g_state;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return (z ^ (z >> 31));
}

static uint32_t
r32(void)
{

	return ((uint32_t)(nrand() >> 32));
}

static unsigned
rr(unsigned n)
{

	return ((unsigned)(r32() % n));
}

static int
rand_int_edge(void)
{

	switch (rr(10)) {
	case 0: return (0);
	case 1: return (1);
	case 2: return (-1);
	case 3: return (2);
	case 4: return (3);
	case 5: return (4);
	case 6: return (INT_MIN);
	case 7: return (INT_MAX);
	default: return ((int)r32());
	}
}

static int
rand_ns(void)
{

	switch (rr(12)) {
	case 0: return (0);
	case 1: return (1);
	case 2: return (1);
	case 3: return (2);
	case 4: return (2);
	case 5: return (3);
	case 6: return (-1);
	case 7: return (-2);
	case 8: return (INT_MIN);
	case 9: return (INT_MAX);
	default: return ((int)r32());
	}
}

static long
rand_ssize(void)
{

	switch (rr(8)) {
	case 0: return (0);
	case 1: return (1);
	case 2: return (-1);
	case 3: return (LONG_MIN);
	case 4: return (LONG_MAX);
	case 5: return (24);
	default: return ((long)(((uint64_t)r32() << 32) | r32()));
	}
}

static uint32_t
rand_tag(void)
{

	switch (rr(9)) {
	case 0: return (0);
	case 1: return (1);
	case 2: return (2);
	case 3: return (3);
	case 4: return (4);
	case 5: return (0xffffffffu);
	case 6: return (0xfffffffeu);
	case 7: return (0x80000000u);
	default: return (r32());
	}
}

static AE
rand_ae(void)
{
	AE e;

	e.ae_tag = rand_tag();
	e.ae_id = r32();
	e.ae_perm = r32();
	e.ae_entry_type = (P::acl_entry_type_t)r32();
	e.ae_flags = (P::acl_flag_t)r32();
	return (e);
}

static const char *g_word[] = {
	"", "u", "s", "e", "user", "system", "empty",
	"us", "use", "usr", "userr", "users", "usee", "uses",
	"sys", "syst", "syste", "systen", "systems", "systemm", "sysem",
	"USER", "SYSTEM", "User", "System", "uSer", "systeM",
	"user ", " user", "user\t", "sytem", "systm", "usera",
	"a", "z", "0", "/", "..", "/tmp/x", "empt", "emptyy"
};
static const unsigned g_nword = sizeof(g_word) / sizeof(g_word[0]);

static unsigned char
rand_byte(void)
{

	switch (rr(8)) {
	case 0: return (0x00);
	case 1: return (0x01);
	case 2: return (0x7f);
	case 3: return (0x80);
	case 4: return (0xfe);
	case 5: return (0xff);
	case 6: return ((unsigned char)"userstym"[rr(8)]);
	default: return ((unsigned char)r32());
	}
}

static Bytes
rand_bytes(void)
{
	Bytes x;
	unsigned mode = rr(10);

	if (mode < 4) {
		/* an exact or near-exact keyword */
		x = mk(g_word[rr(g_nword)]);
	} else if (mode < 7) {
		/* keyword with one byte perturbed, inserted or dropped */
		unsigned op = rr(3);

		x = mk(g_word[rr(g_nword)]);
		if (op == 0 && x.n > 0) {
			x.b[rr((unsigned)x.n)] = rand_byte();
		} else if (op == 1 && x.n < 24) {
			size_t at = x.n == 0 ? 0 : rr((unsigned)x.n + 1);

			memmove(x.b + at + 1, x.b + at, x.n - at);
			x.b[at] = rand_byte();
			x.n++;
		} else if (x.n > 0) {
			size_t at = rr((unsigned)x.n);

			memmove(x.b + at, x.b + at + 1, x.n - at - 1);
			x.n--;
		}
	} else {
		/* free-form bytes, including embedded NULs and high bits */
		size_t n = rr(13);

		for (size_t i = 0; i < n; i++)
			x.b[i] = rand_byte();
		x.n = n;
	}
	return (x);
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases					      */
/* ------------------------------------------------------------------ */

static void
edge_cases(void)
{
	static const int fds[] = {
		INT_MIN, -2, -1, 0, 1, 2, 3, 255, 256, 65535, INT_MAX
	};
	static const P::acl_type_t types[] = {
		INT_MIN, -2, -1, 0, 1, 2, 3, 4, 5, 6, INT_MAX
	};
	static const unsigned long buflens[] = {
		0, 1, 2, 127, 128, 255, 256, 4095, 4096
	};
	const unsigned nfd = sizeof(fds) / sizeof(fds[0]);
	const unsigned ntype = sizeof(types) / sizeof(types[0]);
	const unsigned nbuf = sizeof(buflens) / sizeof(buflens[0]);
	Bytes paths[32];
	unsigned np = 0;
	unsigned i, j, k;

	/* mac_set_fd / mac_set_proc */
	for (i = 0; i < nfd; i++)
		for (j = 0; j < 3; j++)
			for (k = 0; k < nbuf; k++)
				run_mac_set_fd(fds[i], (int)j, buflens[k]);
	for (j = 0; j < 3; j++)
		for (k = 0; k < nbuf; k++)
			run_mac_set_proc((int)j, buflens[k]);

	/* paths: empty, one byte, NUL-heavy, high-bit, boundary lengths */
	paths[np++] = mk("");
	paths[np++] = mk("a");
	paths[np++] = mk("/");
	paths[np++] = mk("/tmp/file");
	paths[np++] = mkn("\x00", 1);
	paths[np++] = mkn("a\x00b", 3);
	paths[np++] = mkn("\x00\x00\x00\x00", 4);
	paths[np++] = mkn("\x80", 1);
	paths[np++] = mkn("\xff", 1);
	paths[np++] = mkn("\x7f", 1);
	paths[np++] = mkn("\x80\x81\xfe\xff", 4);
	paths[np++] = mkn("/tmp/\xff\x80", 7);
	{
		unsigned char big[24];

		for (i = 0; i < 24; i++)
			big[i] = (unsigned char)(0x80 + (i & 0x3f));
		paths[np++] = mkn(big, 1);
		paths[np++] = mkn(big, 2);
		paths[np++] = mkn(big, 23);
		paths[np++] = mkn(big, 24);
	}
	for (i = 0; i < np; i++) {
		for (j = 0; j < 3; j++) {
			run_mac_set_path(0, 0, paths[i], (int)j, 128);
			run_mac_set_path(1, 0, paths[i], (int)j, 128);
		}
		run_mac_set_path(0, 1, paths[i], 1, 128);
		run_mac_set_path(1, 1, paths[i], 1, 128);
		for (k = 0; k < ntype; k++) {
			run_acl_delete_path(0, 0, paths[i], types[k]);
			run_acl_delete_path(1, 0, paths[i], types[k]);
			run_acl_delete_path(2, 0, paths[i], types[k]);
			run_acl_delete_path(3, 0, paths[i], types[k]);
		}
		run_acl_delete_path(0, 1, paths[i], 0);
		run_acl_delete_path(1, 1, paths[i], 0);
		run_acl_delete_path(2, 1, paths[i], 1);
		run_acl_delete_path(3, 1, paths[i], 2);
	}
	for (i = 0; i < nfd; i++)
		for (k = 0; k < ntype; k++)
			run_acl_delete_fd(fds[i], types[k]);

	/* extattr_namespace_to_string: both sides of every case label */
	for (int ns = -8; ns <= 8; ns++) {
		run_ns_to_str(ns, 0);
		run_ns_to_str(ns, 1);
	}
	run_ns_to_str(INT_MIN, 0);
	run_ns_to_str(INT_MIN, 1);
	run_ns_to_str(INT_MAX, 0);
	run_ns_to_str(INT_MAX, 1);
	run_ns_to_str(0x00000001, 0);
	run_ns_to_str(0x00000002, 0);
	run_ns_to_str(0x00010001, 0);
	run_ns_to_str(0x00010002, 0);

	/* extattr_string_to_namespace */
	for (i = 0; i < g_nword; i++) {
		run_str_to_ns(mk(g_word[i]), 0);
		run_str_to_ns(mk(g_word[i]), 1);
	}
	/* every single byte value as a one-character string */
	for (unsigned v = 0; v <= 0xff; v++) {
		unsigned char c = (unsigned char)v;

		run_str_to_ns(mkn(&c, 1), 0);
	}
	/* "user" and "system" with each position replaced, and off-by-one */
	{
		static const char *kw[2] = { "user", "system" };
		static const unsigned char hb[6] = {
			0x00, 0x7f, 0x80, 0x81, 0xfe, 0xff
		};

		for (i = 0; i < 2; i++) {
			size_t len = strlen(kw[i]);

			for (j = 0; j < len; j++)
				for (k = 0; k < 6; k++) {
					Bytes x = mk(kw[i]);

					x.b[j] = hb[k];
					run_str_to_ns(x, 0);
					run_str_to_ns(x, 1);
				}
			run_str_to_ns(mkn(kw[i], len - 1), 0);
			for (k = 0; k < 6; k++) {
				Bytes s2 = mk(kw[i]);

				s2.b[s2.n++] = hb[k];
				run_str_to_ns(s2, 0);
			}
			{
				Bytes s3 = mk(kw[i]);

				s3.b[s3.n++] = 0x00;
				s3.b[s3.n++] = 0xff;
				s3.b[s3.n++] = 'x';
				run_str_to_ns(s3, 0);
			}
		}
	}
	run_str_to_ns(mkn("", 0), 0);
	run_str_to_ns(mkn("\x00user", 5), 0);
	run_str_to_ns(mkn("user\x00system", 11), 0);
	run_str_to_ns(mkn("system\x00user", 11), 0);

	/* acl_copy_entry: every pointer arrangement x every brand pairing */
	{
		static const uint32_t tags[9] = {
			0, 1, 2, 3, 4, 5, 0xfffffffdu, 0xfffffffeu, 0xffffffffu
		};
		static const uint32_t fills[3] = {
			0x00000000u, 0xffffffffu, 0x5a5aa5a5u
		};

		for (i = 0; i < 9; i++)
			for (j = 0; j < 9; j++)
				for (k = 0; k < 3; k++) {
					AE d, s;

					d.ae_tag = tags[i];
					d.ae_id = fills[k];
					d.ae_perm = ~fills[k];
					d.ae_entry_type =
					    (P::acl_entry_type_t)fills[k];
					d.ae_flags = (P::acl_flag_t)~fills[k];
					s.ae_tag = tags[j];
					s.ae_id = ~fills[k];
					s.ae_perm = fills[k];
					s.ae_entry_type =
					    (P::acl_entry_type_t)~fills[k];
					s.ae_flags = (P::acl_flag_t)fills[k];
					for (int pm = 0; pm < 5; pm++)
						run_copy_entry(pm, d, s);
				}
	}

	/* acl_copy_ext / acl_copy_int */
	{
		static const long sizes[] = {
			LONG_MIN, -2, -1, 0, 1, 2, 63, 64, 65, LONG_MAX
		};

		for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
			run_copy_ext(0, sizes[i]);
			run_copy_ext(1, sizes[i]);
		}
		run_copy_int(0);
		run_copy_int(1);
	}
}

/* ------------------------------------------------------------------ */
/* Randomised sweep						      */
/* ------------------------------------------------------------------ */

static void
sweep(unsigned long iters)
{
	unsigned long it;

	for (it = 0; it < iters; it++) {
		int labelmode = (int)rr(3);
		unsigned long buflen = (unsigned long)r32();
		Bytes path = rand_bytes();
		int pathnull = (rr(16) == 0);
		P::acl_type_t type = rand_int_edge();

		run_mac_set_fd(rand_int_edge(), labelmode, buflen);
		run_mac_set_proc(labelmode, buflen);
		run_mac_set_path(0, pathnull, path, labelmode, buflen);
		run_mac_set_path(1, pathnull, path, (int)rr(3), buflen);

		run_acl_delete_path(0, pathnull, path, type);
		run_acl_delete_path(1, pathnull, path, type);
		run_acl_delete_path(2, pathnull, path, type);
		run_acl_delete_path(3, pathnull, path, rand_int_edge());
		run_acl_delete_fd(rand_int_edge(), rand_int_edge());

		run_ns_to_str(rand_ns(), (int)rr(2));
		run_str_to_ns(rand_bytes(), (int)rr(2));

		run_copy_entry((int)rr(5), rand_ae(), rand_ae());
		if ((it & 3) == 0)
			run_copy_entry(4, rand_ae(), rand_ae());

		run_copy_ext((int)rr(2), rand_ssize());
		run_copy_int((int)rr(2));
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	const unsigned long ITERS = 200000;
	unsigned long long tf;

	signal(SIGSEGV, crash_handler);
	signal(SIGBUS, crash_handler);

	printf("PBSD b0110 differential harness\n");
	printf("hand-written edge cases + %lu randomised iterations "
	    "(fixed seed 0x243F6A8885A308D3)\n\n", ITERS);

	edge_cases();
	sweep(ITERS);

	tf = print_table();
	printf("\n%s\n", tf == 0 ?
	    "PASS: port matches the oracle in every case" :
	    "FAIL: port diverges from the oracle");
	return (tf == 0 ? 0 : 1);
}
