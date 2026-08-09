/*
 * Differential test for PBSD batch b0146s2 (sys/kern/subr_efi_map.c).
 *
 * Every function of the batch is driven twice - once through the C oracle
 * (ref_*) and once through the C++23 port - from byte-identical input buffers.
 * These functions all return void and communicate only through
 *
 *	(a) the two physmem region entry points,
 *	(b) the kernel printf, and
 *	(c) the descriptor pointers handed to the iteration callback,
 *
 * so all three are captured into a trace string which is compared verbatim.
 * Descriptor pointers are recorded as OFFSETS from each side's own buffer
 * base, never as addresses.  The two input buffers are pre-filled with the
 * guard byte 0x7f and, after every case, compared against each other and
 * against a pristine copy over their ENTIRE length, so a port that scribbles
 * anywhere in or around the memory map is caught even though the return values
 * (void) could never show it.
 */
#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

import pbsd.sys.kern.b0146s2;

namespace P = pbsd::sys_kern::b0146s2;

/* ------------------------------------------------------------------ */
/* The oracle's view of the world (mirrors oracle.c).                 */

extern "C" {

struct efi_map_header {
	uint64_t	memory_size;
	uint64_t	descriptor_size;
	uint32_t	descriptor_version;
};

struct efi_md {
	uint32_t	md_type;
	uint32_t	md_pad;
	uint64_t	md_phys;
	uint64_t	md_virt;
	uint64_t	md_pages;
	uint64_t	md_attr;
};

typedef void (*efi_map_entry_cb)(struct efi_md *, void *);

void ref_efi_map_foreach_entry(struct efi_map_header *efihdr,
    efi_map_entry_cb cb, void *argp);
void ref_handle_efi_map_entry(struct efi_md *p, void *argp);
void ref_efi_map_add_entries(struct efi_map_header *efihdr);
void ref_efi_map_exclude_entries(struct efi_map_header *efihdr);
void ref_print_efi_map_entry(struct efi_md *p, void *argp);
void ref_efi_map_print_entries(struct efi_map_header *efihdr);

}

static_assert(sizeof(void *) == 8, "this harness assumes LP64");
static_assert(sizeof(struct efi_map_header) == 24);
static_assert(sizeof(struct efi_md) == 40);
static_assert(sizeof(struct efi_md) == sizeof(P::efi_md));
static_assert(sizeof(struct efi_map_header) == sizeof(P::efi_map_header));
static_assert(offsetof(struct efi_md, md_type) == offsetof(P::efi_md, md_type));
static_assert(offsetof(struct efi_md, md_phys) == offsetof(P::efi_md, md_phys));
static_assert(offsetof(struct efi_md, md_virt) == offsetof(P::efi_md, md_virt));
static_assert(offsetof(struct efi_md, md_pages) ==
    offsetof(P::efi_md, md_pages));
static_assert(offsetof(struct efi_md, md_attr) == offsetof(P::efi_md, md_attr));
static_assert(offsetof(struct efi_map_header, memory_size) ==
    offsetof(P::efi_map_header, memory_size));
static_assert(offsetof(struct efi_map_header, descriptor_size) ==
    offsetof(P::efi_map_header, descriptor_size));
static_assert(offsetof(struct efi_map_header, descriptor_version) ==
    offsetof(P::efi_map_header, descriptor_version));

/* ------------------------------------------------------------------ */
/* Observation sinks, shared by the oracle and the port.              */

static std::string g_ref, g_port;
static std::string *g_out = &g_ref;
static long g_events;
static std::string g_case;
static const char *g_side = "?";

/*
 * A mutated port can turn the descriptor walk into a runaway loop (i-- for
 * i++, or != for <).  Die with a failing status instead of hanging: the most
 * elaborate legitimate case in this file produces a few hundred events.
 */
static const long EVENT_LIMIT = 100000;

