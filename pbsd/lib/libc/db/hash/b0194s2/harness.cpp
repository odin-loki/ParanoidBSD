// Differential harness for batch b0194s2: hash_page.c routines.

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

import pbsd.lib.libc.db.hash.b0194s2;

namespace P = pbsd::lib_libc_db_hash::b0194s2;

extern "C" {
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef unsigned char u_char;

typedef struct {
	void *data;
	size_t size;
} DBT;

typedef struct _bufhead BUFHEAD;

struct _bufhead {
	BUFHEAD *prev;
	BUFHEAD *next;
	BUFHEAD *ovfl;
	u_int32_t addr;
	char *page;
	char flags;
};

#define NCACHED 32

typedef struct hashhdr {
	int32_t magic;
	int32_t version;
	u_int32_t lorder;
	int32_t bsize;
	int32_t bshift;
	int32_t dsize;
	int32_t ssize;
	int32_t sshift;
	int32_t ovfl_point;
	int32_t last_freed;
	u_int32_t max_bucket;
	u_int32_t high_mask;
	u_int32_t low_mask;
	u_int32_t ffactor;
	int32_t nkeys;
	int32_t hdrpages;
	int32_t h_charkey;
	int32_t spares[NCACHED];
	u_int16_t bitmaps[NCACHED];
} HASHHDR;

typedef struct htab {
	HASHHDR hdr;
	int nsegs;
	int exsegs;
	u_int32_t (*hash)(const void *, size_t);
	int flags;
	int fp;
	char *tmp_buf;
	char *tmp_key;
	BUFHEAD *cpage;
	int cbucket;
	int cndx;
	int error;
	int new_file;
	int save_file;
	u_int32_t *mapp[NCACHED];
	int nmaps;
	int nbufs;
	BUFHEAD bufhead;
	void **dir;
} HTAB;

typedef struct {
	BUFHEAD *newp;
	BUFHEAD *oldp;
	BUFHEAD *nextp;
	u_int16_t next_addr;
} SPLIT_RETURN;

void hash_mock_reset(void);
void hash_mock_register_page(u_int32_t addr, void *page);

typedef struct {
	unsigned get_buf_calls, call_hash_calls, big_delete_calls, big_split_calls,
	    big_insert_calls, expand_calls, reclaim_calls;
	unsigned mkostemp_calls, getenv_calls, sigmask_calls, write_calls;
	int get_buf_force_null, get_buf_fail_after, call_hash_ret, big_delete_ret,
	    big_split_ret, big_insert_ret, expand_ret;
	int mkostemp_ret;
	char *getenv_val;
	int write_ret;
	u_int32_t next_buf_addr;
	int nbufs;
} hash_mock_state;

extern hash_mock_state hash_mock;

void ref_putpair(char *, const DBT *, const DBT *);
int ref___delpair(HTAB *, BUFHEAD *, int);
int ref___split_page(HTAB *, u_int32_t, u_int32_t);
int ref_ugly_split(HTAB *, u_int32_t, BUFHEAD *, BUFHEAD *, int, int);
int ref___addel(HTAB *, BUFHEAD *, const DBT *, const DBT *);
BUFHEAD *ref___add_ovflpage(HTAB *, BUFHEAD *);
int ref___get_page(HTAB *, char *, u_int32_t, int, int, int);
int ref___put_page(HTAB *, char *, u_int32_t, int, int);
int ref___ibitmap(HTAB *, int, int, int);
u_int32_t ref_first_free(u_int32_t);
u_int16_t ref_overflow_page(HTAB *);
void ref___free_ovflpage(HTAB *, BUFHEAD *);
int ref_open_temp(HTAB *);
void ref_squeeze_key(u_int16_t *, const DBT *, const DBT *);
u_int32_t *ref_fetch_bitmap(HTAB *, int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_CAP = 512;
constexpr size_t BUF_CAP = 576;
constexpr unsigned long long SWEEP_ITERS = 200000;

#undef BIG_ENDIAN
#undef LITTLE_ENDIAN
#define HASH_BIG_ENDIAN 4321
#define HASH_LITTLE_ENDIAN 1234
#define EFTYPE 79

#define BUF_MOD 0x0001
#define BUF_PIN 0x0008
#define OVFLPAGE 0
#define PARTIAL_KEY 1
#define FULL_KEY_DATA 3
#define REAL_KEY 4
#define DEF_FFACTOR 65536
#define ALL_SET 0xffffffffu

enum Fn {
	F_PUTPAIR,
	F_FIRST_FREE,
	F_SQUEEZE_KEY,
	F_GET_PAGE,
	F_PUT_PAGE,
	F_IBITMAP,
	F_FETCH_BITMAP,
	F_DELPAIR,
	F_ADD_OVFLPAGE,
	F_FREE_OVFLPAGE,
	F_OVERFLOW_PAGE,
	F_ADDEL,
	F_SPLIT_PAGE,
	F_UGLY_SPLIT,
	F_OPEN_TEMP,
	F_COUNT
};

const char *fn_name[F_COUNT] = {
	"putpair",
	"first_free",
	"squeeze_key",
	"__get_page",
	"__put_page",
	"__ibitmap",
	"fetch_bitmap",
	"__delpair",
	"__add_ovflpage",
	"__free_ovflpage",
	"overflow_page",
	"__addel",
	"__split_page",
	"ugly_split",
	"open_temp",
};

unsigned long long n_cases[F_COUNT];
unsigned long long n_fails[F_COUNT];
unsigned reported[F_COUNT];

uint64_t rng = 0xb0194a2deadbeefULL;

uint64_t nextr()
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return rng;
}

uint32_t below(uint32_t n)
{
	return n ? (uint32_t)(nextr() % n) : 0;
}

void fail(Fn fn, const char *msg)
{
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: %s\n", fn_name[fn], msg);
	}
}

void pass_case(Fn fn)
{
	n_cases[fn]++;
}

void check_eq(Fn fn, bool ok, const char *msg)
{
	pass_case(fn);
	if (!ok)
		fail(fn, msg);
}

void guard_fill(unsigned char *b, size_t n)
{
	std::memset(b, GUARD, n);
}

