// Differential test for PBSD batch b0146.

import pbsd.sys.kern.b0146;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

namespace port = pbsd::sys_kern::b0146;

#define GUARD     0x7f
#define PAD       32u
#define SWEEP     200000L
#define MAX_PRINT 12

#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "hashinit_flags",        0, 0, 0 },
	{ "hashinit",              0, 0, 0 },
	{ "hashdestroy",           0, 0, 0 },
	{ "phashinit_flags",       0, 0, 0 },
	{ "phashinit",             0, 0, 0 },
	{ "sys_getrandom",         0, 0, 0 },
	{ "efi_map_foreach_entry", 0, 0, 0 },
	{ "efi_map_add_entries",   0, 0, 0 },
	{ "efi_map_exclude_entries", 0, 0, 0 },
	{ "efi_map_print_entries", 0, 0, 0 },
	{ "sema_init",             0, 0, 0 },
	{ "sema_destroy",          0, 0, 0 },
	{ "_sema_post",            0, 0, 0 },
	{ "_sema_wait",            0, 0, 0 },
	{ "_sema_timedwait",       0, 0, 0 },
	{ "_sema_trywait",         0, 0, 0 },
	{ "sema_value",            0, 0, 0 },
};

enum {
	R_HASH_FLAGS = 0,
	R_HASH,
	R_HASH_DESTROY,
	R_PHASH_FLAGS,
	R_PHASH,
	R_GETRANDOM,
	R_EFI_FOREACH,
	R_EFI_ADD,
	R_EFI_EXCLUDE,
	R_EFI_PRINT,
	R_SEMA_INIT,
	R_SEMA_DESTROY,
	R_SEMA_POST,
	R_SEMA_WAIT,
	R_SEMA_TIMEDWAIT,
	R_SEMA_TRYWAIT,
	R_SEMA_VALUE,
};

static uint64_t rng_state = 0x00b0146faceULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-24s %-24s %s\n", r.name, label, detail);
	}
}

static void
case_row(int row)
{
	rows[row].cases++;
}

/* ----------------------------------------------------------- printf wrap */

static int g_port_printf_calls;
static int g_ref_printf_calls;

extern "C" int
__wrap_printf(const char *fmt, ...)
{
	(void)fmt;
	if (g_port_printf_calls >= 0)
		g_port_printf_calls++;
	else
		g_ref_printf_calls++;
	return (0);
}

static void
printf_reset(int *port_calls, int *ref_calls)
{
	g_port_printf_calls = 0;
	g_ref_printf_calls = -1;
	*port_calls = 0;
	*ref_calls = 0;
}

static void
printf_track_port(int *port_calls)
{
	g_port_printf_calls = 0;
	g_ref_printf_calls = -1;
	*port_calls = 0;
}

static void
printf_track_ref(int *ref_calls)
{
	g_port_printf_calls = -1;
	g_ref_printf_calls = 0;
	*ref_calls = 0;
}

static void
printf_collect_port(int *port_calls)
{
	*port_calls = g_port_printf_calls;
	g_port_printf_calls = -1;
	g_ref_printf_calls = -1;
}

static void
printf_collect_ref(int *ref_calls)
{
	*ref_calls = g_ref_printf_calls;
	g_port_printf_calls = -1;
	g_ref_printf_calls = -1;
}

/* ----------------------------------------------------------- oracle decls */