static void
bump(const char *what)
{

	if (++g_events > EVENT_LIMIT) {
		std::fprintf(stderr, "FATAL: over %ld %s events in one call - "
		    "runaway loop in the %s\n    case: %s\n", EVENT_LIMIT, what,
		    g_side, g_case.c_str());
		std::fflush(stderr);
		std::exit(1);
	}
}

static void
sink_hardware_region(uint64_t pa, uint64_t sz)
{
	char t[80];

	bump("physmem");
	std::snprintf(t, sizeof(t), "H %016llx %016llx\n",
	    (unsigned long long)pa, (unsigned long long)sz);
	*g_out += t;
}

static void
sink_exclude_region(uint64_t pa, uint64_t sz, uint32_t flags)
{
	char t[96];

	bump("physmem");
	std::snprintf(t, sizeof(t), "E %016llx %016llx %08lx\n",
	    (unsigned long long)pa, (unsigned long long)sz,
	    (unsigned long)flags);
	*g_out += t;
}

static int
sink_printf(const char *fmt, va_list ap)
{
	char t[512];
	int n;

	bump("printf");
	n = std::vsnprintf(t, sizeof(t), fmt, ap);
	*g_out += "P|";
	*g_out += t;
	*g_out += '\001';
	return (n);
}

extern "C" void
pbsd_physmem_hardware_region(uint64_t pa, uint64_t sz)
{

	sink_hardware_region(pa, sz);
}

extern "C" void
pbsd_physmem_exclude_region(uint64_t pa, uint64_t sz, uint32_t flags)
{

	sink_exclude_region(pa, sz, flags);
}

extern "C" int
pbsd_kern_printf(const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = sink_printf(fmt, ap);
	va_end(ap);
	return (n);
}

/* The same three services under their unprefixed kernel names. */
extern "C" void
physmem_hardware_region(uint64_t pa, uint64_t sz)
{

	sink_hardware_region(pa, sz);
}

extern "C" void
physmem_exclude_region(uint64_t pa, uint64_t sz, uint32_t flags)
{

	sink_exclude_region(pa, sz, flags);
}

extern "C" int
efi_test_printf(const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = sink_printf(fmt, ap);
	va_end(ap);
	return (n);
}

/* ------------------------------------------------------------------ */
/* Iteration callback: records offsets, never dereferences p.         */

static const void *g_cb_base;
static const void *g_cb_argp;

static uint64_t
off_of(const void *p, const void *base)
{

	/* Unsigned wrap-around keeps out-of-range pointers well defined. */
	return ((uint64_t)(uintptr_t)p - (uint64_t)(uintptr_t)base);
}

static void
rec_common(const void *p, void *argp)
{
	char t[80];

	bump("callback");
	std::snprintf(t, sizeof(t), "C %016llx %d\n",
	    (unsigned long long)off_of(p, g_cb_base), argp == g_cb_argp);
	*g_out += t;
}

static void
rec_cb_ref(struct efi_md *p, void *argp)
{

	rec_common(p, argp);
}

static void
rec_cb_port(P::efi_md *p, void *argp)
{

	rec_common(p, argp);
}

/* ------------------------------------------------------------------ */
/* Bookkeeping.                                                       */

struct Stats {
	const char *name;
	long cases;
	long fails;
};

static Stats st_foreach = { "efi_map_foreach_entry", 0, 0 };
static Stats st_handle = { "handle_efi_map_entry", 0, 0 };
static Stats st_add = { "efi_map_add_entries", 0, 0 };
static Stats st_excl = { "efi_map_exclude_entries", 0, 0 };
static Stats st_print1 = { "print_efi_map_entry", 0, 0 };
static Stats st_printall = { "efi_map_print_entries", 0, 0 };
static Stats st_next = { "efi_next_descriptor", 0, 0 };

static int g_reported;

static void
fail(Stats &s, const std::string &what)
{

	s.fails++;
	if (g_reported++ < 25)
		std::fprintf(stderr, "FAIL [%s] %s\n", s.name, what.c_str());
}

