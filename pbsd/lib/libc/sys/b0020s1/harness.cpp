/*
 * PBSD b0020s1 differential test.
 *
 * Drives the C++23 port and the ref_ oracle with identical arguments and
 * compares everything observable: the return value, errno, which interposition
 * slot was reached, the arguments the interposed callee received (pointers as
 * offsets from each side's own buffer base) and the entire output buffer of
 * each side including the guard bytes past the nominal write window.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.sys.b0020s1;

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_pdwait,
	INTERPOS_wait4,
	INTERPOS_MAX
};

extern interpos_func_t __pbsd_interposing[INTERPOS_MAX];

int ref_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop);

}

namespace {

constexpr std::size_t align16(std::size_t n)
{
	return (n + 15u) & ~static_cast<std::size_t>(15u);
}

constexpr unsigned char GUARD = 0x7f;

constexpr std::size_t STATUS_OFF = 16;
constexpr std::size_t RU_OFF = 48;
constexpr std::size_t INFO_OFF = RU_OFF + align16(sizeof(struct __wrusage));
constexpr std::size_t BUF_SZ = INFO_OFF + align16(sizeof(struct __siginfo)) + 64;

/* How much of each output object the interposed callee scribbles on. */
constexpr std::size_t RU_WRITE = 24;
constexpr std::size_t INFO_WRITE = 20;

struct Record {
	unsigned long	ncalls;
	int		slot;
	int		fd;
	int		options;
	int		*status;
	struct __wrusage *ru;
	struct __siginfo *infop;
};

Record g_rec;

int
mock_body(int slot, int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	g_rec.ncalls++;
	g_rec.slot = slot;
	g_rec.fd = fd;
	g_rec.options = options;
	g_rec.status = status;
	g_rec.ru = ru;
	g_rec.infop = infop;

	std::uint32_t s = static_cast<std::uint32_t>(fd) * 2654435761u;
	s ^= static_cast<std::uint32_t>(options) * 40503u + 0x2545f491u;
	s ^= static_cast<std::uint32_t>(slot) * 0x9e3779b9u + 11u;
	s ^= (status != nullptr ? 0x01000193u : 0u);
	s ^= (ru != nullptr ? 0x0f0f0f0fu : 0u);
	s ^= (infop != nullptr ? 0x33cc55aau : 0u);

	if (status != nullptr)
		*status = static_cast<int>(s);
	if (ru != nullptr) {
		unsigned char *p = reinterpret_cast<unsigned char *>(ru);
		for (std::size_t i = 0; i < RU_WRITE; i++)
			p[i] = static_cast<unsigned char>((s >> (i & 7u)) + i);
	}
	if (infop != nullptr) {
		unsigned char *p = reinterpret_cast<unsigned char *>(infop);
		for (std::size_t i = 0; i < INFO_WRITE; i++)
			p[i] = static_cast<unsigned char>(
			    (s >> ((i + 3u) & 7u)) ^ (i * 5u));
	}

	errno = static_cast<int>(s % 97u) + 1;
	return static_cast<int>(s) - 1;
}

int
mock_slot0(int fd, int *st, int op, struct __wrusage *ru, struct __siginfo *in)
{
	return mock_body(0, fd, st, op, ru, in);
}

int
mock_slot1(int fd, int *st, int op, struct __wrusage *ru, struct __siginfo *in)
{
	return mock_body(1, fd, st, op, ru, in);
}

int
mock_slot2(int fd, int *st, int op, struct __wrusage *ru, struct __siginfo *in)
{
	return mock_body(2, fd, st, op, ru, in);
}

int
mock_slot3(int fd, int *st, int op, struct __wrusage *ru, struct __siginfo *in)
{
	return mock_body(3, fd, st, op, ru, in);
}

void
install_mocks()
{
	using fn_t = int (*)(int, int *, int, struct __wrusage *,
	    struct __siginfo *);

	__pbsd_interposing[INTERPOS_system] =
	    reinterpret_cast<interpos_func_t>(static_cast<fn_t>(mock_slot0));
	__pbsd_interposing[INTERPOS_tcdrain] =
	    reinterpret_cast<interpos_func_t>(static_cast<fn_t>(mock_slot1));
	__pbsd_interposing[INTERPOS_pdwait] =
	    reinterpret_cast<interpos_func_t>(static_cast<fn_t>(mock_slot2));
	__pbsd_interposing[INTERPOS_wait4] =
	    reinterpret_cast<interpos_func_t>(static_cast<fn_t>(mock_slot3));
}

alignas(16) unsigned char g_buf_port[BUF_SZ];
alignas(16) unsigned char g_buf_ref[BUF_SZ];

std::ptrdiff_t
off_of(const void *p, const unsigned char *base)
{
	if (p == nullptr)
		return -1;
	return reinterpret_cast<const unsigned char *>(p) - base;
}

unsigned long g_cases;
unsigned long g_failures;
unsigned long g_reported;

void
fail(const char *what, int fd, int options, bool has_status, bool has_ru,
    bool has_infop)
{
	g_failures++;
	if (g_reported < 20) {
		g_reported++;
		std::printf("FAIL pdwait: %s  fd=%d options=%d "
		    "status=%s ru=%s infop=%s\n", what, fd, options,
		    has_status ? "buf" : "NULL", has_ru ? "buf" : "NULL",
		    has_infop ? "buf" : "NULL");
	}
}