void copy_guarded(unsigned char *dst, const unsigned char *src, size_t n,
    size_t cap)
{
	guard_fill(dst, cap);
	for (size_t i = 0; i < n; i++)
		dst[i] = src[i];
}

bool bufs_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

void init_htab_port(P::HTAB &h, int bsize = (int)PAGE_CAP)
{
	std::memset(&h, 0, sizeof(h));
	h.hdr.bsize = bsize;
	h.hdr.bshift = 9;
	h.hdr.lorder = HASH_LITTLE_ENDIAN;
	h.hdr.hdrpages = 1;
	h.hdr.ffactor = DEF_FFACTOR;
	h.hdr.max_bucket = 0;
	h.fp = -1;
}

void init_htab_ref(HTAB &h, int bsize = (int)PAGE_CAP)
{
	std::memset(&h, 0, sizeof(h));
	h.hdr.bsize = bsize;
	h.hdr.bshift = 9;
	h.hdr.lorder = HASH_LITTLE_ENDIAN;
	h.hdr.hdrpages = 1;
	h.hdr.ffactor = DEF_FFACTOR;
	h.hdr.max_bucket = 0;
	h.fp = -1;
}

void page_init_raw(int bsize, unsigned char *p)
{
	auto *bp = (u_int16_t *)p;
	bp[0] = 0;
	bp[1] = (u_int16_t)(bsize - 3 * sizeof(u_int16_t));
	bp[2] = (u_int16_t)bsize;
}

void setup_pair_page(unsigned char *page, int bsize, const unsigned char *key,
    size_t klen, const unsigned char *val, size_t vlen, int nextra)
{
	page_init_raw(bsize, page);
	auto *bp = (u_int16_t *)page;
	u_int16_t off = (u_int16_t)bsize;
	int n = 0;
	off -= (u_int16_t)klen;
	std::memcpy(page + off, key, klen);
	bp[++n] = off;
	off -= (u_int16_t)vlen;
	std::memcpy(page + off, val, vlen);
	bp[++n] = off;
	bp[0] = (u_int16_t)n;
	bp[n + 1] = off - (u_int16_t)((n + 3) * sizeof(u_int16_t));
	bp[n + 2] = off;
	for (int i = 0; i < nextra; i++) {
		n += 2;
		bp[0] = (u_int16_t)n;
		bp[n - 1] = 50;
		bp[n] = OVFLPAGE;
		bp[n + 1] = bp[n - 2];
		bp[n + 2] = bp[n - 1] - 4;
	}
}

/* --- putpair --- */

void test_putpair_once(const unsigned char *key, size_t klen,
    const unsigned char *val, size_t vlen, int bsize, const char *tag)
{
	unsigned char pa[BUF_CAP];
	unsigned char pb[BUF_CAP];
	unsigned char ka[64], kb[64];
	unsigned char va[64], vb[64];

	copy_guarded(pa, pa, 0, BUF_CAP);
	copy_guarded(pb, pb, 0, BUF_CAP);
	page_init_raw(bsize, pa);
	page_init_raw(bsize, pb);

	copy_guarded(ka, key, klen, sizeof ka);
	copy_guarded(kb, key, klen, sizeof kb);
	copy_guarded(va, val, vlen, sizeof va);
	copy_guarded(vb, val, vlen, sizeof vb);

	P::DBT kp{}, vp{}, kr{}, vr{};
	kp.data = ka;
	kp.size = klen;
	vp.data = va;
	vp.size = vlen;
	kr.data = kb;
	kr.size = klen;
	vr.data = vb;
	vr.size = vlen;

	P::putpair((char *)pa, &kp, &vp);
	ref_putpair((char *)pb, (DBT *)&kr, (DBT *)&vr);

	char msg[128];
	std::snprintf(msg, sizeof msg, "%s k=%zu v=%zu b=%d", tag, klen, vlen,
	    bsize);
	check_eq(F_PUTPAIR, bufs_eq(pa, pb, BUF_CAP), msg);
}

void edge_putpair()
{
	static const unsigned char empty[] = { 0 };
	static const unsigned char one_ff[] = { 0xff };
	static const unsigned char nul[] = { 0x00 };
	static const unsigned char hi[] = { 0x80, 0xff, 0x7f };
	static const unsigned char kv[] = { 'k', 'e', 'y' };
	static const unsigned char dv[] = { 'd', 'a', 't' };

	test_putpair_once(empty, 0, empty, 0, 512, "empty");
	test_putpair_once(one_ff, 1, nul, 1, 512, "ff_nul");
	test_putpair_once(hi, 3, hi, 3, 512, "hibytes");
	test_putpair_once(kv, 3, dv, 3, 512, "ascii");
	test_putpair_once(kv, 1, dv, 1, 256, "bs256");
	for (size_t k = 1; k <= 32; k++) {
		unsigned char kb[32], vb[32];
		for (size_t i = 0; i < k; i++) {
			kb[i] = (unsigned char)(0xa0 ^ i);
			vb[i] = (unsigned char)(0x50 ^ i);
		}
		test_putpair_once(kb, k, vb, k, 512, "ramp");
	}
}

void random_putpair(unsigned long long n)
{
	unsigned char key[48], val[48];
	for (unsigned long long i = 0; i < n; i++) {
		size_t klen = below(33);
		size_t vlen = below(33);
		for (size_t j = 0; j < klen; j++)
			key[j] = (unsigned char)(nextr() & 0xff);
		for (size_t j = 0; j < vlen; j++)
			val[j] = (unsigned char)(nextr() & 0xff);
		test_putpair_once(key, klen, val, vlen, 512, "rand");
	}
}

/* --- first_free --- */

void test_first_free_once(u_int32_t map, u_int32_t expect, const char *tag)
{
	u_int32_t got = P::first_free(map);
	u_int32_t ref = ref_first_free(map);
	char msg[96];
	std::snprintf(msg, sizeof msg, "%s map=0x%08x got=%u ref=%u exp=%u",
	    tag, (unsigned)map, (unsigned)got, (unsigned)ref, (unsigned)expect);
	check_eq(F_FIRST_FREE, got == ref && got == expect, msg);
}

