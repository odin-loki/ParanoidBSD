// Differential test for PBSD batch b0146s2 (subr_efi_map.c).

import pbsd.sys.kern.b0146s2;

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::sys_kern::b0146s2;

#define GUARD 0x7f
#define PAD   32u
#define SWEEP 200000L
#define MAX_PRINT 12

enum : std::uint64_t {
	EFI_MD_ATTR_UC = 0x0000000000000001UL,
	EFI_MD_ATTR_WC = 0x0000000000000002UL,
	EFI_MD_ATTR_WT = 0x0000000000000004UL,
	EFI_MD_ATTR_WB = 0x0000000000000008UL,
	EFI_MD_ATTR_UCE = 0x0000000000000010UL,
	EFI_MD_ATTR_WP = 0x0000000000001000UL,
	EFI_MD_ATTR_RP = 0x0000000000002000UL,
	EFI_MD_ATTR_XP = 0x0000000000004000UL,
	EFI_MD_ATTR_NV = 0x0000000000008000UL,
	EFI_MD_ATTR_MORE_RELIABLE = 0x0000000000010000UL,
	EFI_MD_ATTR_RO = 0x0000000000020000UL,
	EFI_MD_ATTR_RT = 0x8000000000000000UL,
};

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "efi_map_foreach_entry",   0, 0, 0 },
	{ "efi_map_add_entries",     0, 0, 0 },
	{ "efi_map_exclude_entries", 0, 0, 0 },
	{ "efi_map_print_entries",   0, 0, 0 },
};

enum {
	R_EFI_FOREACH = 0,
	R_EFI_ADD,
	R_EFI_EXCLUDE,
	R_EFI_PRINT,
};

/* ------------------------------------------------------------------ */
/* Shared kernel environment (physmem + printf), used by port + oracle */
/* ------------------------------------------------------------------ */

struct physmem_log_entry {
	int exclude;
	std::uint64_t phys;
	std::uint64_t size;
	std::uint32_t exflag;
};

enum : int { PHYSMEM_LOG_MAX = 4096, PRINT_OUT_MAX = 262144 };

struct kenv_state {
	physmem_log_entry physmem[PHYSMEM_LOG_MAX];
	int physmem_count;
	char print_out[PRINT_OUT_MAX];
	std::size_t print_len;
};

static kenv_state g_env;

extern "C" void
pbsd_env_reset(void)
{
	g_env.physmem_count = 0;
	g_env.print_len = 0;
	g_env.print_out[0] = '\0';
}

extern "C" void
pbsd_physmem_hardware_region(std::uint64_t pa, std::uint64_t sz)
{
	if (g_env.physmem_count < PHYSMEM_LOG_MAX) {
		physmem_log_entry *e = &g_env.physmem[g_env.physmem_count];

		e->exclude = 0;
		e->phys = pa;
		e->size = sz;
		e->exflag = 0;
		g_env.physmem_count++;
	}
}

extern "C" void
pbsd_physmem_exclude_region(std::uint64_t pa, std::uint64_t sz,
    std::uint32_t flags)
{
	if (g_env.physmem_count < PHYSMEM_LOG_MAX) {
		physmem_log_entry *e = &g_env.physmem[g_env.physmem_count];

		e->exclude = 1;
		e->phys = pa;
		e->size = sz;
		e->exflag = flags;
		g_env.physmem_count++;
	}
}

extern "C" int
pbsd_kern_printf(const char *fmt, ...)
{
	char tmp[1024];
	va_list ap;
	std::size_t len;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n <= 0)
		return (n);
	len = static_cast<std::size_t>(n);
	if (len > sizeof(tmp) - 1)
		len = sizeof(tmp) - 1;
	if (g_env.print_len + len + 1 < PRINT_OUT_MAX) {
		std::memcpy(g_env.print_out + g_env.print_len, tmp, len);
		g_env.print_len += len;
		g_env.print_out[g_env.print_len] = '\0';
	}
	return (n);
}

static void
kenv_snapshot(kenv_state *snap)
{
	*snap = g_env;
}

static bool
physmem_logs_equal(const kenv_state &a, const kenv_state &b)
{
	if (a.physmem_count != b.physmem_count)
		return (false);
	for (int i = 0; i < a.physmem_count; i++) {
		if (a.physmem[i].exclude != b.physmem[i].exclude ||
		    a.physmem[i].phys != b.physmem[i].phys ||
		    a.physmem[i].size != b.physmem[i].size ||
		    a.physmem[i].exflag != b.physmem[i].exflag)
			return (false);
	}
	return (true);
}

static bool
print_logs_equal(const kenv_state &a, const kenv_state &b)
{
	return (a.print_len == b.print_len &&
	    std::memcmp(a.print_out, b.print_out, a.print_len) == 0);
}

