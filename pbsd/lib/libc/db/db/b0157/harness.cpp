/*
 * Differential harness for batch b0157: db dispatcher and wrappers.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif
#ifndef O_SHLOCK
#define O_SHLOCK 0
#endif

import pbsd.lib.libc.db.db.b0157;

namespace P = pbsd::lib_libc_db_db::b0157;

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0

typedef P::DBT DBT;
typedef P::DBTYPE DBTYPE;
typedef P::DB DB;

#if UINT_MAX > 65535
#define	DB_LOCK		0x20000000
#define	DB_SHMEM	0x40000000
#define	DB_TXN		0x80000000
#else
#define	DB_LOCK		    0x2000
#define	DB_SHMEM	    0x4000
#define	DB_TXN		    0x8000
#endif

#define	USE_OPEN_FLAGS							\
	(O_CREAT | O_EXCL | O_EXLOCK | O_NOFOLLOW | O_NONBLOCK | 	\
	 O_RDONLY | O_RDWR | O_SHLOCK | O_SYNC | O_TRUNC | O_CLOEXEC)
#define	DB_FLAGS	(DB_LOCK | DB_SHMEM | DB_TXN)
#define	ALLOWED_FLAGS	(USE_OPEN_FLAGS | DB_FLAGS)

typedef struct {
	unsigned long	flags;
	unsigned int	cachesize;
	int		maxkeypage;
	int		minkeypage;
	unsigned int	psize;
	int		(*compare)(const DBT *, const DBT *);
	size_t		(*prefix)(const DBT *, const DBT *);
	int		lorder;
} BTREEINFO;

typedef struct {
	unsigned int	bsize;
	unsigned int	ffactor;
	unsigned int	nelem;
	unsigned int	cachesize;
	uint32_t	(*hash)(const void *, size_t);
	int	lorder;
} HASHINFO;

typedef struct {
	unsigned long	flags;
	unsigned int	cachesize;
	unsigned int	psize;
	int		lorder;
	size_t		reclen;
	unsigned char	bval;
	char	*bfname;
} RECNOINFO;

extern "C" {
DB *ref_dbopen(const char *, int, int, DBTYPE, const void *);
int ref_cfi_libc_db_dbclose(DB *);
int ref_cfi_libc_db_dbdel(const DB *, const DBT *, unsigned int);
int ref_cfi_libc_db_dbget(const DB *, const DBT *, DBT *, unsigned int);
int ref_cfi_libc_db_dbput(const DB *, DBT *, const DBT *, unsigned int);
int ref_cfi_libc_db_dbseq(const DB *, DBT *, DBT *, unsigned int);
int ref_cfi_libc_db_dbsync(const DB *, unsigned int);
int ref_cfi_libc_db_dbfd(const DB *);
void ref___dbpanic(DB *);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_dbopen = { "dbopen", 0, 0, 0 };
static Stat st_dbclose = { "dbclose", 0, 0, 0 };
static Stat st_dbdel = { "dbdel", 0, 0, 0 };
static Stat st_dbget = { "dbget", 0, 0, 0 };
static Stat st_dbput = { "dbput", 0, 0, 0 };
static Stat st_dbseq = { "dbseq", 0, 0, 0 };
static Stat st_dbsync = { "dbsync", 0, 0, 0 };
static Stat st_dbfd = { "dbfd", 0, 0, 0 };
static Stat st_dbpanic = { "__dbpanic", 0, 0, 0 };

static const int MAX_SHOW = 8;
static const unsigned char GUARD = 0x7f;
static const size_t PAYLOAD = 48;
static const size_t SIDE = 8;
static const size_t ARENA = SIDE + PAYLOAD + SIDE;

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
	int coin() { return (int)(next() & 1u); }
};

static bool
begin_fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown >= MAX_SHOW)
		return false;
	st.shown++;
	std::printf("FAIL %s: %s\n", st.name, what);
	return true;
}

struct OpenCall {
	int which;
	const char *fname;
	int flags;
	int mode;
	const void *openinfo;
	int dflags;
};

struct OpenMock {
	unsigned phase;
	unsigned bt_calls;
	unsigned hash_calls;
	unsigned rec_calls;
	OpenCall last_bt;
	OpenCall last_hash;
	OpenCall last_rec;
	DB bt_db;
	DB hash_db;
	DB rec_db;
};

static OpenMock g_open;

static void
open_mock_reset()
{
	std::memset(&g_open, 0, sizeof g_open);
	g_open.bt_db.type = DB_BTREE;
	g_open.hash_db.type = DB_HASH;
	g_open.rec_db.type = DB_RECNO;
}

extern "C" void *
__bt_open(const char *fname, int flags, int mode, const void *openinfo, int dflags)
{
	g_open.bt_calls++;
	g_open.last_bt.which = 0;
	g_open.last_bt.fname = fname;
	g_open.last_bt.flags = flags;
	g_open.last_bt.mode = mode;
	g_open.last_bt.openinfo = openinfo;
	g_open.last_bt.dflags = dflags;
	return (&g_open.bt_db);
}

extern "C" void *
__hash_open(const char *fname, int flags, int mode, const void *openinfo, int dflags)
{
	g_open.hash_calls++;
	g_open.last_hash.which = 1;
	g_open.last_hash.fname = fname;
	g_open.last_hash.flags = flags;
	g_open.last_hash.mode = mode;
	g_open.last_hash.openinfo = openinfo;
	g_open.last_hash.dflags = dflags;
	return (&g_open.hash_db);
}

extern "C" void *
__rec_open(const char *fname, int flags, int mode, const void *openinfo, int dflags)
{
	g_open.rec_calls++;
	g_open.last_rec.which = 2;
	g_open.last_rec.fname = fname;
	g_open.last_rec.flags = flags;
	g_open.last_rec.mode = mode;
	g_open.last_rec.openinfo = openinfo;
	g_open.last_rec.dflags = dflags;
	return (&g_open.rec_db);
}

static void
compare_open_call(const OpenCall &a, const OpenCall &b, const char *which,
    const char *tag)
{
	if (a.fname != b.fname && !(a.fname == nullptr && b.fname == nullptr) &&
	    !(a.fname != nullptr && b.fname != nullptr &&
	      std::strcmp(a.fname, b.fname) == 0)) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s %s fname mismatch", tag, which);
		begin_fail(st_dbopen, msg);
	}
	if (a.flags != b.flags) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s %s flags %d vs %d", tag, which,
		    a.flags, b.flags);
		begin_fail(st_dbopen, msg);
	}
	if (a.mode != b.mode) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s %s mode %d vs %d", tag, which,
		    a.mode, b.mode);
		begin_fail(st_dbopen, msg);
	}
	if (a.dflags != b.dflags) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s %s dflags %d vs %d", tag, which,
		    a.dflags, b.dflags);
		begin_fail(st_dbopen, msg);
	}
	if (a.openinfo != b.openinfo) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s %s openinfo ptr mismatch", tag,
		    which);
		begin_fail(st_dbopen, msg);
	}
}

static void
t_dbopen(const char *fname, int flags, int mode, DBTYPE type,
    const void *openinfo, const char *tag)
{
	st_dbopen.cases++;

	open_mock_reset();
	errno = 0;
	DB *ref = ref_dbopen(fname, flags, mode, type, openinfo);
	int ref_errno = errno;
	unsigned ref_bt = g_open.bt_calls;
	unsigned ref_hash = g_open.hash_calls;
	unsigned ref_rec = g_open.rec_calls;
	OpenCall ref_last_bt = g_open.last_bt;
	OpenCall ref_last_hash = g_open.last_hash;
	OpenCall ref_last_rec = g_open.last_rec;

	open_mock_reset();
	errno = 0;
	DB *got = P::dbopen(fname, flags, mode, type, openinfo);
	int got_errno = errno;
	unsigned got_bt = g_open.bt_calls;
	unsigned got_hash = g_open.hash_calls;
	unsigned got_rec = g_open.rec_calls;
	OpenCall got_last_bt = g_open.last_bt;
	OpenCall got_last_hash = g_open.last_hash;
	OpenCall got_last_rec = g_open.last_rec;

	if ((ref == nullptr) != (got == nullptr)) {
		char msg[192];
		std::snprintf(msg, sizeof msg,
		    "%s return ptr mismatch ref=%p got=%p", tag,
		    (void *)ref, (void *)got);
		begin_fail(st_dbopen, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbopen, msg);
	}
	if (ref_bt != got_bt || ref_hash != got_hash || ref_rec != got_rec) {
		char msg[192];
		std::snprintf(msg, sizeof msg,
		    "%s open call counts bt %u/%u hash %u/%u rec %u/%u", tag,
		    ref_bt, got_bt, ref_hash, got_hash, ref_rec, got_rec);
		begin_fail(st_dbopen, msg);
	}
	if (ref != nullptr && got != nullptr) {
		if (ref->type != got->type) {
			char msg[128];
			std::snprintf(msg, sizeof msg, "%s returned DB type mismatch",
			    tag);
			begin_fail(st_dbopen, msg);
		}
	}
	if (ref_bt)
		compare_open_call(ref_last_bt, got_last_bt, "bt", tag);
	if (ref_hash)
		compare_open_call(ref_last_hash, got_last_hash, "hash", tag);
	if (ref_rec)
		compare_open_call(ref_last_rec, got_last_rec, "rec", tag);
}

struct MethodLog {
	int ret;
	unsigned call_count;
	unsigned last_flags;
	unsigned char key_arena[ARENA];
	unsigned char data_arena[ARENA];
	size_t key_len;
	size_t data_len;
	unsigned char fill[PAYLOAD];
	size_t fill_len;
};

struct TestHarness {
	DB db;
	MethodLog close_m;
	MethodLog del_m;
	MethodLog get_m;
	MethodLog put_m;
	MethodLog seq_m;
	MethodLog sync_m;
	MethodLog fd_m;
};

static TestHarness g_ref_h;
static TestHarness g_port_h;

static void
fill_arena(unsigned char *arena, const unsigned char *body, size_t len)
{
	std::memset(arena, GUARD, ARENA);
	for (size_t i = 0; i < len && i < PAYLOAD; i++)
		arena[SIDE + i] = body[i];
}

static bool
arenas_equal(const unsigned char *a, const unsigned char *b)
{
	return std::memcmp(a, b, ARENA) == 0;
}

static DBT
make_dbt(unsigned char *arena, size_t len)
{
	DBT d;
	d.data = arena + SIDE;
	d.size = len > PAYLOAD ? PAYLOAD : len;
	return d;
}

static int
h_close(DB *dbp)
{
	TestHarness *t = (TestHarness *)dbp;
	t->close_m.call_count++;
	return (t->close_m.ret);
}

static int
h_del(const DB *dbp, const DBT *key, unsigned int flags)
{
	TestHarness *t = (TestHarness *)(dbp);
	t->del_m.call_count++;
	t->del_m.last_flags = flags;
	if (key != nullptr && key->data != nullptr) {
		t->del_m.key_len = key->size;
		fill_arena(t->del_m.key_arena, (const unsigned char *)key->data,
		    key->size);
	}
	return (t->del_m.ret);
}

static int
h_get(const DB *dbp, const DBT *key, DBT *data, unsigned int flags)
{
	TestHarness *t = (TestHarness *)(dbp);
	t->get_m.call_count++;
	t->get_m.last_flags = flags;
	if (key != nullptr && key->data != nullptr) {
		t->get_m.key_len = key->size;
		fill_arena(t->get_m.key_arena, (const unsigned char *)key->data,
		    key->size);
	}
	if (data != nullptr && data->data != nullptr && t->get_m.fill_len > 0) {
		size_t n = t->get_m.fill_len;
		if (n > data->size)
			n = data->size;
		if (n > PAYLOAD)
			n = PAYLOAD;
		std::memcpy(data->data, t->get_m.fill, n);
	}
	return (t->get_m.ret);
}

static int
h_put(const DB *dbp, DBT *key, const DBT *data, unsigned int flags)
{
	TestHarness *t = (TestHarness *)(dbp);
	t->put_m.call_count++;
	t->put_m.last_flags = flags;
	if (key != nullptr && key->data != nullptr) {
		t->put_m.key_len = key->size;
		fill_arena(t->put_m.key_arena, (const unsigned char *)key->data,
		    key->size);
	}
	if (data != nullptr && data->data != nullptr) {
		t->put_m.data_len = data->size;
		fill_arena(t->put_m.data_arena, (const unsigned char *)data->data,
		    data->size);
	}
	return (t->put_m.ret);
}

static int
h_seq(const DB *dbp, DBT *key, DBT *data, unsigned int flags)
{
	TestHarness *t = (TestHarness *)(dbp);
	t->seq_m.call_count++;
	t->seq_m.last_flags = flags;
	if (key != nullptr && key->data != nullptr) {
		t->seq_m.key_len = key->size;
		fill_arena(t->seq_m.key_arena, (const unsigned char *)key->data,
		    key->size);
	}
	if (data != nullptr && data->data != nullptr && t->seq_m.fill_len > 0) {
		size_t n = t->seq_m.fill_len;
		if (n > data->size)
			n = data->size;
		if (n > PAYLOAD)
			n = PAYLOAD;
		std::memcpy(data->data, t->seq_m.fill, n);
	}
	return (t->seq_m.ret);
}

static int
h_sync(const DB *dbp, unsigned int flags)
{
	TestHarness *t = (TestHarness *)(dbp);
	t->sync_m.call_count++;
	t->sync_m.last_flags = flags;
	return (t->sync_m.ret);
}

static int
h_fd(const DB *dbp)
{
	TestHarness *t = (TestHarness *)(dbp);
	t->fd_m.call_count++;
	return (t->fd_m.ret);
}

static void
init_harness(TestHarness *t, int close_ret, int del_ret, int get_ret, int put_ret,
    int seq_ret, int sync_ret, int fd_ret, size_t fill_len,
    const unsigned char *fill)
{
	std::memset(t, 0, sizeof *t);
	t->db.type = DB_BTREE;
	t->db.close = h_close;
	t->db.del = h_del;
	t->db.get = h_get;
	t->db.put = h_put;
	t->db.seq = h_seq;
	t->db.sync = h_sync;
	t->db.fd = h_fd;
	t->db.internal = (void *)0xdeadbeef;
	t->close_m.ret = close_ret;
	t->del_m.ret = del_ret;
	t->get_m.ret = get_ret;
	t->put_m.ret = put_ret;
	t->seq_m.ret = seq_ret;
	t->sync_m.ret = sync_ret;
	t->fd_m.ret = fd_ret;
	t->get_m.fill_len = fill_len;
	t->seq_m.fill_len = fill_len;
	if (fill != nullptr && fill_len > 0) {
		size_t n = fill_len > PAYLOAD ? PAYLOAD : fill_len;
		std::memcpy(t->get_m.fill, fill, n);
		std::memcpy(t->seq_m.fill, fill, n);
	}
}

static void
compare_method_logs(const MethodLog &a, const MethodLog &b, const char *tag,
    Stat &st, bool check_data)
{
	if (a.call_count != b.call_count) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s call_count %u vs %u", tag,
		    a.call_count, b.call_count);
		begin_fail(st, msg);
	}
	if (a.last_flags != b.last_flags) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s flags %u vs %u", tag,
		    a.last_flags, b.last_flags);
		begin_fail(st, msg);
	}
	if (!arenas_equal(a.key_arena, b.key_arena)) {
		char msg[128];
		std::snprintf(msg, sizeof msg, "%s key arena mismatch", tag);
		begin_fail(st, msg);
	}
	if (check_data && !arenas_equal(a.data_arena, b.data_arena)) {
		char msg[128];
		std::snprintf(msg, sizeof msg, "%s data arena mismatch", tag);
		begin_fail(st, msg);
	}
}

static void
t_dbclose(DB *ref_db, DB *port_db, const char *tag)
{
	st_dbclose.cases++;
	g_ref_h.close_m.call_count = 0;
	g_ref_h.close_m.last_flags = 0;
	g_port_h.close_m.call_count = 0;
	g_port_h.close_m.last_flags = 0;

	errno = 0;
	int ref = ref_cfi_libc_db_dbclose(ref_db);
	int ref_errno = errno;
	unsigned ref_calls = g_ref_h.close_m.call_count;

	errno = 0;
	int got = P::cfi_libc_db_dbclose(port_db);
	int got_errno = errno;
	unsigned got_calls = g_port_h.close_m.call_count;

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbclose, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbclose, msg);
	}
	if (ref_calls != got_calls) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s close calls %u vs %u", tag,
		    ref_calls, got_calls);
		begin_fail(st_dbclose, msg);
	}
}

static void
t_dbdel(DB *ref_db, DB *port_db, const DBT *key, unsigned int flags,
    const char *tag)
{
	st_dbdel.cases++;
	g_ref_h.del_m.call_count = 0;
	g_ref_h.del_m.last_flags = 0;
	std::memset(g_ref_h.del_m.key_arena, GUARD, ARENA);
	g_port_h.del_m.call_count = 0;
	g_port_h.del_m.last_flags = 0;
	std::memset(g_port_h.del_m.key_arena, GUARD, ARENA);

	errno = 0;
	int ref = ref_cfi_libc_db_dbdel(ref_db, key, flags);
	int ref_errno = errno;
	MethodLog ref_log = g_ref_h.del_m;

	errno = 0;
	int got = P::cfi_libc_db_dbdel(port_db, key, flags);
	int got_errno = errno;
	MethodLog got_log = g_port_h.del_m;

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbdel, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbdel, msg);
	}
	compare_method_logs(ref_log, got_log, tag, st_dbdel, false);
}

static void
t_dbget(DB *ref_db, DB *port_db, const DBT *key, DBT *ref_data, DBT *port_data,
    unsigned int flags, const char *tag)
{
	st_dbget.cases++;
	g_ref_h.get_m.call_count = 0;
	g_ref_h.get_m.last_flags = 0;
	std::memset(g_ref_h.get_m.key_arena, GUARD, ARENA);
	g_port_h.get_m.call_count = 0;
	g_port_h.get_m.last_flags = 0;
	std::memset(g_port_h.get_m.key_arena, GUARD, ARENA);

	errno = 0;
	int ref = ref_cfi_libc_db_dbget(ref_db, key, ref_data, flags);
	int ref_errno = errno;
	MethodLog ref_log = g_ref_h.get_m;
	unsigned char ref_arena[ARENA];
	std::memcpy(ref_arena, (unsigned char *)ref_data->data - SIDE, ARENA);

	errno = 0;
	int got = P::cfi_libc_db_dbget(port_db, key, port_data, flags);
	int got_errno = errno;
	MethodLog got_log = g_port_h.get_m;
	unsigned char got_arena[ARENA];
	std::memcpy(got_arena, (unsigned char *)port_data->data - SIDE, ARENA);

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbget, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbget, msg);
	}
	compare_method_logs(ref_log, got_log, tag, st_dbget, false);
	if (std::memcmp(ref_arena, got_arena, ARENA) != 0) {
		char msg[128];
		std::snprintf(msg, sizeof msg, "%s data arena mismatch", tag);
		begin_fail(st_dbget, msg);
	}
}

static void
t_dbput(DB *ref_db, DB *port_db, DBT *ref_key, DBT *port_key,
    const DBT *ref_data, const DBT *port_data, unsigned int flags,
    const char *tag)
{
	st_dbput.cases++;
	g_ref_h.put_m.call_count = 0;
	g_ref_h.put_m.last_flags = 0;
	std::memset(g_ref_h.put_m.key_arena, GUARD, ARENA);
	std::memset(g_ref_h.put_m.data_arena, GUARD, ARENA);
	g_port_h.put_m.call_count = 0;
	g_port_h.put_m.last_flags = 0;
	std::memset(g_port_h.put_m.key_arena, GUARD, ARENA);
	std::memset(g_port_h.put_m.data_arena, GUARD, ARENA);

	errno = 0;
	int ref = ref_cfi_libc_db_dbput(ref_db, ref_key, ref_data, flags);
	int ref_errno = errno;
	MethodLog ref_log = g_ref_h.put_m;

	errno = 0;
	int got = P::cfi_libc_db_dbput(port_db, port_key, port_data, flags);
	int got_errno = errno;
	MethodLog got_log = g_port_h.put_m;

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbput, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbput, msg);
	}
	compare_method_logs(ref_log, got_log, tag, st_dbput, true);
}

static void
t_dbseq(DB *ref_db, DB *port_db, DBT *ref_key, DBT *port_key,
    DBT *ref_data, DBT *port_data, unsigned int flags, const char *tag)
{
	st_dbseq.cases++;
	g_ref_h.seq_m.call_count = 0;
	g_ref_h.seq_m.last_flags = 0;
	std::memset(g_ref_h.seq_m.key_arena, GUARD, ARENA);
	g_port_h.seq_m.call_count = 0;
	g_port_h.seq_m.last_flags = 0;
	std::memset(g_port_h.seq_m.key_arena, GUARD, ARENA);

	errno = 0;
	int ref = ref_cfi_libc_db_dbseq(ref_db, ref_key, ref_data, flags);
	int ref_errno = errno;
	MethodLog ref_log = g_ref_h.seq_m;
	unsigned char ref_arena[ARENA];
	std::memcpy(ref_arena, (unsigned char *)ref_data->data - SIDE, ARENA);

	errno = 0;
	int got = P::cfi_libc_db_dbseq(port_db, port_key, port_data, flags);
	int got_errno = errno;
	MethodLog got_log = g_port_h.seq_m;
	unsigned char got_arena[ARENA];
	std::memcpy(got_arena, (unsigned char *)port_data->data - SIDE, ARENA);

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbseq, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbseq, msg);
	}
	compare_method_logs(ref_log, got_log, tag, st_dbseq, false);
	if (std::memcmp(ref_arena, got_arena, ARENA) != 0) {
		char msg[128];
		std::snprintf(msg, sizeof msg, "%s data arena mismatch", tag);
		begin_fail(st_dbseq, msg);
	}
}

static void
t_dbsync(DB *ref_db, DB *port_db, unsigned int flags, const char *tag)
{
	st_dbsync.cases++;
	g_ref_h.sync_m.call_count = 0;
	g_ref_h.sync_m.last_flags = 0;
	g_port_h.sync_m.call_count = 0;
	g_port_h.sync_m.last_flags = 0;

	errno = 0;
	int ref = ref_cfi_libc_db_dbsync(ref_db, flags);
	int ref_errno = errno;
	MethodLog ref_log = g_ref_h.sync_m;

	errno = 0;
	int got = P::cfi_libc_db_dbsync(port_db, flags);
	int got_errno = errno;
	MethodLog got_log = g_port_h.sync_m;

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbsync, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbsync, msg);
	}
	compare_method_logs(ref_log, got_log, tag, st_dbsync, false);
}

static void
t_dbfd(DB *ref_db, DB *port_db, const char *tag)
{
	st_dbfd.cases++;
	g_ref_h.fd_m.call_count = 0;
	g_ref_h.fd_m.last_flags = 0;
	g_port_h.fd_m.call_count = 0;
	g_port_h.fd_m.last_flags = 0;

	errno = 0;
	int ref = ref_cfi_libc_db_dbfd(ref_db);
	int ref_errno = errno;
	MethodLog ref_log = g_ref_h.fd_m;

	errno = 0;
	int got = P::cfi_libc_db_dbfd(port_db);
	int got_errno = errno;
	MethodLog got_log = g_port_h.fd_m;

	if (ref != got) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s ret %d vs %d", tag, ref, got);
		begin_fail(st_dbfd, msg);
	}
	if (ref_errno != got_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s errno %d vs %d", tag, ref_errno,
		    got_errno);
		begin_fail(st_dbfd, msg);
	}
	compare_method_logs(ref_log, got_log, tag, st_dbfd, false);
}

static void
t_dbpanic_pair(const char *tag)
{
	st_dbpanic.cases++;

	static const unsigned char fill[] = {
		0x00, 0x80, 0xff, 0x7f, 0x01, 0xfe, 0x55, 0xaa
	};
	init_harness(&g_ref_h, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS,
	    RET_SUCCESS, RET_SUCCESS, 42, sizeof fill, fill);
	init_harness(&g_port_h, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS,
	    RET_SUCCESS, RET_SUCCESS, 42, sizeof fill, fill);

	ref___dbpanic(&g_ref_h.db);
	P::__dbpanic(&g_port_h.db);

	unsigned char ref_key[SIDE + PAYLOAD + SIDE];
	unsigned char port_key[SIDE + PAYLOAD + SIDE];
	unsigned char ref_data[SIDE + PAYLOAD + SIDE];
	unsigned char port_data[SIDE + PAYLOAD + SIDE];
	fill_arena(ref_key, (const unsigned char *)"key", 3);
	fill_arena(port_key, (const unsigned char *)"key", 3);
	fill_arena(ref_data, (const unsigned char *)"dat", 3);
	fill_arena(port_data, (const unsigned char *)"dat", 3);
	DBT rk = make_dbt(ref_key, 3);
	DBT pk = make_dbt(port_key, 3);
	DBT rd = make_dbt(ref_data, PAYLOAD);
	DBT pd = make_dbt(port_data, PAYLOAD);

	errno = 0;
	int ref_close = ref_cfi_libc_db_dbclose(&g_ref_h.db);
	int ref_close_errno = errno;
	errno = 0;
	int got_close = P::cfi_libc_db_dbclose(&g_port_h.db);
	int got_close_errno = errno;
	if (ref_close != got_close || ref_close_errno != got_close_errno) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s close after panic", tag);
		begin_fail(st_dbpanic, msg);
	}

	errno = 0;
	int ref_del = ref_cfi_libc_db_dbdel(&g_ref_h.db, &rk, 1);
	int ref_del_errno = errno;
	errno = 0;
	int got_del = P::cfi_libc_db_dbdel(&g_port_h.db, &pk, 1);
	int got_del_errno = errno;
	if (ref_del != got_del || ref_del_errno != got_del_errno ||
	    ref_del != RET_ERROR) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s del after panic", tag);
		begin_fail(st_dbpanic, msg);
	}

	errno = 0;
	int ref_fd = ref_cfi_libc_db_dbfd(&g_ref_h.db);
	int ref_fd_errno = errno;
	errno = 0;
	int got_fd = P::cfi_libc_db_dbfd(&g_port_h.db);
	int got_fd_errno = errno;
	if (ref_fd != got_fd || ref_fd_errno != got_fd_errno ||
	    ref_fd != RET_ERROR) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s fd after panic", tag);
		begin_fail(st_dbpanic, msg);
	}

	errno = 0;
	int ref_get = ref_cfi_libc_db_dbget(&g_ref_h.db, &rk, &rd, 2);
	int ref_get_errno = errno;
	errno = 0;
	int got_get = P::cfi_libc_db_dbget(&g_port_h.db, &pk, &pd, 2);
	int got_get_errno = errno;
	if (ref_get != got_get || ref_get_errno != got_get_errno ||
	    ref_get != RET_ERROR) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s get after panic", tag);
		begin_fail(st_dbpanic, msg);
	}

	errno = 0;
	int ref_put = ref_cfi_libc_db_dbput(&g_ref_h.db, &rk, &rd, 3);
	int ref_put_errno = errno;
	errno = 0;
	int got_put = P::cfi_libc_db_dbput(&g_port_h.db, &pk, &pd, 3);
	int got_put_errno = errno;
	if (ref_put != got_put || ref_put_errno != got_put_errno ||
	    ref_put != RET_ERROR) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s put after panic", tag);
		begin_fail(st_dbpanic, msg);
	}

	errno = 0;
	int ref_seq = ref_cfi_libc_db_dbseq(&g_ref_h.db, &rk, &rd, 4);
	int ref_seq_errno = errno;
	errno = 0;
	int got_seq = P::cfi_libc_db_dbseq(&g_port_h.db, &pk, &pd, 4);
	int got_seq_errno = errno;
	if (ref_seq != got_seq || ref_seq_errno != got_seq_errno ||
	    ref_seq != RET_ERROR) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s seq after panic", tag);
		begin_fail(st_dbpanic, msg);
	}

	errno = 0;
	int ref_sync = ref_cfi_libc_db_dbsync(&g_ref_h.db, 5);
	int ref_sync_errno = errno;
	errno = 0;
	int got_sync = P::cfi_libc_db_dbsync(&g_port_h.db, 5);
	int got_sync_errno = errno;
	if (ref_sync != got_sync || ref_sync_errno != got_sync_errno ||
	    ref_sync != RET_ERROR) {
		char msg[160];
		std::snprintf(msg, sizeof msg, "%s sync after panic", tag);
		begin_fail(st_dbpanic, msg);
	}
}

static void
edge_dbopen()
{
	static BTREEINFO btinfo = { 1, 1024, 4, 2, 4096, nullptr, nullptr, 4321 };
	static HASHINFO hinfo = { 256, 4, 100, 8192, nullptr, 1234 };
	static RECNOINFO rinfo = { 0, 2048, 512, 5678, 16, 0x80, nullptr };

	t_dbopen(nullptr, 0, 0, DB_BTREE, nullptr, "null_fname_bt");
	t_dbopen(nullptr, 0, 0, DB_HASH, nullptr, "null_fname_hash");
	t_dbopen(nullptr, 0, 0, DB_RECNO, nullptr, "null_fname_rec");

	t_dbopen("test.db", 0, 0600, DB_BTREE, &btinfo, "zero_flags_bt");
	t_dbopen("h.db", O_RDONLY, 0644, DB_HASH, &hinfo, "rdonly_hash");
	t_dbopen("r.db", O_RDWR | O_CREAT, 0666, DB_RECNO, &rinfo, "rdwr_rec");

	t_dbopen("x.db", USE_OPEN_FLAGS, 0777, DB_BTREE, &btinfo, "all_open_flags");
	t_dbopen("y.db", DB_FLAGS, 0, DB_HASH, &hinfo, "all_db_flags");
	t_dbopen("z.db", ALLOWED_FLAGS, 0555, DB_RECNO, &rinfo, "all_allowed");

	for (int bit = 0; bit < 31; bit++) {
		int f = (1 << bit);
		if ((f & ALLOWED_FLAGS) != 0)
			t_dbopen("bit.db", f, 0, DB_BTREE, &btinfo, "single_allowed");
	}

	t_dbopen("bad.db", ALLOWED_FLAGS | 1, 0, DB_BTREE, &btinfo, "invalid_lsb");
	t_dbopen("bad2.db", ALLOWED_FLAGS | 0x100000, 0, DB_HASH, &hinfo,
	    "invalid_high");
	t_dbopen("bad3.db", ~0, 0, DB_RECNO, &rinfo, "invalid_all");

	t_dbopen("type.db", 0, 0, (DBTYPE)3, &btinfo, "invalid_type");
	t_dbopen("type2.db", 0, 0, (DBTYPE)-1, &hinfo, "neg_type");

	t_dbopen("mask.db", O_RDONLY | DB_LOCK, 0700, DB_BTREE, &btinfo,
	    "mask_open_db");
	t_dbopen("mask2.db", O_TRUNC | DB_TXN | DB_SHMEM, 0, DB_HASH, &hinfo,
	    "mask_combo");
}

static void
edge_wrappers()
{
	static const unsigned char key_body[] = {
		0x00, 0x80, 0xff, 0x7f, 0x01, 0x55, 0xaa, 0xfe
	};
	static const unsigned char data_body[] = {
		0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80
	};
	unsigned char ref_key[ARENA];
	unsigned char port_key[ARENA];
	unsigned char ref_data[ARENA];
	unsigned char port_data[ARENA];
	fill_arena(ref_key, key_body, sizeof key_body);
	fill_arena(port_key, key_body, sizeof key_body);
	fill_arena(ref_data, data_body, sizeof data_body);
	fill_arena(port_data, data_body, sizeof data_body);
	DBT rk = make_dbt(ref_key, sizeof key_body);
	DBT pk = make_dbt(port_key, sizeof key_body);
	DBT rd = make_dbt(ref_data, sizeof data_body);
	DBT pd = make_dbt(port_data, sizeof data_body);

	init_harness(&g_ref_h, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS,
	    RET_SUCCESS, RET_SUCCESS, 3, sizeof data_body, data_body);
	init_harness(&g_port_h, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS, RET_SUCCESS,
	    RET_SUCCESS, RET_SUCCESS, 3, sizeof data_body, data_body);

	t_dbclose(nullptr, nullptr, "null_close");
	t_dbdel(nullptr, nullptr, nullptr, 0, "null_del");
	t_dbget(nullptr, nullptr, nullptr, nullptr, nullptr, 0, "null_get");
	t_dbput(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, "null_put");
	t_dbseq(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, "null_seq");
	t_dbsync(nullptr, nullptr, 0, "null_sync");
	t_dbfd(nullptr, nullptr, "null_fd");

	t_dbclose(&g_ref_h.db, &g_port_h.db, "close_ok");
	t_dbdel(&g_ref_h.db, &g_port_h.db, &rk, 0, "del_ok");
	t_dbget(&g_ref_h.db, &g_port_h.db, &rk, &rd, &pd, 0, "get_ok");
	t_dbput(&g_ref_h.db, &g_port_h.db, &rk, &pk, &rd, &pd, 0, "put_ok");
	t_dbseq(&g_ref_h.db, &g_port_h.db, &rk, &pk, &rd, &pd, 0, "seq_ok");
	t_dbsync(&g_ref_h.db, &g_port_h.db, 0, "sync_ok");
	t_dbfd(&g_ref_h.db, &g_port_h.db, "fd_ok");

	init_harness(&g_ref_h, RET_ERROR, RET_ERROR, RET_ERROR, RET_ERROR,
	    RET_ERROR, RET_ERROR, -1, 0, nullptr);
	init_harness(&g_port_h, RET_ERROR, RET_ERROR, RET_ERROR, RET_ERROR,
	    RET_ERROR, RET_ERROR, -1, 0, nullptr);
	t_dbclose(&g_ref_h.db, &g_port_h.db, "close_err");
	t_dbdel(&g_ref_h.db, &g_port_h.db, &rk, 0xff, "del_err");
	t_dbget(&g_ref_h.db, &g_port_h.db, &rk, &rd, &pd, 0x80, "get_err");
	t_dbput(&g_ref_h.db, &g_port_h.db, &rk, &pk, &rd, &pd, 0xfe, "put_err");
	t_dbseq(&g_ref_h.db, &g_port_h.db, &rk, &pk, &rd, &pd, 0x7f, "seq_err");
	t_dbsync(&g_ref_h.db, &g_port_h.db, 0x55, "sync_err");
	t_dbfd(&g_ref_h.db, &g_port_h.db, "fd_err");

	t_dbpanic_pair("panic_edge");
}

static void
random_dbopen(Rng &rng, unsigned long long n)
{
	static BTREEINFO btinfo;
	static HASHINFO hinfo;
	static RECNOINFO rinfo;
	char fname_buf[2][32];

	for (unsigned long long i = 0; i < n; i++) {
		int use_fname = rng.coin();
		const char *fname = nullptr;
		if (use_fname) {
			size_t len = 1 + rng.below(20);
			for (size_t j = 0; j < len; j++)
				fname_buf[0][j] = (char)('a' + (rng.below(26)));
			fname_buf[0][len] = '\0';
			fname = fname_buf[0];
		}

		int flags = (int)(rng.next() & 0x7fffffffu);
		if (rng.coin())
			flags &= (int)ALLOWED_FLAGS;
		else if (rng.coin())
			flags |= (int)(1u << rng.below(29));

		int mode = (int)(rng.next() & 07777u);
		DBTYPE type = (DBTYPE)rng.below(6);
		const void *info = nullptr;
		if (rng.coin()) {
			switch (type) {
			case DB_BTREE:
				btinfo.flags = rng.next();
				btinfo.cachesize = rng.below(1u << 20);
				btinfo.lorder = (int)rng.below(2);
				info = &btinfo;
				break;
			case DB_HASH:
				hinfo.bsize = rng.below(4096) + 1;
				hinfo.ffactor = rng.below(16) + 1;
				hinfo.lorder = (int)rng.below(2);
				info = &hinfo;
				break;
			case DB_RECNO:
				rinfo.cachesize = rng.below(1u << 18);
				rinfo.bval = (unsigned char)(rng.next() & 0xff);
				rinfo.lorder = (int)rng.below(2);
				info = &rinfo;
				break;
			default:
				info = &btinfo;
				break;
			}
		}

		t_dbopen(fname, flags, mode, type, info, "random");
	}
}

static void
random_wrappers(Rng &rng, unsigned long long n)
{
	unsigned char key_body[PAYLOAD];
	unsigned char data_body[PAYLOAD];
	unsigned char ref_key[ARENA];
	unsigned char port_key[ARENA];
	unsigned char ref_data[ARENA];
	unsigned char port_data[ARENA];

	for (unsigned long long i = 0; i < n; i++) {
		size_t klen = rng.below((std::uint32_t)PAYLOAD + 1);
		size_t dlen = rng.below((std::uint32_t)PAYLOAD + 1);
		for (size_t j = 0; j < klen; j++)
			key_body[j] = (unsigned char)(rng.next() & 0xff);
		for (size_t j = 0; j < dlen; j++)
			data_body[j] = (unsigned char)(rng.next() & 0xff);

		int close_ret = (int)((rng.next() & 3) == 0 ? RET_ERROR : RET_SUCCESS);
		int del_ret = (int)((rng.next() & 3) == 1 ? RET_ERROR : RET_SUCCESS);
		int get_ret = (int)((rng.next() & 3) == 2 ? RET_ERROR : RET_SUCCESS);
		int put_ret = (int)((rng.next() & 3) == 0 ? RET_ERROR : RET_SUCCESS);
		int seq_ret = (int)((rng.next() & 3) == 1 ? RET_ERROR : RET_SUCCESS);
		int sync_ret = (int)((rng.next() & 3) == 2 ? RET_ERROR : RET_SUCCESS);
		int fd_ret = (int)(rng.next() & 0x7fff);
		size_t fill_len = rng.below((std::uint32_t)PAYLOAD + 1);

		init_harness(&g_ref_h, close_ret, del_ret, get_ret, put_ret, seq_ret,
		    sync_ret, fd_ret, fill_len, data_body);
		init_harness(&g_port_h, close_ret, del_ret, get_ret, put_ret, seq_ret,
		    sync_ret, fd_ret, fill_len, data_body);

		fill_arena(ref_key, key_body, klen);
		fill_arena(port_key, key_body, klen);
		fill_arena(ref_data, data_body, dlen);
		fill_arena(port_data, data_body, dlen);
		DBT rk = make_dbt(ref_key, klen);
		DBT pk = make_dbt(port_key, klen);
		DBT rd = make_dbt(ref_data, dlen);
		DBT pd = make_dbt(port_data, dlen);
		unsigned flags = (unsigned)(rng.next() & 0xff);

		if (rng.coin())
			t_dbclose(nullptr, nullptr, "rand_null_close");
		else
			t_dbclose(&g_ref_h.db, &g_port_h.db, "rand_close");

		if (rng.coin())
			t_dbdel(nullptr, nullptr, nullptr, flags, "rand_null_del");
		else
			t_dbdel(&g_ref_h.db, &g_port_h.db, &rk, flags, "rand_del");

		if (rng.coin())
			t_dbget(nullptr, nullptr, nullptr, nullptr, nullptr, flags,
			    "rand_null_get");
		else
			t_dbget(&g_ref_h.db, &g_port_h.db, &rk, &rd, &pd, flags,
			    "rand_get");

		if (rng.coin())
			t_dbput(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			    flags, "rand_null_put");
		else
			t_dbput(&g_ref_h.db, &g_port_h.db, &rk, &pk, &rd, &pd, flags,
			    "rand_put");

		if (rng.coin())
			t_dbseq(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			    flags, "rand_null_seq");
		else
			t_dbseq(&g_ref_h.db, &g_port_h.db, &rk, &pk, &rd, &pd, flags,
			    "rand_seq");

		if (rng.coin())
			t_dbsync(nullptr, nullptr, flags, "rand_null_sync");
		else
			t_dbsync(&g_ref_h.db, &g_port_h.db, flags, "rand_sync");

		if (rng.coin())
			t_dbfd(nullptr, nullptr, "rand_null_fd");
		else
			t_dbfd(&g_ref_h.db, &g_port_h.db, "rand_fd");

		if ((rng.next() & 7) == 0)
			t_dbpanic_pair("rand_panic");
	}
}

static void
print_table()
{
	Stat all[] = {
		st_dbopen, st_dbclose, st_dbdel, st_dbget, st_dbput,
		st_dbseq, st_dbsync, st_dbfd, st_dbpanic
	};
	const char *hdr = "function     cases      failures";
	const char *sep = "--------------------------------";

	std::printf("\n%s\n%s\n", hdr, sep);
	for (size_t i = 0; i < sizeof all / sizeof all[0]; i++)
		std::printf("%-12s %-10llu %-10llu\n",
		    all[i].name, all[i].cases, all[i].fails);
	std::printf("%s\n", sep);
}

int
main()
{
	edge_dbopen();
	edge_wrappers();

	Rng rng(0xB0157D00DULL);
	random_dbopen(rng, 100000);
	random_wrappers(rng, 100000);

	print_table();

	unsigned long long total_fails = st_dbopen.fails + st_dbclose.fails +
	    st_dbdel.fails + st_dbget.fails + st_dbput.fails + st_dbseq.fails +
	    st_dbsync.fails + st_dbfd.fails + st_dbpanic.fails;
	return total_fails == 0 ? 0 : 1;
}