/* ------------------------------------------------------------------ */
/* Test buffers.                                                      */

static const size_t BUFSZ = 4096;
static const size_t HDR_OFF = 256;		/* headroom for underflow */
static const size_t MAP_OFF = HDR_OFF + 32;	/* where efisz lands */
static const uint8_t GUARD = 0x7f;

struct Desc {
	uint32_t md_type;
	uint32_t pad;
	uint64_t phys;
	uint64_t virt;
	uint64_t pages;
	uint64_t attr;
};

struct MapSpec {
	uint64_t memory_size;
	uint64_t descriptor_size;
	uint32_t version;
	uint64_t stride;		/* descriptor placement stride */
	std::vector<Desc> descs;
};

static void
put64(std::vector<uint8_t> &b, size_t off, uint64_t v)
{

	std::memcpy(&b[off], &v, 8);
}

static void
put32(std::vector<uint8_t> &b, size_t off, uint32_t v)
{

	std::memcpy(&b[off], &v, 4);
}

static void
put_desc(std::vector<uint8_t> &b, size_t off, const Desc &d)
{

	put32(b, off + 0, d.md_type);
	put32(b, off + 4, d.pad);
	put64(b, off + 8, d.phys);
	put64(b, off + 16, d.virt);
	put64(b, off + 24, d.pages);
	put64(b, off + 32, d.attr);
}

/*
 * Trap descriptors sit immediately before the first and after the last real
 * descriptor.  They carry types that produce observable physmem traffic, so a
 * walk that starts one entry early, or runs one entry long, diverges even when
 * the real descriptors are exhausted.
 */
static Desc
trap_desc(uint64_t k)
{
	Desc d;

	d.md_type = (k & 1) ? 7u /* FREE */ : 6u /* RT_DATA */;
	d.pad = 0x7a7a7a7au;
	d.phys = 0x7ade0000ULL + k;
	d.virt = 0x7ade8000ULL + k;
	d.pages = 5 + k;
	d.attr = 0x0000000000021015ULL;
	return (d);
}

static void
build(std::vector<uint8_t> &b, const MapSpec &s)
{
	uint64_t stride;
	size_t n, i;

	std::fill(b.begin(), b.end(), GUARD);

	stride = s.stride;
	n = s.descs.size();
	if (stride != 0 && stride <= 512) {
		if (MAP_OFF >= stride + 40)
			put_desc(b, (size_t)(MAP_OFF - stride), trap_desc(0));
		for (i = 0; i < n; i++) {
			size_t off = (size_t)(MAP_OFF + i * stride);

			if (off + 40 > BUFSZ)
				std::abort();
			put_desc(b, off, s.descs[i]);
		}
		for (i = n; i < n + 2; i++) {
			size_t off = (size_t)(MAP_OFF + i * stride);

			if (off + 40 <= BUFSZ)
				put_desc(b, off, trap_desc(i + 1));
		}
	}

	/*
	 * The header goes in last: a trap descriptor placed before the map can
	 * reach into it, and the header must be exactly as the case specifies.
	 */
	put64(b, HDR_OFF + 0, s.memory_size);
	put64(b, HDR_OFF + 8, s.descriptor_size);
	put32(b, HDR_OFF + 16, s.version);
}

static std::vector<uint8_t> bufA(BUFSZ), bufB(BUFSZ), bufP(BUFSZ);

static std::string
describe(const MapSpec &s)
{
	char t[160];

	std::snprintf(t, sizeof(t), "memory_size=%llu descriptor_size=%llu "
	    "stride=%llu descs=%zu", (unsigned long long)s.memory_size,
	    (unsigned long long)s.descriptor_size,
	    (unsigned long long)s.stride, s.descs.size());
	return (std::string(t));
}