extern "C" {
struct thread {
	long td_retval[2];
};
struct getrandom_args {
	void *buf;
	size_t buflen;
	unsigned int flags;
};
struct efi_map_header;
struct efi_md;
struct malloc_type;
struct mtx {
	pthread_mutex_t lock;
	const char *name;
};
struct cv {
	pthread_cond_t cond;
	const char *name;
};
struct sema {
	struct mtx sema_mtx;
	struct cv sema_cv;
	int sema_value;
	int sema_waiters;
};

struct physmem_log_entry {
	int exclude;
	uint64_t phys;
	uint64_t size;
	int exflag;
};

typedef void (*efi_map_entry_cb)(struct efi_md *p, void *argp);

void *ref_hashinit_flags(int, struct malloc_type *, unsigned long *, int);
void *ref_hashinit(int, struct malloc_type *, unsigned long *);
void ref_hashdestroy(void *, struct malloc_type *, unsigned long);
void *ref_phashinit_flags(int, struct malloc_type *, unsigned long *, int);
void *ref_phashinit(int, struct malloc_type *, unsigned long *);
int ref_sys_getrandom(struct thread *, struct getrandom_args *);
void ref_efi_map_foreach_entry(struct efi_map_header *, efi_map_entry_cb, void *);
void ref_efi_map_add_entries(struct efi_map_header *);
void ref_efi_map_exclude_entries(struct efi_map_header *);
void ref_efi_map_print_entries(struct efi_map_header *);
void ref_sema_init(struct sema *, int, const char *);
void ref_sema_destroy(struct sema *);
void ref__sema_post(struct sema *, const char *, int);
void ref__sema_wait(struct sema *, const char *, int);
int ref__sema_timedwait(struct sema *, int, const char *, int);
int ref__sema_trywait(struct sema *, const char *, int);
int ref_sema_value(struct sema *);

void oracle_malloc_reset(void);
void oracle_malloc_fail_at(int);
void oracle_read_random_reset(void);
void oracle_read_random_configure(int, int, ssize_t);
void oracle_physmem_reset(void);
int oracle_physmem_log_count(void);
const struct physmem_log_entry *oracle_physmem_log_entry(int);
}

/* ----------------------------------------------------------- helpers */

static struct malloc_type g_mtype = { "test" };

static bool
bufs_match(const unsigned char *a, const unsigned char *b, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++) {
		if (a[i] != b[i])
			return (false);
	}
	return (true);
}

static bool
physmem_logs_match(void)
{
	int n = port::physmem_log_count();
	int rn = oracle_physmem_log_count();

	if (n != rn)
		return (false);
	for (int i = 0; i < n; i++) {
		const port::physmem_log_rec *p = port::physmem_log_rec(i);
		const physmem_log_entry *r = oracle_physmem_log_entry(i);

		if (p->exclude != r->exclude || p->phys != r->phys ||
		    p->size != r->size || p->exflag != r->exflag)
			return (false);
	}
	return (true);
}

struct efi_fixture {
	std::uint8_t *blob;
	std::size_t blob_size;
	port::efi_map_header *hdr;
};

static void
efi_fixture_init(efi_fixture *fx, int ndesc, std::size_t desc_size)
{
	std::size_t efisz = (sizeof(port::efi_map_header) + 0xf) & ~0xf;
	std::size_t total = efisz + ndesc * desc_size + PAD * 2;

	fx->blob_size = total;
	fx->blob = static_cast<std::uint8_t *>(std::malloc(total));
	std::memset(fx->blob, GUARD, total);
	fx->hdr = reinterpret_cast<port::efi_map_header *>(fx->blob + PAD);
	std::memset(fx->hdr, 0, efisz + ndesc * desc_size);
	fx->hdr->memory_size = ndesc * desc_size;
	fx->hdr->descriptor_size = desc_size;
}

static void
efi_fixture_fini(efi_fixture *fx)
{
	std::free(fx->blob);
	fx->blob = nullptr;
}

static port::efi_md *
efi_desc(efi_fixture *fx, int idx)
{
	std::size_t efisz = (sizeof(port::efi_map_header) + 0xf) & ~0xf;

	return (reinterpret_cast<port::efi_md *>(
	    reinterpret_cast<std::uint8_t *>(fx->hdr) + efisz +
	    idx * fx->hdr->descriptor_size));
}

struct cb_log {
	int count;
	std::uint32_t types[64];
	std::uint64_t phys[64];
};

static void
port_cb(port::efi_md *p, void *arg)
{
	cb_log *log = static_cast<cb_log *>(arg);

	if (log->count < 64) {
		log->types[log->count] = p->md_type;
		log->phys[log->count] = p->md_phys;
	}
	log->count++;
}