/* ------------------------------------------------------------------ */
/* Oracle declarations                                                */
/* ------------------------------------------------------------------ */

extern "C" {
struct efi_map_header {
	std::uint64_t memory_size;
	std::uint64_t descriptor_size;
	std::uint32_t descriptor_version;
};

struct efi_md {
	std::uint32_t md_type;
	std::uint32_t md_pad;
	std::uint64_t md_phys;
	std::uint64_t md_virt;
	std::uint64_t md_pages;
	std::uint64_t md_attr;
};

typedef void (*efi_map_entry_cb)(struct efi_md *p, void *argp);

void ref_efi_map_foreach_entry(struct efi_map_header *, efi_map_entry_cb,
    void *);
void ref_efi_map_add_entries(struct efi_map_header *);
void ref_efi_map_exclude_entries(struct efi_map_header *);
void ref_efi_map_print_entries(struct efi_map_header *);
}

/* ------------------------------------------------------------------ */
/* Harness helpers                                                    */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x00b0146b2faceULL;

static inline std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline std::uint32_t
rnd32(void)
{
	return (static_cast<std::uint32_t>(rnd64() >> 32));
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-28s %-20s %s\n", r.name, label, detail);
	}
}

static void
case_row(int row)
{
	rows[row].cases++;
}

struct efi_fixture {
	std::uint8_t *blob;
	std::size_t blob_size;
	port::efi_map_header *hdr;
};

