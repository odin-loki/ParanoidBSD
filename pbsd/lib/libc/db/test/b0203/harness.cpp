/*
 * Differential harness for PBSD batch b0203 (dbtest.c helpers).
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.db.test.b0203;

namespace P = pbsd::lib_libc_db_test::b0203;

typedef P::DBT DBT;
typedef P::DB DB;
typedef P::DBTYPE DBTYPE;
typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long u_long;

using P::DB_BTREE;
using P::DB_HASH;
using P::DB_RECNO;

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_IAFTER	4
#define	R_IBEFORE	5
#define	R_LAST		6
#define	R_NEXT		7
#define	R_NOOVERWRITE	8
#define	R_PREV		9
#define	R_SETCURSOR	10

typedef struct {
	unsigned long flags;
	unsigned int cachesize;
	int maxkeypage;
	int minkeypage;
	unsigned int psize;
	int (*compare)(const DBT *, const DBT *);
	size_t (*prefix)(const DBT *, const DBT *);
	int lorder;
} BTREEINFO;

typedef struct {
	unsigned int bsize;
	unsigned int ffactor;
	unsigned int nelem;
	unsigned int cachesize;
	uint32_t (*hash)(const void *, size_t);
	int lorder;
} HASHINFO;

typedef struct {
	unsigned long flags;
	unsigned int cachesize;
	unsigned int psize;
	int lorder;
	size_t reclen;
	unsigned char bval;
	char *bfname;
} RECNOINFO;

extern "C" {
extern DBTYPE type;
extern void *infop;
extern u_long lineno;
extern u_int flags;
extern int ofd;

void ref_compare(DBT *, DBT *);
DBTYPE ref_dbtype(char *);
void ref_dump(DB *, int);
void ref_err(const char *, ...);
void ref_get(DB *, DBT *);
void ref_getdata(DB *, DBT *, DBT *);
void ref_put(DB *, DBT *, DBT *);
void ref_rem(DB *, DBT *);
char *ref_sflags(int);
void ref_synk(DB *);
void *ref_rfile(char *, size_t *);
void ref_seq(DB *, DBT *);
u_int ref_setflags(char *);
void *ref_setinfo(DBTYPE, char *);
void ref_usage(void);
void *ref_xmalloc(char *, size_t);
}

struct Stat {
	const char *name;
	long cases;
	long fails;
};

static constexpr unsigned char GUARD = 0x7f;
static constexpr long SWEEP = 30000;
static std::uint64_t rng = 0xB0203C0DEULL;

static std::uint64_t
nextrand(void)
{
	rng += 0x9E3779B97F4A7C15ULL;
	std::uint64_t z = rng;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::fprintf(stderr, "FAIL %s: %s\n", st.name, msg);
}

static void
sync_ref(u_long ln, u_int fl, int outfd)
{
	lineno = ln;
	flags = fl;
	ofd = outfd;
}

static void
sync_port(u_long ln, u_int fl, int outfd)
{
	P::lineno = ln;
	P::flags = fl;
	P::ofd = outfd;
}

static std::string
read_all(int fd)
{
	std::string out;
	char buf[4096];
	for (;;) {
		ssize_t n = read(fd, buf, sizeof(buf));
		if (n <= 0)
			break;
		out.append(buf, (size_t)n);
	}
	return out;
}

static int
make_pipe_out(void)
{
	int pfd[2];
	if (pipe(pfd) != 0)
		return -1;
	return pfd[1];
}

static int
pipe_read_end(int write_end)
{
	int pfd[2];
	if (pipe(pfd) != 0)
		return -1;
	dup2(pfd[1], write_end);
	close(pfd[1]);
	return pfd[0];
}

struct ExitObs {
	int code;
	std::string err;
};

template <typename Fn>
static ExitObs
run_child(Fn fn)
{
	fflush(nullptr);
	int ep[2];
	pipe(ep);
	pid_t p = fork();
	if (p == 0) {
		dup2(ep[1], STDERR_FILENO);
		close(ep[1]);
		fn();
		_exit(99);
	}
	close(ep[1]);
	ExitObs o;
	o.err = read_all(ep[0]);
	close(ep[0]);
	int st = 0;
	waitpid(p, &st, 0);
	o.code = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
	return o;
}

static bool
exit_same(const ExitObs &a, const ExitObs &b)
{
	return a.code == b.code && a.err == b.err;
}

/* ---------------- mock DB ---------------- */

struct MockState {
	int get_ret = 0;
	int put_ret = 0;
	int del_ret = 0;
	int sync_ret = 0;
	int get_errno = EINVAL;
	int seq_errno = EINVAL;
	u_char get_data[4096]{};
	size_t get_size = 0;
	static constexpr int MAX_SEQ = 64;
	int seq_len = 0;
	int seq_pos = 0;
	int seq_script[MAX_SEQ]{};
	u_char seq_bufs[MAX_SEQ][512]{};
	size_t seq_sizes[MAX_SEQ]{};
};

static MockState g_mock;

static int
mock_get(const DB *, const DBT *, DBT *data, unsigned int)
{
	if (g_mock.get_ret == -1) {
		errno = g_mock.get_errno;
		return -1;
	}
	if (g_mock.get_ret == 1)
		return 1;
	data->data = g_mock.get_data;
	data->size = g_mock.get_size;
	return 0;
}

static int
mock_put(DB *, DBT *, const DBT *, unsigned int)
{
	if (g_mock.put_ret == -1) {
		errno = EINVAL;
		return -1;
	}
	return g_mock.put_ret;
}

static int
mock_del(const DB *, const DBT *, unsigned int)
{
	if (g_mock.del_ret == -1) {
		errno = EINVAL;
		return -1;
	}
	return g_mock.del_ret;
}