static void
compare(Stats &s, const std::string &desc)
{

	s.cases++;
	if (g_ref != g_port) {
		fail(s, desc + "\n    ref  trace: " + g_ref +
		    "\n    port trace: " + g_port);
		return;
	}
	if (bufA != bufB) {
		fail(s, desc + "\n    the two input buffers differ afterwards");
		return;
	}
	if (bufA != bufP) {
		fail(s, desc + "\n    the oracle modified its input buffer");
		return;
	}
	if (bufB != bufP)
		fail(s, desc + "\n    the port modified its input buffer");
}

enum which { W_ADD, W_EXCL, W_PRINT, W_FOREACH };

static void
run_map_case(Stats &s, const MapSpec &spec, enum which w)
{
	struct efi_map_header *ha;
	P::efi_map_header *hb;
	int sentinel = 0;

	build(bufA, spec);
	bufP = bufA;
	bufB = bufA;
	ha = (struct efi_map_header *)(bufA.data() + HDR_OFF);
	hb = (P::efi_map_header *)(bufB.data() + HDR_OFF);
	g_case = describe(spec);

	g_side = "oracle";
	g_ref.clear();
	g_out = &g_ref;
	g_events = 0;
	g_cb_base = bufA.data();
	g_cb_argp = &sentinel;
	switch (w) {
	case W_ADD:
		ref_efi_map_add_entries(ha);
		break;
	case W_EXCL:
		ref_efi_map_exclude_entries(ha);
		break;
	case W_PRINT:
		ref_efi_map_print_entries(ha);
		break;
	case W_FOREACH:
		ref_efi_map_foreach_entry(ha, rec_cb_ref, &sentinel);
		break;
	}

	g_side = "port";
	g_port.clear();
	g_out = &g_port;
	g_events = 0;
	g_cb_base = bufB.data();
	switch (w) {
	case W_ADD:
		P::efi_map_add_entries(hb);
		break;
	case W_EXCL:
		P::efi_map_exclude_entries(hb);
		break;
	case W_PRINT:
		P::efi_map_print_entries(hb);
		break;
	case W_FOREACH:
		P::efi_map_foreach_entry(hb, rec_cb_port, &sentinel);
		break;
	}

	compare(s, g_case);
}

static void
run_entry_case(Stats &s, const Desc &d, bool is_print, bool exclude)
{
	bool exa = exclude, exb = exclude;
	char t[200];

	std::fill(bufA.begin(), bufA.end(), GUARD);
	put_desc(bufA, MAP_OFF, d);
	bufP = bufA;
	bufB = bufA;

	std::snprintf(t, sizeof(t), "type=%lu phys=%llx virt=%llx pages=%llx "
	    "attr=%llx exclude=%d", (unsigned long)d.md_type,
	    (unsigned long long)d.phys, (unsigned long long)d.virt,
	    (unsigned long long)d.pages, (unsigned long long)d.attr,
	    (int)exclude);
	g_case = t;

	g_side = "oracle";
	g_ref.clear();
	g_out = &g_ref;
	g_events = 0;
	if (is_print)
		ref_print_efi_map_entry((struct efi_md *)(bufA.data() +
		    MAP_OFF), NULL);
	else
		ref_handle_efi_map_entry((struct efi_md *)(bufA.data() +
		    MAP_OFF), &exa);

	g_side = "port";
	g_port.clear();
	g_out = &g_port;
	g_events = 0;
	if (is_print)
		P::print_efi_map_entry((P::efi_md *)(bufB.data() + MAP_OFF),
		    NULL);
	else
		P::handle_efi_map_entry((P::efi_md *)(bufB.data() + MAP_OFF),
		    &exb);

	compare(s, g_case);
	if (exa != exb)
		fail(s, g_case + "\n    the exclude flag was modified");
}

/*
 * efi_next_descriptor() comes from machine/efi.h rather than from the batch
 * source, so the oracle keeps it file-static and there is no ref_ symbol to
 * call.  Check the port's copy against the arithmetic it is specified to
 * perform; the stride it produces is compared against the oracle indirectly,
 * on every efi_map_foreach_entry case above.
 */
