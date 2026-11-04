/*
 * Differential harness for batch b0248 (__ttyslot, freebsd11_devname,
 * freebsd11_devname_r).  Every case drives both the C++23 port and the C
 * oracle; return values, pointer offsets, and full guard buffers are compared.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>

import pbsd.lib.libc.gen.b0248;

namespace P = pbsd::lib_libc_gen::b0248;

extern "C" {
int ref___ttyslot(void);
char *ref_freebsd11_devname(uint32_t dev, mode_t type);
char *ref_freebsd11_devname_r(uint32_t dev, mode_t type, char *buf, int len);
}

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum { F_TTYSLOT, F_DEVNAME, F_DEVNAME_R, NFUNC };

static const char *const fname[NFUNC] = {
	"__ttyslot", "freebsd11_devname", "freebsd11_devname_r"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 12;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-20s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xc0ffeebaddecade1ULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

static mode_t
randmode(void)
{
	return (mode_t)(nextrand() & 0xffffu);
}

/* ------------------------------------------------------------------ */
/* devname / devname_r mocks (linker --wrap)                           */
/* ------------------------------------------------------------------ */

static char devname_storage[128];
static char devname_storage_base[sizeof devname_storage];

enum DevnameMode {
	DM_OK_STATIC,
	DM_OK_BUF,
	DM_NULL,
	DM_ERR_R
};

struct DevnameScript {
	DevnameMode mode;
	char payload[96];
	int ret_null;
};

static DevnameScript dn_script[16];
static int dn_script_n;
static int dn_script_i;

static void
dn_reset(const DevnameScript *scripts, int n)
{
	dn_script_n = n;
	dn_script_i = 0;
	if (n > 0)
		std::memcpy(dn_script, scripts, (size_t)n * sizeof(DevnameScript));
}

static const DevnameScript *
dn_next(void)
{
	if (dn_script_i >= dn_script_n)
		return nullptr;
	return &dn_script[dn_script_i++];
}

static void
format_dev(char *dst, size_t dstsz, dev_t dev, mode_t type)
{
	std::snprintf(dst, dstsz, "d%jx:t%o", (uintmax_t)dev, (unsigned)type);
}

extern "C" char *
__wrap_devname(dev_t dev, mode_t type)
{
	const DevnameScript *s = dn_next();

	if (s == nullptr) {
		format_dev(devname_storage, sizeof devname_storage, dev, type);
		return devname_storage;
	}
	switch (s->mode) {
	case DM_NULL:
		return nullptr;
	case DM_OK_STATIC:
		std::strncpy(devname_storage, s->payload, sizeof devname_storage - 1);
		devname_storage[sizeof devname_storage - 1] = '\0';
		return devname_storage;
	default:
		format_dev(devname_storage, sizeof devname_storage, dev, type);
		return devname_storage;
	}
}

extern "C" char *
__wrap_devname_r(dev_t dev, mode_t type, char *buf, int len)
{
	const DevnameScript *s = dn_next();

	if (buf == nullptr || len <= 0)
		return nullptr;
	if (s == nullptr) {
		format_dev(buf, (size_t)len, dev, type);
		return buf;
	}
	switch (s->mode) {
	case DM_NULL:
	case DM_ERR_R:
		return nullptr;
	case DM_OK_BUF:
		std::strncpy(buf, s->payload, (size_t)len - 1);
		buf[len - 1] = '\0';
		return buf;
	default:
		format_dev(buf, (size_t)len, dev, type);
		return buf;
	}
}

/* ------------------------------------------------------------------ */
/* Comparison helpers                                                 */
/* ------------------------------------------------------------------ */

static bool
ptr_offset_ok(const char *got, const char *got_base, const char *exp,
    const char *exp_base)
{
	if (exp == nullptr && got == nullptr)
		return true;
	if (exp == nullptr || got == nullptr)
		return false;
	return (got - got_base) == (exp - exp_base);
}

struct GuardBuf {
	static constexpr size_t PRE = 32;
	static constexpr size_t WIN = 256;
	static constexpr size_t POST = 32;
	unsigned char bytes[PRE + WIN + POST];

	void init(void)
	{
		std::memset(bytes, 0x7f, sizeof bytes);
	}

	char *win(void)
	{
		return (char *)(bytes + PRE);
	}

	size_t winsz(void) const
	{
		return WIN;
	}

	bool identical(const GuardBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof bytes) == 0;
	}
};

/* ------------------------------------------------------------------ */
/* __ttyslot                                                          */
/* ------------------------------------------------------------------ */