extern "C" static void
ref_cb(struct efi_md *p, void *arg)
{
	cb_log *log = static_cast<cb_log *>(arg);

	if (log->count < 64) {
		log->types[log->count] = p->md_type;
		log->phys[log->count] = p->md_phys;
	}
	log->count++;
}

static bool
cb_logs_match(const cb_log &a, const cb_log &b)
{
	if (a.count != b.count)
		return (false);
	for (int i = 0; i < a.count && i < 64; i++) {
		if (a.types[i] != b.types[i] || a.phys[i] != b.phys[i])
			return (false);
	}
	return (true);
}

/* ----------------------------------------------------------- hash tests */

static void
check_hash_bucket_inits(void *tbl, unsigned long mask, int row)
{
	struct generic_head { void *lh_first; };
	auto *hp = static_cast<generic_head *>(tbl);

	for (unsigned long i = 0; i <= mask; i++) {
		if (hp[i].lh_first != nullptr) {
			fail_row(row, "bucket-init", "non-empty bucket");
			return;
		}
	}
}

static void
test_hashinit_flags_one(int elements, int flags, int fail_at, int row)
{
	unsigned long port_mask = 0xdeadbeefUL;
	unsigned long ref_mask = 0xdeadbeefUL;
	void *port_tbl = nullptr;
	void *ref_tbl = nullptr;

	case_row(row);
	port::malloc_reset();
	oracle_malloc_reset();
	port::malloc_fail_at(fail_at);
	oracle_malloc_fail_at(fail_at);

	port_tbl = port::hashinit_flags(elements, &g_mtype, &port_mask, flags);
	ref_tbl = ref_hashinit_flags(elements, &g_mtype, &ref_mask, flags);

	if ((port_tbl == nullptr) != (ref_tbl == nullptr)) {
		fail_row(row, "alloc", "null mismatch");
		goto out;
	}
	if (port_tbl != nullptr && port_mask != ref_mask) {
		fail_row(row, "mask", "hashmask mismatch");
		goto out;
	}
	if (port_tbl != nullptr)
		check_hash_bucket_inits(port_tbl, port_mask, row);

out:
	if (port_tbl != nullptr)
		port::hashdestroy(port_tbl, &g_mtype, port_mask);
	if (ref_tbl != nullptr)
		ref_hashdestroy(ref_tbl, &g_mtype, ref_mask);
}

static void
test_hash_hand(void)
{
	const int flags[] = { HASH_WAITOK, HASH_NOWAIT };
	const int elems[] = { 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127,
	    128, 255, 256, 1023, 1024, 4095, 4096, 8191, 16384 };

	for (int e : elems) {
		for (int f : flags)
			test_hashinit_flags_one(e, f, 0, R_HASH_FLAGS);
	}
	test_hashinit_flags_one(16, HASH_WAITOK, 1, R_HASH_FLAGS);

	for (int e : elems) {
		case_row(R_HASH);
		unsigned long pm = 0, rm = 0;
		port::malloc_reset();
		oracle_malloc_reset();
		void *pt = port::hashinit(e, &g_mtype, &pm);
		void *rt = ref_hashinit(e, &g_mtype, &rm);
		if ((pt == nullptr) != (rt == nullptr) || pm != rm)
			fail_row(R_HASH, "hashinit", "mismatch");
		if (pt != nullptr)
			port::hashdestroy(pt, &g_mtype, pm);
		if (rt != nullptr)
			ref_hashdestroy(rt, &g_mtype, rm);
	}

	case_row(R_HASH_DESTROY);
	{
		unsigned long mask = 0;
		void *tbl = port::hashinit(8, &g_mtype, &mask);
		port::hashdestroy(tbl, &g_mtype, mask);
		tbl = ref_hashinit(8, &g_mtype, &mask);
		ref_hashdestroy(tbl, &g_mtype, mask);
	}
}