static void
run_next_case(size_t off, uint64_t size)
{
	char t[128];
	uint64_t want, got;

	want = (uint64_t)off + size;
	got = off_of(P::efi_next_descriptor((P::efi_md *)(bufB.data() + off),
	    (size_t)size), bufB.data());
	st_next.cases++;
	if (want != got) {
		std::snprintf(t, sizeof(t), "off=%zu size=%llu: want=+%llx "
		    "got=+%llx", off, (unsigned long long)size,
		    (unsigned long long)want, (unsigned long long)got);
		fail(st_next, std::string(t));
	}
}

/* ------------------------------------------------------------------ */
/* Fixed-seed PRNG.                                                   */

static uint64_t rs = 0x243f6a8885a308d3ULL;

static uint64_t
rnd(void)
{

	rs ^= rs << 13;
	rs ^= rs >> 7;
	rs ^= rs << 17;
	return (rs);
}

static uint64_t
rnd_mod(uint64_t n)
{

	return (rnd() % n);
}

static const uint64_t attr_bits[] = {
	0x0000000000000001ULL,	/* UC */
	0x0000000000000002ULL,	/* WC */
	0x0000000000000004ULL,	/* WT */
	0x0000000000000008ULL,	/* WB */
	0x0000000000000010ULL,	/* UCE */
	0x0000000000001000ULL,	/* WP */
	0x0000000000002000ULL,	/* RP */
	0x0000000000004000ULL,	/* XP */
	0x0000000000008000ULL,	/* NV */
	0x0000000000010000ULL,	/* MORE_RELIABLE */
	0x0000000000020000ULL,	/* RO */
	0x8000000000000000ULL,	/* RT */
};

static const uint64_t page_edges[] = {
	0, 1, 2, 3, 0xfff, 0x1000,
	0x000fffffffffffffULL,	/* * EFI_PAGE_SIZE still fits */
	0x0010000000000000ULL,	/* * EFI_PAGE_SIZE wraps to zero */
	0x0010000000000001ULL,
	0xffffffffffffffffULL,
};

static const uint64_t phys_edges[] = {
	0, 1, 0xfffULL, 0x1000ULL, 0x123456789abcdef0ULL,
	0x7fffffffffffffffULL, 0x8000000000000000ULL, 0xffffffffffffffffULL,
};

#define	NELEM(a)	(sizeof(a) / sizeof((a)[0]))

static uint64_t
rnd_pages(void)
{

	switch (rnd_mod(4)) {
	case 0:
		return (rnd_mod(8));
	case 1:
		return (page_edges[rnd_mod(NELEM(page_edges))]);
	case 2:
		return (rnd() >> rnd_mod(64));
	default:
		return (rnd());
	}
}

static uint64_t
rnd_phys(void)
{

	switch (rnd_mod(3)) {
	case 0:
		return (phys_edges[rnd_mod(NELEM(phys_edges))]);
	case 1:
		return (rnd() & 0xfffffffffffff000ULL);
	default:
		return (rnd());
	}
}

static uint64_t
rnd_attr(void)
{
	uint64_t a;
	int i;

	switch (rnd_mod(5)) {
	case 0:
		return (0);
	case 1:
		return (attr_bits[rnd_mod(NELEM(attr_bits))]);
	case 2:
		return (0xffffffffffffffffULL);
	case 3:
		a = 0;
		for (i = 0; i < (int)NELEM(attr_bits); i++)
			if (rnd_mod(2))
				a |= attr_bits[i];
		return (a);
	default:
		return (rnd());
	}
}

static uint32_t
rnd_type(void)
{

	switch (rnd_mod(4)) {
	case 0:
		return ((uint32_t)rnd_mod(10));	/* the handled types */
	case 1:
		return ((uint32_t)rnd_mod(20));	/* around nitems(types) */
	case 2:
		return ((uint32_t)rnd_mod(256));
	default:
		return ((uint32_t)rnd());
	}
}