static int
mock_seq(DB *, DBT *, DBT *data, unsigned int)
{
	if (g_mock.seq_pos >= g_mock.seq_len)
		return 1;
	int ret = g_mock.seq_script[g_mock.seq_pos];
	if (ret == 0) {
		data->data = g_mock.seq_bufs[g_mock.seq_pos];
		data->size = g_mock.seq_sizes[g_mock.seq_pos];
	} else if (ret == -1)
		errno = g_mock.seq_errno;
	g_mock.seq_pos++;
	return ret;
}

static int
mock_sync(DB *, unsigned int)
{
	if (g_mock.sync_ret == -1) {
		errno = EINVAL;
		return -1;
	}
	return g_mock.sync_ret;
}

static DB
make_db(void)
{
	DB db{};
	db.get = mock_get;
	db.put = mock_put;
	db.del = mock_del;
	db.seq = mock_seq;
	db.sync = mock_sync;
	return db;
}

static void
mock_reset(void)
{
	g_mock = MockState{};
}

static void
mock_seq_push(int ret, const void *data, size_t sz)
{
	int i = g_mock.seq_len++;
	g_mock.seq_script[i] = ret;
	if (ret == 0 && data != nullptr && sz > 0) {
		std::memcpy(g_mock.seq_bufs[i], data, sz);
		g_mock.seq_sizes[i] = sz;
	}
}

template <typename Fn>
static std::string
capture_stdout_ref(Fn fn)
{
	fflush(stdout);
	int pfd[2];
	pipe(pfd);
	int saved = dup(STDOUT_FILENO);
	dup2(pfd[1], STDOUT_FILENO);
	close(pfd[1]);
	fn();
	fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	std::string s = read_all(pfd[0]);
	close(pfd[0]);
	return s;
}

template <typename Fn>
static std::string
capture_stdout_port(Fn fn)
{
	fflush(stdout);
	int pfd[2];
	pipe(pfd);
	int saved = dup(STDOUT_FILENO);
	dup2(pfd[1], STDOUT_FILENO);
	close(pfd[1]);
	fn();
	fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	std::string s = read_all(pfd[0]);
	close(pfd[0]);
	return s;
}

template <typename Fn>
static std::string
capture_ofd_ref(int outfd, Fn fn)
{
	int rd = pipe_read_end(outfd);
	fn();
	close(outfd);
	std::string s = read_all(rd);
	close(rd);
	return s;
}

template <typename Fn>
static std::string
capture_ofd_port(int outfd, Fn fn)
{
	int rd = pipe_read_end(outfd);
	fn();
	close(outfd);
	std::string s = read_all(rd);
	close(rd);
	return s;
}

static std::string
capture_stderr_pair(void (*ref_fn)(void), void (*port_fn)(void))
{
	ExitObs r = run_child(ref_fn);
	ExitObs p = run_child(port_fn);
	if (!exit_same(r, p))
		return "mismatch";
	return r.err;
}

/* ---------------- compare ---------------- */

static Stat st_compare = { "compare", 0, 0 };

static void
test_compare_buf(const u_char *a, size_t asz, const u_char *b, size_t bsz)
{
	st_compare.cases++;
	u_char buf1[512], buf2[512];
	std::memset(buf1, GUARD, sizeof(buf1));
	std::memset(buf2, GUARD, sizeof(buf2));
	size_t c1 = asz < 256 ? asz : 256;
	size_t c2 = bsz < 256 ? bsz : 256;
	std::memcpy(buf1 + 8, a, c1);
	std::memcpy(buf2 + 8, b, c2);
	DBT d1{buf1 + 8, c1};
	DBT d2{buf2 + 8, c2};
	auto rout = capture_stdout_ref([&] { ref_compare(&d1, &d2); });
	auto pout = capture_stdout_port([&] { P::compare(&d1, &d2); });
	if (rout != pout)
		fail(st_compare, "stdout mismatch");
}

static void
run_compare_tests(void)
{
	test_compare_buf((const u_char *)"", 0, (const u_char *)"", 0);
	test_compare_buf((const u_char *)"a", 1, (const u_char *)"a", 1);
	test_compare_buf((const u_char *)"a", 1, (const u_char *)"b", 1);
	test_compare_buf((const u_char *)"abc", 3, (const u_char *)"ab", 2);
	test_compare_buf((const u_char *)"ab", 2, (const u_char *)"abc", 3);
	u_char hb[] = {0x80, 0xff, 0x00, 0x7f};
	test_compare_buf(hb, 4, hb, 4);
	test_compare_buf(hb, 4, hb, 3);
	hb[2] = 0x01;
	test_compare_buf(hb, 4, hb, 4);

	for (long i = 0; i < SWEEP; i++) {
		u_char a[64], b[64];
		size_t asz = (size_t)(nextrand() % 65);
		size_t bsz = (size_t)(nextrand() % 65);
		for (size_t j = 0; j < asz; j++)
			a[j] = (u_char)(nextrand() & 0xff);
		for (size_t j = 0; j < bsz; j++)
			b[j] = (u_char)(nextrand() & 0xff);
		test_compare_buf(a, asz, b, bsz);
	}
}

/* ---------------- sflags / dbtype / setflags ---------------- */

static Stat st_sflags = { "sflags", 0, 0 };
static Stat st_dbtype = { "dbtype", 0, 0 };
static Stat st_setflags = { "setflags", 0, 0 };

static void
run_sflags_tests(void)
{
	int vals[] = {R_CURSOR, R_FIRST, R_IAFTER, R_IBEFORE, R_LAST, R_NEXT,
	    R_NOOVERWRITE, R_PREV, R_SETCURSOR, 0, 2, 11, 99, -1, 255};
	for (int v : vals) {
		st_sflags.cases++;
		const char *r = ref_sflags(v);
		const char *p = P::sflags(v);
		if (std::strcmp(r, p) != 0)
			fail(st_sflags, "string mismatch");
	}
	for (long i = 0; i < SWEEP; i++) {
		st_sflags.cases++;
		int v = (int)(nextrand() & 0xff);
		if (std::strcmp(ref_sflags(v), P::sflags(v)) != 0)
			fail(st_sflags, "sweep mismatch");
	}
}