static void
test_phash_one(int elements, int flags, int fail_at, int row)
{
	unsigned long port_n = 0xdeadbeefUL;
	unsigned long ref_n = 0xdeadbeefUL;
	void *port_tbl = nullptr;
	void *ref_tbl = nullptr;

	case_row(row);
	port::malloc_reset();
	oracle_malloc_reset();
	port::malloc_fail_at(fail_at);
	oracle_malloc_fail_at(fail_at);

	port_tbl = port::phashinit_flags(elements, &g_mtype, &port_n, flags);
	ref_tbl = ref_phashinit_flags(elements, &g_mtype, &ref_n, flags);

	if ((port_tbl == nullptr) != (ref_tbl == nullptr)) {
		fail_row(row, "alloc", "null mismatch");
		goto out;
	}
	if (port_tbl != nullptr && port_n != ref_n) {
		fail_row(row, "nentries", "count mismatch");
		goto out;
	}
	if (port_tbl != nullptr)
		check_hash_bucket_inits(port_tbl, port_n - 1, row);

out:
	if (port_tbl != nullptr)
		port::hashdestroy(port_tbl, &g_mtype, port_n - 1);
	if (ref_tbl != nullptr)
		ref_hashdestroy(ref_tbl, &g_mtype, ref_n - 1);
}

static void
test_phash_hand(void)
{
	const int primes_bound[] = { 1, 2, 12, 13, 14, 30, 31, 32, 60, 61, 62,
	    126, 127, 128, 250, 251, 508, 509, 760, 761, 1020, 1021, 32748,
	    32749, 32750, 50000, 100000 };
	const int flags[] = { HASH_WAITOK, HASH_NOWAIT };

	for (int e : primes_bound) {
		for (int f : flags)
			test_phash_one(e, f, 0, R_PHASH_FLAGS);
	}
	test_phash_one(31, HASH_WAITOK, 1, R_PHASH_FLAGS);

	for (int e : primes_bound) {
		case_row(R_PHASH);
		unsigned long pn = 0, rn = 0;
		port::malloc_reset();
		oracle_malloc_reset();
		void *pt = port::phashinit(e, &g_mtype, &pn);
		void *rt = ref_phashinit(e, &g_mtype, &rn);
		if ((pt == nullptr) != (rt == nullptr) || pn != rn)
			fail_row(R_PHASH, "phashinit", "mismatch");
		if (pt != nullptr)
			port::hashdestroy(pt, &g_mtype, pn - 1);
		if (rt != nullptr)
			ref_hashdestroy(rt, &g_mtype, rn - 1);
	}
}

static void
test_hash_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int elements = (int)(rnd32() % 20000) + 1;
		int flags = (rnd32() & 1) ? HASH_WAITOK : HASH_NOWAIT;
		int fail_at = ((rnd32() % 20) == 0) ? 1 : 0;

		test_hashinit_flags_one(elements, flags, fail_at, R_HASH_FLAGS);
		test_phash_one(elements, flags, fail_at, R_PHASH_FLAGS);
	}
}

/* ----------------------------------------------------------- getrandom */

static void
test_getrandom_one(std::size_t buflen, unsigned int flags, int rr_err,
    int rr_block, ssize_t rr_xfer)
{
	case_row(R_GETRANDOM);

	const std::size_t total = buflen + PAD * 2;
	unsigned char *pb = static_cast<unsigned char *>(std::malloc(total));
	unsigned char *rb = static_cast<unsigned char *>(std::malloc(total));

	std::memset(pb, GUARD, total);
	std::memset(rb, GUARD, total);

	port::thread ptd = {};
	struct thread rtd = {};
	port::getrandom_args puap = { pb + PAD, buflen, flags };
	struct getrandom_args ruap = { rb + PAD, buflen, flags };

	port::read_random_reset();
	oracle_read_random_reset();
	port::read_random_configure(rr_err, rr_block, rr_xfer);
	oracle_read_random_configure(rr_err, rr_block, rr_xfer);

	int pr = port::sys_getrandom(&ptd, &puap);
	int rr = ref_sys_getrandom(&rtd, &ruap);

	if (pr != rr)
		fail_row(R_GETRANDOM, "retval", "error code");
	if (ptd.td_retval[0] != rtd.td_retval[0])
		fail_row(R_GETRANDOM, "td_retval", "bytes returned");
	if (!bufs_match(pb, rb, total))
		fail_row(R_GETRANDOM, "buffer", "guard/data mismatch");

	std::free(pb);
	std::free(rb);
}