static bool
ttyslot_ok(const char *ctx)
{
	int a = P::__ttyslot();
	int b = ref___ttyslot();

	if (a != b) {
		char msg[64];

		std::snprintf(msg, sizeof msg, "port=%d ref=%d", a, b);
		report(F_TTYSLOT, ctx, msg);
		return false;
	}
	return true;
}

static void
test_ttyslot_edges(void)
{
	ncases[F_TTYSLOT]++;
	ttyslot_ok("edge once");
	ncases[F_TTYSLOT]++;
	ttyslot_ok("edge twice");
}

static void
test_ttyslot_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_TTYSLOT]++;
		ttyslot_ok(ctx);
	}
}

/* ------------------------------------------------------------------ */
/* freebsd11_devname                                                  */
/* ------------------------------------------------------------------ */

static bool
devname_ok(uint32_t dev, mode_t type, const DevnameScript *scripts, int n,
    const char *ctx)
{
	char *a, *b;
	bool ok = true;

	dn_reset(scripts, n);
	a = P::freebsd11_devname(dev, type);

	dn_reset(scripts, n);
	b = ref_freebsd11_devname(dev, type);

	if (!ptr_offset_ok(a, devname_storage_base, b, devname_storage_base)) {
		char msg[160];

		std::snprintf(msg, sizeof msg,
		    "dev=%u type=%o port_off=%td ref_off=%td port=%p ref=%p",
		    dev, (unsigned)type,
		    a != nullptr ? a - devname_storage_base : (ptrdiff_t)-1,
		    b != nullptr ? b - devname_storage_base : (ptrdiff_t)-1,
		    (void *)a, (void *)b);
		report(F_DEVNAME, ctx, msg);
		ok = false;
	}
	return ok;
}

static void
test_devname_edges(void)
{
	static const uint32_t devs[] = {
		0, 1, 2, 0xff, 0x100, 0xffff, 0x10000, 0xffffffffu
	};
	static const mode_t types[] = {
		0, 0010000, 0020000, 0060000, 0100000, 0140000, 0170000
	};
	DevnameScript nullscr = { DM_NULL, {}, 1 };
	DevnameScript okscr = { DM_OK_STATIC, "compat-dev", 0 };
	unsigned char hb[] = { 0x80, 0xff, 0xfe, 0x00 };

	for (uint32_t dev : devs) {
		for (mode_t type : types) {
			char ctx[64];

			std::snprintf(ctx, sizeof ctx, "edge dev=%u type=%o",
			    dev, (unsigned)type);
			ncases[F_DEVNAME]++;
			devname_ok(dev, type, nullptr, 0, ctx);
		}
	}

	ncases[F_DEVNAME]++;
	devname_ok(5, 0060000, &nullscr, 1, "null script");
	ncases[F_DEVNAME]++;
	devname_ok(7, 0020000, &okscr, 1, "static payload");

	{
		DevnameScript seq[2];

		seq[0] = { DM_OK_STATIC, { 'a' }, 0 };
		seq[1] = { DM_NULL, {}, 1 };
		ncases[F_DEVNAME]++;
		devname_ok(9, 0060000, seq, 2, "two-step script");
	}

	(void)hb;
}