void
run_case(int fd, int options, bool has_status, bool has_ru, bool has_infop)
{
	g_cases++;

	std::memset(g_buf_port, GUARD, BUF_SZ);
	std::memset(g_buf_ref, GUARD, BUF_SZ);

	int *st_p = has_status ?
	    reinterpret_cast<int *>(g_buf_port + STATUS_OFF) : nullptr;
	int *st_r = has_status ?
	    reinterpret_cast<int *>(g_buf_ref + STATUS_OFF) : nullptr;
	struct __wrusage *ru_p = has_ru ?
	    reinterpret_cast<struct __wrusage *>(g_buf_port + RU_OFF) : nullptr;
	struct __wrusage *ru_r = has_ru ?
	    reinterpret_cast<struct __wrusage *>(g_buf_ref + RU_OFF) : nullptr;
	struct __siginfo *in_p = has_infop ?
	    reinterpret_cast<struct __siginfo *>(g_buf_port + INFO_OFF) :
	    nullptr;
	struct __siginfo *in_r = has_infop ?
	    reinterpret_cast<struct __siginfo *>(g_buf_ref + INFO_OFF) :
	    nullptr;

	std::memset(&g_rec, 0, sizeof(g_rec));
	errno = 0;
	int ret_p = pbsd::lib_libc_sys::b0020s1::pdwait(fd, st_p, options,
	    ru_p, in_p);
	int errno_p = errno;
	Record rec_p = g_rec;

	std::memset(&g_rec, 0, sizeof(g_rec));
	errno = 0;
	int ret_r = ref_pdwait(fd, st_r, options, ru_r, in_r);
	int errno_r = errno;
	Record rec_r = g_rec;

	if (ret_p != ret_r) {
		fail("return value", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (errno_p != errno_r) {
		fail("errno", fd, options, has_status, has_ru, has_infop);
		return;
	}
	if (rec_p.ncalls != 1 || rec_r.ncalls != 1 ||
	    rec_p.ncalls != rec_r.ncalls) {
		fail("call count", fd, options, has_status, has_ru, has_infop);
		return;
	}
	if (rec_p.slot != rec_r.slot) {
		fail("interposition slot", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (rec_p.fd != rec_r.fd || rec_p.fd != fd) {
		fail("fd argument", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (rec_p.options != rec_r.options || rec_p.options != options) {
		fail("options argument", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (off_of(rec_p.status, g_buf_port) !=
	    off_of(rec_r.status, g_buf_ref)) {
		fail("status pointer offset", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (off_of(rec_p.ru, g_buf_port) != off_of(rec_r.ru, g_buf_ref)) {
		fail("ru pointer offset", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (off_of(rec_p.infop, g_buf_port) !=
	    off_of(rec_r.infop, g_buf_ref)) {
		fail("infop pointer offset", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
	if (std::memcmp(g_buf_port, g_buf_ref, BUF_SZ) != 0) {
		fail("output buffer", fd, options, has_status, has_ru,
		    has_infop);
		return;
	}
}

void
run_all_null_combos(int fd, int options)
{
	for (int mask = 0; mask < 8; mask++)
		run_case(fd, options, (mask & 1) != 0, (mask & 2) != 0,
		    (mask & 4) != 0);
}

const int g_edge_ints[] = {
	(-2147483647 - 1),		/* INT_MIN */
	-2147483647,
	-65536,
	-256,
	-129,
	-128,
	-127,
	-2,
	-1,
	0,
	1,
	2,
	3,
	4,
	8,
	16,
	127,
	128,
	129,
	255,
	256,
	32767,
	32768,
	65535,
	65536,
	2147483646,
	2147483647,			/* INT_MAX */
};

std::uint64_t g_rng_state;

std::uint32_t
rng_next()
{
	/* xorshift64*, fixed seed. */
	std::uint64_t x = g_rng_state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	g_rng_state = x;
	return static_cast<std::uint32_t>((x * 0x2545f4914f6cdd1dULL) >> 32);
}

int
rng_int()
{
	std::uint32_t r = rng_next();

	switch (r & 3u) {
	case 0:
		/* Full 32-bit range. */
		return static_cast<int>(rng_next());
	case 1:
		/* Small non-negative, exercises the low fd/options range. */
		return static_cast<int>(rng_next() % 64u);
	case 2:
		/* Small negative. */
		return -static_cast<int>(rng_next() % 64u) - 1;
	default: {
		/* Boundary values and single high-bit bytes. */
		const std::size_t n = sizeof(g_edge_ints) /
		    sizeof(g_edge_ints[0]);
		return g_edge_ints[rng_next() % n];
	}
	}
}

} /* namespace */

int
main()
{
	install_mocks();

	const std::size_t nedge = sizeof(g_edge_ints) / sizeof(g_edge_ints[0]);

	/* Hand-written edge cases: every boundary fd against every boundary
	 * options value, against every combination of NULL output pointers. */
	for (std::size_t i = 0; i < nedge; i++)
		for (std::size_t j = 0; j < nedge; j++)
			run_all_null_combos(g_edge_ints[i], g_edge_ints[j]);

	/* fd and options equal, so that an argument swap is invisible here and
	 * must be caught by the asymmetric cases above and below. */
	for (std::size_t i = 0; i < nedge; i++)
		run_all_null_combos(g_edge_ints[i], g_edge_ints[i]);

	/* Fixed-seed randomised sweep. */
	g_rng_state = 0x0020000100000001ULL;
	for (unsigned long iter = 0; iter < 250000UL; iter++) {
		int fd = rng_int();
		int options = rng_int();
		std::uint32_t m = rng_next();
		run_case(fd, options, (m & 1u) != 0, (m & 2u) != 0,
		    (m & 4u) != 0);
	}

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12lu %12lu\n", "pdwait", g_cases, g_failures);

	if (g_failures != 0) {
		std::printf("\nRESULT: FAIL (%lu of %lu cases diverged)\n",
		    g_failures, g_cases);
		return 1;
	}
	std::printf("\nRESULT: PASS (all %lu cases matched)\n", g_cases);
	return 0;
}