static void
test_getrandom_hand(void)
{
	const unsigned int flag_sets[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0x10, 0x80000000U
	};
	const std::size_t lens[] = { 0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64,
	    127, 128, 255, 256, 1024, 0x80000000UL, 0x80000001UL };

	for (std::size_t len : lens) {
		for (unsigned int fl : flag_sets)
			test_getrandom_one(len, fl, 0, 0, -1);
	}

	test_getrandom_one(64, 1, 35, 0, -1);
	test_getrandom_one(64, 1, 0, 1, -1);
	test_getrandom_one(64, 0, 0, 0, 0);
	test_getrandom_one(64, 0, 0, 0, 32);
	test_getrandom_one(64, 4, 0, 0, -1);
	test_getrandom_one(64, 5, 0, 1, -1);
}

static void
test_getrandom_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		std::size_t len = rnd32() % 512;
		unsigned int flags = rnd32() & 0xff;
		int err = 0;
		int block = (rnd32() & 3) == 0 ? 1 : 0;
		ssize_t xfer = (rnd32() & 7) == 0 ? (ssize_t)(rnd32() % 256) : -1;

		if ((rnd32() % 50) == 0)
			err = 35;
		if ((rnd32() % 100) == 0)
			len = 0x80000000UL + (rnd32() % 16);
		test_getrandom_one(len, flags, err, block, xfer);
	}
}

/* ----------------------------------------------------------- efi map */

static void
fill_desc(port::efi_md *d, std::uint32_t type, std::uint64_t phys,
    std::uint64_t pages, std::uint64_t attr)
{
	d->md_type = type;
	d->md_pad = 0;
	d->md_phys = phys;
	d->md_virt = phys + 0x1000;
	d->md_pages = pages;
	d->md_attr = attr;
}

static void
test_efi_foreach_one(int ndesc, std::size_t desc_size, int zero_desc)
{
	case_row(R_EFI_FOREACH);
	efi_fixture fx;
	efi_fixture_init(&fx, ndesc, desc_size);
	if (zero_desc)
		fx.hdr->descriptor_size = 0;

	for (int i = 0; i < ndesc; i++) {
		port::efi_md *d = efi_desc(&fx, i);
		fill_desc(d, (std::uint32_t)(7 + (i % 8)), (std::uint64_t)(i * 0x1000),
		    (std::uint64_t)(i + 1), (std::uint64_t)(1u << (i % 12)));
	}

	cb_log plog = {}, rlog = {};
	port::efi_map_foreach_entry(fx.hdr, port_cb, &plog);
	ref_efi_map_foreach_entry(reinterpret_cast<struct efi_map_header *>(fx.hdr),
	    ref_cb, &rlog);

	if (!cb_logs_match(plog, rlog))
		fail_row(R_EFI_FOREACH, "callback", "invocation mismatch");

	efi_fixture_fini(&fx);
}