static void
run_dbtype_tests(void)
{
	const char *types[] = {"btree", "hash", "recno"};
	for (const char *t : types) {
		st_dbtype.cases++;
		char rb[16], pb[16];
		std::strcpy(rb, t);
		std::strcpy(pb, t);
		if (ref_dbtype(rb) != (DBTYPE)P::dbtype(pb))
			fail(st_dbtype, "type mismatch");
	}
	st_dbtype.cases++;
	ExitObs r = run_child([] { char s[] = "bogus"; ref_dbtype(s); });
	ExitObs p = run_child([] { char s[] = "bogus"; P::dbtype(s); });
	if (!exit_same(r, p))
		fail(st_dbtype, "unknown type exit");
	for (long i = 0; i < SWEEP; i++) {
		st_dbtype.cases++;
		char rb[32], pb[32];
		size_t n = (size_t)(nextrand() % 8);
		for (size_t j = 0; j < n; j++)
			rb[j] = pb[j] = (char)('a' + (nextrand() % 26));
		rb[n] = pb[n] = '\0';
		if (n == 5 && std::memcmp(rb, "btree", 6) == 0)
			continue;
		if (n == 4 && std::memcmp(rb, "hash", 5) == 0)
			continue;
		if (n == 5 && std::memcmp(rb, "recno", 6) == 0)
			continue;
		ExitObs rr = run_child([&] { ref_dbtype(rb); });
		ExitObs pp = run_child([&] { P::dbtype(pb); });
		if (!exit_same(rr, pp))
			fail(st_dbtype, "sweep unknown");
	}
}

static void
run_setflags_tests(void)
{
	const char *names[] = {"R_CURSOR", "R_FIRST", "R_IAFTER", "R_IBEFORE",
	    "R_LAST", "R_NEXT", "R_NOOVERWRITE", "R_PREV", "R_SETCURSOR"};
	for (const char *n : names) {
		st_setflags.cases++;
		char rb[64], pb[64];
		std::strcpy(rb, n);
		std::strcpy(pb, n);
		sync_ref(7, 0, STDOUT_FILENO);
		sync_port(7, 0, STDOUT_FILENO);
		if (ref_setflags(rb) != P::setflags(pb))
			fail(st_setflags, "flag value");
	}
	st_setflags.cases++;
	{
		char rb[] = "  R_NEXT\n", pb[] = "  R_NEXT\n";
		sync_ref(1, 0, STDOUT_FILENO);
		sync_port(1, 0, STDOUT_FILENO);
		if (ref_setflags(rb) != P::setflags(pb))
			fail(st_setflags, "ws newline");
	}
	st_setflags.cases++;
	{
		char rb[] = "\n", pb[] = "\n";
		if (ref_setflags(rb) != 0 || P::setflags(pb) != 0)
			fail(st_setflags, "empty");
	}
	st_setflags.cases++;
	ExitObs r = run_child([] {
		char s[] = "R_BOGUS";
		lineno = 99;
		ref_setflags(s);
	});
	ExitObs p = run_child([] {
		char s[] = "R_BOGUS";
		P::lineno = 99;
		P::setflags(s);
	});
	if (!exit_same(r, p))
		fail(st_setflags, "unknown flag");
	for (long i = 0; i < SWEEP; i++) {
		st_setflags.cases++;
		char rb[48], pb[48];
		unsigned pick = (unsigned)(nextrand() % 12);
		if (pick < 9) {
			std::snprintf(rb, sizeof(rb), "%s", names[pick]);
			std::snprintf(pb, sizeof(pb), "%s", names[pick]);
			sync_ref(3, 0, STDOUT_FILENO);
			sync_port(3, 0, STDOUT_FILENO);
			if (ref_setflags(rb) != P::setflags(pb))
				fail(st_setflags, "sweep known");
		} else if (pick == 9) {
			rb[0] = pb[0] = '\n';
			rb[1] = pb[1] = '\0';
			if (ref_setflags(rb) != P::setflags(pb))
				fail(st_setflags, "sweep nl");
		} else {
			std::snprintf(rb, sizeof(rb), "X%lu", (unsigned long)i);
			std::snprintf(pb, sizeof(pb), "X%lu", (unsigned long)i);
			sync_ref(4, 0, STDOUT_FILENO);
			sync_port(4, 0, STDOUT_FILENO);
			ExitObs rr = run_child([&] { ref_setflags(rb); });
			ExitObs pp = run_child([&] { P::setflags(pb); });
			if (!exit_same(rr, pp))
				fail(st_setflags, "sweep bad");
		}
	}
}

/* ---------------- setinfo ---------------- */

static Stat st_setinfo = { "setinfo", 0, 0 };

static bool
btree_same(void *r, void *p)
{
	auto *a = (BTREEINFO *)r;
	auto *b = (BTREEINFO *)p;
	return a->flags == b->flags && a->cachesize == b->cachesize &&
	    a->maxkeypage == b->maxkeypage && a->minkeypage == b->minkeypage &&
	    a->lorder == b->lorder && a->psize == b->psize;
}

static bool
hash_same(void *r, void *p)
{
	auto *a = (HASHINFO *)r;
	auto *b = (HASHINFO *)p;
	return a->bsize == b->bsize && a->ffactor == b->ffactor &&
	    a->nelem == b->nelem && a->cachesize == b->cachesize &&
	    a->lorder == b->lorder;
}

static bool
recno_same(void *r, void *p)
{
	auto *a = (RECNOINFO *)r;
	auto *b = (RECNOINFO *)p;
	return a->flags == b->flags && a->cachesize == b->cachesize &&
	    a->lorder == b->lorder && a->reclen == b->reclen &&
	    a->bval == b->bval && a->psize == b->psize;
}