void edge_first_free()
{
	test_first_free_once(0u, 0u, "zero");
	test_first_free_once(1u, 1u, "bit0set");
	test_first_free_once(0xfffffffeu, 0u, "bit0clear");
	test_first_free_once(ALL_SET, 32u, "allset");
	for (int b = 0; b < 32; b++) {
		u_int32_t m = ALL_SET ^ (1u << b);
		test_first_free_once(m, (u_int32_t)b, "single_clear");
	}
}

void random_first_free(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++) {
		u_int32_t map = (u_int32_t)nextr();
		u_int32_t got = P::first_free(map);
		u_int32_t ref = ref_first_free(map);
		char msg[64];
		std::snprintf(msg, sizeof msg, "rand map=0x%08x", (unsigned)map);
		check_eq(F_FIRST_FREE, got == ref, msg);
	}
}

void setup_squeeze_page(unsigned char *page, int bsize,
    const unsigned char *ek, size_t eklen, const unsigned char *ev, size_t evlen,
    u_int16_t ovfl_pageno)
{
	auto *bp = (u_int16_t *)page;
	u_int16_t off = (u_int16_t)bsize;
	off -= (u_int16_t)eklen;
	std::memcpy(page + off, ek, eklen);
	bp[1] = off;
	off -= (u_int16_t)evlen;
	std::memcpy(page + off, ev, evlen);
	bp[2] = off;
	int n = 2;
	bp[n + 1] = ovfl_pageno;
	bp[n + 2] = OVFLPAGE;
	bp[0] = (u_int16_t)(n + 2);
	bp[n + 3] = off - (u_int16_t)((bp[0] + 3) * sizeof(u_int16_t));
	bp[n + 4] = off;
}

/* --- squeeze_key --- */

void test_squeeze_once(int bsize, size_t klen, size_t vlen, const char *tag)
{
	unsigned char pa[BUF_CAP], pb[BUF_CAP];
	unsigned char ka[32], kb[32], va[32], vb[32];
	unsigned char ek[32], ev[32];
	unsigned char keyf[32], valf[32];

	for (size_t i = 0; i < klen; i++) {
		ek[i] = (unsigned char)(0xA0 ^ i);
		ev[i] = (unsigned char)(0xB0 ^ i);
		keyf[i] = (unsigned char)(0x80 ^ i);
		valf[i] = (unsigned char)(0x40 ^ i);
	}
	guard_fill(pa, BUF_CAP);
	guard_fill(pb, BUF_CAP);
	setup_squeeze_page(pa, bsize, ek, klen, ev, vlen, 100);
	std::memcpy(pb, pa, BUF_CAP);

	copy_guarded(ka, keyf, klen, sizeof ka);
	copy_guarded(kb, keyf, klen, sizeof kb);
	copy_guarded(va, valf, vlen, sizeof va);
	copy_guarded(vb, valf, vlen, sizeof vb);

	P::DBT kp{}, vp{}, kr{}, vr{};
	kp.data = ka;
	kp.size = klen;
	vp.data = va;
	vp.size = vlen;
	kr.data = kb;
	kr.size = klen;
	vr.data = vb;
	vr.size = vlen;

	P::squeeze_key((u_int16_t *)pa, &kp, &vp);
	ref_squeeze_key((u_int16_t *)pb, (DBT *)&kr, (DBT *)&vr);

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s k=%zu v=%zu", tag, klen, vlen);
	check_eq(F_SQUEEZE_KEY, bufs_eq(pa, pb, BUF_CAP), msg);
}

void edge_squeeze_key()
{
	test_squeeze_once(512, 1, 1, "tiny");
	test_squeeze_once(512, 8, 8, "eight");
	test_squeeze_once(512, 16, 1, "bigk");
	test_squeeze_once(512, 1, 16, "bigv");
}

void random_squeeze_key(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_squeeze_once(512, below(17) + 1, below(17) + 1, "rand");
}

/* --- __get_page --- */

void test_get_page_mem(int bsize, const char *tag)
{
	unsigned char pa[BUF_CAP], pb[BUF_CAP];
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, bsize);
	init_htab_ref(hr, bsize);
	guard_fill(pa, BUF_CAP);
	guard_fill(pb, BUF_CAP);

	int rp = P::__get_page(&hp, (char *)pa, 0, 0, 0, 0);
	int rr = ref___get_page(&hr, (char *)pb, 0, 0, 0, 0);

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s ret_p=%d ret_r=%d b=%d", tag, rp, rr,
	    bsize);
	check_eq(F_GET_PAGE, rp == rr && bufs_eq(pa, pb, BUF_CAP), msg);
}

void test_get_page_disk(int bsize, ssize_t rsize, int partial, const char *tag)
{
	unsigned char filebuf[BUF_CAP];
	unsigned char pa[BUF_CAP], pb[BUF_CAP];
	guard_fill(filebuf, BUF_CAP);
	for (int i = 0; i < bsize && partial; i++)
		filebuf[i] = (unsigned char)(i * 3);

	char tmpl[] = "/tmp/h0194XXXXXX";
	int fd = mkstemp(tmpl);
	if (fd < 0)
		return;
	if (rsize >= 0) {
		if (rsize > 0)
			(void)write(fd, filebuf, (size_t)rsize);
	} else {
		close(fd);
		unlink(tmpl);
		return;
	}

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, bsize);
	init_htab_ref(hr, bsize);
	hp.fp = fd;
	hr.fp = fd;
	hp.hdr.lorder = HASH_LITTLE_ENDIAN;
	hr.hdr.lorder = HASH_LITTLE_ENDIAN;

	guard_fill(pa, BUF_CAP);
	guard_fill(pb, BUF_CAP);

	errno = 0;
	int rp = P::__get_page(&hp, (char *)pa, 1, 1, 1, 0);
	int ep = errno;
	errno = 0;
	int rr = ref___get_page(&hr, (char *)pb, 1, 1, 1, 0);
	int er = errno;

	char msg[128];
	std::snprintf(msg, sizeof msg,
	    "%s rp=%d rr=%d ep=%d er=%d rsize=%zd", tag, rp, rr, ep, er, rsize);
	check_eq(F_GET_PAGE, rp == rr && ep == er && bufs_eq(pa, pb, BUF_CAP),
	    msg);

	close(fd);
	unlink(tmpl);
}

