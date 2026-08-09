/*
 * Differential harness for PBSD batch b0289 (btree.tests/main.c helpers).
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

import pbsd.lib.libc.db.test.btree.tests.b0289;

namespace P = pbsd::lib_libc_db_test_btree_tests::b0289;

typedef P::DBT DBT;
typedef P::DB DB;
typedef uint32_t recno_t;

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_IAFTER	4
#define	R_IBEFORE	5
#define	R_LAST		6
#define	R_NEXT		7
#define	R_NOOVERWRITE	8
#define	R_PREV		9
#define	R_APPEND	12

extern "C" {
extern int recno;
extern char *progname;

int ref_parse(char *, char **, int);
void ref_append(DB *, char **);
void ref_cursor(DB *, char **);
void ref_delcur(DB *, char **);
void ref_delete(DB *, char **);
void ref_first(DB *, char **);
void ref_get(DB *, char **);
void ref_help(DB *, char **);
void ref_iafter(DB *, char **);
void ref_ibefore(DB *, char **);
void ref_icursor(DB *, char **);
void ref_insert(DB *, char **);
void ref_keydata(DBT *, DBT *);
void ref_last(DB *, char **);
void ref_list(DB *, char **);
void ref_next(DB *, char **);
void ref_previous(DB *, char **);
void ref_usage(void);
}

struct Stat {
	const char *name;
	long cases;
	long fails;
};

static constexpr unsigned char GUARD = 0x7f;
static constexpr long SWEEP = 200000;
static std::uint64_t rng = 0xB0289C0DEULL;

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
sync_recno(int r)
{
	recno = r;
	P::recno = r;
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

template <typename Fn>
static std::string
capture_stdout(Fn fn)
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

/* ---------------- mock DB ---------------- */

struct MockState {
	int put_ret = 0;
	int get_ret = 0;
	int del_ret = 0;
	int get_errno = EINVAL;
	int seq_errno = EINVAL;
	static constexpr int MAX_SEQ = 64;
	int seq_len = 0;
	int seq_pos = 0;
	int seq_script[MAX_SEQ]{};
	char seq_keys[MAX_SEQ][256]{};
	size_t seq_key_sizes[MAX_SEQ]{};
	char seq_vals[MAX_SEQ][256]{};
	size_t seq_val_sizes[MAX_SEQ]{};
};

static MockState g_mock;

static int
mock_put(DB *, DBT *, const DBT *, unsigned int)
{
	if (g_mock.put_ret == RET_ERROR) {
		errno = EINVAL;
		return RET_ERROR;
	}
	return g_mock.put_ret;
}

static int
mock_get(const DB *, const DBT *, DBT *data, unsigned int)
{
	if (g_mock.get_ret == RET_ERROR) {
		errno = g_mock.get_errno;
		return RET_ERROR;
	}
	if (g_mock.get_ret == RET_SPECIAL)
		return RET_SPECIAL;
	data->data = g_mock.seq_vals[0];
	data->size = g_mock.seq_val_sizes[0];
	return RET_SUCCESS;
}

static int
mock_del(const DB *, const DBT *, unsigned int)
{
	if (g_mock.del_ret == RET_ERROR) {
		errno = EINVAL;
		return RET_ERROR;
	}
	return g_mock.del_ret;
}

static int
mock_seq(DB *, DBT *key, DBT *data, unsigned int)
{
	if (g_mock.seq_pos >= g_mock.seq_len) {
		if (g_mock.seq_len == 0)
			return RET_SPECIAL;
		return RET_SPECIAL;
	}
	int ret = g_mock.seq_script[g_mock.seq_pos];
	if (ret == RET_SUCCESS) {
		key->data = g_mock.seq_keys[g_mock.seq_pos];
		key->size = g_mock.seq_key_sizes[g_mock.seq_pos];
		data->data = g_mock.seq_vals[g_mock.seq_pos];
		data->size = g_mock.seq_val_sizes[g_mock.seq_pos];
	} else if (ret == RET_ERROR)
		errno = g_mock.seq_errno;
	g_mock.seq_pos++;
	return ret;
}

static int
mock_sync(DB *)
{
	return RET_SUCCESS;
}