static void
test_setinfo_pair(DBTYPE dt, const char *spec, bool (*same)(void *, void *))
{
	st_setinfo.cases++;
	char rb[64], pb[64];
	std::strcpy(rb, spec);
	std::strcpy(pb, spec);
	void *vr = ref_setinfo(dt, rb);
	void *vp = P::setinfo(dt, pb);
	if (!same(vr, vp))
		fail(st_setinfo, spec);
}

static void
run_setinfo_tests(void)
{
	test_setinfo_pair(DB_BTREE, "flags=7", btree_same);
	test_setinfo_pair(DB_BTREE, "cachesize=8192", btree_same);
	test_setinfo_pair(DB_BTREE, "maxkeypage=4", btree_same);
	test_setinfo_pair(DB_BTREE, "minkeypage=2", btree_same);
	test_setinfo_pair(DB_BTREE, "lorder=1234", btree_same);
	test_setinfo_pair(DB_BTREE, "psize=512", btree_same);
	test_setinfo_pair(DB_HASH, "bsize=4096", hash_same);
	test_setinfo_pair(DB_HASH, "ffactor=32", hash_same);
	test_setinfo_pair(DB_HASH, "nelem=100", hash_same);
	test_setinfo_pair(DB_HASH, "cachesize=16384", hash_same);
	test_setinfo_pair(DB_HASH, "lorder=4321", hash_same);
	test_setinfo_pair(DB_RECNO, "flags=3", recno_same);
	test_setinfo_pair(DB_RECNO, "cachesize=2048", recno_same);
	test_setinfo_pair(DB_RECNO, "lorder=999", recno_same);
	test_setinfo_pair(DB_RECNO, "reclen=80", recno_same);
	test_setinfo_pair(DB_RECNO, "bval=10", recno_same);
	test_setinfo_pair(DB_RECNO, "psize=1024", recno_same);

	const char *bfields[] = {"flags", "cachesize", "maxkeypage", "minkeypage",
	    "lorder", "psize"};
	const char *hfields[] = {"bsize", "ffactor", "nelem", "cachesize",
	    "lorder"};
	const char *rfields[] = {"flags", "cachesize", "lorder", "reclen", "bval",
	    "psize"};
	for (long i = 0; i < SWEEP; i++) {
		st_setinfo.cases++;
		char rb[64], pb[64];
		unsigned kind = (unsigned)(nextrand() % 3);
		unsigned val = (unsigned)(nextrand() % 200000);
		if (kind == 0) {
			std::snprintf(rb, sizeof(rb), "%s=%u", bfields[nextrand() % 6],
			    val);
			std::strcpy(pb, rb);
			void *vr = ref_setinfo(DB_BTREE, rb);
			void *vp = P::setinfo(DB_BTREE, pb);
			if (!btree_same(vr, vp))
				fail(st_setinfo, "sweep btree");
		} else if (kind == 1) {
			std::snprintf(rb, sizeof(rb), "%s=%u", hfields[nextrand() % 5],
			    val);
			std::strcpy(pb, rb);
			void *vr = ref_setinfo(DB_HASH, rb);
			void *vp = P::setinfo(DB_HASH, pb);
			if (!hash_same(vr, vp))
				fail(st_setinfo, "sweep hash");
		} else {
			std::snprintf(rb, sizeof(rb), "%s=%u", rfields[nextrand() % 6],
			    val);
			std::strcpy(pb, rb);
			void *vr = ref_setinfo(DB_RECNO, rb);
			void *vp = P::setinfo(DB_RECNO, pb);
			if (!recno_same(vr, vp))
				fail(st_setinfo, "sweep recno");
		}
	}
	st_setinfo.cases++;
	ExitObs r = run_child([] { char s[] = "nosep"; ref_setinfo(DB_BTREE, s); });
	ExitObs p = run_child([] { char s[] = "nosep"; P::setinfo(DB_BTREE, s); });
	if (!exit_same(r, p))
		fail(st_setinfo, "nosep");
	st_setinfo.cases++;
	r = run_child([] { char s[] = "flags=x"; ref_setinfo(DB_BTREE, s); });
	p = run_child([] { char s[] = "flags=x"; P::setinfo(DB_BTREE, s); });
	if (!exit_same(r, p))
		fail(st_setinfo, "nondigit");
	st_setinfo.cases++;
	r = run_child([] { char s[] = "bogus=1"; ref_setinfo(DB_BTREE, s); });
	p = run_child([] { char s[] = "bogus=1"; P::setinfo(DB_BTREE, s); });
	if (!exit_same(r, p))
		fail(st_setinfo, "unknown");
}

/* ---------------- xmalloc / rfile ---------------- */

static Stat st_xmalloc = { "xmalloc", 0, 0 };
static Stat st_rfile = { "rfile", 0, 0 };

static void
run_xmalloc_tests(void)
{
	const char *samples[] = {"", "a", "\0b", "\xff\x80"};
	const size_t lens[] = {0, 1, 2, 2};
	for (size_t i = 0; i < 4; i++) {
		st_xmalloc.cases++;
		char src[16];
		std::memcpy(src, samples[i], lens[i]);
		void *r = ref_xmalloc(src, lens[i]);
		void *p = P::xmalloc(src, lens[i]);
		if (std::memcmp(r, p, lens[i]) != 0)
			fail(st_xmalloc, "content");
		free(r);
		free(p);
	}
	for (long i = 0; i < SWEEP; i++) {
		st_xmalloc.cases++;
		size_t n = (size_t)(nextrand() % 128);
		std::vector<char> src(n);
		for (size_t j = 0; j < n; j++)
			src[j] = (char)(nextrand() & 0xff);
		void *r = ref_xmalloc(src.data(), n);
		void *p = P::xmalloc(src.data(), n);
		if (std::memcmp(r, p, n) != 0)
			fail(st_xmalloc, "sweep");
		free(r);
		free(p);
	}
}