void test_get_page_swap(int bsize, const char *tag)
{
	unsigned char filebuf[BUF_CAP];
	unsigned char pa[BUF_CAP], pb[BUF_CAP];
	guard_fill(filebuf, BUF_CAP);
	auto *bp = (u_int16_t *)filebuf;
	bp[0] = 2;
	bp[1] = 100;
	bp[2] = 200;
	bp[3] = 150;
	bp[4] = 120;

	char tmpl[] = "/tmp/h0194sXXXXXX";
	int fd = mkstemp(tmpl);
	if (fd < 0)
		return;
	(void)write(fd, filebuf, bsize);

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, bsize);
	init_htab_ref(hr, bsize);
	hp.fp = fd;
	hr.fp = fd;
	hp.hdr.lorder = HASH_BIG_ENDIAN;
	hr.hdr.lorder = HASH_BIG_ENDIAN;

	guard_fill(pa, BUF_CAP);
	guard_fill(pb, BUF_CAP);

	int rp = P::__get_page(&hp, (char *)pa, 2, 0, 1, 0);
	int rr = ref___get_page(&hr, (char *)pb, 2, 0, 1, 0);

	char msg[64];
	std::snprintf(msg, sizeof msg, "%s rp=%d rr=%d", tag, rp, rr);
	check_eq(F_GET_PAGE, rp == rr && bufs_eq(pa, pb, BUF_CAP), msg);

	close(fd);
	unlink(tmpl);
}

void edge_get_page()
{
	test_get_page_mem(512, "mem512");
	test_get_page_mem(256, "mem256");
	test_get_page_disk(512, 512, 0, "full");
	test_get_page_disk(512, 0, 0, "eof");
	test_get_page_disk(512, 256, 1, "partial");
	test_get_page_swap(512, "swap");
}

void random_get_page(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++) {
		if (nextr() & 1)
			test_get_page_mem(512, "rand");
		else
			test_get_page_disk(512, (ssize_t)512, (int)(nextr() & 1),
			    "rand");
	}
}

/* --- __put_page --- */

void test_put_page_once(int swap, int bsize, const char *tag)
{
	unsigned char pa[BUF_CAP], pb[BUF_CAP];
	guard_fill(pa, BUF_CAP);
	guard_fill(pb, BUF_CAP);
	page_init_raw(bsize, pa);
	std::memcpy(pb, pa, BUF_CAP);
	pa[64] = (unsigned char)(nextr() & 0xff);
	pb[64] = pa[64];

	char tmpl[] = "/tmp/h0194pXXXXXX";
	int fd = mkstemp(tmpl);
	if (fd < 0)
		return;

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, bsize);
	init_htab_ref(hr, bsize);
	hp.fp = fd;
	hr.fp = fd;
	hp.hdr.lorder = swap ? HASH_BIG_ENDIAN : HASH_LITTLE_ENDIAN;
	hr.hdr.lorder = swap ? HASH_BIG_ENDIAN : HASH_LITTLE_ENDIAN;

	errno = 0;
	int rp = P::__put_page(&hp, (char *)pa, 1, 1, 0);
	int ep = errno;
	errno = 0;
	int rr = ref___put_page(&hr, (char *)pb, 1, 1, 0);
	int er = errno;

	close(fd);
	unlink(tmpl);

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s rp=%d rr=%d swap=%d", tag, rp, rr,
	    swap);
	check_eq(F_PUT_PAGE, rp == rr && ep == er && bufs_eq(pa, pb, BUF_CAP),
	    msg);
}

void edge_put_page()
{
	test_put_page_once(0, 512, "noswap");
	test_put_page_once(1, 512, "swap");
}

void random_put_page(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_put_page_once((int)(nextr() & 1), 512, "rand");
}

/* --- __ibitmap --- */

void test_ibitmap_once(int nbits, int ndx, const char *tag)
{
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);

	int rp = P::__ibitmap(&hp, 10, nbits, ndx);
	int rr = ref___ibitmap(&hr, 10, nbits, ndx);

	bool ok = rp == rr;
	if (rp == 0 && rr == 0) {
		ok = ok && hp.hdr.bitmaps[ndx] == hr.hdr.bitmaps[ndx];
		ok = ok && hp.nmaps == hr.nmaps;
		if (hp.mapp[ndx] && hr.mapp[ndx])
			ok = ok &&
			    std::memcmp(hp.mapp[ndx], hr.mapp[ndx], PAGE_CAP) ==
				0;
	}
	char msg[96];
	std::snprintf(msg, sizeof msg, "%s nbits=%d ndx=%d rp=%d rr=%d", tag,
	    nbits, ndx, rp, rr);
	check_eq(F_IBITMAP, ok, msg);

	if (hp.mapp[ndx])
		std::free(hp.mapp[ndx]);
	if (hr.mapp[ndx])
		std::free(hr.mapp[ndx]);
}

void edge_ibitmap()
{
	test_ibitmap_once(1, 0, "one");
	test_ibitmap_once(32, 0, "32");
	test_ibitmap_once(33, 1, "33");
	test_ibitmap_once(64, 2, "64");
	test_ibitmap_once(100, 3, "100");
}

void random_ibitmap(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_ibitmap_once((int)(below(120) + 1), (int)below(8), "rand");
}

/* --- fetch_bitmap --- */

void test_fetch_bitmap_once(int ndx, int preload, const char *tag)
{
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.nmaps = 4;
	hr.nmaps = 4;
	hp.hdr.bitmaps[ndx] = 5;
	hr.hdr.bitmaps[ndx] = 5;

	if (preload) {
		hp.mapp[ndx] = (u_int32_t *)std::malloc(PAGE_CAP);
		hr.mapp[ndx] = (u_int32_t *)std::malloc(PAGE_CAP);
		std::memset(hp.mapp[ndx], 0xaa, PAGE_CAP);
		std::memset(hr.mapp[ndx], 0xaa, PAGE_CAP);
	}

	hash_mock_reset();
	hp.fp = -1;
	hr.fp = -1;

	u_int32_t *gp = P::fetch_bitmap(&hp, ndx);
	u_int32_t *gr = ref_fetch_bitmap(&hr, ndx);

	bool ok = true;
	if (gp == nullptr && gr == nullptr)
		ok = true;
	else if (gp && gr)
		ok = std::memcmp(gp, gr, PAGE_CAP) == 0;
	else
		ok = false;

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s ndx=%d preload=%d", tag, ndx,
	    preload);
	check_eq(F_FETCH_BITMAP, ok, msg);

	for (int i = 0; i < 4; i++) {
		if (hp.mapp[i])
			std::free(hp.mapp[i]);
		if (hr.mapp[i])
			std::free(hr.mapp[i]);
	}
}