static void
test_devname_random(void)
{
	for (int i = 0; i < 200000; i++) {
		uint32_t dev = randu32();
		mode_t type = randmode();
		DevnameScript scr;
		const DevnameScript *pscr = nullptr;
		int nscr = 0;
		char ctx[48];

		if ((nextrand() & 15u) == 0u) {
			scr.mode = DM_NULL;
			scr.payload[0] = '\0';
			pscr = &scr;
			nscr = 1;
		} else if ((nextrand() & 7u) == 0u) {
			size_t n = (size_t)(nextrand() % (sizeof scr.payload - 1));
			scr.mode = DM_OK_STATIC;
			for (size_t j = 0; j < n; j++)
				scr.payload[j] = (char)(nextrand() & 0xffu);
			scr.payload[n] = '\0';
			pscr = &scr;
			nscr = 1;
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_DEVNAME]++;
		devname_ok(dev, type, pscr, nscr, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* freebsd11_devname_r                                                */
/* ------------------------------------------------------------------ */

static bool
devname_r_ok(uint32_t dev, mode_t type, int len,
    const DevnameScript *scripts, int n, const char *ctx)
{
	GuardBuf port_buf, ref_buf;
	char *a, *b;
	bool ok = true;

	if (len < 0)
		len = 0;
	if ((size_t)len > port_buf.winsz())
		len = (int)port_buf.winsz();

	port_buf.init();
	ref_buf.init();

	dn_reset(scripts, n);
	a = P::freebsd11_devname_r(dev, type, port_buf.win(), len);

	dn_reset(scripts, n);
	b = ref_freebsd11_devname_r(dev, type, ref_buf.win(), len);

	if (!ptr_offset_ok(a, port_buf.win(), b, ref_buf.win())) {
		char msg[192];

		std::snprintf(msg, sizeof msg,
		    "dev=%u type=%o len=%d port_off=%td ref_off=%td",
		    dev, (unsigned)type, len,
		    a != nullptr ? a - port_buf.win() : (ptrdiff_t)-1,
		    b != nullptr ? b - ref_buf.win() : (ptrdiff_t)-1);
		report(F_DEVNAME_R, ctx, msg);
		ok = false;
	}
	if (!port_buf.identical(ref_buf)) {
		report(F_DEVNAME_R, ctx, "full guard buffer mismatch");
		ok = false;
	}
	return ok;
}

static void
test_devname_r_edges(void)
{
	static const uint32_t devs[] = {
		0, 1, 0xff, 0x10000, 0xffffffffu
	};
	static const mode_t types[] = {
		0, 0020000, 0060000, 0100000, 0170000
	};
	static const int lens[] = { 0, 1, 2, 3, 8, 16, 64, 256 };
	DevnameScript nullscr = { DM_ERR_R, {}, 1 };
	DevnameScript okscr = { DM_OK_BUF, "r-compat", 0 };
	unsigned char hb[] = { 0x80, 0xff, 0xfe, 0x7f };

	for (uint32_t dev : devs) {
		for (mode_t type : types) {
			for (int len : lens) {
				char ctx[72];

				std::snprintf(ctx, sizeof ctx,
				    "edge dev=%u type=%o len=%d", dev,
				    (unsigned)type, len);
				ncases[F_DEVNAME_R]++;
				devname_r_ok(dev, type, len, nullptr, 0, ctx);
			}
		}
	}

	ncases[F_DEVNAME_R]++;
	devname_r_ok(3, 0060000, 32, &nullscr, 1, "null return");
	ncases[F_DEVNAME_R]++;
	devname_r_ok(4, 0020000, 10, &okscr, 1, "buf payload");

	{
		DevnameScript longscr = { DM_OK_BUF, {}, 0 };
		std::memset(longscr.payload, 'Z', sizeof longscr.payload - 1);
		longscr.payload[sizeof longscr.payload - 1] = '\0';
		ncases[F_DEVNAME_R]++;
		devname_r_ok(11, 0060000, 5, &longscr, 1, "truncate");
	}

	{
		DevnameScript hbscr = { DM_OK_BUF, {}, 0 };
		std::memcpy(hbscr.payload, hb, 4);
		hbscr.payload[4] = '\0';
		ncases[F_DEVNAME_R]++;
		devname_r_ok(12, 0100000, 32, &hbscr, 1, "high-bit payload");
	}

	ncases[F_DEVNAME_R]++;
	devname_r_ok(13, 0060000, 1, &okscr, 1, "len 1 NUL only");
}

static void
test_devname_r_random(void)
{
	for (int i = 0; i < 200000; i++) {
		uint32_t dev = randu32();
		mode_t type = randmode();
		int len = (int)(nextrand() % (GuardBuf::WIN + 8u));
		DevnameScript scr;
		const DevnameScript *pscr = nullptr;
		int nscr = 0;
		char ctx[48];

		if ((nextrand() & 15u) == 0u) {
			scr.mode = DM_ERR_R;
			pscr = &scr;
			nscr = 1;
		} else if ((nextrand() & 7u) == 0u) {
			size_t n = (size_t)(nextrand() % (sizeof scr.payload - 1));
			scr.mode = DM_OK_BUF;
			for (size_t j = 0; j < n; j++)
				scr.payload[j] = (char)(nextrand() & 0xffu);
			scr.payload[n] = '\0';
			pscr = &scr;
			nscr = 1;
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_DEVNAME_R]++;
		devname_r_ok(dev, type, len, pscr, nscr, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	std::memcpy(devname_storage_base, devname_storage,
	    sizeof devname_storage);

	test_ttyslot_edges();
	test_ttyslot_random();
	test_devname_edges();
	test_devname_random();
	test_devname_r_edges();
	test_devname_r_random();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-22s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NFUNC; i++)
		total_fail += nfails[i];

	return total_fail == 0 ? 0 : 1;
}