static std::string
make_temp_file(const void *data, size_t n)
{
	char tmpl[] = "/tmp/pbsd_b0203_XXXXXX";
	int fd = mkstemp(tmpl);
	if (fd < 0)
		return "";
	if (n > 0)
		(void)write(fd, data, n);
	close(fd);
	return tmpl;
}

static void
run_rfile_tests(void)
{
	st_rfile.cases++;
	{
		std::string path = make_temp_file("", 0);
		size_t rl = 99, pl = 99;
		void *r = ref_rfile((char *)path.c_str(), &rl);
		void *p = P::rfile((char *)path.c_str(), &pl);
		if (rl != pl || rl != 0)
			fail(st_rfile, "empty");
		free(r);
		free(p);
		unlink(path.c_str());
	}
	u_char bin[] = {0, 0x80, 0xff, 'z'};
	st_rfile.cases++;
	{
		std::string path = make_temp_file(bin, sizeof(bin));
		size_t rl = 0, pl = 0;
		void *r = ref_rfile((char *)path.c_str(), &rl);
		void *p = P::rfile((char *)path.c_str(), &pl);
		if (rl != pl || rl != sizeof(bin) || std::memcmp(r, p, rl) != 0)
			fail(st_rfile, "binary");
		free(r);
		free(p);
		unlink(path.c_str());
	}
	st_rfile.cases++;
	{
		std::string path = make_temp_file("data\n", 5);
		char spec[128];
		std::snprintf(spec, sizeof(spec), "  %s\n", path.c_str());
		char spec2[128];
		std::strcpy(spec2, spec);
		size_t rl = 0, pl = 0;
		void *r = ref_rfile(spec, &rl);
		void *p = P::rfile(spec2, &pl);
		if (rl != pl || std::memcmp(r, p, rl) != 0)
			fail(st_rfile, "ws nl");
		free(r);
		free(p);
		unlink(path.c_str());
	}
	st_rfile.cases++;
	ExitObs r = run_child([] {
		size_t l = 0;
		ref_rfile((char *)"/nonexistent/pbsd_b0203", &l);
	});
	ExitObs p = run_child([] {
		size_t l = 0;
		P::rfile((char *)"/nonexistent/pbsd_b0203", &l);
	});
	if (!exit_same(r, p))
		fail(st_rfile, "missing");
	for (long i = 0; i < SWEEP; i++) {
		st_rfile.cases++;
		size_t n = (size_t)(nextrand() % 256);
		std::vector<u_char> data(n);
		for (size_t j = 0; j < n; j++)
			data[j] = (u_char)(nextrand() & 0xff);
		std::string path = make_temp_file(data.data(), n);
		size_t rl = 0, pl = 0;
		void *vr = ref_rfile((char *)path.c_str(), &rl);
		void *vp = P::rfile((char *)path.c_str(), &pl);
		if (rl != pl || std::memcmp(vr, vp, rl) != 0)
			fail(st_rfile, "sweep");
		free(vr);
		free(vp);
		unlink(path.c_str());
	}
}

/* ---------------- DB ops ---------------- */

static Stat st_get = { "get", 0, 0 };
static Stat st_getdata = { "getdata", 0, 0 };
static Stat st_put = { "put", 0, 0 };
static Stat st_rem = { "rem", 0, 0 };
static Stat st_synk = { "synk", 0, 0 };
static Stat st_seq = { "seq", 0, 0 };
static Stat st_dump = { "dump", 0, 0 };

static void
run_get_case(int ret, int out_is_stdout, u_long ln, const char *payload,
    size_t psz)
{
	st_get.cases++;
	mock_reset();
	g_mock.get_ret = ret;
	std::memcpy(g_mock.get_data, payload, psz);
	g_mock.get_size = psz;
	DB db = make_db();
	DBT key{};
	u_char kbuf[] = "key";
	key.data = kbuf;
	key.size = 3;
	int wfd = out_is_stdout ? STDOUT_FILENO : make_pipe_out();
	sync_ref(ln, 0, wfd);
	sync_port(ln, 0, wfd);
	std::string rout, pout;
	if (ret == -1) {
		ExitObs r = run_child([&] { ref_get(&db, &key); });
		ExitObs p = run_child([&] { P::get(&db, &key); });
		if (!exit_same(r, p))
			fail(st_get, "err");
		return;
	}
	if (out_is_stdout) {
		rout = capture_stdout_ref([&] { ref_get(&db, &key); });
		pout = capture_stdout_port([&] { P::get(&db, &key); });
	} else if (ret == 0) {
		rout = capture_ofd_ref(wfd, [&] { ref_get(&db, &key); });
		int wfd2 = make_pipe_out();
		sync_ref(ln, 0, wfd2);
		sync_port(ln, 0, wfd2);
		pout = capture_ofd_port(wfd2, [&] { P::get(&db, &key); });
	} else if (ret == 1) {
		rout = capture_ofd_ref(wfd, [&] { ref_get(&db, &key); });
		int wfd2 = make_pipe_out();
		sync_ref(ln, 0, wfd2);
		sync_port(ln, 0, wfd2);
		pout = capture_ofd_port(wfd2, [&] { P::get(&db, &key); });
	}
	if (rout != pout)
		fail(st_get, "output");
}