void edge_fetch_bitmap()
{
	test_fetch_bitmap_once(0, 0, "cold");
	test_fetch_bitmap_once(1, 1, "warm");
	test_fetch_bitmap_once(3, 0, "ndx3");
}

void random_fetch_bitmap(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_fetch_bitmap_once((int)below(4), (int)(nextr() & 1), "rand");
}

/* --- __delpair --- */

void test_delpair_once(int ndx, int big, int cndx_match, const char *tag)
{
	unsigned char pagea[BUF_CAP], pageb[BUF_CAP];
	static unsigned char keyf[] = { 1, 2, 3, 4 };
	static unsigned char valf[] = { 5, 6 };

	guard_fill(pagea, BUF_CAP);
	guard_fill(pageb, BUF_CAP);
	setup_pair_page(pagea, 512, keyf, 4, valf, 2, 0);
	std::memcpy(pageb, pagea, 512);
	if (big) {
		auto *bp = (u_int16_t *)pagea;
		bp[2] = 1;
		std::memcpy(pageb, pagea, 512);
	}

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.hdr.nkeys = 1;
	hr.hdr.nkeys = 1;
	if (cndx_match) {
		hp.cndx = ndx;
		hr.cndx = ndx;
	}

	P::BUFHEAD bpa{}, bpb{};
	bpa.page = (char *)pagea;
	bpb.page = (char *)pageb;
	bpa.flags = 0;
	bpb.flags = 0;

	hash_mock_reset();
	hash_mock.big_delete_ret = -1;

	int rp = P::__delpair(&hp, &bpa, ndx);
	int rr = ref___delpair(&hr, (BUFHEAD *)(void *)&bpb, ndx);

	char msg[128];
	std::snprintf(msg, sizeof msg,
	    "%s ndx=%d big=%d cndx=%d rp=%d rr=%d nkeys_p=%d nkeys_r=%d", tag,
	    ndx, big, cndx_match, rp, rr, hp.hdr.nkeys, hr.hdr.nkeys);
	bool ok = rp == rr && hp.hdr.nkeys == hr.hdr.nkeys &&
	    bpa.flags == bpb.flags;
	if (!big && rp == 0)
		ok = ok && bufs_eq(pagea, pageb, BUF_CAP);
	check_eq(F_DELPAIR, ok, msg);
}

void edge_delpair()
{
	test_delpair_once(1, 0, 0, "first");
	test_delpair_once(1, 0, 1, "cndx");
	test_delpair_once(1, 1, 0, "big");
	unsigned char page[BUF_CAP];
	static unsigned char k1[] = { 'a', 'b' };
	static unsigned char v1[] = { 'c' };
	static unsigned char k2[] = { 'd', 'e' };
	static unsigned char v2[] = { 'f' };
	guard_fill(page, BUF_CAP);
	setup_pair_page(page, 512, k1, 2, v1, 1, 0);
	auto *bp = (u_int16_t *)page;
	u_int16_t off = bp[2];
	off -= 2;
	std::memcpy(page + off, k2, 2);
	bp[3] = off;
	off -= 1;
	std::memcpy(page + off, v2, 1);
	bp[4] = off;
	bp[0] = 4;
	bp[5] = off - 10;
	bp[6] = off;
	test_delpair_once(1, 0, 0, "two_pair");
}

void random_delpair(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++) {
		int ndx = 1;
		test_delpair_once(ndx, (int)(nextr() & 1), (int)(nextr() & 1),
		    "rand");
	}
}

/* --- __add_ovflpage --- */

void test_add_ovflpage_once(int ffactor, int sp0, const char *tag)
{
	unsigned char pagea[BUF_CAP], pageb[BUF_CAP];
	unsigned char page_init[BUF_CAP];
	guard_fill(page_init, BUF_CAP);
	page_init_raw(512, page_init);
	auto *bp = (u_int16_t *)page_init;
	bp[0] = (u_int16_t)sp0;
	bp[1] = 400;
	bp[2] = 300;
	std::memcpy(pagea, page_init, BUF_CAP);
	std::memcpy(pageb, page_init, BUF_CAP);

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.hdr.ffactor = ffactor;
	hr.hdr.ffactor = ffactor;
	hp.hdr.spares[0] = 50;
	hr.hdr.spares[0] = 50;
	hp.hdr.ovfl_point = 0;
	hr.hdr.ovfl_point = 0;
	hp.hdr.last_freed = 0;
	hr.hdr.last_freed = 0;

	P::BUFHEAD bpa{}, bpb{};
	bpa.page = (char *)pagea;
	bpb.page = (char *)pageb;
	bpa.addr = 7;
	bpb.addr = 7;
	bpa.flags = 0;
	bpb.flags = 0;

	hash_mock_reset();

	P::BUFHEAD *op = P::__add_ovflpage(&hp, &bpa);

	std::memcpy(pageb, page_init, BUF_CAP);
	bpb.flags = 0;
	hash_mock_reset();
	BUFHEAD *or_ = ref___add_ovflpage(&hr, (BUFHEAD *)(void *)&bpb);

	char msg[128];
	std::snprintf(msg, sizeof msg,
	    "%s ff=%d sp0=%d op=%p or=%p flags_p=%d flags_r=%d", tag, ffactor,
	    sp0, (void *)op, (void *)or_, bpa.flags, bpb.flags);
	bool ok = (op == nullptr && or_ == nullptr) ||
	    (op != nullptr && or_ != nullptr);
	if (ok && op) {
		ok = bufs_eq(pagea, pageb, BUF_CAP) && bpa.flags == bpb.flags &&
		    hp.hdr.ffactor == hr.hdr.ffactor;
	}
	check_eq(F_ADD_OVFLPAGE, ok, msg);
}