static DB
make_db(void)
{
	DB db{};
	db.put = mock_put;
	db.get = mock_get;
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
mock_seq_push(int ret, const char *key, size_t ksz, const char *val, size_t vsz)
{
	int i = g_mock.seq_len++;
	g_mock.seq_script[i] = ret;
	if (key != nullptr && ksz > 0) {
		std::memcpy(g_mock.seq_keys[i], key, ksz);
		g_mock.seq_key_sizes[i] = ksz;
	}
	if (val != nullptr && vsz > 0) {
		std::memcpy(g_mock.seq_vals[i], val, vsz);
		g_mock.seq_val_sizes[i] = vsz;
	}
}

static std::string
make_temp_path(void)
{
	char tmpl[] = "/tmp/b0289.XXXXXX";
	int fd = mkstemp(tmpl);
	if (fd >= 0)
		close(fd);
	return tmpl;
}

/* ---------------- parse ---------------- */

static Stat st_parse = { "parse", 0, 0 };

static void
test_parse_buf(char *rbuf, char *pbuf, size_t bufsz, int maxargc)
{
	st_parse.cases++;
	char *rargv[8]{};
	char *pargv[8]{};
	int rr = ref_parse(rbuf, rargv, maxargc);
	int rp = P::parse(pbuf, pargv, maxargc);
	if (rr != rp)
		fail(st_parse, "return");
	if (std::memcmp(rbuf, pbuf, bufsz) != 0)
		fail(st_parse, "buffer");
	for (int i = 0; i < rr && i < maxargc; i++) {
		ptrdiff_t ro = rargv[i] - rbuf;
		ptrdiff_t po = pargv[i] - pbuf;
		if (ro != po)
			fail(st_parse, "argv offset");
		if (std::strcmp(rargv[i], pargv[i]) != 0)
			fail(st_parse, "argv str");
	}
}

static void
run_parse_one(const char *input, int maxargc)
{
	const size_t bufsz = 256;
	char rbuf[bufsz], pbuf[bufsz];
	std::memset(rbuf, GUARD, bufsz);
	std::memset(pbuf, GUARD, bufsz);
	std::strncpy(rbuf + 4, input, bufsz - 5);
	std::strncpy(pbuf + 4, input, bufsz - 5);
	test_parse_buf(rbuf + 4, pbuf + 4, bufsz, maxargc);
}

static void
run_parse_tests(void)
{
	run_parse_one("", 3);
	run_parse_one("   ", 3);
	run_parse_one("\t\n", 3);
	run_parse_one("a", 3);
	run_parse_one("  a", 3);
	run_parse_one("a  ", 3);
	run_parse_one("a b", 3);
	run_parse_one("a\tb\tc", 3);
	run_parse_one("a b c d e", 3);
	run_parse_one("a b c d e", 1);
	run_parse_one("a b c d e", 0);
	run_parse_one("\x80\xff\xfe", 3);
	run_parse_one(" \x80 b \xff", 3);
	run_parse_one("one", 3);
	run_parse_one("x y", 2);
	run_parse_one("  leading  two  ", 3);
	for (long i = 0; i < SWEEP; i++) {
		char in[64];
		int n = (int)(nextrand() % 40);
		for (int j = 0; j < n; j++) {
			unsigned v = (unsigned)(nextrand() % 256);
			if (v == 0)
				in[j] = ' ';
			else if (v < 10)
				in[j] = '\t';
			else
				in[j] = (char)v;
		}
		in[n] = '\0';
		int mx = (int)(nextrand() % 4);
		run_parse_one(in, mx);
	}
}

/* ---------------- keydata / help / usage ---------------- */

static Stat st_keydata = { "keydata", 0, 0 };
static Stat st_help = { "help", 0, 0 };
static Stat st_usage = { "usage", 0, 0 };

static void
run_keydata_case(int rno, const char *k, size_t ksz, const char *d, size_t dsz)
{
	st_keydata.cases++;
	sync_recno(rno);
	DBT rk{}, pk{}, rd{}, pd{};
	char kbuf_r[64], kbuf_p[64], dbuf_r[64], dbuf_p[64];
	std::memset(kbuf_r, GUARD, sizeof(kbuf_r));
	std::memset(kbuf_p, GUARD, sizeof(kbuf_p));
	std::memset(dbuf_r, GUARD, sizeof(dbuf_r));
	std::memset(dbuf_p, GUARD, sizeof(dbuf_p));
	if (ksz > 0) {
		std::memcpy(kbuf_r + 4, k, ksz);
		std::memcpy(kbuf_p + 4, k, ksz);
		kbuf_r[4 + ksz] = '\0';
		kbuf_p[4 + ksz] = '\0';
	}
	if (dsz > 0) {
		std::memcpy(dbuf_r + 4, d, dsz);
		std::memcpy(dbuf_p + 4, d, dsz);
		dbuf_r[4 + dsz] = '\0';
		dbuf_p[4 + dsz] = '\0';
	}
	rk.data = kbuf_r + 4;
	rk.size = ksz;
	pk.data = kbuf_p + 4;
	pk.size = ksz;
	rd.data = dbuf_r + 4;
	rd.size = dsz;
	pd.data = dbuf_p + 4;
	pd.size = dsz;
	std::string rout = capture_stdout([&] { ref_keydata(&rk, &rd); });
	std::string pout = capture_stdout([&] { P::keydata(&pk, &pd); });
	if (rout != pout)
		fail(st_keydata, "stdout");
}

static void
run_keydata_tests(void)
{
	run_keydata_case(0, "", 0, "", 0);
	run_keydata_case(0, "k", 2, "d", 2);
	run_keydata_case(0, "key", 4, "data", 5);
	run_keydata_case(1, "k", 2, "d", 2);
	run_keydata_case(0, "k", 0, "onlydata", 9);
	run_keydata_case(0, "\x80\xff", 2, "\xfe", 2);
	for (long i = 0; i < SWEEP; i++) {
		char k[16], d[16];
		size_t kz = (size_t)(nextrand() % 10);
		size_t dz = (size_t)(nextrand() % 10);
		for (size_t j = 0; j < kz; j++)
			k[j] = (char)(nextrand() & 0xff);
		for (size_t j = 0; j < dz; j++)
			d[j] = (char)(nextrand() & 0xff);
		run_keydata_case((int)(nextrand() & 1), k, kz, d, dz);
	}
}

static void
run_help_tests(void)
{
	st_help.cases++;
	DB db = make_db();
	char *av[] = { (char *)"help" };
	std::string rout = capture_stdout([&] { ref_help(&db, av); });
	std::string pout = capture_stdout([&] { P::help(&db, av); });
	if (rout != pout)
		fail(st_help, "stdout");
	for (long i = 0; i < 1000; i++) {
		st_help.cases++;
		rout = capture_stdout([&] { ref_help(&db, av); });
		pout = capture_stdout([&] { P::help(&db, av); });
		if (rout != pout)
			fail(st_help, "sweep");
	}
}

static void
run_usage_tests(void)
{
	st_usage.cases++;
	progname = (char *)"btree.test";
	P::progname = (char *)"btree.test";
	ExitObs r = run_child([] { ref_usage(); });
	ExitObs p = run_child([] { P::usage(); });
	if (!exit_same(r, p))
		fail(st_usage, "basic");
	for (long i = 0; i < 1000; i++) {
		st_usage.cases++;
		char name[32];
		std::snprintf(name, sizeof(name), "prog%ld", i);
		progname = name;
		P::progname = name;
		r = run_child([] { ref_usage(); });
		p = run_child([] { P::usage(); });
		if (!exit_same(r, p))
			fail(st_usage, "sweep");
	}
}

/* ---------------- put-style ops ---------------- */

static Stat st_append = { "append", 0, 0 };
static Stat st_iafter = { "iafter", 0, 0 };
static Stat st_ibefore = { "ibefore", 0, 0 };
static Stat st_icursor = { "icursor", 0, 0 };
static Stat st_insert = { "insert", 0, 0 };

static void
run_put_op(Stat &st, void (*ref_fn)(DB *, char **),
    void (*port_fn)(DB *, char **), int rno, int pret, const char *a1,
    const char *a2, bool recno_only = false)
{
	st.cases++;
	sync_recno(rno);
	mock_reset();
	g_mock.put_ret = pret;
	DB db = make_db();
	recno_t rn = 42;
	char k1[32], k2[32];
	char *argv_r[4];
	char *argv_p[4];
	if (rno) {
		argv_r[1] = (char *)&rn;
		argv_p[1] = (char *)&rn;
	} else {
		std::strncpy(k1, a1, sizeof(k1) - 1);
		k1[sizeof(k1) - 1] = '\0';
		argv_r[1] = k1;
		argv_p[1] = k1;
	}
	std::strncpy(k2, a2, sizeof(k2) - 1);
	k2[sizeof(k2) - 1] = '\0';
	argv_r[0] = (char *)"cmd";
	argv_p[0] = (char *)"cmd";
	argv_r[2] = k2;
	argv_p[2] = k2;
	if (recno_only && !rno) {
		ExitObs r = run_child([&] { ref_fn(&db, argv_r); });
		ExitObs p = run_child([&] { port_fn(&db, argv_p); });
		if (!exit_same(r, p))
			fail(st, "recno guard");
		return;
	}
	if (pret == RET_ERROR) {
		ExitObs r = run_child([&] { ref_fn(&db, argv_r); });
		ExitObs p = run_child([&] { port_fn(&db, argv_p); });
		if (!exit_same(r, p))
			fail(st, "err");
		return;
	}
	if (pret == RET_SPECIAL) {
		std::string rout = capture_stdout([&] { ref_fn(&db, argv_r); });
		std::string pout = capture_stdout([&] { port_fn(&db, argv_p); });
		if (rout != pout)
			fail(st, "special");
		return;
	}
	ref_fn(&db, argv_r);
	port_fn(&db, argv_p);
}

static void
run_put_sweep(Stat &st, void (*ref_fn)(DB *, char **),
    void (*port_fn)(DB *, char **), bool recno_only = false)
{
	for (long i = 0; i < SWEEP / 5; i++) {
		int rno = (int)(nextrand() & 1);
		int pret = (int)(nextrand() % 3) - 1;
		char a1[16], a2[16];
		for (int j = 0; j < 8; j++) {
			a1[j] = (char)('a' + (nextrand() % 26));
			a2[j] = (char)(nextrand() & 0xff);
		}
		a1[8] = a2[8] = '\0';
		run_put_op(st, ref_fn, port_fn, rno, pret, a1, a2,
		    recno_only && !rno);
	}
}

static void
run_put_tests(void)
{
	run_put_op(st_append, ref_append, P::append, 1, RET_SUCCESS, "1", "data", false);
	run_put_op(st_append, ref_append, P::append, 0, RET_SUCCESS, "k", "d", true);
	run_put_op(st_append, ref_append, P::append, 1, RET_SPECIAL, "2", "dup", false);
	run_put_op(st_append, ref_append, P::append, 1, RET_ERROR, "3", "err", false);
	run_put_sweep(st_append, ref_append, P::append, true);

	run_put_op(st_iafter, ref_iafter, P::iafter, 1, RET_SUCCESS, "1", "x", false);
	run_put_op(st_iafter, ref_iafter, P::iafter, 0, RET_SUCCESS, "k", "d", true);
	run_put_sweep(st_iafter, ref_iafter, P::iafter, true);

	run_put_op(st_ibefore, ref_ibefore, P::ibefore, 1, RET_SUCCESS, "1", "x", false);
	run_put_op(st_ibefore, ref_ibefore, P::ibefore, 0, RET_SUCCESS, "k", "d", true);
	run_put_sweep(st_ibefore, ref_ibefore, P::ibefore, true);

	run_put_op(st_icursor, ref_icursor, P::icursor, 0, RET_SUCCESS, "key", "val");
	run_put_op(st_icursor, ref_icursor, P::icursor, 1, RET_SPECIAL, "1", "dup");
	run_put_op(st_icursor, ref_icursor, P::icursor, 0, RET_ERROR, "k", "e");
	run_put_sweep(st_icursor, ref_icursor, P::icursor);

	run_put_op(st_insert, ref_insert, P::insert, 0, RET_SUCCESS, "k", "v");
	run_put_op(st_insert, ref_insert, P::insert, 1, RET_SPECIAL, "1", "dup");
	run_put_sweep(st_insert, ref_insert, P::insert);
}

/* ---------------- seq / get / delete ---------------- */

static Stat st_cursor = { "cursor", 0, 0 };
static Stat st_first = { "first", 0, 0 };
static Stat st_last = { "last", 0, 0 };
static Stat st_next = { "next", 0, 0 };
static Stat st_previous = { "previous", 0, 0 };
static Stat st_get = { "get", 0, 0 };
static Stat st_delete = { "delete", 0, 0 };
static Stat st_delcur = { "delcur", 0, 0 };

static void
run_seq_op(Stat &st, void (*ref_fn)(DB *, char **),
    void (*port_fn)(DB *, char **), int rno, int sret, const char *keystr,
    const char *kdata, size_t ksz, const char *vdata, size_t vsz)
{
	st.cases++;
	sync_recno(rno);
	mock_reset();
	mock_seq_push(sret, kdata, ksz, vdata, vsz);
	if (st.name == st_get.name) {
		g_mock.get_ret = sret;
		if (sret == RET_SUCCESS) {
			std::memcpy(g_mock.seq_vals[0], vdata, vsz);
			g_mock.seq_val_sizes[0] = vsz;
		}
	}
	DB db = make_db();
	recno_t rn = 7;
	char kbuf[32];
	char *argv_r[2], *argv_p[2];
	argv_r[0] = (char *)"cmd";
	argv_p[0] = (char *)"cmd";
	if (rno) {
		argv_r[1] = (char *)&rn;
		argv_p[1] = (char *)&rn;
	} else {
		std::strncpy(kbuf, keystr, sizeof(kbuf) - 1);
		kbuf[sizeof(kbuf) - 1] = '\0';
		argv_r[1] = kbuf;
		argv_p[1] = kbuf;
	}
	if (sret == RET_ERROR) {
		ExitObs r = run_child([&] { ref_fn(&db, argv_r); });
		ExitObs p = run_child([&] { port_fn(&db, argv_p); });
		if (!exit_same(r, p))
			fail(st, "err");
		return;
	}
	if (sret == RET_SPECIAL) {
		std::string rout = capture_stdout([&] { ref_fn(&db, argv_r); });
		std::string pout = capture_stdout([&] { port_fn(&db, argv_p); });
		if (rout != pout)
			fail(st, "special");
		return;
	}
	std::string rout = capture_stdout([&] { ref_fn(&db, argv_r); });
	std::string pout = capture_stdout([&] { port_fn(&db, argv_p); });
	if (rout != pout)
		fail(st, "success");
}

static void
run_seq_sweep(Stat &st, void (*ref_fn)(DB *, char **),
    void (*port_fn)(DB *, char **))
{
	for (long i = 0; i < SWEEP / 5; i++) {
		int rno = (int)(nextrand() & 1);
		int sret = (int)(nextrand() % 3) - 1;
		char kd[16], vd[16];
		size_t kz = (size_t)(nextrand() % 8) + 1;
		size_t vz = (size_t)(nextrand() % 8) + 1;
		for (size_t j = 0; j < kz; j++)
			kd[j] = (char)(nextrand() & 0xff);
		for (size_t j = 0; j < vz; j++)
			vd[j] = (char)(nextrand() & 0xff);
		kd[kz] = vd[vz] = '\0';
		run_seq_op(st, ref_fn, port_fn, rno, sret, kd, kd, kz + 1, vd, vz + 1);
	}
}

static void
run_seq_tests(void)
{
	const char *k = "foo";
	const char *v = "bar";
	run_seq_op(st_cursor, ref_cursor, P::cursor, 0, RET_SUCCESS, k, k, 4, v, 4);
	run_seq_op(st_cursor, ref_cursor, P::cursor, 0, RET_SPECIAL, k, k, 4, v, 4);
	run_seq_op(st_cursor, ref_cursor, P::cursor, 0, RET_ERROR, k, k, 4, v, 4);
	run_seq_sweep(st_cursor, ref_cursor, P::cursor);

	run_seq_op(st_first, ref_first, P::first, 0, RET_SUCCESS, k, k, 4, v, 4);
	run_seq_op(st_first, ref_first, P::first, 0, RET_SPECIAL, k, k, 4, v, 4);
	run_seq_sweep(st_first, ref_first, P::first);

	run_seq_op(st_last, ref_last, P::last, 0, RET_SUCCESS, k, k, 4, v, 4);
	run_seq_op(st_last, ref_last, P::last, 0, RET_SPECIAL, k, k, 4, v, 4);
	run_seq_sweep(st_last, ref_last, P::last);

	run_seq_op(st_next, ref_next, P::next, 0, RET_SUCCESS, k, k, 4, v, 4);
	run_seq_op(st_next, ref_next, P::next, 0, RET_SPECIAL, k, k, 4, v, 4);
	run_seq_sweep(st_next, ref_next, P::next);

	run_seq_op(st_previous, ref_previous, P::previous, 0, RET_SUCCESS, k, k,
	    4, v, 4);
	run_seq_op(st_previous, ref_previous, P::previous, 0, RET_SPECIAL, k, k,
	    4, v, 4);
	run_seq_sweep(st_previous, ref_previous, P::previous);

	run_seq_op(st_get, ref_get, P::get, 0, RET_SUCCESS, k, k, 4, v, 4);
	run_seq_op(st_get, ref_get, P::get, 0, RET_SPECIAL, k, k, 4, v, 4);
	run_seq_op(st_get, ref_get, P::get, 1, RET_SUCCESS, k, k, 4, v, 4);
	run_seq_sweep(st_get, ref_get, P::get);

	run_seq_op(st_delete, ref_delete, P::delete_fn, 0, RET_SUCCESS, k, k, 4, v,
	    4);
	run_seq_op(st_delete, ref_delete, P::delete_fn, 0, RET_SPECIAL, k, k, 4, v,
	    4);
	run_seq_op(st_delete, ref_delete, P::delete_fn, 0, RET_ERROR, k, k, 4, v,
	    4);
	run_seq_sweep(st_delete, ref_delete, P::delete_fn);

	st_delcur.cases++;
	mock_reset();
	g_mock.del_ret = RET_ERROR;
	DB db = make_db();
	char *av[] = { (char *)"delc" };
	ExitObs r = run_child([&] { ref_delcur(&db, av); });
	ExitObs p = run_child([&] { P::delcur(&db, av); });
	if (!exit_same(r, p))
		fail(st_delcur, "err");
	st_delcur.cases++;
	mock_reset();
	g_mock.del_ret = RET_SUCCESS;
	r = run_child([&] { ref_delcur(&db, av); });
	p = run_child([&] { P::delcur(&db, av); });
	if (!exit_same(r, p))
		fail(st_delcur, "ok");
	for (long i = 0; i < SWEEP / 5; i++) {
		st_delcur.cases++;
		mock_reset();
		g_mock.del_ret = (nextrand() & 1) ? RET_SUCCESS : RET_ERROR;
		r = run_child([&] { ref_delcur(&db, av); });
		p = run_child([&] { P::delcur(&db, av); });
		if (!exit_same(r, p))
			fail(st_delcur, "sweep");
	}
}

/* ---------------- list ---------------- */

static Stat st_list = { "list", 0, 0 };

static void
run_list_tests(void)
{
	st_list.cases++;
	DB db = make_db();
	char *av[] = { (char *)"list", (char *)"/nonexistent/b0289/path" };
	ExitObs r = run_child([&] { ref_list(&db, av); });
	ExitObs p = run_child([&] { P::list(&db, av); });
	if (!exit_same(r, p))
		fail(st_list, "open fail");

	st_list.cases++;
	std::string path = make_temp_path();
	mock_reset();
	mock_seq_push(RET_SUCCESS, "alpha", 6, "A", 2);
	mock_seq_push(RET_SUCCESS, "beta", 5, "B", 2);
	mock_seq_push(RET_ERROR, nullptr, 0, nullptr, 0);
	char *av2[] = { (char *)"list", (char *)path.c_str() };
	ExitObs r2 = run_child([&] { ref_list(&db, av2); });
	ExitObs p2 = run_child([&] { P::list(&db, av2); });
	if (!exit_same(r2, p2))
		fail(st_list, "seq err");
	FILE *fp = std::fopen(path.c_str(), "r");
	std::string content;
	if (fp) {
		char line[128];
		while (std::fgets(line, sizeof(line), fp))
			content += line;
		std::fclose(fp);
	}
	if (content != "alpha\nbeta\n")
		fail(st_list, "file content");
	unlink(path.c_str());

	for (long i = 0; i < SWEEP / 5; i++) {
		st_list.cases++;
		path = make_temp_path();
		mock_reset();
		int items = (int)(nextrand() % 5);
		for (int j = 0; j < items; j++) {
			char k[8];
			std::snprintf(k, sizeof(k), "k%d", j);
			mock_seq_push(RET_SUCCESS, k, std::strlen(k) + 1, "v", 2);
		}
		int endret = (nextrand() & 1) ? RET_SPECIAL : RET_ERROR;
		mock_seq_push(endret, nullptr, 0, nullptr, 0);
		char pbuf[64];
		std::strncpy(pbuf, path.c_str(), sizeof(pbuf) - 1);
		char *av3[] = { (char *)"list", pbuf };
		r = run_child([&] { ref_list(&db, av3); });
		p = run_child([&] { P::list(&db, av3); });
		if (!exit_same(r, p))
			fail(st_list, "sweep");
		unlink(path.c_str());
	}
}

int
main(void)
{
	run_parse_tests();
	run_keydata_tests();
	run_help_tests();
	run_usage_tests();
	run_put_tests();
	run_seq_tests();
	run_list_tests();

	Stat stats[] = {
	    st_parse, st_keydata, st_help, st_usage, st_append, st_iafter,
	    st_ibefore, st_icursor, st_insert, st_cursor, st_first, st_last,
	    st_next, st_previous, st_get, st_delete, st_delcur, st_list,
	};

	std::printf("PBSD batch b0289 differential test\n");
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
