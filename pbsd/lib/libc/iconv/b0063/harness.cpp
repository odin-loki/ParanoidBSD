/*
 * Differential test for batch b0063: pbsd.lib.libc.iconv.b0063 (port) versus
 * the ref_ oracle.  Instrumented __bsd_* mocks in oracle.c record every
 * forwarded argument and drive observable return values and side effects.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.iconv.b0063;

namespace port = pbsd::lib_libc_iconv::b0063;

using port::__iconv_bool;

extern "C" {
const char *ref_iconv_canonicalize(const char *);
int ref___iconv_get_list(char ***, size_t *, bool);
void ref_iconvlist(int (*)(unsigned int, const char *const *, void *), void *);
void ref_iconv_set_relocation_prefix(const char *, const char *);

typedef struct B0063MockSnap {
	unsigned long long	ncalls;
	const char		*canonicalize_arg;
	const char		*canonicalize_ret;
	int			get_list_ret;
	char			***get_list_a;
	size_t			*get_list_b;
	bool			get_list_c;
	char			**get_list_out;
	size_t			get_list_count;
	int			(*iconvlist_fn)(unsigned int, const char *const *, void *);
	void			*iconvlist_data;
	unsigned int		iconvlist_cb_count;
	unsigned int		iconvlist_cb_arg0;
	const char		*iconvlist_cb_names[4];
	void			*iconvlist_cb_user;
	int			iconvlist_cb_rets[4];
	const char		*reloc_a;
	const char		*reloc_b;
} B0063MockSnap;

void b0063_mock_reset(void);
void b0063_mock_snap(B0063MockSnap *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t STR_POOL = 4096;
constexpr std::size_t CANON_BUF = 512;
constexpr std::size_t SLOT_PAD = 32;

using MockSnap = B0063MockSnap;

enum {
	F_CANON,
	F_GET_LIST,
	F_LIST,
	F_RELOC,
	F_COUNT
};

const char *const fname[F_COUNT] = {
	"iconv_canonicalize",
	"__iconv_get_list",
	"iconvlist",
	"iconv_set_relocation_prefix",
};

unsigned long ncase[F_COUNT];
unsigned long nfail[F_COUNT];
unsigned nprinted[F_COUNT];

void
report_fail(int f, const char *why)
{

	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-28s : %s\n", fname[f], why);
}

bool
snaps_equal(const MockSnap &a, const MockSnap &b)
{

	if (a.ncalls != b.ncalls)
		return (false);
	if (a.canonicalize_arg != b.canonicalize_arg)
		return (false);
	if (a.canonicalize_ret != b.canonicalize_ret)
		return (false);
	if (a.get_list_ret != b.get_list_ret)
		return (false);
	if (a.get_list_c != b.get_list_c)
		return (false);
	if (a.get_list_out != b.get_list_out)
		return (false);
	if (a.get_list_count != b.get_list_count)
		return (false);
	if (a.iconvlist_fn != b.iconvlist_fn)
		return (false);
	if (a.iconvlist_data != b.iconvlist_data)
		return (false);
	if (a.iconvlist_cb_count != b.iconvlist_cb_count)
		return (false);
	if (a.iconvlist_cb_arg0 != b.iconvlist_cb_arg0)
		return (false);
	if (a.iconvlist_cb_user != b.iconvlist_cb_user)
		return (false);
	if (a.reloc_a != b.reloc_a)
		return (false);
	if (a.reloc_b != b.reloc_b)
		return (false);
	for (unsigned i = 0; i < 4; i++) {
		if (a.iconvlist_cb_names[i] != b.iconvlist_cb_names[i])
			return (false);
		if (a.iconvlist_cb_rets[i] != b.iconvlist_cb_rets[i])
			return (false);
	}
	return (true);
}

std::uint64_t rng = 0x0063b00630063006ULL;

std::uint64_t
rng_next(void)
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

unsigned char
rand_byte(void)
{

	return ((unsigned char)(rng_next() & 0xffu));
}

int
user_cb(unsigned int count, const char *const *names, void *arg)
{
	unsigned long hits = 1;

	if (arg != nullptr)
		hits = *(unsigned long *)arg;

	hits = hits * 131u + count;
	for (unsigned int i = 0; i < count; i++) {
		const char *s = names[i];

		if (s != nullptr) {
			while (*s != '\0')
				hits = hits * 33u + (unsigned char)*s++;
		}
	}
	if (arg != nullptr)
		*(unsigned long *)arg = hits;
	return ((int)(hits & 0x7fffffffu));
}

void
fill_pool(unsigned char *pool, std::size_t len, std::size_t str_len,
    unsigned char lead, unsigned char tail)
{
	std::size_t i;

	std::memset(pool, GUARD, len);
	if (str_len + 2 > len)
		str_len = len - 2;
	pool[0] = lead;
	for (i = 1; i <= str_len; i++)
		pool[i] = (unsigned char)(0x20u + (i * 17u + lead) % 95u);
	pool[str_len + 1] = tail;
	pool[str_len + 2] = '\0';
}

void
fill_pool_random(unsigned char *pool, std::size_t len, std::size_t str_len)
{
	std::size_t i;

	std::memset(pool, GUARD, len);
	if (str_len + 2 > len)
		str_len = len - 2;
	for (i = 0; i <= str_len; i++)
		pool[i] = rand_byte();
	pool[str_len + 1] = '\0';
}

bool
bufs_equal(const unsigned char *a, const unsigned char *b, std::size_t n)
{

	return (std::memcmp(a, b, n) == 0);
}

void
check_canonicalize(const char *arg)
{
	MockSnap ref_s, port_s;
	const char *ref_ret, *port_ret;
	char msg[160];

	ncase[F_CANON]++;

	b0063_mock_reset();
	ref_ret = ref_iconv_canonicalize(arg);
	b0063_mock_snap(&ref_s);

	b0063_mock_reset();
	port_ret = port::iconv_canonicalize(arg);
	b0063_mock_snap(&port_s);

	if (!snaps_equal(ref_s, port_s)) {
		std::snprintf(msg, sizeof(msg), "mock snapshot mismatch");
		report_fail(F_CANON, msg);
		return;
	}
	if (ref_ret != port_ret) {
		std::snprintf(msg, sizeof(msg),
		    "return ptr ref=%p port=%p", (const void *)ref_ret,
		    (const void *)port_ret);
		report_fail(F_CANON, msg);
		return;
	}
	if (arg == nullptr) {
		if (ref_ret != nullptr)
			report_fail(F_CANON, "expected NULL return");
		return;
	}
	if (ref_ret != nullptr) {
		unsigned char rb[CANON_BUF + SLOT_PAD * 2];
		unsigned char pb[CANON_BUF + SLOT_PAD * 2];

		std::memset(rb, GUARD, sizeof(rb));
		std::memset(pb, GUARD, sizeof(pb));
		std::memcpy(rb + SLOT_PAD, ref_ret, CANON_BUF);
		std::memcpy(pb + SLOT_PAD, port_ret, CANON_BUF);
		if (!bufs_equal(rb, pb, sizeof(rb)))
			report_fail(F_CANON, "canonical string buffer mismatch");
	}
}

void
test_canonicalize_pool(unsigned char *pool, std::size_t off)
{

	check_canonicalize((const char *)(pool + off));
}

struct GetListSlots {
	alignas(char ***) unsigned char a_slot[sizeof(char ***) + SLOT_PAD * 2];
	alignas(size_t) unsigned char b_slot[sizeof(size_t) + SLOT_PAD * 2];
	alignas(char ***) unsigned char out_slot[sizeof(char **) * 4 + SLOT_PAD * 2];
};

void
check_get_list(__iconv_bool c, char ***a_in, size_t *b_in)
{
	GetListSlots rs, ps;
	MockSnap ref_s, port_s;
	char ***a_r, ***a_p;
	size_t *b_r, *b_p;
	int ref_ret, port_ret;
	char msg[160];

	ncase[F_GET_LIST]++;

	std::memset(&rs, GUARD, sizeof(rs));
	std::memset(&ps, GUARD, sizeof(ps));

	a_r = (char ***)(rs.a_slot + SLOT_PAD);
	b_r = (size_t *)(rs.b_slot + SLOT_PAD);
	a_p = (char ***)(ps.a_slot + SLOT_PAD);
	b_p = (size_t *)(ps.b_slot + SLOT_PAD);

	if (a_in != nullptr)
		*a_r = *a_p = *a_in;
	if (b_in != nullptr)
		*b_r = *b_p = *b_in;

	b0063_mock_reset();
	ref_ret = ref___iconv_get_list(a_r, b_r, c);
	b0063_mock_snap(&ref_s);

	b0063_mock_reset();
	port_ret = port::__iconv_get_list(a_p, b_p, c);
	b0063_mock_snap(&port_s);

	if (ref_ret != port_ret) {
		std::snprintf(msg, sizeof(msg), "ret ref=%d port=%d c=%d",
		    ref_ret, port_ret, (int)c);
		report_fail(F_GET_LIST, msg);
	}
	if (!snaps_equal(ref_s, port_s))
		report_fail(F_GET_LIST, "mock snapshot mismatch");
	if (!bufs_equal(rs.a_slot, ps.a_slot, sizeof(rs.a_slot)))
		report_fail(F_GET_LIST, "a_slot buffer mismatch");
	if (!bufs_equal(rs.b_slot, ps.b_slot, sizeof(rs.b_slot)))
		report_fail(F_GET_LIST, "b_slot buffer mismatch");
	if (!bufs_equal(rs.out_slot, ps.out_slot, sizeof(rs.out_slot)))
		report_fail(F_GET_LIST, "out_slot buffer mismatch");
	if (*a_r != nullptr && *a_p != nullptr) {
		if ((*a_r)[0] != (*a_p)[0] || (*a_r)[1] != (*a_p)[1])
			report_fail(F_GET_LIST, "list pointer mismatch");
	}
	if (*b_r != *b_p)
		report_fail(F_GET_LIST, "count mismatch");
}

void
check_iconvlist(void *userdata)
{
	MockSnap ref_s, port_s;
	unsigned long shared_hits = 1;
	unsigned long ref_result = 1, port_result = 1;
	void *data = userdata == nullptr ? nullptr : &shared_hits;
	char msg[160];

	ncase[F_LIST]++;

	shared_hits = 1;
	b0063_mock_reset();
	ref_iconvlist(user_cb, data);
	b0063_mock_snap(&ref_s);
	ref_result = shared_hits;

	if (data != nullptr)
		shared_hits = 1;
	b0063_mock_reset();
	port::iconvlist(user_cb, data);
	b0063_mock_snap(&port_s);
	port_result = shared_hits;

	if (!snaps_equal(ref_s, port_s)) {
		std::snprintf(msg, sizeof(msg), "mock snapshot mismatch");
		report_fail(F_LIST, msg);
	}
	if (data != nullptr && ref_result != port_result) {
		std::snprintf(msg, sizeof(msg), "cb digest ref=%lu port=%lu",
		    ref_result, port_result);
		report_fail(F_LIST, msg);
	}
}

void
check_reloc(const char *a, const char *b)
{
	MockSnap ref_s, port_s;

	ncase[F_RELOC]++;

	b0063_mock_reset();
	ref_iconv_set_relocation_prefix(a, b);
	b0063_mock_snap(&ref_s);

	b0063_mock_reset();
	port::iconv_set_relocation_prefix(a, b);
	b0063_mock_snap(&port_s);

	if (!snaps_equal(ref_s, port_s))
		report_fail(F_RELOC, "mock snapshot mismatch");
}

void
run_edge_cases(void)
{
	unsigned char pool[STR_POOL];
	static char dummy0[] = "seed0";
	static char dummy1[] = "seed1";
	static char *entries[] = {dummy0, dummy1, nullptr};
	char **seed_list = entries;
	char ***a_seed = &seed_list;
	size_t b_seed = 99;
	unsigned i;

	check_canonicalize(nullptr);
	fill_pool(pool, sizeof(pool), 0, 'A', 'z');
	test_canonicalize_pool(pool, 0);
	fill_pool(pool, sizeof(pool), 1, 0x00, 0xff);
	test_canonicalize_pool(pool, 1);
	fill_pool(pool, sizeof(pool), 3, 0x80, 0xfe);
	test_canonicalize_pool(pool, 0);

	for (i = 0; i < 256; i++) {
		pool[0] = (unsigned char)i;
		pool[1] = '\0';
		test_canonicalize_pool(pool, 0);
	}

	pool[0] = pool[1] = pool[2] = '\0';
	test_canonicalize_pool(pool, 0);
	pool[0] = 'U';
	pool[1] = 'T';
	pool[2] = 'F';
	pool[3] = '-';
	pool[4] = '8';
	pool[5] = '\0';
	test_canonicalize_pool(pool, 0);

	for (i = 0; i < 64; i++) {
		pool[i] = (unsigned char)(0x80u | (i & 0x7f));
		pool[i + 1] = '\0';
		test_canonicalize_pool(pool, 0);
	}

	check_get_list((__iconv_bool)0, nullptr, nullptr);
	check_get_list((__iconv_bool)1, nullptr, nullptr);
	check_get_list((__iconv_bool)0, a_seed, nullptr);
	check_get_list((__iconv_bool)1, a_seed, nullptr);
	check_get_list((__iconv_bool)0, nullptr, &b_seed);
	check_get_list((__iconv_bool)1, nullptr, &b_seed);
	check_get_list((__iconv_bool)0, a_seed, &b_seed);
	check_get_list((__iconv_bool)1, a_seed, &b_seed);

	check_iconvlist(nullptr);
	{
		unsigned long bucket = 1;

		check_iconvlist(&bucket);
	}
	{
		unsigned long opaque = 0xdeadbeefUL;

		check_iconvlist(&opaque);
	}

	fill_pool(pool, sizeof(pool), 8, 'p', '/');
	check_reloc(nullptr, nullptr);
	check_reloc((const char *)pool, nullptr);
	check_reloc(nullptr, (const char *)pool);
	check_reloc((const char *)pool, (const char *)(pool + 4));
	check_reloc((const char *)(pool + 1), (const char *)(pool + 7));
}

void
run_random_sweep(unsigned long iterations)
{
	unsigned char pool_a[STR_POOL];
	unsigned char pool_b[STR_POOL];
	unsigned long i;

	for (i = 0; i < iterations; i++) {
		std::size_t len = (std::size_t)(rng_next() % (STR_POOL / 4));
		std::size_t off_a = (std::size_t)(rng_next() % 16);
		std::size_t off_b = (std::size_t)(rng_next() % 16);
		__iconv_bool c = (__iconv_bool)(rng_next() & 1u);
		unsigned mode = (unsigned)(rng_next() % 4u);

		fill_pool_random(pool_a, sizeof(pool_a), len);
		fill_pool_random(pool_b, sizeof(pool_b), len);

		switch (mode) {
		case 0:
			if ((rng_next() & 0xfu) == 0u)
				check_canonicalize(nullptr);
			else
				test_canonicalize_pool(pool_a, off_a);
			break;
		case 1: {
			char ***a_seed = nullptr;
			size_t b_seed = 0;
			static char d0[] = "x";
			static char *slot = d0;
			static char **pp = &slot;

			if (rng_next() & 1u)
				a_seed = &pp;
			if (rng_next() & 1u)
				b_seed = (size_t)rng_next();
			check_get_list(c, a_seed,
			    (rng_next() & 1u) ? &b_seed : nullptr);
			break;
		}
		case 2: {
			unsigned long rnd_hits = 1;

			if (rng_next() & 1u)
				check_iconvlist(nullptr);
			else
				check_iconvlist(&rnd_hits);
			break;
		}
		default:
			if ((rng_next() & 3u) == 0u)
				check_reloc(nullptr, nullptr);
			else if ((rng_next() & 1u) == 0u)
				check_reloc((const char *)(pool_a + off_a),
				    (const char *)(pool_b + off_b));
			else
				check_reloc((const char *)(pool_a + off_a),
				    nullptr);
			break;
		}
	}
}

} /* namespace */

int
main(void)
{
	unsigned f;
	unsigned long total_fail = 0;

	run_edge_cases();
	run_random_sweep(200000ul);

	std::printf("\n%-32s %10s %10s\n", "function", "cases", "failures");
	for (f = 0; f < F_COUNT; f++) {
		std::printf("%-32s %10lu %10lu\n", fname[f], ncase[f],
		    nfail[f]);
		total_fail += nfail[f];
	}

	return (total_fail == 0 ? 0 : 1);
}