static void
run_get_tests(void)
{
	run_get_case(0, 1, 5, "hello", 5);
	run_get_case(0, 0, 5, "pipe", 4);
	run_get_case(1, 0, 42, "", 0);
	run_get_case(1, 1, 7, "", 0);
	st_get.cases++;
	{
		mock_reset();
		g_mock.get_ret = -1;
		DB db = make_db();
		DBT key{};
		char k[] = "k";
		key.data = k;
		key.size = 1;
		sync_ref(9, 0, STDOUT_FILENO);
		sync_port(9, 0, STDOUT_FILENO);
		ExitObs r = run_child([&] { ref_get(&db, &key); });
		ExitObs p = run_child([&] { P::get(&db, &key); });
		if (!exit_same(r, p))
			fail(st_get, "err");
	}
	for (long i = 0; i < SWEEP; i++) {
		int ret = (int)(nextrand() % 3) - 1;
		if (ret < -1)
			ret = 1;
		int stdout_fd = (int)(nextrand() & 1);
		size_t n = (size_t)(nextrand() % 32);
		char buf[32];
		for (size_t j = 0; j < n; j++)
			buf[j] = (char)(nextrand() & 0xff);
		run_get_case(ret, stdout_fd, (u_long)(i + 1), buf, n);
	}
}

static void
run_getdata_tests(void)
{
	st_getdata.cases++;
	mock_reset();
	g_mock.get_ret = 0;
	const char *d = "payload";
	std::memcpy(g_mock.get_data, d, 7);
	g_mock.get_size = 7;
	DB db = make_db();
	DBT key{}, rd{}, pd{};
	char k[] = "k";
	key.data = k;
	key.size = 1;
	u_char rg[32], pg[32];
	std::memset(rg, GUARD, sizeof(rg));
	std::memset(pg, GUARD, sizeof(pg));
	rd.data = rg + 4;
	pd.data = pg + 4;
	sync_ref(1, 0, STDOUT_FILENO);
	sync_port(1, 0, STDOUT_FILENO);
	ref_getdata(&db, &key, &rd);
	P::getdata(&db, &key, &pd);
	if (rd.size != pd.size || std::memcmp(rd.data, pd.data, rd.size) != 0)
		fail(st_getdata, "success");
	for (long i = 0; i < SWEEP; i++) {
		st_getdata.cases++;
		mock_reset();
		int ret = (int)(nextrand() % 3) - 1;
		if (ret < -1)
			ret = 1;
		g_mock.get_ret = ret;
		size_t n = (size_t)(nextrand() % 40);
		for (size_t j = 0; j < n; j++)
			g_mock.get_data[j] = (u_char)(nextrand() & 0xff);
		g_mock.get_size = n;
		DB db2 = make_db();
		DBT k2{}, r2{}, p2{};
		char kb[8];
		k2.data = kb;
		k2.size = 1;
		kb[0] = (char)('a' + (i % 26));
		u_char gr[64], gp[64];
		std::memset(gr, GUARD, sizeof(gr));
		std::memset(gp, GUARD, sizeof(gp));
		r2.data = gr + 8;
		p2.data = gp + 8;
		sync_ref((u_long)i, 0, STDOUT_FILENO);
		sync_port((u_long)i, 0, STDOUT_FILENO);
		if (ret == 0) {
			ref_getdata(&db2, &k2, &r2);
			P::getdata(&db2, &k2, &p2);
			if (r2.size != p2.size ||
			    std::memcmp(r2.data, p2.data, r2.size) != 0)
				fail(st_getdata, "sweep ok");
		} else {
			ExitObs r = run_child([&] { ref_getdata(&db2, &k2, &r2); });
			ExitObs p = run_child([&] { P::getdata(&db2, &k2, &p2); });
			if (!exit_same(r, p))
				fail(st_getdata, "sweep err");
		}
	}
}

static void
run_put_tests(void)
{
	st_put.cases++;
	mock_reset();
	g_mock.put_ret = 1;
	DB db = make_db();
	DBT k{}, d{};
	char kb[] = "k", dbf[] = "d";
	k.data = kb;
	k.size = 1;
	d.data = dbf;
	d.size = 1;
	int wfd = make_pipe_out();
	sync_ref(3, 0, wfd);
	sync_port(3, 0, wfd);
	std::string r = capture_ofd_ref(wfd, [&] { ref_put(&db, &k, &d); });
	int wfd2 = make_pipe_out();
	sync_ref(3, 0, wfd2);
	sync_port(3, 0, wfd2);
	std::string p = capture_ofd_port(wfd2, [&] { P::put(&db, &k, &d); });
	if (r != p)
		fail(st_put, "nooverwrite");
	for (long i = 0; i < SWEEP; i++) {
		st_put.cases++;
		mock_reset();
		int ret = (int)(nextrand() % 3) - 1;
		if (ret < -1)
			ret = 1;
		g_mock.put_ret = ret;
		DB db2 = make_db();
		DBT k2{}, d2{};
		char kb2[4] = {'k'}, db2b[4] = {'v'};
		k2.data = kb2;
		k2.size = 1;
		d2.data = db2b;
		d2.size = 1;
		int wf = make_pipe_out();
		sync_ref((u_long)i, 0, wf);
		sync_port((u_long)i, 0, wf);
		if (ret == 1) {
			std::string rs = capture_ofd_ref(wf, [&] {
				ref_put(&db2, &k2, &d2);
			});
			int wf2 = make_pipe_out();
			sync_ref((u_long)i, 0, wf2);
			sync_port((u_long)i, 0, wf2);
			std::string ps = capture_ofd_port(wf2, [&] {
				P::put(&db2, &k2, &d2);
			});
			if (rs != ps)
				fail(st_put, "sweep");
		} else if (ret == 0) {
			ref_put(&db2, &k2, &d2);
			P::put(&db2, &k2, &d2);
		} else {
			ExitObs r = run_child([&] { ref_put(&db2, &k2, &d2); });
			ExitObs p = run_child([&] { P::put(&db2, &k2, &d2); });
			if (!exit_same(r, p))
				fail(st_put, "sweep err");
		}
	}
}