void edge_add_ovflpage()
{
	test_add_ovflpage_once(DEF_FFACTOR, 4, "def");
	test_add_ovflpage_once(DEF_FFACTOR, 8, "sp8");
	test_add_ovflpage_once(0, 10, "ff0");
	hash_mock_reset();
	hash_mock.get_buf_force_null = 1;
	test_add_ovflpage_once(DEF_FFACTOR, 4, "nullbuf");
}

void random_add_ovflpage(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++) {
		hash_mock_reset();
		if (nextr() & 3)
			hash_mock.get_buf_force_null = 1;
		test_add_ovflpage_once(
		    (int)((nextr() & 1) ? DEF_FFACTOR : 0),
		    (int)(below(16) + 2), "rand");
	}
}

/* --- __free_ovflpage --- */

void test_free_ovflpage_once(u_int32_t addr, const char *tag)
{
	u_int32_t mapa[PAGE_CAP / sizeof(u_int32_t)];
	u_int32_t mapb[PAGE_CAP / sizeof(u_int32_t)];
	std::memset(mapa, 0, sizeof(mapa));
	std::memset(mapb, 0, sizeof(mapb));
	mapa[0] = ALL_SET;
	mapb[0] = ALL_SET;

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.hdr.spares[0] = 10;
	hr.hdr.spares[0] = 10;
	hp.hdr.ovfl_point = 0;
	hr.hdr.ovfl_point = 0;
	hp.hdr.last_freed = 20;
	hr.hdr.last_freed = 20;
	hp.mapp[0] = mapa;
	hr.mapp[0] = mapb;

	P::BUFHEAD bpa{}, bpb{};
	bpa.addr = addr;
	bpb.addr = addr;

	hash_mock_reset();

	P::__free_ovflpage(&hp, &bpa);
	ref___free_ovflpage(&hr, (BUFHEAD *)(void *)&bpb);

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s addr=%u lf_p=%d lf_r=%d", tag,
	    (unsigned)addr, hp.hdr.last_freed, hr.hdr.last_freed);
	check_eq(F_FREE_OVFLPAGE,
	    hp.hdr.last_freed == hr.hdr.last_freed &&
		std::memcmp(mapa, mapb, sizeof(mapa)) == 0,
	    msg);
}

void edge_free_ovflpage()
{
	test_free_ovflpage_once(1, "a1");
	test_free_ovflpage_once(5, "a5");
	test_free_ovflpage_once(0x800, "high");
}

void random_free_ovflpage(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_free_ovflpage_once((u_int32_t)(below(0x1000) + 1), "rand");
}

/* --- overflow_page --- */

void test_overflow_page_once(int lastf, u_int32_t map0, const char *tag)
{
	u_int32_t mapa[PAGE_CAP / sizeof(u_int32_t)];
	u_int32_t mapb[PAGE_CAP / sizeof(u_int32_t)];
	std::memset(mapa, 0, sizeof(mapa));
	std::memset(mapb, 0, sizeof(mapb));
	mapa[0] = map0;
	mapb[0] = map0;

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.hdr.spares[0] = 100;
	hr.hdr.spares[0] = 100;
	hp.hdr.ovfl_point = 0;
	hr.hdr.ovfl_point = 0;
	hp.hdr.last_freed = lastf;
	hr.hdr.last_freed = lastf;
	hp.mapp[0] = mapa;
	hr.mapp[0] = mapb;
	hp.nmaps = 1;
	hr.nmaps = 1;

	hash_mock_reset();

	u_int16_t ap = P::overflow_page(&hp);
	u_int16_t ar = ref_overflow_page(&hr);

	char msg[128];
	std::snprintf(msg, sizeof msg,
	    "%s lf=%d map0=0x%x ap=%u ar=%u lf_p=%d lf_r=%d", tag, lastf,
	    (unsigned)map0, (unsigned)ap, (unsigned)ar, hp.hdr.last_freed,
	    hr.hdr.last_freed);
	bool ok = ap == ar && hp.hdr.last_freed == hr.hdr.last_freed &&
	    hp.hdr.spares[0] == hr.hdr.spares[0] &&
	    std::memcmp(mapa, mapb, sizeof(mapa)) == 0;
	check_eq(F_OVERFLOW_PAGE, ok, msg);
}

void edge_overflow_page()
{
	test_overflow_page_once(0, ALL_SET ^ 1u, "bit0");
	test_overflow_page_once(5, ALL_SET ^ (1u << 5), "bit5");
	test_overflow_page_once(0, 0u, "map0");
}

void random_overflow_page(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_overflow_page_once((int)below(32), (u_int32_t)nextr(),
		    "rand");
}

/* --- __addel --- */

void test_addel_once(int squeeze, int expand, const char *tag)
{
	unsigned char pagea[BUF_CAP], pageb[BUF_CAP];
	unsigned char ka[32], kb[32], va[32], vb[32];
	guard_fill(pagea, BUF_CAP);
	guard_fill(pageb, BUF_CAP);

	if (squeeze) {
		setup_pair_page(pagea, 512, (unsigned char *)"k", 1,
		    (unsigned char *)"v", 1, 1);
	} else {
		page_init_raw(512, pagea);
	}
	std::memcpy(pageb, pagea, 512);

	size_t klen = below(8) + 1;
	size_t vlen = below(8) + 1;
	for (size_t i = 0; i < klen; i++)
		ka[i] = kb[i] = (unsigned char)(0x90 ^ i);
	for (size_t i = 0; i < vlen; i++)
		va[i] = vb[i] = (unsigned char)(0x60 ^ i);

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.hdr.nkeys = 0;
	hr.hdr.nkeys = 0;
	hp.hdr.max_bucket = 0;
	hr.hdr.max_bucket = 0;
	hp.hdr.ffactor = 1;
	hr.hdr.ffactor = 1;

	P::BUFHEAD bpa{}, bpb{};
	bpa.page = (char *)pagea;
	bpb.page = (char *)pageb;
	bpa.flags = 0;
	bpb.flags = 0;

	hash_mock_reset();
	hash_mock.expand_ret = expand;
	hash_mock.call_hash_ret = 0;

	P::DBT kp{}, vp{}, kr{}, vr{};
	kp.data = ka;
	kp.size = klen;
	vp.data = va;
	vp.size = vlen;
	kr.data = kb;
	kr.size = klen;
	vr.data = vb;
	vr.size = vlen;

	int rp = P::__addel(&hp, &bpa, &kp, &vp);
	int rr = ref___addel(&hr, (BUFHEAD *)(void *)&bpb, (DBT *)&kr,
	    (DBT *)&vr);

	char msg[160];
	std::snprintf(msg, sizeof msg,
	    "%s sq=%d ex=%d rp=%d rr=%d nkeys_p=%d nkeys_r=%d", tag, squeeze,
	    expand, rp, rr, hp.hdr.nkeys, hr.hdr.nkeys);
	bool ok = rp == rr && hp.hdr.nkeys == hr.hdr.nkeys &&
	    bpa.flags == bpb.flags;
	if (rp == 0)
		ok = ok && bufs_eq(pagea, pageb, BUF_CAP);
	check_eq(F_ADDEL, ok, msg);
}

