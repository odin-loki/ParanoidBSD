/*
 * Differential harness for batch b0226: mpool.c + mpool-compat.c.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.db.mpool.b0226;

namespace P = pbsd::lib_libc_db_mpool::b0226;

extern "C" {
typedef uint32_t pgno_t;
typedef unsigned int u_int;
typedef uint8_t u_int8_t;

#define HASHSIZE 128

#define TRACEBUF
#define TAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;						\
	struct type **tqh_last;						\
	TRACEBUF								\
}
#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;						\
	struct type **tqe_prev;						\
	TRACEBUF								\
}

typedef struct _bkt {
	TAILQ_ENTRY(_bkt) hq;
	TAILQ_ENTRY(_bkt) q;
	void *page;
	pgno_t pgno;
	u_int8_t flags;
} BKT;

typedef struct MPOOL {
	TAILQ_HEAD(_lqh, _bkt) lqh;
	TAILQ_HEAD(_hqh, _bkt) hqh[HASHSIZE];
	pgno_t curcache;
	pgno_t maxcache;
	pgno_t npages;
	unsigned long pagesize;
	int fd;
	void (*pgin)(void *, pgno_t, void *);
	void (*pgout)(void *, pgno_t, void *);
	void *pgcookie;
} MPOOL;

MPOOL *ref_mpool_open(void *, int, pgno_t, pgno_t);
void ref_mpool_filter(MPOOL *, void (*)(void *, pgno_t, void *),
    void (*)(void *, pgno_t, void *), void *);
void *ref_mpool_new(MPOOL *, pgno_t *, u_int);
int ref_mpool_delete(MPOOL *, void *);
void *ref_mpool_get(MPOOL *, pgno_t, u_int);
int ref_mpool_put(MPOOL *, void *, u_int);
int ref_mpool_close(MPOOL *);
int ref_mpool_sync(MPOOL *);
void *ref___mpool_new__44bsd(MPOOL *, pgno_t *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr long SWEEP_ITERS = 200000;
constexpr pgno_t MAX_PAGE_NUMBER = 0xffffffffu;

constexpr u_int H_DIRTY = 0x01;
constexpr u_int H_IGNOREPIN = 0x01;
constexpr u_int H_PAGE_REQUEST = 0x01;
constexpr u_int H_PAGE_NEXT = 0x02;
constexpr int RET_SUCCESS = 0;
constexpr int RET_ERROR = -1;

struct Stat {
	const char *name;
	long cases;
	long fails;
};

static std::uint64_t rng = 0xB0226C0DEULL;

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

static int
mk_temp_file(const unsigned char *data, size_t len)
{
	char path[] = "/tmp/mpool_b0226_XXXXXX";
        int fd = mkostemp(path, 0);
	if (fd < 0)
		return -1;
	(void)unlink(path);
	if (len > 0) {
		size_t off = 0;
		while (off < len) {
			ssize_t n = write(fd, data + off, len - off);
			if (n <= 0) {
				close(fd);
				return -1;
			}
			off += (size_t)n;
		}
	}
	if (lseek(fd, 0, SEEK_SET) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static std::vector<unsigned char>
fill_pattern(size_t len, unsigned seed)
{
	std::vector<unsigned char> v(len, GUARD);
	for (size_t i = 0; i < len; i++)
		v[i] = (unsigned char)((seed + i * 17) & 0xff);
	return v;
}

static bool
same_page_bytes(const void *a, const void *b, size_t pagesize)
{
	return std::memcmp(a, b, pagesize) == 0;
}

static bool
bufs_equal(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size()) == 0;
}

struct FilterCtx {
	int pgin;
	int pgout;
	unsigned char tag;
};

static void
ref_pgin_cb(void *cookie, pgno_t, void *page)
{
	auto *c = static_cast<FilterCtx *>(cookie);
	c->pgin++;
	if (page)
		*(unsigned char *)page ^= c->tag;
}

static void
ref_pgout_cb(void *cookie, pgno_t, void *page)
{
	auto *c = static_cast<FilterCtx *>(cookie);
	c->pgout++;
	if (page)
		*(unsigned char *)page ^= c->tag;
}

static void
port_pgin_cb(void *cookie, P::pgno_t, void *page)
{
	auto *c = static_cast<FilterCtx *>(cookie);
	c->pgin++;
	if (page)
		*(unsigned char *)page ^= c->tag;
}

static void
port_pgout_cb(void *cookie, P::pgno_t, void *page)
{
	auto *c = static_cast<FilterCtx *>(cookie);
	c->pgout++;
	if (page)
		*(unsigned char *)page ^= c->tag;
}

static ssize_t
read_file_at(int fd, off_t off, void *buf, size_t len)
{
	return pread(fd, buf, len, off);
}

static void
test_mpool_open(Stat &st)
{
	st.cases++;

	int fd = mk_temp_file(nullptr, 0);
	if (fd < 0) {
		fail(st, "mk_temp_file");
		return;
	}
	auto data = fill_pattern(2048, 0x80);
	close(fd);
	fd = mk_temp_file(data.data(), data.size());
	if (fd < 0) {
		fail(st, "mk_temp_file data");
		return;
	}

	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	int fd2 = dup(fd);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	if (!rmp || !pmp) {
		fail(st, "open valid file");
		close(fd);
		close(fd2);
		return;
	}
	if (rmp->npages != pmp->npages || rmp->pagesize != pmp->pagesize ||
	    rmp->maxcache != pmp->maxcache) {
		fail(st, "open fields mismatch");
	}
	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);

	st.cases++;
	errno = 0;
	rmp = ref_mpool_open(nullptr, -1, 512, 4);
	errno = 0;
	pmp = P::mpool_open(nullptr, -1, 512, 4);
	if (rmp != nullptr || pmp != nullptr)
		fail(st, "bad fd should fail");
	if (errno == 0)
		fail(st, "bad fd errno");

	st.cases++;
	int pfd[2];
	if (pipe(pfd) == 0) {
		errno = 0;
		rmp = ref_mpool_open(nullptr, pfd[0], 512, 4);
		int e1 = errno;
		errno = 0;
		pmp = P::mpool_open(nullptr, pfd[1], 512, 4);
		int e2 = errno;
		if (rmp != nullptr || pmp != nullptr)
			fail(st, "pipe fd should fail");
		if (e1 != ESPIPE || e2 != ESPIPE)
			fail(st, "pipe errno ESPIPE");
		close(pfd[0]);
		close(pfd[1]);
	}

	st.cases++;
	auto tiny = fill_pattern(100, 0xff);
	fd = mk_temp_file(tiny.data(), tiny.size());
	fd2 = dup(fd);
	rmp = ref_mpool_open(nullptr, fd, 512, 4);
	pmp = P::mpool_open(nullptr, fd2, 512, 4);
	if (!rmp || !pmp) {
		fail(st, "non-aligned open");
	} else if (rmp->npages != pmp->npages) {
		fail(st, "npages non-aligned");
	}
	if (rmp)
		ref_mpool_close(rmp);
	if (pmp)
		P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_mpool_filter(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(1024, 3);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	if (!rmp || !pmp) {
		fail(st, "open for filter");
		close(fd);
		close(fd2);
		return;
	}
	FilterCtx rc{0, 0, 0x55};
	FilterCtx pc{0, 0, 0x55};
	ref_mpool_filter(rmp, ref_pgin_cb, ref_pgout_cb, &rc);
	P::mpool_filter(pmp, port_pgin_cb, port_pgout_cb, &pc);

	void *rp = ref_mpool_get(rmp, 0, 0);
	void *pp = P::mpool_get(pmp, 0, 0);
	if (!rp || !pp) {
		fail(st, "get after filter");
	} else if (!same_page_bytes(rp, pp, 512)) {
		fail(st, "filter pgin page bytes");
	}
	if (rc.pgin != pc.pgin)
		fail(st, "pgin count");

	ref_mpool_put(rmp, rp, H_DIRTY);
	P::mpool_put(pmp, pp, H_DIRTY);
	if (ref_mpool_sync(rmp) != P::mpool_sync(pmp))
		fail(st, "sync ret after filter");
	if (rc.pgout != pc.pgout)
		fail(st, "pgout count");

	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_compat_new(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(512, 0xcc);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	pgno_t rpg = 0, ppg = 0;
	void *rn = ref___mpool_new__44bsd(rmp, &rpg);
	void *pn = P::__mpool_new__44bsd(pmp, &ppg);
	if (!rn || !pn || rpg != ppg)
		fail(st, "compat new");
	if (rn) {
		ref_mpool_put(rmp, rn, 0);
		P::mpool_put(pmp, pn, 0);
	}
	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_mpool_new(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(512, 7);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	if (!rmp || !pmp) {
		fail(st, "open for new");
		close(fd);
		close(fd2);
		return;
	}

	pgno_t rpg = 0, ppg = 0;
	void *rn = ref_mpool_new(rmp, &rpg, H_PAGE_NEXT);
	void *pn = P::mpool_new(pmp, &ppg, H_PAGE_NEXT);
	if (!rn || !pn) {
		fail(st, "new next null");
	} else if (rpg != ppg) {
		fail(st, "new next pgno");
	} else if (rmp->npages != pmp->npages) {
		fail(st, "new next npages");
	}

	ref_mpool_put(rmp, rn, 0);
	P::mpool_put(pmp, pn, 0);

	st.cases++;
	rpg = 99;
	ppg = 99;
	rn = ref_mpool_new(rmp, &rpg, H_PAGE_REQUEST);
	pn = P::mpool_new(pmp, &ppg, H_PAGE_REQUEST);
	if (!rn || !pn) {
		fail(st, "new request null");
	} else if (rpg != 99 || ppg != 99) {
		fail(st, "new request pgno addr");
	} else if (rmp->npages != pmp->npages) {
		fail(st, "new request npages");
	}
	ref_mpool_put(rmp, rn, 0);
	P::mpool_put(pmp, pn, 0);

	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_mpool_get(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(1536, 0xa0);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 2);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 2);
	if (!rmp || !pmp) {
		fail(st, "open for get");
		close(fd);
		close(fd2);
		return;
	}

	void *r0 = ref_mpool_get(rmp, 0, 0);
	void *p0 = P::mpool_get(pmp, 0, 0);
	if (!r0 || !p0 || !same_page_bytes(r0, p0, 512))
		fail(st, "get page 0");

	void *r1 = ref_mpool_get(rmp, 1, 0);
	void *p1 = P::mpool_get(pmp, 1, 0);
	if (!r1 || !p1 || !same_page_bytes(r1, p1, 512))
		fail(st, "get page 1");

	st.cases++;
	void *r0c = ref_mpool_get(rmp, 0, 0);
	void *p0c = P::mpool_get(pmp, 0, 0);
	if (!r0c || !p0c)
		fail(st, "cached get null");
	else if (r0c != r0 || p0c != p0)
		fail(st, "cached same pointer");
	else if (!same_page_bytes(r0c, p0c, 512))
		fail(st, "cached bytes");

	st.cases++;
	void *r2 = ref_mpool_get(rmp, 2, H_IGNOREPIN);
	void *p2 = P::mpool_get(pmp, 2, H_IGNOREPIN);
	if (!r2 || !p2)
		fail(st, "beyond eof get null");
	else if (!same_page_bytes(r2, p2, 512))
		fail(st, "beyond eof bytes");

	ref_mpool_put(rmp, r0, 0);
	P::mpool_put(pmp, p0, 0);
	ref_mpool_put(rmp, r1, 0);
	P::mpool_put(pmp, p1, 0);
	ref_mpool_put(rmp, r0c, 0);
	P::mpool_put(pmp, p0c, 0);
	ref_mpool_put(rmp, r2, 0);
	P::mpool_put(pmp, p2, 0);

	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);

	st.cases++;
	auto half = fill_pattern(256, 0x81);
	fd = mk_temp_file(half.data(), half.size());
	fd2 = dup(fd);
	errno = 0;
	rmp = ref_mpool_open(nullptr, fd, 512, 2);
	pmp = P::mpool_open(nullptr, fd2, 512, 2);
	int e1 = errno;
	errno = 0;
	r0 = ref_mpool_get(rmp, 0, 0);
	int e2 = errno;
	errno = 0;
	p0 = P::mpool_get(pmp, 0, 0);
	int e3 = errno;
	if (r0 != nullptr || p0 != nullptr)
		fail(st, "partial read should fail");
	if (e2 != EINVAL || e3 != EINVAL)
		fail(st, "partial read errno");
	if (rmp)
		ref_mpool_close(rmp);
	if (pmp)
		P::mpool_close(pmp);
	close(fd);
	close(fd2);
	(void)e1;
}

static void
test_mpool_put(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(512, 0x42);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	if (!rmp || !pmp) {
		fail(st, "open for put");
		close(fd);
		close(fd2);
		return;
	}
	void *rp = ref_mpool_get(rmp, 0, 0);
	void *pp = P::mpool_get(pmp, 0, 0);
	((unsigned char *)rp)[0] = 0xde;
	((unsigned char *)pp)[0] = 0xde;
	int rr = ref_mpool_put(rmp, rp, H_DIRTY);
	int pr = P::mpool_put(pmp, pp, H_DIRTY);
	if (rr != pr || rr != RET_SUCCESS)
		fail(st, "put dirty ret");
	if (ref_mpool_sync(rmp) != P::mpool_sync(pmp))
		fail(st, "sync after dirty");
	unsigned char rb[512], pb[512];
	if (read_file_at(fd, 0, rb, 512) != 512 ||
	    read_file_at(fd2, 0, pb, 512) != 512 ||
	    std::memcmp(rb, pb, 512) != 0)
		fail(st, "file after dirty sync");
	if (rb[0] != 0xde)
		fail(st, "dirty byte on disk");

	st.cases++;
	rp = ref_mpool_get(rmp, 0, 0);
	pp = P::mpool_get(pmp, 0, 0);
	rr = ref_mpool_put(rmp, rp, 0);
	pr = P::mpool_put(pmp, pp, 0);
	if (rr != pr)
		fail(st, "put clean ret");

	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_mpool_delete(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(1024, 11);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	if (!rmp || !pmp) {
		fail(st, "open for delete");
		close(fd);
		close(fd2);
		return;
	}
	pgno_t rpg = 0, ppg = 0;
	void *rn = ref_mpool_new(rmp, &rpg, H_PAGE_NEXT);
	void *pn = P::mpool_new(pmp, &ppg, H_PAGE_NEXT);
	if (!rn || !pn)
		fail(st, "new for delete");
	if (ref_mpool_delete(rmp, rn) != P::mpool_delete(pmp, pn))
		fail(st, "delete ret");
	if (rmp->curcache != pmp->curcache)
		fail(st, "curcache after delete");
	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_mpool_close(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(512, 1);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 4);
	void *rp = ref_mpool_get(rmp, 0, 0);
	void *pp = P::mpool_get(pmp, 0, 0);
	ref_mpool_put(rmp, rp, 0);
	P::mpool_put(pmp, pp, 0);
	if (ref_mpool_close(rmp) != P::mpool_close(pmp))
		fail(st, "close ret");
	close(fd);
	close(fd2);
}

static void
test_mpool_sync(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(2048, 0x33);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 2);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 2);
	for (pgno_t pg = 0; pg < 3; pg++) {
		void *rp = ref_mpool_get(rmp, pg, 0);
		void *pp = P::mpool_get(pmp, pg, 0);
		((unsigned char *)rp)[pg] = (unsigned char)(0x80 + pg);
		((unsigned char *)pp)[pg] = (unsigned char)(0x80 + pg);
		ref_mpool_put(rmp, rp, H_DIRTY);
		P::mpool_put(pmp, pp, H_DIRTY);
	}
	if (ref_mpool_sync(rmp) != P::mpool_sync(pmp))
		fail(st, "sync multi dirty");
	for (pgno_t pg = 0; pg < 3; pg++) {
		unsigned char rb[512], pb[512];
		read_file_at(fd, (off_t)(512 * pg), rb, 512);
		read_file_at(fd2, (off_t)(512 * pg), pb, 512);
		if (rb[pg] != pb[pg] || rb[pg] != (unsigned char)(0x80 + pg))
			fail(st, "sync disk content");
	}

	st.cases++;
	char ropath[] = "/tmp/mpool_b0226_ro_XXXXXX";
	int wro = mkostemp(ropath, 0);
	if (wro >= 0) {
		unsigned char z[512] = {0};
		write(wro, z, 512);
		int wdup = dup(wro);
		int wdup2 = dup(wro);
		MPOOL *rmp2 = ref_mpool_open(nullptr, wdup, 512, 2);
		P::MPOOL *pmp2 = P::mpool_open(nullptr, wdup2, 512, 2);
		void *rp = ref_mpool_get(rmp2, 0, 0);
		void *pp = P::mpool_get(pmp2, 0, 0);
		((unsigned char *)rp)[1] = 0xfe;
		((unsigned char *)pp)[1] = 0xfe;
		ref_mpool_put(rmp2, rp, H_DIRTY);
		P::mpool_put(pmp2, pp, H_DIRTY);
		close(wdup);
		close(wdup2);
		close(wro);
		int rofd = open(ropath, O_RDONLY);
		int rofd2 = open(ropath, O_RDONLY);
		rmp2->fd = rofd;
		pmp2->fd = rofd2;
		int rs = ref_mpool_sync(rmp2);
		int ps = P::mpool_sync(pmp2);
		if (rs != ps || rs != RET_ERROR)
			fail(st, "sync write error ret");
		ref_mpool_close(rmp2);
		P::mpool_close(pmp2);
		close(rofd);
		close(rofd2);
		unlink(ropath);
	}

	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_eviction(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(4096, 0x5a);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 2);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 2);
	std::vector<void *> rpages, ppages;
	for (int i = 0; i < 5; i++) {
		void *rp = ref_mpool_get(rmp, (pgno_t)i, 0);
		void *pp = P::mpool_get(pmp, (pgno_t)i, 0);
		if (!rp || !pp || !same_page_bytes(rp, pp, 512))
			fail(st, "eviction get");
		rpages.push_back(rp);
		ppages.push_back(pp);
	}
	for (int i = 0; i < 5; i++) {
		ref_mpool_put(rmp, rpages[i], 0);
		P::mpool_put(pmp, ppages[i], 0);
	}
	if (rmp->curcache != pmp->curcache)
		fail(st, "eviction curcache");
	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
test_overflow_abort(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(512, 0);
	int fd = mk_temp_file(data.data(), data.size());
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 4);
	P::MPOOL *pmp = P::mpool_open(nullptr, dup(fd), 512, 4);
	if (!rmp || !pmp) {
		fail(st, "open overflow");
		close(fd);
		return;
	}
	rmp->npages = MAX_PAGE_NUMBER;
	pmp->npages = MAX_PAGE_NUMBER;
	pgno_t pg = 0;

	pid_t c1 = fork();
	if (c1 == 0) {
		ref_mpool_new(rmp, &pg, H_PAGE_NEXT);
		_exit(2);
	}
	pid_t c2 = fork();
	if (c2 == 0) {
		P::mpool_new(pmp, &pg, H_PAGE_NEXT);
		_exit(2);
	}
	int st1 = 0, st2 = 0;
	waitpid(c1, &st1, 0);
	waitpid(c2, &st2, 0);
	bool a1 = (st1 != 0) && (WIFSIGNALED(st1) && WTERMSIG(st1) == SIGABRT);
	bool a2 = (st2 != 0) && (WIFSIGNALED(st2) && WTERMSIG(st2) == SIGABRT);
	if (!a1 || !a2)
		fail(st, "overflow abort");
	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
}

static void
test_hashkey_edges(Stat &st)
{
	st.cases++;
	auto data = fill_pattern(512 * 130, 0x12);
	int fd = mk_temp_file(data.data(), data.size());
	int fd2 = dup(fd);
	MPOOL *rmp = ref_mpool_open(nullptr, fd, 512, 8);
	P::MPOOL *pmp = P::mpool_open(nullptr, fd2, 512, 8);
	pgno_t keys[] = {0, 1, 2, 127, 128, 129, 255, 256, MAX_PAGE_NUMBER - 1};
	for (pgno_t k : keys) {
		if (k >= rmp->npages)
			continue;
		void *rp = ref_mpool_get(rmp, k, 0);
		void *pp = P::mpool_get(pmp, k, 0);
		if (!rp || !pp || !same_page_bytes(rp, pp, 512))
			fail(st, "hashkey page");
		ref_mpool_put(rmp, rp, 0);
		P::mpool_put(pmp, pp, 0);
	}
	ref_mpool_close(rmp);
	P::mpool_close(pmp);
	close(fd);
	close(fd2);
}

static void
run_sweep(Stat &st)
{
	for (long i = 0; i < SWEEP_ITERS; i++) {
		st.cases++;
		unsigned seed = (unsigned)nextrand();
		pgno_t pagesize = (pgno_t)(256 + (nextrand() % 3) * 256);
		pgno_t maxcache = (pgno_t)(1 + (nextrand() % 6));
		size_t npg = (size_t)(1 + (nextrand() % 8));
		auto filedata = fill_pattern(npg * pagesize, seed);
		if (nextrand() & 1) {
			size_t trim = (size_t)(nextrand() % pagesize);
			if (trim < filedata.size())
				filedata.resize(filedata.size() - trim);
		}
		int fd = mk_temp_file(filedata.data(), filedata.size());
		int fd2 = dup(fd);
		if (fd < 0) {
			fail(st, "sweep mk_temp");
			continue;
		}
		MPOOL *rmp = ref_mpool_open(nullptr, fd, pagesize, maxcache);
		P::MPOOL *pmp = P::mpool_open(nullptr, fd2, pagesize, maxcache);
		if (!rmp || !pmp) {
			if (rmp != nullptr || pmp != nullptr)
				fail(st, "sweep open mismatch");
			if (rmp)
				ref_mpool_close(rmp);
			if (pmp)
				P::mpool_close(pmp);
			close(fd);
			close(fd2);
			continue;
		}
		if (rmp->npages != pmp->npages || rmp->curcache != pmp->curcache)
			fail(st, "sweep open fields");

		unsigned ops = (unsigned)(1 + (nextrand() % 24));
		for (unsigned op = 0; op < ops; op++) {
			unsigned kind = (unsigned)(nextrand() % 7);
			pgno_t pg = (pgno_t)(nextrand() % (rmp->npages + 2));
			unsigned put_dirty = (unsigned)(nextrand() & 3);
			unsigned new_flags = (unsigned)(nextrand() & 1);
			switch (kind) {
			case 0: {
				void *rp = ref_mpool_get(rmp, pg, 0);
				void *pp = P::mpool_get(pmp, pg, 0);
				if ((rp == nullptr) != (pp == nullptr))
					fail(st, "sweep get null mismatch");
				else if (rp && !same_page_bytes(rp, pp, pagesize))
					fail(st, "sweep get bytes");
				if (rp) {
					u_int pf = put_dirty ? H_DIRTY : 0;
					ref_mpool_put(rmp, rp, pf);
					P::mpool_put(pmp, pp, pf);
				}
				break;
			}
			case 1: {
				pgno_t rpg = pg, ppg = pg;
				u_int nf = new_flags ? H_PAGE_REQUEST : H_PAGE_NEXT;
				void *rn = ref_mpool_new(rmp, &rpg, nf);
				void *pn = P::mpool_new(pmp, &ppg, nf);
				if ((rn == nullptr) != (pn == nullptr))
					fail(st, "sweep new null");
				else if (rn && (rpg != ppg ||
						 rmp->npages != pmp->npages))
					fail(st, "sweep new pgno");
				if (rn) {
					ref_mpool_put(rmp, rn, 0);
					P::mpool_put(pmp, pn, 0);
				}
				break;
			}
			case 2: {
				if (ref_mpool_sync(rmp) != P::mpool_sync(pmp))
					fail(st, "sweep sync");
				break;
			}
			case 3: {
				pgno_t rpgc = pg, ppgc = pg;
				void *rp = ref___mpool_new__44bsd(rmp, &rpgc);
				void *pp = P::__mpool_new__44bsd(pmp, &ppgc);
				if ((rp == nullptr) != (pp == nullptr))
					fail(st, "sweep compat null");
				if (rp) {
					ref_mpool_put(rmp, rp, 0);
					P::mpool_put(pmp, pp, 0);
				}
				break;
			}
			case 4: {
				FilterCtx rc{0, 0, (unsigned char)(seed & 0xff)};
				FilterCtx pc{0, 0, (unsigned char)(seed & 0xff)};
				ref_mpool_filter(rmp, ref_pgin_cb, ref_pgout_cb, &rc);
				P::mpool_filter(pmp, port_pgin_cb, port_pgout_cb, &pc);
				break;
			}
			case 5: {
				void *rp = ref_mpool_get(rmp, pg, H_IGNOREPIN);
				void *pp = P::mpool_get(pmp, pg, H_IGNOREPIN);
				if ((rp == nullptr) != (pp == nullptr))
					fail(st, "sweep ignorepin null");
				else if (rp && !same_page_bytes(rp, pp, pagesize))
					fail(st, "sweep ignorepin bytes");
				if (rp) {
					ref_mpool_put(rmp, rp, H_DIRTY);
					P::mpool_put(pmp, pp, H_DIRTY);
				}
				break;
			}
			default:
				break;
			}
			if (rmp->npages != pmp->npages ||
			    rmp->curcache != pmp->curcache)
				fail(st, "sweep state");
		}
		if (ref_mpool_sync(rmp) != P::mpool_sync(pmp))
			fail(st, "sweep final sync");
		ref_mpool_close(rmp);
		P::mpool_close(pmp);
		close(fd);
		close(fd2);
	}
}

static void
print_table(const Stat *stats, size_t n)
{
	std::printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-24s %10s %10s\n", "--------", "-----", "--------");
	long total_c = 0, total_f = 0;
	for (size_t i = 0; i < n; i++) {
		std::printf("%-24s %10ld %10ld\n",
		    stats[i].name, stats[i].cases, stats[i].fails);
		total_c += stats[i].cases;
		total_f += stats[i].fails;
	}
	std::printf("%-24s %10ld %10ld\n", "TOTAL", total_c, total_f);
}

} // namespace

int
main(void)
{
	Stat stats[] = {
	    {"mpool_open", 0, 0},
	    {"mpool_filter", 0, 0},
	    {"mpool_new", 0, 0},
	    {"mpool_get", 0, 0},
	    {"mpool_put", 0, 0},
	    {"mpool_delete", 0, 0},
	    {"mpool_close", 0, 0},
	    {"mpool_sync", 0, 0},
	    {"__mpool_new__44bsd", 0, 0},
	    {"mpool_eviction", 0, 0},
	    {"mpool_overflow_abort", 0, 0},
	    {"mpool_hashkey_edges", 0, 0},
	    {"sweep", 0, 0},
	};

	test_mpool_open(stats[0]);
	test_mpool_filter(stats[1]);
	test_mpool_new(stats[2]);
	test_compat_new(stats[8]);
	test_mpool_get(stats[3]);
	test_mpool_put(stats[4]);
	test_mpool_delete(stats[5]);
	test_mpool_close(stats[6]);
	test_mpool_sync(stats[7]);
	/* __mpool_new__44bsd covered in test_mpool_new */
	test_eviction(stats[9]);
	test_overflow_abort(stats[10]);
	test_hashkey_edges(stats[11]);
	run_sweep(stats[12]);

	print_table(stats, sizeof(stats) / sizeof(stats[0]));
	long fails = 0;
	for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
		fails += stats[i].fails;
	return fails ? 1 : 0;
}
