/*
 * Differential harness for PBSD batch b0285 (rec_search.c, rec_seq.c,
 * rec_close.c).  Every batch function is exercised against the ref_ oracle.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

import pbsd.lib.libc.db.recno.b0285;

namespace P = pbsd::lib_libc_db_recno::b0285;

typedef P::PAGE PAGE;
typedef P::BTREE BTREE;
typedef P::DB DB;
typedef P::DBT DBT;
typedef P::EPG EPG;
typedef P::RINTERNAL RINTERNAL;

enum {
	RET_ERROR = -1,
	RET_SUCCESS = 0,
	RET_SPECIAL = 1
};

enum { SDELETE = 0, SINSERT = 1, SEARCH = 2 };

enum {
	CURS_INIT = 0x08
};

enum {
	R_EOF = 0x00100,
	R_INMEM = 0x00800,
	R_FIXLEN = 0x00200,
	R_MEMMAPPED = 0x00400,
	R_MODIFIED = 0x01000,
	R_RDONLY = 0x02000,
	R_CLOSEFP = 0x00040,
	B_DB_LOCK = 0x04000
};

enum {
	R_CURSOR = 1,
	R_FIRST = 3,
	R_LAST = 6,
	R_NEXT = 7,
	R_PREV = 9,
	R_RECNOSYNC = 11
};

enum {
	NPAGE = 6,
	PGSZ = 256,
	MEMSZ = NPAGE * PGSZ,
	GUARD = 32768,
	BUFSZ = MEMSZ + GUARD,
	CMPZONE = MEMSZ + 128,
	NSLOT = 8,
	NIDXMAX = 6,
	BTDATAOFF = 20,
	NRINTERNAL = 8,
	NSEQ = 8,
	FN_SEARCH = 0,
	FN_SEQ,
	FN_CLOSE,
	FN_SYNC,
	NFN
};

static const char *const FNNAME[NFN] = {
	"__rec_search", "__rec_seq", "__rec_close", "__rec_sync"
};

extern "C" {
EPG *ref___rec_search(BTREE *, uint32_t, int);
int ref___rec_seq(const DB *, DBT *, DBT *, unsigned int);
int ref___rec_close(DB *);
int ref___rec_sync(const DB *, unsigned int);
}

struct RiEnt {
	uint32_t nrecs;
	uint32_t pgno;
};

struct PageCfg {
	uint32_t pgno;
	uint32_t pflags;
	uint16_t nent;
	uint16_t upper;
	uint16_t linp[NSLOT];
	RiEnt ri[NSLOT];
};

struct SeqStep {
	int status;
	uint32_t dsize;
};

struct Env {
	PageCfg pg[NPAGE];
	uint32_t getfail;
	uint32_t tflags;
	int pinned;
	recno_t recno;
	int op;
	recno_t bt_nrecs;
	uint8_t cflags;
	recno_t rcursor;
	uint32_t seqflags;
	recno_t keyrec;
	int key_zero;
	int irec_status;
	int ret_status;
	int sync_flags;
	int fuel;
	int irec_set_nrecs;
	recno_t irec_nrecs;
	int btsync_status;
	int btclose_status;
	int munmap_fail;
	int fclose_fail;
	int close_fail;
	int lseek0_fail;
	int lseek_cur_fail;
	int ftruncate_fail;
	int write_short;
	int writev_short;
	int seq_n;
	SeqStep seq[NSEQ];
	uint8_t bval;
	int rfd;
};

alignas(16) static unsigned char g_bufA[BUFSZ];
alignas(16) static unsigned char g_bufB[BUFSZ];
static unsigned char *g_mem;
static const Env *g_env;
static int g_fuel;
static uint64_t g_loghash;
static int g_logn;
static int g_pin[NPAGE];
static int g_seqcall;
static int g_irec_call;
static long g_mpool_cookie;
static FILE g_mock_file;
static unsigned char g_databuf[64];
static recno_t g_keyrec;
static BTREE g_tree;
static DB g_db;

static inline void
lc(unsigned char c)
{
	g_loghash = (g_loghash ^ c) * 1099511628211ULL;
	++g_logn;
}

static inline void
ls(const char *s)
{
	while (*s)
		lc((unsigned char)*s++);
}

static void
lu(unsigned long long v)
{
	char b[24];
	int n = 0;

	if (v == 0)
		b[n++] = '0';
	while (v) {
		b[n++] = (char)('0' + (int)(v % 10));
		v /= 10;
	}
	while (n)
		lc((unsigned char)b[--n]);
}

static void
li(long long v)
{
	if (v < 0) {
		lc('-');
		lu((unsigned long long)(-(v + 1)) + 1ULL);
	} else
		lu((unsigned long long)v);
}

static inline int
fuel(void)
{
	if (g_fuel <= 0) {
		lc('!');
		return 0;
	}
	--g_fuel;
	return 1;
}

static inline long
memoff(const void *p)
{
	return (long)((const unsigned char *)p - g_mem);
}

static inline unsigned
pidx(const void *p)
{
	long d = memoff(p);

	if (d < 0 || d >= MEMSZ)
		return 0xffffu;
	return (unsigned)(d / PGSZ);
}

extern "C" void *
mpool_get(P::MPOOL *, uint32_t pgno, unsigned int flags)
{
	ls("g");
	lu(pgno);
	lc(',');
	lu(flags);
	if (!fuel()) {
		ls("=X");
		return nullptr;
	}
	if (pgno >= (uint32_t)NPAGE || ((g_env->getfail >> pgno) & 1u)) {
		ls("=X");
		return nullptr;
	}
	++g_pin[pgno];
	ls("=ok");
	return g_mem + (size_t)pgno * PGSZ;
}

extern "C" int
mpool_put(P::MPOOL *, void *p, unsigned int flags)
{
	ls("p");
	li(memoff(p));
	lc(',');
	lu(flags);
	if (memoff(p) >= 0 && memoff(p) < MEMSZ)
		--g_pin[(unsigned)(memoff(p) / PGSZ)];
	return 0;
}

extern "C" int
__rec_ret(BTREE *, EPG *e, uint32_t nrec, DBT *key, DBT *data)
{
	ls("r");
	lu(pidx(e->page));
	lc(',');
	lu(e->index);
	lc(',');
	lu(nrec);
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	int st = g_env->ret_status;
	if (st == RET_SUCCESS) {
		if (key != nullptr) {
			g_keyrec = nrec;
			key->data = &g_keyrec;
			key->size = sizeof(g_keyrec);
		}
		if (data != nullptr) {
			data->data = g_databuf;
			data->size = (size_t)(1 + (e->index & 7));
		}
	}
	ls("=");
	li(st);
	return st;
}

extern "C" int
mock_irec(BTREE *t, uint32_t nrec)
{
	ls("i");
	lu(nrec);
	++g_irec_call;
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	if (g_env->irec_set_nrecs)
		t->bt_nrecs = g_env->irec_nrecs;
	ls("=");
	li(g_env->irec_status);
	return g_env->irec_status;
}

extern "C" int
__bt_sync(const DB *, unsigned int flags)
{
	ls("bs");
	lu(flags);
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	ls("=");
	li(g_env->btsync_status);
	return g_env->btsync_status;
}

extern "C" int
__bt_close(DB *)
{
	ls("bc");
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	ls("=");
	li(g_env->btclose_status);
	return g_env->btclose_status;
}

extern "C" int
munmap(void *, size_t len)
{
	ls("mm");
	lu((unsigned long long)len);
	if (!fuel()) {
		ls("=X");
		return -1;
	}
	if (g_env->munmap_fail) {
		ls("=fail");
		return -1;
	}
	ls("=ok");
	return 0;
}

extern "C" int
fclose(FILE *fp)
{
	ls("fc");
	li(fp == &g_mock_file ? 1 : 0);
	if (!fuel()) {
		ls("=X");
		return EOF;
	}
	if (g_env->fclose_fail) {
		ls("=fail");
		return EOF;
	}
	ls("=ok");
	return 0;
}

extern "C" int
_close(int fd)
{
	ls("cl");
	li(fd);
	if (!fuel()) {
		ls("=X");
		return -1;
	}
	if (g_env->close_fail) {
		ls("=fail");
		return -1;
	}
	ls("=ok");
	return 0;
}

extern "C" off_t
lseek(int fd, off_t off, int whence)
{
	ls("ls");
	li(fd);
	lc(',');
	li((long long)off);
	lc(',');
	li(whence);
	if (!fuel()) {
		ls("=X");
		return (off_t)-1;
	}
	if (whence == SEEK_SET && g_env->lseek0_fail) {
		ls("=fail");
		return (off_t)-1;
	}
	if (whence == SEEK_CUR && g_env->lseek_cur_fail) {
		ls("=fail");
		return (off_t)-1;
	}
	ls("=ok");
	return whence == SEEK_CUR ? 42 : 0;
}

extern "C" ssize_t
_write(int fd, const void *buf, size_t len)
{
	ls("w");
	li(fd);
	lc(',');
	lu((unsigned long long)len);
	if (!fuel()) {
		ls("=X");
		return -1;
	}
	if (g_env->write_short) {
		ls("=short");
		return (ssize_t)(len > 0 ? len - 1 : 0);
	}
	ls("=ok");
	return (ssize_t)len;
}

extern "C" ssize_t
_writev(int fd, const struct iovec *iov, int cnt)
{
	size_t total = 0;

	ls("wv");
	li(fd);
	lc(',');
	li(cnt);
	if (!fuel()) {
		ls("=X");
		return -1;
	}
	for (int i = 0; i < cnt; ++i)
		total += iov[i].iov_len;
	if (g_env->writev_short) {
		ls("=short");
		return (ssize_t)(total > 0 ? total - 1 : 0);
	}
	ls("=ok");
	return (ssize_t)total;
}

extern "C" int
ftruncate(int fd, off_t len)
{
	ls("ft");
	li(fd);
	lc(',');
	li((long long)len);
	if (!fuel()) {
		ls("=X");
		return -1;
	}
	if (g_env->ftruncate_fail) {
		ls("=fail");
		return -1;
	}
	ls("=ok");
	return 0;
}

extern "C" int
mock_seq(const DB *, DBT *key, DBT *data, unsigned int flags)
{
	int i = g_seqcall < g_env->seq_n ? g_seqcall : g_env->seq_n - 1;

	ls("sq");
	lu(flags);
	lc(',');
	lu((unsigned)i);
	++g_seqcall;
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	const SeqStep &s = g_env->seq[i];
	if (s.status == RET_SUCCESS && data != nullptr) {
		memset(g_databuf, (int)(0x80 | (i & 0x7f)), sizeof g_databuf);
		data->data = g_databuf;
		data->size = s.dsize;
	}
	ls("=");
	li(s.status);
	return s.status;
}

struct Rng {
	uint64_t s;

	explicit Rng(uint64_t seed) : s(seed) {}

	uint64_t next(void)
	{
		uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}

	uint32_t n(uint32_t m) { return m ? (uint32_t)(next() % m) : 0u; }
};

static void
layout_ri(PageCfg &c, Rng &r)
{
	uint32_t off = PGSZ;

	for (int s = 0; s < NSLOT; ++s) {
		c.ri[s].nrecs = 1u + r.n(20);
		c.ri[s].pgno = 2u + r.n(4);
		if (c.ri[s].pgno >= (uint32_t)NPAGE)
			c.ri[s].pgno = (uint32_t)(2 + s % 4);
	}
	for (int k = 0; k < c.nent; ++k) {
		int s = k % NSLOT;
		off -= NRINTERNAL;
		c.linp[s] = (uint16_t)off;
	}
	uint32_t up = PGSZ;
	for (int s = 0; s < c.nent; ++s)
		if (c.linp[s % NSLOT] < up)
			up = c.linp[s % NSLOT];
	if (c.nent == 0)
		up = off;
	c.upper = (uint16_t)up;
}

static inline void
wr16(unsigned char *p, uint16_t v)
{
	memcpy(p, &v, 2);
}

static inline void
wr32(unsigned char *p, uint32_t v)
{
	memcpy(p, &v, 4);
}

static void
build(const Env &E, unsigned char *buf)
{
	memset(buf, 0x7f, BUFSZ);
	for (int p = 0; p < NPAGE; ++p) {
		unsigned char *b = buf + (size_t)p * PGSZ;
		const PageCfg &c = E.pg[p];

		wr32(b + 0, c.pgno);
		wr32(b + 4, 0);
		wr32(b + 8, 0);
		wr32(b + 12, c.pflags);
		wr16(b + 16, (uint16_t)(BTDATAOFF + 2 * c.nent));
		wr16(b + 18, c.upper);
		for (int s = 0; s < NSLOT; ++s)
			wr16(b + 20 + 2 * s, c.linp[s]);
		for (int s = 0; s < NSLOT; ++s) {
			unsigned char *rec = b + c.linp[s];
			wr32(rec + 0, c.ri[s].nrecs);
			wr32(rec + 4, c.ri[s].pgno);
		}
	}

	memset(&g_tree, 0, sizeof g_tree);
	memset(&g_db, 0, sizeof g_db);
	g_tree.bt_mp = (P::MPOOL *)&g_mpool_cookie;
	g_tree.bt_dbp = &g_db;
	g_tree.bt_sp = g_tree.bt_stack;
	g_tree.bt_pinned = E.pinned < 0 ? nullptr
	    : (PAGE *)(buf + (size_t)E.pinned * PGSZ);
	g_tree.bt_cursor.flags = E.cflags;
	g_tree.bt_cursor.rcursor = E.rcursor;
	g_tree.bt_nrecs = E.bt_nrecs;
	g_tree.bt_bval = E.bval;
	g_tree.bt_rfd = E.rfd;
	g_tree.bt_rfp = (E.tflags & R_CLOSEFP) ? &g_mock_file : nullptr;
	g_tree.bt_smap = (char *)(buf + MEMSZ);
	g_tree.bt_msize = 64;
	g_tree.flags = (uint32_t)E.tflags;
	g_tree.bt_irec = mock_irec;
	g_db.internal = &g_tree;
	g_db.seq = mock_seq;
}

struct Snap {
	int rc;
	int err;
	int ep_null;
	unsigned ep_off;
	uint16_t ep_index;
	unsigned cur_page_off;
	uint16_t cur_index;
	unsigned pinned_off;
	unsigned spdepth;
	uint32_t stkpg[50];
	uint16_t stkidx[50];
	uint32_t rcursor;
	uint8_t cflags;
	uint32_t tflags;
	uint32_t bt_nrecs;
	uint64_t loghash;
	int logn;
	int pin[NPAGE];
	unsigned char mem[CMPZONE];
};

static Snap g_sa, g_sb;

static void
snapshot(Snap &S, int rc, EPG *ep, const unsigned char *buf)
{
	memset(&S, 0, sizeof S);
	S.rc = rc;
	S.err = errno;
	if (ep == nullptr) {
		S.ep_null = 1;
	} else {
		S.ep_null = 0;
		S.ep_off = ep->page == nullptr ? 0xffffu
		    : (unsigned)memoff(ep->page);
		S.ep_index = ep->index;
	}
	S.cur_page_off = g_tree.bt_cur.page == nullptr ? 0xffffu
	    : (unsigned)memoff(g_tree.bt_cur.page);
	S.cur_index = g_tree.bt_cur.index;
	S.pinned_off = g_tree.bt_pinned == nullptr ? 0xffffu
	    : (unsigned)(((const unsigned char *)g_tree.bt_pinned - buf));
	S.spdepth = (unsigned)(g_tree.bt_sp - g_tree.bt_stack);
	for (int i = 0; i < 50; ++i) {
		S.stkpg[i] = g_tree.bt_stack[i].pgno;
		S.stkidx[i] = g_tree.bt_stack[i].index;
	}
	S.rcursor = g_tree.bt_cursor.rcursor;
	S.cflags = g_tree.bt_cursor.flags;
	S.tflags = g_tree.flags;
	S.bt_nrecs = g_tree.bt_nrecs;
	S.loghash = g_loghash;
	S.logn = g_logn;
	for (int i = 0; i < NPAGE; ++i)
		S.pin[i] = g_pin[i];
	memcpy(S.mem, buf, CMPZONE);
}

static void
run(const Env &E, int fn, unsigned char *buf, Snap &S, int useport)
{
	int rc = 0;
	EPG *ep = nullptr;
	DBT key, data;

	g_mem = buf;
	g_env = &E;
	g_fuel = E.fuel;
	g_loghash = 1469598103934665603ULL;
	g_logn = 0;
	g_seqcall = 0;
	g_irec_call = 0;
	memset(g_pin, 0, sizeof g_pin);
	build(E, buf);

	recno_t krec = E.keyrec;
	key.data = E.key_zero ? nullptr : &krec;
	key.size = sizeof(krec);
	memset(&data, 0, sizeof data);

	errno = 0;
	switch (fn) {
	case FN_SEARCH:
		ep = useport ? P::__rec_search(&g_tree, E.recno,
		    static_cast<P::SRCHOP>(E.op))
		    : ref___rec_search(&g_tree, E.recno, E.op);
		rc = ep == nullptr ? 0 : 1;
		break;
	case FN_SEQ:
		rc = useport ? P::__rec_seq(&g_db, &key, &data, E.seqflags)
		    : ref___rec_seq(&g_db, &key, &data, E.seqflags);
		break;
	case FN_CLOSE:
		rc = useport ? P::__rec_close(&g_db)
		    : ref___rec_close(&g_db);
		break;
	case FN_SYNC:
		rc = useport ? P::__rec_sync(&g_db, (unsigned)E.sync_flags)
		    : ref___rec_sync(&g_db, (unsigned)E.sync_flags);
		break;
	}
	snapshot(S, rc, ep, buf);
}

static int
runcase(const Env &E, int fn)
{
	run(E, fn, g_bufA, g_sa, 1);
	run(E, fn, g_bufB, g_sb, 0);
	return memcmp(&g_sa, &g_sb, sizeof(Snap)) == 0;
}

static void
fixup(Env &E, int fn)
{
	static const uint32_t LEAF[4] = { 2, 3, 4, 5 };
	static const uint32_t INTL[2] = { 1, 4 };

	for (int p = 0; p < NPAGE; ++p) {
		if (E.pg[p].pflags & 0x10) {
			E.pg[p].nent = 0;
			E.pg[p].upper = PGSZ;
		} else {
			if (E.pg[p].nent < 1)
				E.pg[p].nent = 1;
			for (int s = 0; s < NSLOT; ++s) {
				E.pg[p].ri[s].pgno =
				    (E.pg[p].ri[s].pgno & 1u) ? LEAF[s % 4]
				    : INTL[s % 2];
				if (E.pg[p].ri[s].pgno >= (uint32_t)NPAGE)
					E.pg[p].ri[s].pgno = LEAF[s % 4];
			}
		}
		layout_ri(E.pg[p], Rng(0xabc + p));
	}
	if (E.pinned >= NPAGE)
		E.pinned = NPAGE - 1;
	if (E.fuel < 8)
		E.fuel = 8;
	if (fn == FN_SEQ && E.key_zero)
		E.key_zero = 0;
	if (fn == FN_SYNC) {
		if (E.seq_n < 1)
			E.seq_n = 1;
		for (int i = 0; i < E.seq_n; ++i) {
			if (E.seq[i].status == RET_SUCCESS && E.seq[i].dsize == 0)
				E.seq[i].dsize = 1;
		}
		E.seq[E.seq_n - 1].status = RET_SPECIAL;
	}
}

static void
genEnv(Env &E, Rng &r, int fn)
{
	memset(&E, 0, sizeof E);

	for (int p = 0; p < NPAGE; ++p) {
		int isleaf = (p >= 2);
		int n = isleaf ? 0 : 1 + (int)r.n(NIDXMAX);
		if (n > NIDXMAX)
			n = NIDXMAX;
		E.pg[p].nent = (uint16_t)n;
		E.pg[p].pgno = (uint32_t)p;
		E.pg[p].pflags = isleaf ? 0x10u : 0x08u;
		layout_ri(E.pg[p], r);
	}

	E.getfail = r.n(5) == 0 ? r.n(64) : 0u;
	E.tflags = (r.n(2) ? R_MODIFIED : 0u)
	    | (r.n(2) ? R_RDONLY : 0u)
	    | (r.n(3) == 0 ? R_INMEM : 0u)
	    | (r.n(4) == 0 ? R_EOF : 0u)
	    | (r.n(4) == 0 ? R_FIXLEN : 0u)
	    | (r.n(5) == 0 ? R_MEMMAPPED : 0u)
	    | (r.n(5) == 0 ? R_CLOSEFP : 0u)
	    | (r.n(6) == 0 ? B_DB_LOCK : 0u);
	E.pinned = (int)r.n(NPAGE + 1) - 1;
	E.recno = r.n(100);
	E.op = (int)r.n(3);
	E.bt_nrecs = r.n(50);
	E.cflags = (uint8_t)(r.n(2) ? CURS_INIT : 0) | (uint8_t)r.n(8);
	E.rcursor = 1u + r.n(40);
	static const unsigned SF[8] = {
		R_CURSOR, R_FIRST, R_LAST, R_NEXT, R_PREV, 0, 2, 99
	};
	E.seqflags = SF[r.n(8)];
	E.keyrec = r.n(50);
	E.key_zero = (int)r.n(8) == 0;
	E.irec_status = r.n(3) == 0 ? RET_ERROR : RET_SUCCESS;
	E.irec_set_nrecs = r.n(2);
	E.irec_nrecs = 1u + r.n(60);
	E.ret_status = r.n(4) == 0 ? RET_ERROR : RET_SUCCESS;
	E.sync_flags = r.n(2) ? R_RECNOSYNC : r.n(16);
	E.fuel = 16 + (int)r.n(80);
	E.btsync_status = r.n(3) == 0 ? RET_ERROR : RET_SUCCESS;
	E.btclose_status = r.n(4) == 0 ? RET_ERROR : RET_SUCCESS;
	E.munmap_fail = r.n(6) == 0;
	E.fclose_fail = r.n(6) == 0;
	E.close_fail = r.n(6) == 0;
	E.lseek0_fail = r.n(7) == 0;
	E.lseek_cur_fail = r.n(7) == 0;
	E.ftruncate_fail = r.n(7) == 0;
	E.write_short = r.n(8) == 0;
	E.writev_short = r.n(8) == 0;
	E.bval = (uint8_t)(0x80 | r.n(128));
	E.rfd = 3 + (int)r.n(4);
	E.seq_n = 1 + (int)r.n(NSEQ);
	for (int i = 0; i < NSEQ; ++i) {
		E.seq[i].status = i + 1 < E.seq_n ? RET_SUCCESS
		    : (r.n(3) == 0 ? RET_ERROR : RET_SPECIAL);
		E.seq[i].dsize = 1u + r.n(32);
	}
	fixup(E, fn);
}

static long g_cases[NFN];
static long g_fails[NFN];
static int g_shown;

static void
check(const Env &E, int fn, const char *what, long which)
{
	++g_cases[fn];
	if (runcase(E, fn))
		return;
	++g_fails[fn];
	if (g_shown < 16) {
		++g_shown;
		printf("  MISMATCH %-12s %s #%ld: rc %d/%d errno %d/%d "
		    "ep %u/%u idx %u/%u mem %s log %llx/%llx\n",
		    FNNAME[fn], what, which, g_sa.rc, g_sb.rc,
		    g_sa.err, g_sb.err,
		    g_sa.ep_off, g_sb.ep_off, g_sa.ep_index, g_sb.ep_index,
		    memcmp(g_sa.mem, g_sb.mem, CMPZONE) ? "diff" : "same",
		    (unsigned long long)g_sa.loghash,
		    (unsigned long long)g_sb.loghash);
	}
}

static void
edge_search(void)
{
	Env E;
	long c = 0;

	for (int op = 0; op < 3; ++op)
	for (int depth = 0; depth < 3; ++depth)
	for (uint32_t rec = 0; rec < 40; ++rec)
	for (int gf = 0; gf < 4; ++gf) {
		Rng r(0x1000 + c);
		genEnv(E, r, FN_SEARCH);
		E.op = op;
		E.recno = rec;
		E.fuel = 64;
		if (depth == 0) {
			E.pg[1].nent = 1;
			E.pg[1].ri[0].nrecs = 50;
			E.pg[1].ri[0].pgno = 2;
			E.pg[2].pflags = 0x10;
		} else if (depth == 1) {
			E.pg[1].nent = 2;
			E.pg[1].ri[0].nrecs = 5;
			E.pg[1].ri[0].pgno = 2;
			E.pg[1].ri[1].nrecs = 10;
			E.pg[1].ri[1].pgno = 3;
			E.pg[2].pflags = E.pg[3].pflags = 0x10;
		} else {
			E.pg[1].nent = 1;
			E.pg[1].ri[0].pgno = 4;
			E.pg[4].nent = 2;
			E.pg[4].ri[0].nrecs = 3;
			E.pg[4].ri[0].pgno = 2;
			E.pg[4].ri[1].nrecs = 7;
			E.pg[4].ri[1].pgno = 3;
			E.pg[2].pflags = E.pg[3].pflags = 0x10;
		}
		E.getfail = gf == 0 ? 0u : gf == 1 ? 2u : gf == 2 ? 4u : 0x3eu;
		fixup(E, FN_SEARCH);
		check(E, FN_SEARCH, "edge", c++);
	}
}

static void
edge_seq(void)
{
	Env E;
	long c = 0;
	static const unsigned FL[7] = {
		R_CURSOR, R_FIRST, R_LAST, R_NEXT, R_PREV, 0, 99
	};

	for (int fi = 0; fi < 7; ++fi)
	for (int ci = 0; ci < 2; ++ci)
	for (uint32_t kr = 0; kr < 5; ++kr)
	for (uint32_t nr = 0; nr < 20; ++nr)
	for (uint32_t rc = 0; rc < 20; ++rc)
	for (int pin = -1; pin < NPAGE; ++pin)
	for (int dbk = 0; dbk < 2; ++dbk)
	for (int irec = 0; irec < 3; ++irec) {
		Rng r(0x5000 + c);
		genEnv(E, r, FN_SEQ);
		E.seqflags = FL[fi];
		E.cflags = ci ? (uint8_t)CURS_INIT : 0;
		E.keyrec = kr;
		E.bt_nrecs = nr;
		E.rcursor = rc;
		E.pinned = pin;
		E.tflags = (E.tflags & ~(R_EOF | R_INMEM | B_DB_LOCK))
		    | (dbk ? B_DB_LOCK : 0);
		if (irec == 0) {
			E.irec_status = RET_SUCCESS;
			E.irec_set_nrecs = 1;
			E.irec_nrecs = 30;
		} else if (irec == 1) {
			E.irec_status = RET_ERROR;
		} else {
			E.irec_status = RET_SUCCESS;
			E.irec_set_nrecs = 0;
		}
		E.ret_status = RET_SUCCESS;
		E.fuel = 64;
		fixup(E, FN_SEQ);
		check(E, FN_SEQ, "edge", c++);
	}
}

static void
edge_close(void)
{
	Env E;
	long c = 0;

	for (int mm = 0; mm < 2; ++mm)
	for (int im = 0; im < 2; ++im)
	for (int cf = 0; cf < 2; ++cf)
	for (int mf = 0; mf < 2; ++mf)
	for (int ff = 0; ff < 2; ++ff)
	for (int clf = 0; clf < 2; ++clf)
	for (int bcf = 0; bcf < 2; ++bcf)
	for (int pin = -1; pin < 2; ++pin) {
		Rng r(0x9000 + c);
		genEnv(E, r, FN_CLOSE);
		E.tflags = R_MODIFIED
		    | (mm ? R_MEMMAPPED : 0)
		    | (im ? R_INMEM : 0)
		    | (cf ? R_CLOSEFP : 0)
		    | R_RDONLY;
		E.munmap_fail = mf;
		E.fclose_fail = ff;
		E.close_fail = clf;
		E.btclose_status = bcf ? RET_ERROR : RET_SUCCESS;
		E.sync_flags = 0;
		E.pinned = pin;
		E.lseek0_fail = 0;
		E.fuel = 96;
		fixup(E, FN_CLOSE);
		check(E, FN_CLOSE, "edge", c++);
	}
}

static void
edge_sync(void)
{
	Env E;
	long c = 0;

	for (int rs = 0; rs < 2; ++rs)
	for (int mod = 0; mod < 2; ++mod)
	for (int fix = 0; fix < 2; ++fix)
	for (int eof = 0; eof < 2; ++eof)
	for (int im = 0; im < 2; ++im)
	for (int sn = 1; sn <= 4; ++sn)
	for (int l0 = 0; l0 < 2; ++l0)
	for (int wc = 0; wc < 2; ++wc)
	for (int ft = 0; ft < 2; ++ft)
	for (int ws = 0; ws < 2; ++ws) {
		Rng r(0xd000 + c);
		genEnv(E, r, FN_SYNC);
		E.sync_flags = rs ? R_RECNOSYNC : 0;
		E.tflags = (mod ? R_MODIFIED : 0)
		    | (fix ? R_FIXLEN : 0)
		    | (eof ? R_EOF : 0)
		    | (im ? R_INMEM : 0);
		E.seq_n = sn;
		for (int i = 0; i < sn; ++i) {
			E.seq[i].status = RET_SUCCESS;
			E.seq[i].dsize = (uint32_t)(1 + i);
		}
		E.lseek0_fail = l0;
		E.lseek_cur_fail = wc;
		E.ftruncate_fail = ft;
		E.write_short = fix && ws;
		E.writev_short = !fix && ws;
		E.irec_status = RET_SUCCESS;
		E.btsync_status = RET_SUCCESS;
		E.fuel = 96;
		fixup(E, FN_SYNC);
		check(E, FN_SYNC, "edge", c++);
	}
}

static void
sweep(int fn, uint64_t seed, long n)
{
	Env E;

	for (long i = 0; i < n; ++i) {
		Rng r(seed + (uint64_t)i * 0x9E3779B97F4A7C15ULL);
		genEnv(E, r, fn);
		check(E, fn, "rand", i);
	}
}

int
main(void)
{
	edge_search();
	edge_seq();
	edge_close();
	edge_sync();

	sweep(FN_SEARCH, 0x28500001ULL, 50000);
	sweep(FN_SEQ, 0x28500002ULL, 50000);
	sweep(FN_CLOSE, 0x28500003ULL, 50000);
	sweep(FN_SYNC, 0x28500004ULL, 50000);

	int fail = 0;
	printf("b0285 differential harness\n");
	printf("%-14s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < NFN; ++i) {
		printf("%-14s %10ld %10ld\n", FNNAME[i], g_cases[i], g_fails[i]);
		if (g_fails[i] != 0)
			fail = 1;
	}
	return fail ? 1 : 0;
}