void edge_addel()
{
	test_addel_once(0, 0, "plain");
	test_addel_once(1, 0, "squeeze");
	test_addel_once(0, 0, "expand0");
	hash_mock_reset();
	hash_mock.big_insert_ret = -1;
	unsigned char pagea[BUF_CAP], pageb[BUF_CAP];
	page_init_raw(512, pagea);
	std::memcpy(pageb, pagea, 512);
	auto *bp = (u_int16_t *)pagea;
	bp[1] = 4;
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	P::BUFHEAD bpa{}, bpb{};
	bpa.page = (char *)pagea;
	bpb.page = (char *)pageb;
	unsigned char k[64] = { 0xff };
	unsigned char v[64];
	for (int i = 0; i < 64; i++)
		v[i] = (unsigned char)i;
	P::DBT kp{ k, 64 }, vp{ v, 64 };
	DBT kr{ k, 64 }, vr{ v, 64 };
	hash_mock_reset();
	int rp = P::__addel(&hp, &bpa, &kp, &vp);
	int rr = ref___addel(&hr, (BUFHEAD *)(void *)&bpb, &kr, &vr);
	char msg[64];
	std::snprintf(msg, sizeof msg, "bigins rp=%d rr=%d", rp, rr);
	check_eq(F_ADDEL, rp == rr, msg);
}

void random_addel(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_addel_once((int)(nextr() & 1), (int)(nextr() & 1), "rand");
}

/* --- __split_page --- */

void test_split_page_once(u_int32_t ob, u_int32_t nb, int hashret,
    const char *tag)
{
	if (nb == ob)
		nb = ob + 1;
	unsigned char opa[BUF_CAP], opb[BUF_CAP];
	unsigned char npa[BUF_CAP], npb[BUF_CAP];
	unsigned char op_init[BUF_CAP], np_init[BUF_CAP];
	static unsigned char k[] = { 1, 2, 3 };
	static unsigned char v[] = { 4, 5 };
	guard_fill(op_init, BUF_CAP);
	guard_fill(np_init, BUF_CAP);
	setup_pair_page(op_init, 512, k, 3, v, 2, 0);
	page_init_raw(512, np_init);
	guard_fill(opa, BUF_CAP);
	guard_fill(opb, BUF_CAP);
	guard_fill(npa, BUF_CAP);
	guard_fill(npb, BUF_CAP);
	std::memcpy(opa, op_init, 512);
	std::memcpy(npa, np_init, 512);

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);

	hash_mock_reset();
	hash_mock.call_hash_ret = hashret;
	hash_mock_register_page(ob, opa);
	hash_mock_register_page(nb, npa);

	int rp = P::__split_page(&hp, ob, nb);

	std::memcpy(opb, op_init, 512);
	std::memcpy(npb, np_init, 512);
	hash_mock_reset();
	hash_mock.call_hash_ret = hashret;
	hash_mock_register_page(ob, opb);
	hash_mock_register_page(nb, npb);
	int rr = ref___split_page(&hr, ob, nb);

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s ob=%u nb=%u hr=%d rp=%d rr=%d", tag,
	    (unsigned)ob, (unsigned)nb, hashret, rp, rr);
	check_eq(F_SPLIT_PAGE, rp == rr, msg);
	if (rp == 0 && rr == 0)
		check_eq(F_SPLIT_PAGE,
		    bufs_eq(opa, opb, BUF_CAP) && bufs_eq(npa, npb, BUF_CAP),
		    "page bytes");
}

void edge_split_page()
{
	test_split_page_once(1, 2, 1, "stay");
	test_split_page_once(1, 2, 0, "move");
	unsigned char opa[BUF_CAP], opb[BUF_CAP];
	unsigned char npa[BUF_CAP], npb[BUF_CAP];
	unsigned char op_init[BUF_CAP], np_init[BUF_CAP];
	setup_pair_page(op_init, 512, (unsigned char *)"x", 1, (unsigned char *)"y",
	    1, 0);
	page_init_raw(512, np_init);
	std::memcpy(opa, op_init, 512);
	std::memcpy(npa, np_init, 512);
	auto *bp = (u_int16_t *)opa;
	bp[2] = 1;
	hash_mock_reset();
	hash_mock.big_split_ret = -1;
	hash_mock_register_page(1, opa);
	hash_mock_register_page(2, npa);
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	int rp = P::__split_page(&hp, 1, 2);
	std::memcpy(opb, op_init, 512);
	bp = (u_int16_t *)opb;
	bp[2] = 1;
	std::memcpy(npb, np_init, 512);
	hash_mock_reset();
	hash_mock.big_split_ret = -1;
	hash_mock_register_page(1, opb);
	hash_mock_register_page(2, npb);
	int rr = ref___split_page(&hr, 1, 2);
	char msg[64];
	std::snprintf(msg, sizeof msg, "ugly rp=%d rr=%d", rp, rr);
	check_eq(F_SPLIT_PAGE, rp == rr, msg);
}

void random_split_page(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_split_page_once(below(4) + 1, below(4) + 2,
		    (int)(nextr() & 1), "rand");
}

/* --- ugly_split --- */