static Desc
rnd_desc(void)
{
	Desc d;

	d.md_type = rnd_type();
	d.pad = (uint32_t)rnd();
	d.phys = rnd_phys();
	d.virt = rnd_phys();
	d.pages = rnd_pages();
	d.attr = rnd_attr();
	return (d);
}

/* ------------------------------------------------------------------ */

static const long SWEEP = 200000;

static MapSpec
mk(uint64_t msz, uint64_t dsz, uint64_t stride, size_t ndesc)
{
	MapSpec m;
	size_t k;

	m.memory_size = msz;
	m.descriptor_size = dsz;
	m.version = 1;
	m.stride = stride;
	for (k = 0; k < ndesc; k++) {
		Desc d;

		d.md_type = (uint32_t)(k % 18);
		d.pad = 0xa5a5a5a5u;
		d.phys = 0x100000ULL * (k + 1);
		d.virt = 0x200000ULL * (k + 1);
		d.pages = k + 1;
		d.attr = attr_bits[k % NELEM(attr_bits)] |
		    ((k & 1) ? 0x8000000000000000ULL : 0);
		m.descs.push_back(d);
	}
	return (m);
}

/*
 * Hand-written maps.  Descriptor sizes deliberately include values smaller
 * than sizeof(struct efi_md) - overlapping descriptors, which the original
 * walks quite happily - and memory sizes that put memory_size /
 * descriptor_size on either side of every integer boundary.
 */
static std::vector<MapSpec>
hand_maps(void)
{
	static const uint32_t types[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	    11, 12, 13, 14, 15, 16, 0xffffffffu };
	std::vector<MapSpec> v;
	MapSpec s;
	size_t i;

	/* descriptor_size == 0: the early return, with and without memory. */
	v.push_back(mk(0, 0, 48, 0));
	v.push_back(mk(48, 0, 48, 0));
	v.push_back(mk(4096, 0, 48, 0));
	v.push_back(mk(0xffffffffffffffffULL, 0, 48, 0));

	/* Both sides of the ndesc == 0, 1, 2 boundaries. */
	v.push_back(mk(0, 48, 48, 2));
	v.push_back(mk(1, 48, 48, 2));
	v.push_back(mk(47, 48, 48, 2));
	v.push_back(mk(48, 48, 48, 2));
	v.push_back(mk(49, 48, 48, 2));
	v.push_back(mk(95, 48, 48, 2));
	v.push_back(mk(96, 48, 48, 3));
	v.push_back(mk(97, 48, 48, 3));

	/* Strides below sizeof(struct efi_md). */
	v.push_back(mk(1, 1, 1, 1));
	v.push_back(mk(8, 1, 1, 8));
	v.push_back(mk(64, 8, 8, 8));
	v.push_back(mk(41, 40, 40, 1));
	v.push_back(mk(120, 40, 40, 3));

	/* The usual UEFI descriptor sizes. */
	v.push_back(mk(40 * 8, 40, 40, 8));
	v.push_back(mk(48 * 16, 48, 48, 16));
	v.push_back(mk(48 * 32, 48, 48, 32));
	v.push_back(mk(128 * 4, 128, 128, 4));
	v.push_back(mk(256 * 2, 256, 256, 2));

	/* uint64_t -> int truncation of ndesc. */
	v.push_back(mk(0x100000000ULL * 48, 48, 48, 1));	/* -> 0 */
	v.push_back(mk(0x100000000ULL * 48 + 48, 48, 48, 1));	/* -> 1 */
	v.push_back(mk(0x80000000ULL * 48, 48, 48, 1));		/* -> INT_MIN */
	v.push_back(mk(0xffffffffffffffffULL, 0xffffffffffffffffULL, 48, 1));
	v.push_back(mk(0xffffffffffffffffULL, 0x8000000000000000ULL, 48, 1));

	/* One map per md_type, so every switch label is walked. */
	for (i = 0; i < NELEM(types); i++) {
		s = mk(48 * 3, 48, 48, 3);
		s.descs[0].md_type = types[i];
		s.descs[1].md_type = types[i];
		s.descs[2].md_type = types[i];
		s.descs[0].pages = 0;
		s.descs[1].pages = 1;
		s.descs[2].pages = 0x0010000000000000ULL;
		v.push_back(s);
	}

	/* Every attribute bit, alone and inverted, plus none and all. */
	for (i = 0; i < NELEM(attr_bits); i++) {
		s = mk(48 * 2, 48, 48, 2);
		s.descs[0].attr = attr_bits[i];
		s.descs[1].attr = ~attr_bits[i];
		v.push_back(s);
	}
	s = mk(48 * 2, 48, 48, 2);
	s.descs[0].attr = 0;
	s.descs[1].attr = 0xffffffffffffffffULL;
	v.push_back(s);

	return (v);
}