static void
efi_fixture_init(efi_fixture *fx, int ndesc, std::size_t desc_size,
    std::uint64_t memory_size_override)
{
	std::size_t efisz = (sizeof(port::efi_map_header) + 0xf) & ~0xf;
	std::size_t map_bytes = ndesc * desc_size;
	std::size_t total = PAD + efisz + map_bytes + PAD;

	fx->blob_size = total;
	fx->blob = static_cast<std::uint8_t *>(std::malloc(total));
	std::memset(fx->blob, GUARD, total);
	fx->hdr = reinterpret_cast<port::efi_map_header *>(fx->blob + PAD);
	std::memset(fx->hdr, 0, efisz + map_bytes);
	fx->hdr->descriptor_size = desc_size;
	if (memory_size_override != 0)
		fx->hdr->memory_size = memory_size_override;
	else
		fx->hdr->memory_size = ndesc * desc_size;
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

static void
fill_desc(port::efi_md *d, std::uint32_t type, std::uint64_t phys,
    std::uint64_t virt, std::uint64_t pages, std::uint64_t attr)
{
	d->md_type = type;
	d->md_pad = 0;
	d->md_phys = phys;
	d->md_virt = virt;
	d->md_pages = pages;
	d->md_attr = attr;
}

struct cb_log {
	const std::uint8_t *blob_base;
	int count;
	std::uint32_t types[128];
	std::uint64_t phys[128];
	std::uint64_t pages[128];
	std::size_t desc_off[128];
};

static void
port_cb(port::efi_md *p, void *arg)
{
	cb_log *log = static_cast<cb_log *>(arg);

	if (log->count < 128) {
		log->types[log->count] = p->md_type;
		log->phys[log->count] = p->md_phys;
		log->pages[log->count] = p->md_pages;
		log->desc_off[log->count] =
		    reinterpret_cast<std::uint8_t *>(p) - log->blob_base;
	}
	log->count++;
}

extern "C" void
ref_cb_c(struct efi_md *p, void *arg)
{
	cb_log *log = static_cast<cb_log *>(arg);

	if (log->count < 128) {
		log->types[log->count] = p->md_type;
		log->phys[log->count] = p->md_phys;
		log->pages[log->count] = p->md_pages;
		log->desc_off[log->count] =
		    reinterpret_cast<std::uint8_t *>(p) - log->blob_base;
	}
	log->count++;
}

static bool
cb_logs_match(const cb_log &a, const cb_log &b)
{
	if (a.count != b.count)
		return (false);
	for (int i = 0; i < a.count && i < 128; i++) {
		if (a.types[i] != b.types[i] || a.phys[i] != b.phys[i] ||
		    a.pages[i] != b.pages[i] || a.desc_off[i] != b.desc_off[i])
			return (false);
	}
	return (true);
}

static bool
blobs_match(const std::uint8_t *a, const std::uint8_t *b, std::size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

/* ------------------------------------------------------------------ */
/* Tests                                                              */
/* ------------------------------------------------------------------ */

static void
test_foreach_one(int ndesc, std::size_t desc_size, int zero_desc,
    std::uint64_t memory_override, const std::uint32_t *types, int ntypes)
{
	case_row(R_EFI_FOREACH);

	efi_fixture pfx, rfx;
	std::uint8_t *pb_copy = nullptr;
	std::uint8_t *rb_copy = nullptr;

	efi_fixture_init(&pfx, ndesc, desc_size, memory_override);
	efi_fixture_init(&rfx, ndesc, desc_size, memory_override);
	if (zero_desc) {
		pfx.hdr->descriptor_size = 0;
		rfx.hdr->descriptor_size = 0;
	}

	for (int i = 0; i < ndesc; i++) {
		std::uint32_t t = types != nullptr ? types[i % ntypes] :
		    static_cast<std::uint32_t>(rnd32() % 20);
		std::uint64_t phys = static_cast<std::uint64_t>(i) * 0x10000ULL +
		    (rnd32() & 0xff);
		std::uint64_t pages = static_cast<std::uint64_t>((i % 7) + 1);
		fill_desc(efi_desc(&pfx, i), t, phys, phys + 0x1000, pages,
		    static_cast<std::uint64_t>(1u << (i % 13)));
		fill_desc(efi_desc(&rfx, i), t, phys, phys + 0x1000, pages,
		    static_cast<std::uint64_t>(1u << (i % 13)));
	}

	pb_copy = static_cast<std::uint8_t *>(std::malloc(pfx.blob_size));
	rb_copy = static_cast<std::uint8_t *>(std::malloc(rfx.blob_size));
	std::memcpy(pb_copy, pfx.blob, pfx.blob_size);
	std::memcpy(rb_copy, rfx.blob, rfx.blob_size);

	cb_log plog = {}, rlog = {};
	plog.blob_base = pfx.blob;
	rlog.blob_base = rfx.blob;

	pbsd_env_reset();
	port::efi_map_foreach_entry(pfx.hdr, port_cb, &plog);
	kenv_state psnap;
	kenv_snapshot(&psnap);

	pbsd_env_reset();
	ref_efi_map_foreach_entry(reinterpret_cast<struct efi_map_header *>(rfx.hdr),
	    ref_cb_c, &rlog);
	kenv_state rsnap;
	kenv_snapshot(&rsnap);

	if (!cb_logs_match(plog, rlog))
		fail_row(R_EFI_FOREACH, "callback", "invocation mismatch");
	if (!physmem_logs_equal(psnap, rsnap))
		fail_row(R_EFI_FOREACH, "physmem", "unexpected side effects");
	if (!blobs_match(pfx.blob, pb_copy, pfx.blob_size))
		fail_row(R_EFI_FOREACH, "blob-port", "memory corruption");
	if (!blobs_match(rfx.blob, rb_copy, rfx.blob_size))
		fail_row(R_EFI_FOREACH, "blob-ref", "memory corruption");

	std::free(pb_copy);
	std::free(rb_copy);
	efi_fixture_fini(&pfx);
	efi_fixture_fini(&rfx);
}

static void
test_physmem_one(int exclude_pass, const std::uint32_t *types, int ndesc,
    std::size_t desc_size)
{
	int row = exclude_pass ? R_EFI_EXCLUDE : R_EFI_ADD;

	case_row(row);

	efi_fixture pfx, rfx;
	efi_fixture_init(&pfx, ndesc, desc_size, 0);
	efi_fixture_init(&rfx, ndesc, desc_size, 0);

	for (int i = 0; i < ndesc; i++) {
		std::uint32_t t = types[i];
		std::uint64_t pages = static_cast<std::uint64_t>((i % 5) + 1);
		fill_desc(efi_desc(&pfx, i), t, static_cast<std::uint64_t>(0x10000 * i),
		    0, pages, 0);
		fill_desc(efi_desc(&rfx, i), t, static_cast<std::uint64_t>(0x10000 * i),
		    0, pages, 0);
	}

	pbsd_env_reset();
	if (exclude_pass)
		port::efi_map_exclude_entries(pfx.hdr);
	else
		port::efi_map_add_entries(pfx.hdr);
	kenv_state psnap;
	kenv_snapshot(&psnap);

	pbsd_env_reset();
	if (exclude_pass)
		ref_efi_map_exclude_entries(
		    reinterpret_cast<struct efi_map_header *>(rfx.hdr));
	else
		ref_efi_map_add_entries(
		    reinterpret_cast<struct efi_map_header *>(rfx.hdr));
	kenv_state rsnap;
	kenv_snapshot(&rsnap);

	if (!physmem_logs_equal(psnap, rsnap))
		fail_row(row, "physmem", "log mismatch");

	efi_fixture_fini(&pfx);
	efi_fixture_fini(&rfx);
}

static void
test_print_one(int ndesc, std::size_t desc_size)
{
	case_row(R_EFI_PRINT);

	efi_fixture pfx, rfx;
	efi_fixture_init(&pfx, ndesc, desc_size, 0);
	efi_fixture_init(&rfx, ndesc, desc_size, 0);

	for (int i = 0; i < ndesc; i++) {
		std::uint32_t t = static_cast<std::uint32_t>(i % 16);
		std::uint64_t attr = EFI_MD_ATTR_UC | EFI_MD_ATTR_WB;
		if (i == 1)
			attr = EFI_MD_ATTR_RT | EFI_MD_ATTR_WP | EFI_MD_ATTR_RO;
		if (i == 2)
			attr = EFI_MD_ATTR_WC | EFI_MD_ATTR_WT | EFI_MD_ATTR_UCE |
			    EFI_MD_ATTR_RP | EFI_MD_ATTR_XP | EFI_MD_ATTR_NV |
			    EFI_MD_ATTR_MORE_RELIABLE;
		fill_desc(efi_desc(&pfx, i), t, 0x1000ULL * (i + 1),
		    0x2000ULL * (i + 1), static_cast<std::uint64_t>(i + 1), attr);
		fill_desc(efi_desc(&rfx, i), t, 0x1000ULL * (i + 1),
		    0x2000ULL * (i + 1), static_cast<std::uint64_t>(i + 1), attr);
	}

	pbsd_env_reset();
	port::efi_map_print_entries(pfx.hdr);
	kenv_state psnap;
	kenv_snapshot(&psnap);

	pbsd_env_reset();
	ref_efi_map_print_entries(reinterpret_cast<struct efi_map_header *>(rfx.hdr));
	kenv_state rsnap;
	kenv_snapshot(&rsnap);

	if (!print_logs_equal(psnap, rsnap))
		fail_row(R_EFI_PRINT, "printf", "output mismatch");

	efi_fixture_fini(&pfx);
	efi_fixture_fini(&rfx);
}

static void
test_hand(void)
{
	const std::uint32_t all_types[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 20, 99
	};
	const std::uint32_t phys_types[] = {
		0, 1, 3, 4, 5, 6, 7, 9, 14, 99
	};

	test_foreach_one(0, sizeof(port::efi_md), 1, 0, nullptr, 0);
	test_foreach_one(0, sizeof(port::efi_md), 0, 0, nullptr, 0);
	test_foreach_one(1, sizeof(port::efi_md), 0, 0, all_types, 1);
	test_foreach_one(5, sizeof(port::efi_md), 0, 0, all_types, 18);
	test_foreach_one(3, sizeof(port::efi_md) + 8, 0, 0, all_types, 18);
	test_foreach_one(4, sizeof(port::efi_md), 0,
	    sizeof(port::efi_md) * 3 + 1, all_types, 18);

	for (int i = 0; i < 18; i++) {
		std::uint32_t t[] = { all_types[i] };
		test_physmem_one(0, t, 1, sizeof(port::efi_md));
		test_physmem_one(1, t, 1, sizeof(port::efi_md));
	}

	test_physmem_one(0, phys_types, 10, sizeof(port::efi_md));
	test_physmem_one(1, phys_types, 10, sizeof(port::efi_md));

	test_print_one(0, sizeof(port::efi_md));
	test_print_one(1, sizeof(port::efi_md));
	test_print_one(4, sizeof(port::efi_md));
	test_print_one(3, sizeof(port::efi_md) + 16);
}

static void
test_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int ndesc = static_cast<int>(rnd32() % 24);
		std::size_t desc_size = sizeof(port::efi_md) +
		    (rnd32() % 5) * 8;
		int zero_desc = (rnd32() % 17) == 0 ? 1 : 0;
		std::uint64_t mem_override = 0;

		if ((rnd32() % 23) == 0 && ndesc > 0)
			mem_override = static_cast<std::uint64_t>(ndesc) *
			    desc_size - (rnd32() % desc_size);

		test_foreach_one(ndesc, desc_size, zero_desc, mem_override,
		    nullptr, 0);

		if ((rnd32() & 1) != 0) {
			std::uint32_t types[24];
			int n = ndesc > 0 ? ndesc : 1;
			for (int j = 0; j < n; j++)
				types[j] = rnd32() % 20;
			test_physmem_one((rnd32() & 1) != 0, types, ndesc > 0 ? ndesc : 1,
			    desc_size);
		}

		if ((rnd32() % 50) == 0)
			test_print_one(ndesc > 0 ? ndesc : 1, desc_size);
	}
}

int
main(void)
{
	test_hand();
	test_sweep();

	long total_cases = 0;
	long total_fail = 0;

	std::printf("\n%-28s %12s %12s\n", "function", "cases", "failures");
	for (const auto &r : rows) {
		std::printf("%-28s %12ld %12ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-28s %12ld %12ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