static void
run_rem_tests(void)
{
	auto one = [](int ret, u_int fl, int stdout_fd, u_long ln) {
		st_rem.cases++;
		mock_reset();
		g_mock.del_ret = ret;
		DB db = make_db();
		DBT k{};
		char kb[24];
		std::snprintf(kb, sizeof(kb), "key%lu", (unsigned long)ln);
		k.data = kb;
		k.size = std::strlen(kb);
		int wfd = stdout_fd ? STDOUT_FILENO : make_pipe_out();
		sync_ref(ln, fl, wfd);
		sync_port(ln, fl, wfd);
		if (ret == -1) {
			ExitObs r = run_child([&] { ref_rem(&db, &k); });
			ExitObs p = run_child([&] { P::rem(&db, &k); });
			if (!exit_same(r, p))
				fail(st_rem, "err");
			return;
		}
		if (!stdout_fd && ret == 1) {
			std::string rs = capture_ofd_ref(wfd, [&] {
				ref_rem(&db, &k);
			});
			int wfd2 = make_pipe_out();
			sync_ref(ln, fl, wfd2);
			sync_port(ln, fl, wfd2);
			std::string ps = capture_ofd_port(wfd2, [&] {
				P::rem(&db, &k);
			});
			if (rs != ps)
				fail(st_rem, "pipe");
			return;
		}
		ExitObs r = run_child([&] { ref_rem(&db, &k); });
		ExitObs p = run_child([&] { P::rem(&db, &k); });
		if (!exit_same(r, p))
			fail(st_rem, "stderr");
	};
	one(0, 0, 1, 1);
	one(1, 0, 0, 2);
	one(1, R_CURSOR, 1, 3);
	one(1, 0, 1, 4);
	one(-1, 0, 1, 5);
	for (long i = 0; i < SWEEP; i++)
		one((int)(nextrand() % 3) - (int)(nextrand() % 2),
		    (unsigned)(nextrand() % 2 ? R_CURSOR : R_NEXT),
		    (int)(nextrand() & 1), (u_long)(i + 10));
}

static void
run_synk_tests(void)
{
	st_synk.cases++;
	mock_reset();
	g_mock.sync_ret = 0;
	DB db = make_db();
	sync_ref(1, 0, STDOUT_FILENO);
	sync_port(1, 0, STDOUT_FILENO);
	ref_synk(&db);
	P::synk(&db);
	st_synk.cases++;
	mock_reset();
	g_mock.sync_ret = -1;
	DB db2 = make_db();
	sync_ref(2, 0, STDOUT_FILENO);
	sync_port(2, 0, STDOUT_FILENO);
	ExitObs r = run_child([&] { ref_synk(&db2); });
	ExitObs p = run_child([&] { P::synk(&db2); });
	if (!exit_same(r, p))
		fail(st_synk, "err");
	for (long i = 0; i < SWEEP; i++) {
		st_synk.cases++;
		mock_reset();
		g_mock.sync_ret = (nextrand() & 1) ? 0 : -1;
		DB db3 = make_db();
		sync_ref((u_long)i, (u_int)(nextrand() & 0xf), STDOUT_FILENO);
		sync_port((u_long)i, (u_int)(nextrand() & 0xf), STDOUT_FILENO);
		if (g_mock.sync_ret == 0) {
			ref_synk(&db3);
			P::synk(&db3);
		} else {
			ExitObs rr = run_child([&] { ref_synk(&db3); });
			ExitObs pp = run_child([&] { P::synk(&db3); });
			if (!exit_same(rr, pp))
				fail(st_synk, "sweep");
		}
	}
}

static void
run_seq_tests(void)
{
	auto one = [](int ret, u_int fl, int stdout_fd, u_long ln, const char *d,
	    size_t dz) {
		st_seq.cases++;
		mock_reset();
		if (ret == 0)
			mock_seq_push(0, d, dz);
		else if (ret == -1)
			mock_seq_push(-1, nullptr, 0);
		DB db = make_db();
		DBT k{};
		char kb[16];
		std::snprintf(kb, sizeof(kb), "k%lu", (unsigned long)ln);
		k.data = kb;
		k.size = std::strlen(kb);
		int wfd = stdout_fd ? STDOUT_FILENO : make_pipe_out();
		sync_ref(ln, fl, wfd);
		sync_port(ln, fl, wfd);
		if (ret == -1) {
			ExitObs r = run_child([&] { ref_seq(&db, &k); });
			ExitObs p = run_child([&] { P::seq(&db, &k); });
			if (!exit_same(r, p))
				fail(st_seq, "err");
			return;
		}
		std::string rs, ps;
		if (stdout_fd && ret == 0) {
			rs = capture_stdout_ref([&] { ref_seq(&db, &k); });
			ps = capture_stdout_port([&] { P::seq(&db, &k); });
		} else if (ret == 0) {
			rs = capture_ofd_ref(wfd, [&] { ref_seq(&db, &k); });
			int w2 = make_pipe_out();
			sync_ref(ln, fl, w2);
			sync_port(ln, fl, w2);
			ps = capture_ofd_port(w2, [&] { P::seq(&db, &k); });
		} else if (!stdout_fd) {
			rs = capture_ofd_ref(wfd, [&] { ref_seq(&db, &k); });
			int w2 = make_pipe_out();
			sync_ref(ln, fl, w2);
			sync_port(ln, fl, w2);
			ps = capture_ofd_port(w2, [&] { P::seq(&db, &k); });
		} else {
			ExitObs r = run_child([&] { ref_seq(&db, &k); });
			ExitObs p = run_child([&] { P::seq(&db, &k); });
			if (!exit_same(r, p))
				fail(st_seq, "stderr");
			return;
		}
		if (rs != ps)
			fail(st_seq, "out");
	};
	one(0, R_NEXT, 1, 1, "seq\n", 4);
	one(1, R_CURSOR, 1, 2, "", 0);
	one(1, R_NEXT, 1, 3, "", 0);
	one(1, 0, 0, 4, "", 0);
	for (long i = 0; i < SWEEP; i++) {
		mock_reset();
		g_mock.get_ret = (int)(nextrand() % 3) - 1;
		if (g_mock.get_ret < -1)
			g_mock.get_ret = 1;
		char buf[20];
		size_t n = (size_t)(nextrand() % 10);
		for (size_t j = 0; j < n; j++)
			buf[j] = (char)(nextrand() & 0xff);
		one(g_mock.get_ret, (u_int)((nextrand() % 2) ? R_CURSOR : R_FIRST),
		    (int)(nextrand() & 1), (u_long)i, buf, n);
	}
}