static MapSpec
rnd_map(void)
{
	MapSpec m;
	uint64_t dsz, stride, maxdesc, base;
	size_t n, k;

	switch (rnd_mod(8)) {
	case 0:
		dsz = 0;
		break;
	case 1:
		dsz = 1 + rnd_mod(8);
		break;
	case 2:
		dsz = 40;
		break;
	case 3:
		dsz = 48;
		break;
	default:
		dsz = 1 + rnd_mod(200);
		break;
	}
	m.descriptor_size = dsz;
	stride = dsz == 0 ? 48 : dsz;
	m.stride = stride;
	m.version = (uint32_t)rnd();

	/* Keep every pointer the walk can produce inside the buffer. */
	maxdesc = (BUFSZ - MAP_OFF - 128) / stride;
	if (maxdesc > 8)
		maxdesc = 8;
	if (maxdesc < 2)
		maxdesc = 2;
	n = (size_t)rnd_mod(maxdesc);
	for (k = 0; k < n; k++)
		m.descs.push_back(rnd_desc());

	if (dsz == 0) {
		m.memory_size = rnd();
		return (m);
	}

	/*
	 * memory_size straddles n * descriptor_size so that the floor of the
	 * division lands on both sides of the loop bound.
	 */
	base = (uint64_t)n * dsz;
	switch (rnd_mod(6)) {
	case 0:
		m.memory_size = base;
		break;
	case 1:
		m.memory_size = base ? base - 1 : 0;
		break;
	case 2:
		m.memory_size = base + 1;
		break;
	case 3:
		m.memory_size = base + dsz - 1;
		break;
	case 4:
		m.memory_size = base + dsz;
		break;
	default:
		m.memory_size = rnd_mod((maxdesc + 2) * dsz + 1);
		break;
	}
	return (m);
}