static void
test_efi_physmem_one(int exclude_pass)
{
	int row = exclude_pass ? R_EFI_EXCLUDE : R_EFI_ADD;
	const std::uint32_t types[] = { 0, 1, 3, 4, 5, 6, 7, 9, 14, 99 };
	efi_fixture fx;
	physmem_log_entry port_copy[64];
	int pn;

	case_row(row);
	efi_fixture_init(&fx, 10, sizeof(port::efi_md));
	for (int i = 0; i < 10; i++)
		fill_desc(efi_desc(&fx, i), types[i], (std::uint64_t)(0x10000 * i),
		    (std::uint64_t)(i + 1), 0);

	port::physmem_reset();
	oracle_physmem_reset();
	if (exclude_pass)
		port::efi_map_exclude_entries(fx.hdr);
	else
		port::efi_map_add_entries(fx.hdr);

	pn = port::physmem_log_count();
	if (pn > 64)
		pn = 64;
	for (int i = 0; i < pn; i++) {
		const port::physmem_log_rec *p = port::physmem_log_rec(i);
		port_copy[i].exclude = p->exclude;
		port_copy[i].phys = p->phys;
		port_copy[i].size = p->size;
		port_copy[i].exflag = p->exflag;
	}

	port::physmem_reset();
	oracle_physmem_reset();
	if (exclude_pass)
		ref_efi_map_exclude_entries(
		    reinterpret_cast<struct efi_map_header *>(fx.hdr));
	else
		ref_efi_map_add_entries(
		    reinterpret_cast<struct efi_map_header *>(fx.hdr));

	if (oracle_physmem_log_count() != pn)
		fail_row(row, "physmem", "count mismatch");
	else {
		for (int i = 0; i < pn; i++) {
			const physmem_log_entry *r = oracle_physmem_log_entry(i);
			if (port_copy[i].exclude != r->exclude ||
			    port_copy[i].phys != r->phys ||
			    port_copy[i].size != r->size ||
			    port_copy[i].exflag != r->exflag) {
				fail_row(row, "physmem", "entry mismatch");
				break;
			}
		}
	}

	efi_fixture_fini(&fx);
}

static void
test_efi_print_one(void)
{
	case_row(R_EFI_PRINT);
	efi_fixture fx;
	int port_calls = 0, ref_calls = 0;

	efi_fixture_init(&fx, 3, sizeof(port::efi_md));
	fill_desc(efi_desc(&fx, 0), 7, 0x1000, 4, 0x1 | 0x8);
	fill_desc(efi_desc(&fx, 1), 5, 0x2000, 2, 0x800);
	fill_desc(efi_desc(&fx, 2), 20, 0x3000, 1, 0);

	printf_track_port(&port_calls);
	port::efi_map_print_entries(fx.hdr);
	printf_collect_port(&port_calls);

	printf_track_ref(&ref_calls);
	ref_efi_map_print_entries(reinterpret_cast<struct efi_map_header *>(fx.hdr));
	printf_collect_ref(&ref_calls);

	if (port_calls != ref_calls)
		fail_row(R_EFI_PRINT, "printf", "call count mismatch");

	efi_fixture_fini(&fx);
}

static void
test_efi_hand(void)
{
	test_efi_foreach_one(0, sizeof(port::efi_md), 1);
	test_efi_foreach_one(1, sizeof(port::efi_md), 0);
	test_efi_foreach_one(5, sizeof(port::efi_md), 0);
	test_efi_foreach_one(3, sizeof(port::efi_md) + 8, 0);
	test_efi_physmem_one(0);
	test_efi_physmem_one(1);
	test_efi_print_one();
}

static void
test_efi_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int ndesc = (int)(rnd32() % 16);
		std::size_t desc_size = sizeof(port::efi_md) + (rnd32() % 4) * 8;
		test_efi_foreach_one(ndesc, desc_size, 0);
		if ((rnd32() & 1) != 0)
			test_efi_physmem_one((rnd32() & 1) != 0);
	}
}

/* ----------------------------------------------------------- sema */

struct sema_wait_args {
	port::sema *port_sema;
	struct sema *ref_sema;
	int done;
};

static void *
port_wait_thread(void *arg)
{
	sema_wait_args *a = static_cast<sema_wait_args *>(arg);

	port::_sema_wait(a->port_sema, __FILE__, __LINE__);
	a->done = 1;
	return (nullptr);
}

static void *
ref_wait_thread(void *arg)
{
	sema_wait_args *a = static_cast<sema_wait_args *>(arg);

	ref__sema_wait(a->ref_sema, __FILE__, __LINE__);
	a->done = 1;
	return (nullptr);
}