static void
run_dump_tests(void)
{
	auto run_dump = [](int rev, int stdout_fd) {
		st_dump.cases++;
		mock_reset();
		mock_seq_push(0, "a", 1);
		mock_seq_push(0, "bb", 2);
		mock_seq_push(1, nullptr, 0);
		DB db = make_db();
		int wfd = stdout_fd ? STDOUT_FILENO : make_pipe_out();
		sync_ref(8, 0, wfd);
		sync_port(8, 0, wfd);
		std::string rs, ps;
		if (stdout_fd) {
			rs = capture_stdout_ref([&] { ref_dump(&db, rev); });
			ps = capture_stdout_port([&] { P::dump(&db, rev); });
		} else {
			rs = capture_ofd_ref(wfd, [&] { ref_dump(&db, rev); });
			int w2 = make_pipe_out();
			sync_ref(8, 0, w2);
			sync_port(8, 0, w2);
			ps = capture_ofd_port(w2, [&] { P::dump(&db, rev); });
		}
		if (rs != ps)
			fail(st_dump, "forward");
	};
	run_dump(0, 1);
	run_dump(1, 0);
	st_dump.cases++;
	mock_reset();
	mock_seq_push(1, nullptr, 0);
	DB db = make_db();
	sync_ref(1, 0, STDOUT_FILENO);
	sync_port(1, 0, STDOUT_FILENO);
	std::string rs = capture_stdout_ref([&] { ref_dump(&db, 0); });
	std::string ps = capture_stdout_port([&] { P::dump(&db, 0); });
	if (rs != ps)
		fail(st_dump, "empty");
	for (long i = 0; i < SWEEP; i++) {
		st_dump.cases++;
		mock_reset();
		int items = (int)(nextrand() % 5);
		for (int j = 0; j < items; j++) {
			char b[8];
			b[0] = (char)('a' + j);
			mock_seq_push(0, b, 1);
		}
		mock_seq_push(1, nullptr, 0);
		DB db2 = make_db();
		int rev = (int)(nextrand() & 1);
		int sout = (int)(nextrand() & 1);
		int wfd = sout ? STDOUT_FILENO : make_pipe_out();
		sync_ref((u_long)i, 0, wfd);
		sync_port((u_long)i, 0, wfd);
		std::string r, p;
		if (sout) {
			r = capture_stdout_ref([&] { ref_dump(&db2, rev); });
			p = capture_stdout_port([&] { P::dump(&db2, rev); });
		} else {
			r = capture_ofd_ref(wfd, [&] { ref_dump(&db2, rev); });
			int w2 = make_pipe_out();
			sync_ref((u_long)i, 0, w2);
			sync_port((u_long)i, 0, w2);
			p = capture_ofd_port(w2, [&] { P::dump(&db2, rev); });
		}
		if (r != p)
			fail(st_dump, "sweep");
	}
}

/* ---------------- err / usage ---------------- */

static Stat st_err = { "err", 0, 0 };
static Stat st_usage = { "usage", 0, 0 };

static void
run_err_usage_tests(void)
{
	st_err.cases++;
	ExitObs r = run_child([] { ref_err("test %d", 42); });
	ExitObs p = run_child([] { P::err("test %d", 42); });
	if (!exit_same(r, p))
		fail(st_err, "basic");
	st_usage.cases++;
	r = run_child([] { ref_usage(); });
	p = run_child([] { P::usage(); });
	if (!exit_same(r, p))
		fail(st_usage, "basic");
	for (long i = 0; i < 1000; i++) {
		st_err.cases++;
		char fmt[32];
		std::snprintf(fmt, sizeof(fmt), "e%ld %%d", i);
		ExitObs rr = run_child([&] {
			ref_err(fmt, (int)i);
		});
		ExitObs pp = run_child([&] {
			P::err(fmt, (int)i);
		});
		if (!exit_same(rr, pp))
			fail(st_err, "sweep");
	}
}

int
main(void)
{
	run_compare_tests();
	run_sflags_tests();
	run_dbtype_tests();
	run_setflags_tests();
	run_setinfo_tests();
	run_xmalloc_tests();
	run_rfile_tests();
	run_get_tests();
	run_getdata_tests();
	run_put_tests();
	run_rem_tests();
	run_synk_tests();
	run_seq_tests();
	run_dump_tests();
	run_err_usage_tests();

	Stat stats[] = {
	    st_compare, st_sflags, st_dbtype, st_setflags, st_setinfo,
	    st_xmalloc, st_rfile, st_get, st_getdata, st_put, st_rem,
	    st_synk, st_seq, st_dump, st_err, st_usage,
	};

	std::printf("PBSD batch b0203 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");
	std::printf("%-16s %12s %12s %8s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-16s %12s %12s %8s\n", "----------------", "------------",
	    "------------", "--------");

	long total_cases = 0;
	long total_fails = 0;
	for (Stat &s : stats) {
		total_cases += s.cases;
		total_fails += s.fails;
		std::printf("%-16s %12ld %12ld %8s\n", s.name, s.cases, s.fails,
		    s.fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-16s %12s %12s %8s\n", "----------------", "------------",
	    "------------", "--------");
	std::printf("%-16s %12ld %12ld %8s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");
	std::printf("\n%zu function(s) tested, %ld case(s), %ld failure(s).\n",
	    sizeof(stats) / sizeof(stats[0]), total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