int
main(void)
{
	static const uint32_t types[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	    12, 13, 14, 15, 16, 17, 20, 255, 0x7f7f7f7fu, 0x80000000u,
	    0xfffffffeu, 0xffffffffu };
	std::vector<MapSpec> maps;
	Desc d;
	long i;
	size_t k, j;

	g_ref.reserve(1 << 16);
	g_port.reserve(1 << 16);
	std::fill(bufA.begin(), bufA.end(), GUARD);
	std::fill(bufB.begin(), bufB.end(), GUARD);

	/* --- efi_next_descriptor --------------------------------------- */
	{
		static const uint64_t sizes[] = { 0, 1, 2, 4, 8, 39, 40, 41,
		    47, 48, 64, 128, 255, 256, 1024, 2048 };
		static const size_t offs[] = { 0, 1, 8, 40, MAP_OFF, 1024,
		    2048, BUFSZ - 40 };

		for (k = 0; k < NELEM(offs); k++)
			for (j = 0; j < NELEM(sizes); j++)
				run_next_case(offs[k], sizes[j]);
		for (i = 0; i < SWEEP; i++)
			run_next_case((size_t)rnd_mod(BUFSZ - 40),
			    rnd_mod(BUFSZ));
	}

	/* --- handle_efi_map_entry -------------------------------------- */
	d.pad = 0x5a5a5a5au;
	d.virt = 0xdeadbeefcafeULL;
	d.attr = 0;
	for (k = 0; k < NELEM(types); k++) {
		d.md_type = types[k];
		for (j = 0; j < NELEM(page_edges); j++) {
			size_t q;

			d.pages = page_edges[j];
			for (q = 0; q < NELEM(phys_edges); q++) {
				d.phys = phys_edges[q];
				run_entry_case(st_handle, d, false, false);
				run_entry_case(st_handle, d, false, true);
			}
		}
	}
	for (i = 0; i < SWEEP; i++) {
		d = rnd_desc();
		run_entry_case(st_handle, d, false, rnd_mod(2) != 0);
	}

	/* --- print_efi_map_entry --------------------------------------- */
	d.phys = 0x123456789abcULL;
	d.virt = 0xfedcba987654ULL;
	for (k = 0; k < NELEM(types); k++) {
		d.md_type = types[k];
		for (j = 0; j < 64; j++) {
			d.attr = 1ULL << j;
			d.pages = page_edges[j % NELEM(page_edges)];
			run_entry_case(st_print1, d, true, false);
		}
		d.pages = 0x1234;
		d.attr = 0;
		run_entry_case(st_print1, d, true, false);
		d.attr = 0xffffffffffffffffULL;
		run_entry_case(st_print1, d, true, false);
		d.attr = 0x000000000003f01fULL;	/* every named bit but RT */
		run_entry_case(st_print1, d, true, false);
		d.attr = 0x800000000003f01fULL;	/* every named bit */
		run_entry_case(st_print1, d, true, false);
	}
	for (i = 0; i < SWEEP; i++) {
		d = rnd_desc();
		run_entry_case(st_print1, d, true, false);
	}

	/* --- the map-level entry points -------------------------------- */
	maps = hand_maps();
	for (k = 0; k < maps.size(); k++) {
		run_map_case(st_foreach, maps[k], W_FOREACH);
		run_map_case(st_add, maps[k], W_ADD);
		run_map_case(st_excl, maps[k], W_EXCL);
		run_map_case(st_printall, maps[k], W_PRINT);
	}
	for (i = 0; i < SWEEP; i++) {
		MapSpec m = rnd_map();

		run_map_case(st_foreach, m, W_FOREACH);
		run_map_case(st_add, m, W_ADD);
		run_map_case(st_excl, m, W_EXCL);
		run_map_case(st_printall, m, W_PRINT);
	}

	/* --- the table ------------------------------------------------- */
	{
		Stats *all[] = { &st_foreach, &st_handle, &st_add, &st_excl,
		    &st_print1, &st_printall, &st_next };
		long tc = 0, tf = 0;

		std::printf("\n%-26s %12s %12s  %s\n", "function", "cases",
		    "failures", "result");
		std::printf("%-26s %12s %12s  %s\n",
		    "--------------------------", "------------",
		    "------------", "------");
		for (k = 0; k < NELEM(all); k++) {
			std::printf("%-26s %12ld %12ld  %s\n", all[k]->name,
			    all[k]->cases, all[k]->fails,
			    all[k]->fails == 0 ? "PASS" : "FAIL");
			tc += all[k]->cases;
			tf += all[k]->fails;
		}
		std::printf("%-26s %12ld %12ld  %s\n", "TOTAL", tc, tf,
		    tf == 0 ? "PASS" : "FAIL");
		if (tf != 0)
			std::printf("\nb0146s2: %ld of %ld cases diverged\n",
			    tf, tc);
		return (tf == 0 ? 0 : 1);
	}
}