void test_ugly_split_once(int hashret, const char *tag)
{
	unsigned char opa[BUF_CAP], opb[BUF_CAP];
	unsigned char npa[BUF_CAP], npb[BUF_CAP];
	unsigned char op_init[BUF_CAP], np_init[BUF_CAP];
	static unsigned char k[] = { 9, 8, 7 };
	static unsigned char v[] = { 6, 5 };
	setup_pair_page(op_init, 512, k, 3, v, 2, 0);
	page_init_raw(512, np_init);
	guard_fill(opa, BUF_CAP);
	guard_fill(opb, BUF_CAP);
	guard_fill(npa, BUF_CAP);
	guard_fill(npb, BUF_CAP);
	std::memcpy(opa, op_init, 512);
	std::memcpy(npa, np_init, 512);

	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);

	P::BUFHEAD oba{}, nba{};
	oba.page = (char *)opa;
	nba.page = (char *)npa;
	oba.addr = 1;
	nba.addr = 2;

	hash_mock_reset();
	hash_mock.call_hash_ret = hashret;

	int rp = P::ugly_split(&hp, 0, &oba, &nba, 512, 0);

	std::memcpy(opb, op_init, 512);
	std::memcpy(npb, np_init, 512);
	P::BUFHEAD obb{}, nbb{};
	obb.page = (char *)opb;
	nbb.page = (char *)npb;
	obb.addr = 1;
	nbb.addr = 2;
	hash_mock_reset();
	hash_mock.call_hash_ret = hashret;
	int rr = ref_ugly_split(&hr, 0, (BUFHEAD *)(void *)&obb,
	    (BUFHEAD *)(void *)&nbb, 512, 0);

	char msg[64];
	std::snprintf(msg, sizeof msg, "%s hr=%d rp=%d rr=%d", tag, hashret,
	    rp, rr);
	check_eq(F_UGLY_SPLIT, rp == rr, msg);
	if (rp == 0 && rr == 0)
		check_eq(F_UGLY_SPLIT,
		    bufs_eq(opa, opb, BUF_CAP) && bufs_eq(npa, npb, BUF_CAP),
		    "page bytes");
}

void edge_ugly_split()
{
	test_ugly_split_once(0, "stay");
	test_ugly_split_once(1, "move");
	unsigned char opa[BUF_CAP], opb[BUF_CAP];
	unsigned char npa[BUF_CAP], npb[BUF_CAP];
	unsigned char op_init[BUF_CAP], np_init[BUF_CAP];
	setup_pair_page(op_init, 512, (unsigned char *)"a", 1, (unsigned char *)"b",
	    1, 0);
	page_init_raw(512, np_init);
	std::memcpy(opa, op_init, 512);
	std::memcpy(npa, np_init, 512);
	auto *bp = (u_int16_t *)opa;
	bp[2] = PARTIAL_KEY;
	hash_mock_reset();
	hash_mock.big_split_ret = 0;
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	P::BUFHEAD oba{}, nba{};
	oba.page = (char *)opa;
	nba.page = (char *)npa;
	oba.addr = 1;
	nba.addr = 2;
	int rp = P::ugly_split(&hp, 0, &oba, &nba, 512, 0);

	std::memcpy(opb, op_init, 512);
	bp = (u_int16_t *)opb;
	bp[2] = PARTIAL_KEY;
	std::memcpy(npb, np_init, 512);
	P::BUFHEAD obb{}, nbb{};
	obb.page = (char *)opb;
	nbb.page = (char *)npb;
	obb.addr = 1;
	nbb.addr = 2;
	hash_mock_reset();
	hash_mock.big_split_ret = 0;
	int rr = ref_ugly_split(&hr, 0, (BUFHEAD *)(void *)&obb,
	    (BUFHEAD *)(void *)&nbb, 512, 0);
	char msg[64];
	std::snprintf(msg, sizeof msg, "partial rp=%d rr=%d", rp, rr);
	check_eq(F_UGLY_SPLIT, rp == rr, msg);
}

void random_ugly_split(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_ugly_split_once((int)(nextr() & 1), "rand");
}

/* --- open_temp --- */

void test_open_temp_once(const char *tag)
{
	P::HTAB hp{};
	HTAB hr{};
	init_htab_port(hp, 512);
	init_htab_ref(hr, 512);
	hp.fp = -1;
	hr.fp = -1;

	hash_mock_reset();

	int rp = P::open_temp(&hp);
	int rr = ref_open_temp(&hr);

	char msg[96];
	std::snprintf(msg, sizeof msg, "%s rp=%d rr=%d fp_p=%d fp_r=%d", tag,
	    rp, rr, hp.fp, hr.fp);
	bool ok = rp == rr;
	if (rp == 0 && rr == 0)
		ok = ok && hp.fp >= 0 && hr.fp >= 0;
	check_eq(F_OPEN_TEMP, ok, msg);
	if (hp.fp >= 0)
		close(hp.fp);
	if (hr.fp >= 0)
		close(hr.fp);
}

void edge_open_temp()
{
	test_open_temp_once("basic");
	hash_mock_reset();
	hash_mock.getenv_val = (char *)"";
	test_open_temp_once("tmpdir");
}

void random_open_temp(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; i++)
		test_open_temp_once("rand");
}

void print_table()
{
	const char *hdr = "function        cases      failures";
	const char *sep = "----------------------------------------";
	std::printf("\n%s\n%s\n", hdr, sep);
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-15s %-10llu %-10llu\n", fn_name[i], n_cases[i],
		    n_fails[i]);
	std::printf("%s\n", sep);
}

} // namespace

int main()
{
	edge_putpair();
	edge_first_free();
	edge_squeeze_key();
	edge_get_page();
	edge_put_page();
	edge_ibitmap();
	edge_fetch_bitmap();
	edge_delpair();
	edge_add_ovflpage();
	edge_free_ovflpage();
	edge_overflow_page();
	edge_addel();
	edge_split_page();
	edge_ugly_split();
	edge_open_temp();

	const unsigned long long per = SWEEP_ITERS / 15;
	random_putpair(per);
	random_first_free(per);
	random_squeeze_key(per);
	random_get_page(per);
	random_put_page(per);
	random_ibitmap(per);
	random_fetch_bitmap(per);
	random_delpair(per);
	random_add_ovflpage(per);
	random_free_ovflpage(per);
	random_overflow_page(per);
	random_addel(per);
	random_split_page(per);
	random_ugly_split(per);
	random_open_temp(per);

	print_table();

	unsigned long long fails = 0;
	for (int i = 0; i < F_COUNT; i++)
		fails += n_fails[i];
	return fails ? 1 : 0;
}