static void
test_sema_basic_one(int initial, int row_init)
{
	case_row(row_init);
	port::sema ps = {};
	struct sema rs = {};

	port::sema_init(&ps, initial, "port");
	ref_sema_init(&rs, initial, "ref");

	case_row(R_SEMA_VALUE);
	if (port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_VALUE, "init", "value mismatch");

	case_row(R_SEMA_POST);
	port::_sema_post(&ps, __FILE__, __LINE__);
	ref__sema_post(&rs, __FILE__, __LINE__);
	if (port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_POST, "post", "value mismatch");

	case_row(R_SEMA_TRYWAIT);
	int pt = port::_sema_trywait(&ps, __FILE__, __LINE__);
	int rt = ref__sema_trywait(&rs, __FILE__, __LINE__);
	if (pt != rt || port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_TRYWAIT, "trywait", "mismatch");

	case_row(R_SEMA_TIMEDWAIT);
	int pe = port::_sema_timedwait(&ps, 100, __FILE__, __LINE__);
	int re = ref__sema_timedwait(&rs, 100, __FILE__, __LINE__);
	if (pe != re || port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_TIMEDWAIT, "timedwait", "mismatch");

	case_row(R_SEMA_WAIT);
	port::_sema_wait(&ps, __FILE__, __LINE__);
	ref__sema_wait(&rs, __FILE__, __LINE__);
	if (port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_WAIT, "wait", "value mismatch");

	case_row(R_SEMA_DESTROY);
	port::sema_destroy(&ps);
	ref_sema_destroy(&rs);
}

static void
test_sema_blocking_wait(void)
{
	case_row(R_SEMA_WAIT);
	port::sema ps = {};
	struct sema rs = {};
	sema_wait_args pa = { &ps, &rs, 0 };
	pthread_t pt, rt;

	port::sema_init(&ps, 0, "port");
	ref_sema_init(&rs, 0, "ref");

	pthread_create(&pt, nullptr, port_wait_thread, &pa);
	pthread_create(&rt, nullptr, ref_wait_thread, &pa);

	struct timespec ts = { 0, 50000000L };
	nanosleep(&ts, nullptr);

	port::_sema_post(&ps, __FILE__, __LINE__);
	ref__sema_post(&rs, __FILE__, __LINE__);

	pthread_join(pt, nullptr);
	pthread_join(rt, nullptr);

	if (pa.done != 1 || port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_WAIT, "blocking", "wait mismatch");

	port::sema_destroy(&ps);
	ref_sema_destroy(&rs);
}

static void
test_sema_timedwait_timeout(void)
{
	case_row(R_SEMA_TIMEDWAIT);
	port::sema ps = {};
	struct sema rs = {};

	port::sema_init(&ps, 0, "port");
	ref_sema_init(&rs, 0, "ref");

	int pe = port::_sema_timedwait(&ps, 1, __FILE__, __LINE__);
	int re = ref__sema_timedwait(&rs, 1, __FILE__, __LINE__);

	if (pe != re || port::sema_value(&ps) != ref_sema_value(&rs))
		fail_row(R_SEMA_TIMEDWAIT, "timeout", "mismatch");

	port::sema_destroy(&ps);
	ref_sema_destroy(&rs);
}

static void
test_sema_hand(void)
{
	const int vals[] = { 0, 1, 2, 5, 10, 100, 1000 };

	for (int v : vals)
		test_sema_basic_one(v, R_SEMA_INIT);
	test_sema_blocking_wait();
	test_sema_timedwait_timeout();
}

static void
test_sema_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int v = (int)(rnd32() % 20);
		test_sema_basic_one(v, R_SEMA_INIT);
		if ((rnd32() % 5000) == 0)
			test_sema_blocking_wait();
		if ((rnd32() % 5000) == 0)
			test_sema_timedwait_timeout();
	}
}

/* ----------------------------------------------------------- main */

int
main(void)
{
	test_hash_hand();
	test_phash_hand();
	test_getrandom_hand();
	test_efi_hand();
	test_sema_hand();

	test_hash_sweep();
	test_getrandom_sweep();
	test_efi_sweep();
	test_sema_sweep();

	long total_cases = 0;
	long total_fail = 0;

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	for (const auto &r : rows) {
		std::printf("%-24s %12ld %12ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-24s %12ld %12ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
