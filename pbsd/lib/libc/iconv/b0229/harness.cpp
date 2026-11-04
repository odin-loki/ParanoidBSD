/*
 * Differential harness for PBSD batch b0229.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

import pbsd.lib.libc.iconv.b0229;

namespace P = pbsd::lib_libc_iconv::b0229;

extern "C" {
int ref__citrus_db_factory_create(P::_citrus_db_factory **, uint32_t (*)(P::_citrus_region *), void *);
void ref__citrus_db_factory_free(P::_citrus_db_factory *);
int ref__citrus_db_factory_add(P::_citrus_db_factory *, P::_citrus_region *, int,
    P::_citrus_region *, int);
int ref__citrus_db_factory_add_by_string(P::_citrus_db_factory *, const char *,
    P::_citrus_region *, int);
int ref__citrus_db_factory_add8_by_string(P::_citrus_db_factory *, const char *, uint8_t);
int ref__citrus_db_factory_add16_by_string(P::_citrus_db_factory *, const char *, uint16_t);
int ref__citrus_db_factory_add32_by_string(P::_citrus_db_factory *, const char *, uint32_t);
int ref__citrus_db_factory_add_string_by_string(P::_citrus_db_factory *, const char *,
    const char *);
size_t ref__citrus_db_factory_calc_size(P::_citrus_db_factory *);
int ref__citrus_db_factory_serialize(P::_citrus_db_factory *, const char *,
    P::_citrus_region *);

int ref__citrus_db_open(P::_citrus_db **, P::_citrus_region *, const char *,
    uint32_t (*)(P::_citrus_region *), void *);
void ref__citrus_db_close(P::_citrus_db *);
int ref__citrus_db_lookup(P::_citrus_db *, P::_citrus_region *, P::_citrus_region *,
    P::_citrus_db_locator *);
int ref__citrus_db_lookup_by_string(P::_citrus_db *, const char *, P::_citrus_region *,
    P::_citrus_db_locator *);
int ref__citrus_db_lookup8_by_string(P::_citrus_db *, const char *, uint8_t *,
    P::_citrus_db_locator *);
int ref__citrus_db_lookup16_by_string(P::_citrus_db *, const char *, uint16_t *,
    P::_citrus_db_locator *);
int ref__citrus_db_lookup32_by_string(P::_citrus_db *, const char *, uint32_t *,
    P::_citrus_db_locator *);
int ref__citrus_db_lookup_string_by_string(P::_citrus_db *, const char *, const char **,
    P::_citrus_db_locator *);
int ref__citrus_db_get_number_of_entries(P::_citrus_db *);
int ref__citrus_db_get_entry(P::_citrus_db *, int, P::_citrus_region *,
    P::_citrus_region *);

int ref__citrus_lookup_seq_open(P::_citrus_lookup **, const char *, int);
void ref__citrus_lookup_seq_rewind(P::_citrus_lookup *);
int ref__citrus_lookup_seq_next(P::_citrus_lookup *, P::_citrus_region *,
    P::_citrus_region *);
int ref__citrus_lookup_seq_lookup(P::_citrus_lookup *, const char *, P::_citrus_region *);
int ref__citrus_lookup_get_number_of_entries(P::_citrus_lookup *);
void ref__citrus_lookup_seq_close(P::_citrus_lookup *);
char *ref__citrus_lookup_simple(const char *, const char *, char *, size_t, int);

int ref__citrus_mapper_create_area(P::_citrus_mapper_area **, const char *);
int ref__citrus_mapper_open_direct(P::_citrus_mapper_area *, P::_citrus_mapper **,
    const char *, const char *);
int ref__citrus_mapper_open(P::_citrus_mapper_area *, P::_citrus_mapper **, const char *);
void ref__citrus_mapper_close(P::_citrus_mapper *);
void ref__citrus_mapper_set_persistent(P::_citrus_mapper *);

void b0229_mock_reset(void);
void b0229_mock_set_module(const char *, int (*)(P::_citrus_mapper_ops *));
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t BIGBUF = 8192;

enum FuncId {
	F_FACTORY_CREATE,
	F_FACTORY_FREE,
	F_FACTORY_ADD,
	F_FACTORY_ADD_S,
	F_FACTORY_ADD8,
	F_FACTORY_ADD16,
	F_FACTORY_ADD32,
	F_FACTORY_ADDSTR,
	F_FACTORY_CALC,
	F_FACTORY_SER,
	F_DB_OPEN,
	F_DB_CLOSE,
	F_DB_LOOKUP,
	F_DB_LOOKUP_S,
	F_DB_LOOKUP8,
	F_DB_LOOKUP16,
	F_DB_LOOKUP32,
	F_DB_LOOKUPSTR,
	F_DB_NUM,
	F_DB_ENTRY,
	F_LOOKUP_OPEN,
	F_LOOKUP_REWIND,
	F_LOOKUP_NEXT,
	F_LOOKUP_LOOKUP,
	F_LOOKUP_NUM,
	F_LOOKUP_CLOSE,
	F_LOOKUP_SIMPLE,
	F_MAP_CREATE,
	F_MAP_OPEN_DIR,
	F_MAP_OPEN_DIRECT,
	F_MAP_CLOSE,
	F_MAP_PERSIST,
	F_COUNT
};

const char *const FNAME[F_COUNT] = {
	"db_factory_create", "db_factory_free", "db_factory_add",
	"db_factory_add_by_string", "db_factory_add8_by_string",
	"db_factory_add16_by_string", "db_factory_add32_by_string",
	"db_factory_add_string_by_string", "db_factory_calc_size",
	"db_factory_serialize", "db_open", "db_close", "db_lookup",
	"db_lookup_by_string", "db_lookup8_by_string", "db_lookup16_by_string",
	"db_lookup32_by_string", "db_lookup_string_by_string",
	"db_get_number_of_entries", "db_get_entry", "lookup_seq_open",
	"lookup_seq_rewind", "lookup_seq_next", "lookup_seq_lookup",
	"lookup_get_number_of_entries", "lookup_seq_close", "lookup_simple",
	"mapper_create_area", "mapper_open", "mapper_open_direct",
	"mapper_close", "mapper_set_persistent",
};

unsigned long long NCASE[F_COUNT];
unsigned long long NFAIL[F_COUNT];
unsigned long long NPRINT[F_COUNT];

uint64_t rng = 0xb0229b0229b0229ULL;

uint64_t
rng_next(void)
{
	uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

unsigned char
rb(void)
{
	return ((unsigned char)(rng_next() & 0xffu));
}

void
bump(FuncId f)
{
	NCASE[f]++;
}

void
fail(FuncId f, const char *why)
{
	NFAIL[f]++;
	if (NPRINT[f]++ < 8)
		std::fprintf(stderr, "  FAIL %-34s : %s\n", FNAME[f], why);
}

bool
bufs_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

void
guard_fill(unsigned char *p, size_t n)
{
	std::memset(p, GUARD, n);
}

std::string
tmp_path(const char *suffix)
{
	char tmpl[] = "/tmp/b0229XXXXXX";
	char *d = mkdtemp(tmpl);
	std::string p = d;

	p += suffix;
	return (p);
}

bool
write_file(const std::string &path, const void *data, size_t n)
{
	int fd = open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);

	if (fd < 0)
		return (false);
	ssize_t w = write(fd, data, n);
	close(fd);
	return (w == (ssize_t)n);
}

uint32_t
hash_std(P::_citrus_region *r)
{
	return (P::_citrus_db_hash_std(r));
}

int
build_db(P::_citrus_db_factory **pdf, unsigned char *blob, size_t blobsz,
    const std::vector<std::pair<std::string, std::vector<unsigned char>>> &ents,
    bool use_port)
{
	P::_citrus_db_factory *df = nullptr;
	P::_citrus_region r, data;
	int ret;

	if (use_port)
		ret = P::_citrus_db_factory_create(&df, hash_std, nullptr);
	else
		ret = ref__citrus_db_factory_create(&df, hash_std, nullptr);
	if (ret)
		return (ret);

	for (const auto &e : ents) {
		P::_citrus_region_init(&data, (void *)e.second.data(), e.second.size());
		if (use_port)
			ret = P::_citrus_db_factory_add_by_string(df, e.first.c_str(), &data, 0);
		else
			ret = ref__citrus_db_factory_add_by_string(df, e.first.c_str(), &data, 0);
		if (ret) {
			if (use_port)
				P::_citrus_db_factory_free(df);
			else
				ref__citrus_db_factory_free(df);
			return (ret);
		}
	}

	guard_fill(blob, blobsz);
	P::_citrus_region_init(&r, blob + 64, blobsz - 128);
	if (use_port)
		ret = P::_citrus_db_factory_serialize(df, "LOOKUP\0\0", &r);
	else
		ret = ref__citrus_db_factory_serialize(df, "LOOKUP\0\0", &r);
	if (use_port)
		P::_citrus_db_factory_free(df);
	else
		ref__citrus_db_factory_free(df);
	*pdf = nullptr;
	return (ret);
}

void
test_factory_roundtrip(void)
{
	unsigned char pa[BIGBUF], pb[BIGBUF];
	P::_citrus_db_factory *dfa = nullptr, *dfb = nullptr;
	size_t sa, sb;
	int ra, rbv;

	bump(F_FACTORY_CREATE);
	bump(F_FACTORY_ADD_S);
	bump(F_FACTORY_CALC);
	bump(F_FACTORY_SER);
	bump(F_FACTORY_FREE);

	ra = P::_citrus_db_factory_create(&dfa, hash_std, nullptr);
	rbv = ref__citrus_db_factory_create(&dfb, hash_std, nullptr);
	if (ra != rbv) {
		fail(F_FACTORY_CREATE, "create ret");
		return;
	}

	P::_citrus_region data;
	std::vector<unsigned char> d1 = {0x41};
	P::_citrus_region_init(&data, d1.data(), 1);
	P::_citrus_db_factory_add_by_string(dfa, "alpha", &data, 0);
	ref__citrus_db_factory_add_by_string(dfb, "alpha", &data, 0);

	std::vector<unsigned char> d2 = {0x80, 0xff};
	P::_citrus_region_init(&data, d2.data(), 2);
	P::_citrus_db_factory_add_by_string(dfa, "beta", &data, 0);
	ref__citrus_db_factory_add_by_string(dfb, "beta", &data, 0);

	sa = P::_citrus_db_factory_calc_size(dfa);
	sb = ref__citrus_db_factory_calc_size(dfb);
	if (sa != sb) {
		fail(F_FACTORY_CALC, "calc_size");
		return;
	}

	guard_fill(pa, BIGBUF);
	guard_fill(pb, BIGBUF);
	P::_citrus_region ra_r, rb_r;
	P::_citrus_region_init(&ra_r, pa + 32, BIGBUF - 64);
	P::_citrus_region_init(&rb_r, pb + 32, BIGBUF - 64);
	ra = P::_citrus_db_factory_serialize(dfa, "LOOKUP\0\0", &ra_r);
	rbv = ref__citrus_db_factory_serialize(dfb, "LOOKUP\0\0", &rb_r);
	if (ra != rbv || !bufs_eq(pa, pb, BIGBUF)) {
		fail(F_FACTORY_SER, "serialize");
		return;
	}

	P::_citrus_db_factory_free(dfa);
	ref__citrus_db_factory_free(dfb);
}

void
test_factory_typed_adds(void)
{
	P::_citrus_db_factory *dfa = nullptr, *dfb = nullptr;
	int ra, rbv;

	bump(F_FACTORY_ADD8);
	bump(F_FACTORY_ADD16);
	bump(F_FACTORY_ADD32);
	bump(F_FACTORY_ADDSTR);

	P::_citrus_db_factory_create(&dfa, hash_std, nullptr);
	ref__citrus_db_factory_create(&dfb, hash_std, nullptr);

	ra = P::_citrus_db_factory_add8_by_string(dfa, "k8", (uint8_t)0xab);
	rbv = ref__citrus_db_factory_add8_by_string(dfb, "k8", (uint8_t)0xab);
	if (ra != rbv)
		fail(F_FACTORY_ADD8, "add8 ret");

	ra = P::_citrus_db_factory_add16_by_string(dfa, "k16", (uint16_t)0x1234);
	rbv = ref__citrus_db_factory_add16_by_string(dfb, "k16", (uint16_t)0x1234);
	if (ra != rbv)
		fail(F_FACTORY_ADD16, "add16 ret");

	ra = P::_citrus_db_factory_add32_by_string(dfa, "k32", (uint32_t)0xdeadbeef);
	rbv = ref__citrus_db_factory_add32_by_string(dfb, "k32", (uint32_t)0xdeadbeef);
	if (ra != rbv)
		fail(F_FACTORY_ADD32, "add32 ret");

	ra = P::_citrus_db_factory_add_string_by_string(dfa, "ks", "data\0extra");
	rbv = ref__citrus_db_factory_add_string_by_string(dfb, "ks", "data\0extra");
	if (ra != rbv)
		fail(F_FACTORY_ADDSTR, "addstr ret");

	if (P::_citrus_db_factory_calc_size(dfa) != ref__citrus_db_factory_calc_size(dfb))
		fail(F_FACTORY_CALC, "typed calc");

	P::_citrus_db_factory_free(dfa);
	ref__citrus_db_factory_free(dfb);
}

void
test_db_ops(const unsigned char *blob, size_t blobsz)
{
	P::_citrus_region reg;
	P::_citrus_db *da = nullptr, *db = nullptr;
	P::_citrus_region ka, kb, da_r, db_r;
	P::_citrus_db_locator la, lb;
	uint8_t u8a, u8b;
	uint16_t u16a, u16b;
	uint32_t u32a, u32b;
	const char *stra, *strb;
	int ra, rbv, na, nb;

	bump(F_DB_OPEN);
	bump(F_DB_LOOKUP_S);
	bump(F_DB_LOOKUP8);
	bump(F_DB_LOOKUP16);
	bump(F_DB_LOOKUP32);
	bump(F_DB_LOOKUPSTR);
	bump(F_DB_NUM);
	bump(F_DB_ENTRY);
	bump(F_DB_CLOSE);

	P::_citrus_region_init(&reg, (void *)blob, blobsz);
	ra = P::_citrus_db_open(&da, &reg, "LOOKUP\0\0", hash_std, nullptr);
	rbv = ref__citrus_db_open(&db, &reg, "LOOKUP\0\0", hash_std, nullptr);
	if (ra != rbv) {
		fail(F_DB_OPEN, "open");
		return;
	}

	na = P::_citrus_db_get_number_of_entries(da);
	nb = ref__citrus_db_get_number_of_entries(db);
	if (na != nb)
		fail(F_DB_NUM, "num entries");

	P::_citrus_db_locator_init(&la);
	P::_citrus_db_locator_init(&lb);
	ra = P::_citrus_db_lookup_by_string(da, "alpha", &ka, &la);
	rbv = ref__citrus_db_lookup_by_string(db, "alpha", &kb, &lb);
	if (ra != rbv || P::_citrus_region_size(&ka) != P::_citrus_region_size(&kb))
		fail(F_DB_LOOKUP_S, "lookup alpha");

	ra = P::_citrus_db_lookup8_by_string(da, "k8", &u8a, nullptr);
	rbv = ref__citrus_db_lookup8_by_string(db, "k8", &u8b, nullptr);
	if (ra != rbv || u8a != u8b)
		fail(F_DB_LOOKUP8, "lookup8");

	ra = P::_citrus_db_lookup16_by_string(da, "k16", &u16a, nullptr);
	rbv = ref__citrus_db_lookup16_by_string(db, "k16", &u16b, nullptr);
	if (ra != rbv || u16a != u16b)
		fail(F_DB_LOOKUP16, "lookup16");

	ra = P::_citrus_db_lookup32_by_string(da, "k32", &u32a, nullptr);
	rbv = ref__citrus_db_lookup32_by_string(db, "k32", &u32b, nullptr);
	if (ra != rbv || u32a != u32b)
		fail(F_DB_LOOKUP32, "lookup32");

	ra = P::_citrus_db_lookup_string_by_string(da, "ks", &stra, nullptr);
	rbv = ref__citrus_db_lookup_string_by_string(db, "ks", &strb, nullptr);
	if (ra != rbv)
		fail(F_DB_LOOKUPSTR, "lookupstr ret");
	else if (ra == 0 && std::strcmp(stra, strb) != 0)
		fail(F_DB_LOOKUPSTR, "lookupstr data");

	for (int i = 0; i < na; i++) {
		ra = P::_citrus_db_get_entry(da, i, &ka, &da_r);
		rbv = ref__citrus_db_get_entry(db, i, &kb, &db_r);
		if (ra != rbv || P::_citrus_region_size(&ka) != P::_citrus_region_size(&kb) ||
		    P::_citrus_region_size(&da_r) != P::_citrus_region_size(&db_r) ||
		    std::memcmp(P::_citrus_region_head(&ka), P::_citrus_region_head(&kb),
		    P::_citrus_region_size(&ka)) != 0 ||
		    std::memcmp(P::_citrus_region_head(&da_r), P::_citrus_region_head(&db_r),
		    P::_citrus_region_size(&da_r)) != 0)
			fail(F_DB_ENTRY, "get_entry");
	}

	P::_citrus_db_close(da);
	ref__citrus_db_close(db);
}

void
test_lookup_plain(const std::string &path, int ignore_case)
{
	P::_citrus_lookup *la = nullptr, *lb = nullptr;
	P::_citrus_region ka, kb, da, db;
	char bufa[256], bufb[256];
	int ra, rbv;

	bump(F_LOOKUP_OPEN);
	bump(F_LOOKUP_NEXT);
	bump(F_LOOKUP_LOOKUP);
	bump(F_LOOKUP_NUM);
	bump(F_LOOKUP_REWIND);
	bump(F_LOOKUP_SIMPLE);
	bump(F_LOOKUP_CLOSE);

	ra = P::_citrus_lookup_seq_open(&la, path.c_str(), ignore_case);
	rbv = ref__citrus_lookup_seq_open(&lb, path.c_str(), ignore_case);
	if (ra != rbv) {
		fail(F_LOOKUP_OPEN, "open plain");
		return;
	}

	int na = P::_citrus_lookup_get_number_of_entries(la);
	int nb = ref__citrus_lookup_get_number_of_entries(lb);
	if (na != nb)
		fail(F_LOOKUP_NUM, "plain count");

	while (true) {
		ra = P::_citrus_lookup_seq_next(la, &ka, &da);
		rbv = ref__citrus_lookup_seq_next(lb, &kb, &db);
		if (ra != rbv)
			fail(F_LOOKUP_NEXT, "next ret");
		if (ra != 0)
			break;
		if (P::_citrus_region_size(&ka) != P::_citrus_region_size(&kb) ||
		    std::memcmp(P::_citrus_region_head(&ka), P::_citrus_region_head(&kb),
		    P::_citrus_region_size(&ka)) != 0 ||
		    P::_citrus_region_size(&da) != P::_citrus_region_size(&db) ||
		    std::memcmp(P::_citrus_region_head(&da), P::_citrus_region_head(&db),
		    P::_citrus_region_size(&da)) != 0)
			fail(F_LOOKUP_NEXT, "next data");
	}

	P::_citrus_lookup_seq_rewind(la);
	ref__citrus_lookup_seq_rewind(lb);
	ra = P::_citrus_lookup_seq_lookup(la, "foo", &da);
	rbv = ref__citrus_lookup_seq_lookup(lb, "foo", &db);
	if (ra != rbv)
		fail(F_LOOKUP_LOOKUP, "lookup foo");

	guard_fill((unsigned char *)bufa, sizeof(bufa));
	guard_fill((unsigned char *)bufb, sizeof(bufb));
	char *pa = P::_citrus_lookup_simple(path.c_str(), "foo", bufa, sizeof(bufa),
	    ignore_case);
	char *pb = ref__citrus_lookup_simple(path.c_str(), "foo", bufb, sizeof(bufb),
	    ignore_case);
	if ((pa == nullptr) != (pb == nullptr))
		fail(F_LOOKUP_SIMPLE, "simple null");
	else if (pa && std::strcmp(pa, pb) != 0)
		fail(F_LOOKUP_SIMPLE, "simple data");
	else if (pa && (!bufs_eq((unsigned char *)bufa, (unsigned char *)bufb, sizeof(bufa))))
		fail(F_LOOKUP_SIMPLE, "simple buf");

	P::_citrus_lookup_seq_close(la);
	ref__citrus_lookup_seq_close(lb);
}

static P::_citrus_mapper_ops g_mock_ops;
static int g_init_calls;

static int
mock_getops(P::_citrus_mapper_ops *ops)
{
	*ops = g_mock_ops;
	return (0);
}

static int
mock_init(P::_citrus_mapper_area *, P::_citrus_mapper *, const char *,
    const void *, size_t, P::_citrus_mapper_traits *tr, size_t)
{
	g_init_calls++;
	tr->mt_state_size = 4;
	tr->mt_src_max = 1;
	tr->mt_dst_max = 1;
	return (0);
}

static void
mock_uninit(P::_citrus_mapper *) {}

static int
mock_convert(P::_citrus_mapper *, P::_citrus_index_t *dst, P::_citrus_index_t src,
    void *)
{
	if (dst)
		*dst = src;
	return (0);
}

static void
mock_init_state(void) {}

void
test_mapper(const std::string &dir)
{
	P::_citrus_mapper_area *ma_a = nullptr, *ma_b = nullptr;
	P::_citrus_mapper *cm_a = nullptr, *cm_b = nullptr;
	int ra, rbv;

	b0229_mock_reset();
	g_mock_ops.mo_init = mock_init;
	g_mock_ops.mo_uninit = mock_uninit;
	g_mock_ops.mo_convert = mock_convert;
	g_mock_ops.mo_init_state = mock_init_state;
	::b0229_mock_set_module("testmod", mock_getops);

	bump(F_MAP_CREATE);
	ra = P::_citrus_mapper_create_area(&ma_a, dir.c_str());
	rbv = ref__citrus_mapper_create_area(&ma_b, dir.c_str());
	if (ra != rbv) {
		fail(F_MAP_CREATE, "create_area");
		return;
	}

	bump(F_MAP_OPEN_DIR);
	g_init_calls = 0;
	ra = P::_citrus_mapper_open(ma_a, &cm_a, "mymap");
	int port_init = g_init_calls;
	g_init_calls = 0;
	rbv = ref__citrus_mapper_open(ma_b, &cm_b, "mymap");
	int ref_init = g_init_calls;
	if (ra != rbv || port_init != 1 || ref_init != 1)
		fail(F_MAP_OPEN_DIR, "open");
	if (ra != 0)
		return;

	bump(F_MAP_OPEN_DIR);
	g_init_calls = 0;
	ra = P::_citrus_mapper_open(ma_a, &cm_a, "mymap");
	port_init = g_init_calls;
	g_init_calls = 0;
	rbv = ref__citrus_mapper_open(ma_b, &cm_b, "mymap");
	ref_init = g_init_calls;
	if (ra != rbv || port_init != 0 || ref_init != 0)
		fail(F_MAP_OPEN_DIR, "cache hit");

	bump(F_MAP_OPEN_DIRECT);
	ra = P::_citrus_mapper_open_direct(ma_a, &cm_a, "testmod", "var");
	rbv = ref__citrus_mapper_open_direct(ma_b, &cm_b, "testmod", "var2");
	if (ra != rbv)
		fail(F_MAP_OPEN_DIRECT, "open_direct");

	bump(F_MAP_PERSIST);
	P::_citrus_mapper_set_persistent(cm_a);
	ref__citrus_mapper_set_persistent(cm_b);

	bump(F_MAP_CLOSE);
	P::_citrus_mapper_close(cm_a);
	ref__citrus_mapper_close(cm_b);
	P::_citrus_mapper_close(cm_a);
	ref__citrus_mapper_close(cm_b);
}

void
random_sweep(unsigned n)
{
	unsigned char blob[BIGBUF];
	int ra, rbv;

	for (unsigned i = 0; i < n; i++) {
		std::vector<std::pair<std::string, std::vector<unsigned char>>> ents;
		unsigned nent = (unsigned)(rng_next() % 8u) + 1;

		for (unsigned j = 0; j < nent; j++) {
			std::string key;
			unsigned klen = (unsigned)(rng_next() % 8u);
			for (unsigned k = 0; k < klen; k++)
				key.push_back((char)(rb() ? rb() : ('a' + (k % 26))));
			if (key.empty())
				key = "k";
			std::vector<unsigned char> val;
			unsigned vlen = (unsigned)(rng_next() % 16u) + 1;
			for (unsigned k = 0; k < vlen; k++)
				val.push_back(rb());
			ents.emplace_back(key, val);
		}

		guard_fill(blob, BIGBUF);
		P::_citrus_db_factory *dummy_p = nullptr;
		P::_citrus_db_factory *dummy_r = nullptr;
		int sp = P::_citrus_db_factory_create(&dummy_p, hash_std, nullptr);
		int sr = ref__citrus_db_factory_create(&dummy_r, hash_std, nullptr);
		if (sp != sr) {
			fail(F_FACTORY_CREATE, "rand create");
			continue;
		}
		bump(F_FACTORY_CREATE);
		bump(F_FACTORY_FREE);
		P::_citrus_db_factory_free(dummy_p);
		ref__citrus_db_factory_free(dummy_r);

		P::_citrus_region reg;
		P::_citrus_region_init(&reg, blob + 128, BIGBUF - 256);
		P::_citrus_db_factory *df = nullptr;
		P::_citrus_db_factory_create(&df, hash_std, nullptr);
		for (const auto &e : ents) {
			P::_citrus_region data;
			P::_citrus_region_init(&data, (void *)e.second.data(), e.second.size());
			P::_citrus_db_factory_add_by_string(df, e.first.c_str(), &data, 0);
		}
		size_t used = P::_citrus_db_factory_calc_size(df);
		P::_citrus_db_factory_serialize(df, "LOOKUP\0\0", &reg);
		P::_citrus_db_factory_free(df);

		P::_citrus_region reg_blob;
		P::_citrus_region_init(&reg_blob, blob + 128, used);
		P::_citrus_db *da = nullptr, *db = nullptr;
		bump(F_DB_OPEN);
		ra = P::_citrus_db_open(&da, &reg_blob, "LOOKUP\0\0", hash_std, nullptr);
		rbv = ref__citrus_db_open(&db, &reg_blob, "LOOKUP\0\0", hash_std, nullptr);
		if (ra != rbv)
			fail(F_DB_OPEN, "rand open");

		bump(F_DB_NUM);
		int na = P::_citrus_db_get_number_of_entries(da);
		int nb = ref__citrus_db_get_number_of_entries(db);
		if (na != nb || na != (int)ents.size())
			fail(F_DB_NUM, "rand num");

		for (const auto &e : ents) {
			P::_citrus_region ka, kb;
			bump(F_DB_LOOKUP_S);
			ra = P::_citrus_db_lookup_by_string(da, e.first.c_str(), &ka, nullptr);
			rbv = ref__citrus_db_lookup_by_string(db, e.first.c_str(), &kb, nullptr);
			if (ra != rbv || P::_citrus_region_size(&ka) != P::_citrus_region_size(&kb) ||
			    (ra == 0 &&
			    std::memcmp(P::_citrus_region_head(&ka), P::_citrus_region_head(&kb),
			    P::_citrus_region_size(&ka)) != 0))
				fail(F_DB_LOOKUP_S, "rand lookup");
		}

		for (int idx = 0; idx < na; idx++) {
			P::_citrus_region ka, kb, da_r, db_r;
			bump(F_DB_ENTRY);
			ra = P::_citrus_db_get_entry(da, idx, &ka, &da_r);
			rbv = ref__citrus_db_get_entry(db, idx, &kb, &db_r);
			if (ra != rbv || P::_citrus_region_size(&ka) != P::_citrus_region_size(&kb) ||
			    P::_citrus_region_size(&da_r) != P::_citrus_region_size(&db_r) ||
			    std::memcmp(P::_citrus_region_head(&ka), P::_citrus_region_head(&kb),
			    P::_citrus_region_size(&ka)) != 0 ||
			    std::memcmp(P::_citrus_region_head(&da_r), P::_citrus_region_head(&db_r),
			    P::_citrus_region_size(&da_r)) != 0)
				fail(F_DB_ENTRY, "rand entry");
		}

		bump(F_DB_CLOSE);
		P::_citrus_db_close(da);
		ref__citrus_db_close(db);

		if ((i % 500) == 0) {
			char dir[] = "/tmp/b0229mXXXXXX";
			mkdtemp(dir);
			std::string plain = std::string(dir) + "/plain.txt";
			std::string mdir = std::string(dir) + "/mapper.dir";
			std::string content = "# comment\nfoo\tbar baz\n\n  \nkey\x80\xff\tval\n";
			write_file(plain, content.data(), content.size());
			std::string mcontent = "mymap\ttestmod\targ\n";
			write_file(mdir, mcontent.data(), mcontent.size());
			test_lookup_plain(plain, (int)(rng_next() & 1));
			test_mapper(dir);
		}
	}
}

} // namespace

int
main(void)
{
	unsigned char blob[BIGBUF];
	std::vector<std::pair<std::string, std::vector<unsigned char>>> ents = {
		{"alpha", {0x41}},
		{"beta", {0x80, 0xff}},
		{"k8", {0xab}},
		{"k16", {0x12, 0x34}},
		{"k32", {0xde, 0xad, 0xbe, 0xef}},
		{"ks", {'d', 'a', 't', 'a', '\0'}},
	};

	guard_fill(blob, BIGBUF);
	P::_citrus_db_factory *df = nullptr;
	build_db(&df, blob, BIGBUF, ents, true);
	build_db(&df, blob, BIGBUF, ents, false);

	test_factory_roundtrip();
	test_factory_typed_adds();
	test_db_ops(blob + 64, BIGBUF - 128);

	char dir[] = "/tmp/b0229tXXXXXX";
	mkdtemp(dir);
	std::string plain = std::string(dir) + "/plain.txt";
	std::string mdir = std::string(dir) + "/mapper.dir";
	const char *plain_data =
	    "# c\n"
	    "foo\tbar\n"
	    "\n"
	    "  \t  \n"
	    "empty\t\n"
	    "key\x80\xff\t\xff\x80\n";
	write_file(plain, plain_data, std::strlen(plain_data));
	const char *mdir_data = "mymap\ttestmod\tmyvar\n";
	write_file(mdir, mdir_data, std::strlen(mdir_data));

	test_lookup_plain(plain, 0);
	test_lookup_plain(plain, 1);
	test_mapper(dir);

	random_sweep(200000);

	std::printf("\n%-34s %12s %12s\n", "function", "cases", "failures");
	unsigned long long tc = 0, tf = 0;
	for (int i = 0; i < F_COUNT; i++) {
		if (NCASE[i] || NFAIL[i])
			std::printf("%-34s %12llu %12llu\n", FNAME[i],
			    (unsigned long long)NCASE[i],
			    (unsigned long long)NFAIL[i]);
		tc += NCASE[i];
		tf += NFAIL[i];
	}
	std::printf("%-34s %12llu %12llu\n", "TOTAL", tc, tf);
	return (tf ? 1 : 0);
}
