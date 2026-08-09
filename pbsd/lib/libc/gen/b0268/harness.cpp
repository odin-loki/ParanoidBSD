/*
 * Differential harness for batch b0268 (dirname, __libc_interposing_slot,
 * __arc4random_stir_fbsd11, __arc4random_addrandom_fbsd11).
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.gen.b0268;

namespace P = pbsd::lib_libc_gen::b0268;

typedef void (*interpos_func_t)(void);

enum {
	INTERPOS_accept,
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_close,
	INTERPOS_connect,
	INTERPOS_fcntl,
	INTERPOS_fsync,
	INTERPOS_fork,
	INTERPOS_msync,
	INTERPOS_nanosleep,
	INTERPOS_openat,
	INTERPOS_poll,
	INTERPOS_pselect,
	INTERPOS_recvfrom,
	INTERPOS_recvmsg,
	INTERPOS_select,
	INTERPOS_sendmsg,
	INTERPOS_sendto,
	INTERPOS_setcontext,
	INTERPOS_sigaction,
	INTERPOS_sigprocmask,
	INTERPOS_sigsuspend,
	INTERPOS_sigwait,
	INTERPOS_sigtimedwait,
	INTERPOS_sigwaitinfo,
	INTERPOS_swapcontext,
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_read,
	INTERPOS_readv,
	INTERPOS_wait4,
	INTERPOS_write,
	INTERPOS_writev,
	INTERPOS__pthread_mutex_init_calloc_cb,
	INTERPOS_spinlock,
	INTERPOS_spinunlock,
	INTERPOS_kevent,
	INTERPOS_wait6,
	INTERPOS_ppoll,
	INTERPOS_map_stacks_exec,
	INTERPOS_fdatasync,
	INTERPOS_clock_nanosleep,
	INTERPOS__reserved0,
	INTERPOS_pdfork,
	INTERPOS_uexterr_gettext,
	INTERPOS_pdwait,
	INTERPOS_MAX
};

extern "C" {
char *ref_dirname(char *path);
extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];
interpos_func_t *ref___libc_interposing_slot(int interposno);
void ref___arc4random_stir_fbsd11(void);
void ref___arc4random_addrandom_fbsd11(unsigned char *, int);
}

/* ------------------------------------------------------------------ */
/* syslog mock                                                        */
/* ------------------------------------------------------------------ */

struct SyslogRec {
	int priority;
	char msg[96];
};

static SyslogRec syslog_log[8];
static int syslog_log_n;
static int syslog_total;

extern "C" int
__wrap_syslog(int priority, const char *fmt, ...)
{
	SyslogRec *rec;

	if (syslog_log_n < (int)(sizeof syslog_log / sizeof syslog_log[0])) {
		rec = &syslog_log[syslog_log_n++];
		rec->priority = priority;
		if (fmt != nullptr)
			std::strncpy(rec->msg, fmt, sizeof rec->msg - 1);
		else
			rec->msg[0] = '\0';
		rec->msg[sizeof rec->msg - 1] = '\0';
	}
	syslog_total++;
	return 0;
}

static void
syslog_reset(void)
{
	syslog_log_n = 0;
	syslog_total = 0;
	std::memset(syslog_log, 0, sizeof syslog_log);
}

/* ------------------------------------------------------------------ */
/* libsys interposing mock                                            */
/* ------------------------------------------------------------------ */

static interpos_func_t libsys_mock[INTERPOS_MAX];

extern "C" interpos_func_t *
__libsys_interposing_slot(int interposno)
{
	return (&libsys_mock[interposno]);
}

static void
libsys_mock_reset(void)
{
	for (int i = 0; i < INTERPOS_MAX; i++)
		libsys_mock[i] =
		    (interpos_func_t)(uintptr_t)(0x4000u + (unsigned)i);
}

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum {
	F_DIRNAME,
	F_INTERPOSING_SLOT,
	F_ARC4_STIR,
	F_ARC4_ADDRANDOM,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"dirname",
	"__libc_interposing_slot",
	"__arc4random_stir_fbsd11",
	"__arc4random_addrandom_fbsd11"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-28s %s: %s\n", fname[f], ctx, detail);
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

/* ------------------------------------------------------------------ */
/* Guard buffers                                                      */
/* ------------------------------------------------------------------ */

struct GuardBuf {
	static constexpr size_t PRE = 32;
	static constexpr size_t WIN = 256;
	static constexpr size_t POST = 32;
	unsigned char bytes[PRE + WIN + POST];

	void
	init(void)
	{
		std::memset(bytes, 0x7f, sizeof bytes);
	}

	char *
	win(void)
	{
		return (char *)(bytes + PRE);
	}

	size_t
	winsz(void) const
	{
		return WIN;
	}

	void
	copy_path(const char *s)
	{
		size_t n = std::strlen(s);

		if (n >= WIN)
			n = WIN - 1;
		std::memcpy(win(), s, n);
		win()[n] = '\0';
	}

	void
	copy_path_len(const unsigned char *data, size_t n)
	{
		if (n >= WIN)
			n = WIN - 1;
		std::memcpy(win(), data, n);
		win()[n] = '\0';
	}

	bool
	identical(const GuardBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof bytes) == 0;
	}
};

static bool
ptr_offset_ok(const char *got, const char *base, const char *exp,
    const char *exp_base)
{
	if (exp == nullptr && got == nullptr)
		return true;
	if (exp == nullptr || got == nullptr)
		return false;
	return (got - base) == (exp - exp_base);
}

/* ------------------------------------------------------------------ */
/* dirname                                                            */
/* ------------------------------------------------------------------ */

static bool
dirname_null_ok(const char *ctx)
{
	char *a = P::dirname(nullptr);
	char *b = ref_dirname(nullptr);
	bool ok = true;

	if (a == nullptr || b == nullptr) {
		report(F_DIRNAME, ctx, "null return pointer");
		return false;
	}
	if (std::strcmp(a, b) != 0) {
		char msg[96];

		std::snprintf(msg, sizeof msg, "port=\"%s\" ref=\"%s\"", a, b);
		report(F_DIRNAME, ctx, msg);
		ok = false;
	}
	return ok;
}

static bool
dirname_buf_ok(GuardBuf &gp, GuardBuf &gr, const char *ctx)
{
	char *a = P::dirname(gp.win());
	char *b = ref_dirname(gr.win());
	bool ok = true;

	if (!ptr_offset_ok(a, gp.win(), b, gr.win())) {
		char msg[160];

		std::snprintf(msg, sizeof msg,
		    "offset port=%td ref=%td",
		    a != nullptr ? a - gp.win() : (ptrdiff_t)-1,
		    b != nullptr ? b - gr.win() : (ptrdiff_t)-1);
		report(F_DIRNAME, ctx, msg);
		ok = false;
	}
	if (!gp.identical(gr)) {
		report(F_DIRNAME, ctx, "full guard buffer mismatch");
		ok = false;
	}
	return ok;
}

static void
test_dirname_edge(const char *path, const char *label)
{
	GuardBuf gp, gr;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);

	if (path == nullptr) {
		ncases[F_DIRNAME]++;
		dirname_null_ok(ctx);
		return;
	}

	gp.init();
	gr.init();
	gp.copy_path(path);
	gr.copy_path(path);
	ncases[F_DIRNAME]++;
	dirname_buf_ok(gp, gr, ctx);
}

static void
test_dirname_edge_len(const unsigned char *data, size_t n, const char *label)
{
	GuardBuf gp, gr;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);
	gp.init();
	gr.init();
	gp.copy_path_len(data, n);
	gr.copy_path_len(data, n);
	ncases[F_DIRNAME]++;
	dirname_buf_ok(gp, gr, ctx);
}

static void
test_dirname_edges(void)
{
	static const char *paths[] = {
		"",
		"/",
		"//",
		"///",
		"////",
		"a",
		"/a",
		"a/",
		"/a/",
		"foo",
		"foo/",
		"/foo",
		"/foo/",
		"foo/bar",
		"/foo/bar",
		"/foo/bar/",
		"///foo///bar///",
		"//a//b//",
		".",
		"..",
		"/.",
		"/..",
		"foo/.",
		"foo/..",
		"a/b/c/d/e",
		"/a/b/c/d/e/",
		"no/slash/at/end",
		"slash/at/end/",
		"x/y",
		"/x/y",
		"/x/y/",
		"onlycomponent",
		"/only",
	};

	ncases[F_DIRNAME]++;
	dirname_null_ok("null path");

	for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
		char label[64];

		std::snprintf(label, sizeof label, "edge \"%s\"", paths[i]);
		test_dirname_edge(paths[i], label);
	}

	{
		static const unsigned char hb1[] = { 0x80 };
		static const unsigned char hb2[] = { '/', 0xff, '/' };
		static const unsigned char hb3[] = {
			0xfe, '/', 0x80, 'b', 'a', 'r', '/', 0xff
		};
		static const unsigned char hb4[] = {
			0x80, 0x81, 0x82, '/', 0xfe, 0xff
		};
		static const unsigned char hb5[] = {
			'/', 0x80, 0x81, 0x82, '/'
		};
		static const unsigned char hb6[] = {
			0xff, 0xff, '/', 0x80
		};

		test_dirname_edge_len(hb1, sizeof hb1, "high-bit single 0x80");
		test_dirname_edge_len(hb2, sizeof hb2, "high-bit /\\xff/");
		test_dirname_edge_len(hb3, sizeof hb3, "high-bit mixed path");
		test_dirname_edge_len(hb4, sizeof hb4, "high-bit component");
		test_dirname_edge_len(hb5, sizeof hb5, "high-bit trailing slashes");
		test_dirname_edge_len(hb6, sizeof hb6, "high-bit only slashes");
	}

	{
		GuardBuf gp, gr;

		gp.init();
		gr.init();
		gp.win()[0] = '\0';
		gr.win()[0] = '\0';
		ncases[F_DIRNAME]++;
		dirname_buf_ok(gp, gr, "empty first byte only");
	}

	{
		GuardBuf gp, gr;
		size_t i;

		gp.init();
		gr.init();
		for (i = 0; i + 1 < gp.winsz(); i++)
			gp.win()[i] = '/';
		gp.win()[i] = '\0';
		gr.copy_path_len((const unsigned char *)gp.win(), i);
		ncases[F_DIRNAME]++;
		dirname_buf_ok(gp, gr, "all slashes fill window");
	}
}

static void
fill_random_path(unsigned char *dst, size_t maxn)
{
	size_t n;
	size_t i;
	unsigned char alphabet[] = {
		'/', 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.', '\0', 0x80, 0xfe, 0xff
	};

	if (maxn == 0)
		return;
	n = (size_t)(randu32() % (unsigned)(maxn + 1));
	for (i = 0; i < n; i++)
		dst[i] = alphabet[randu32() % (sizeof alphabet / sizeof alphabet[0])];
	dst[n] = '\0';
}

static void
test_dirname_random(void)
{
	for (int i = 0; i < 200000; i++) {
		GuardBuf gp, gr;
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		gp.init();
		gr.init();
		fill_random_path((unsigned char *)gp.win(), gp.winsz() - 1);
		gr.copy_path_len((const unsigned char *)gp.win(),
		    std::strlen(gp.win()));
		ncases[F_DIRNAME]++;
		dirname_buf_ok(gp, gr, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __libc_interposing_slot                                            */
/* ------------------------------------------------------------------ */

static bool
interposing_slot_ok(int slot, const char *ctx)
{
	interpos_func_t *pa = P::__libc_interposing_slot(slot);
	interpos_func_t *pb = ref___libc_interposing_slot(slot);
	bool ok = true;
	ptrdiff_t off_a;
	ptrdiff_t off_b;

	if (pa == nullptr || pb == nullptr) {
		report(F_INTERPOSING_SLOT, ctx, "null slot pointer");
		return false;
	}

	off_a = pa - P::__libc_interposing;
	off_b = pb - ref___libc_interposing;

	if (off_a != off_b) {
		char msg[128];

		std::snprintf(msg, sizeof msg,
		    "array offset port=%td ref=%td slot=%d",
		    off_a, off_b, slot);
		report(F_INTERPOSING_SLOT, ctx, msg);
		ok = false;
	}

	if (P::__libc_interposing[slot] != nullptr) {
		if (pa != &P::__libc_interposing[slot]) {
			report(F_INTERPOSING_SLOT, ctx, "port ptr not at slot");
			ok = false;
		}
		if (pb != &ref___libc_interposing[slot]) {
			report(F_INTERPOSING_SLOT, ctx, "ref ptr not at slot");
			ok = false;
		}
		if (P::__libc_interposing[slot] != ref___libc_interposing[slot]) {
			report(F_INTERPOSING_SLOT, ctx, "func pointer mismatch");
			ok = false;
		}
	} else {
		ptrdiff_t lsa = pa - libsys_mock;
		ptrdiff_t lsb = pb - libsys_mock;

		if (lsa != lsb || lsa != slot) {
			char msg[128];

			std::snprintf(msg, sizeof msg,
			    "libsys offset port=%td ref=%td slot=%d",
			    lsa, lsb, slot);
			report(F_INTERPOSING_SLOT, ctx, msg);
			ok = false;
		}
		if (pa != &libsys_mock[slot]) {
			report(F_INTERPOSING_SLOT, ctx, "port libsys ptr wrong");
			ok = false;
		}
		if (pb != &libsys_mock[slot]) {
			report(F_INTERPOSING_SLOT, ctx, "ref libsys ptr wrong");
			ok = false;
		}
	}

	return ok;
}

static void
test_interposing_slot_edges(void)
{
	static const int slots[] = {
		INTERPOS_accept,
		INTERPOS_close,
		INTERPOS_connect,
		INTERPOS_system,
		INTERPOS_tcdrain,
		INTERPOS__pthread_mutex_init_calloc_cb,
		INTERPOS_spinlock,
		INTERPOS_spinunlock,
		INTERPOS_map_stacks_exec,
		INTERPOS_uexterr_gettext,
		INTERPOS_pdwait,
		INTERPOS_MAX - 1,
	};

	libsys_mock_reset();

	for (size_t i = 0; i < sizeof slots / sizeof slots[0]; i++) {
		char ctx[64];

		std::snprintf(ctx, sizeof ctx, "edge slot %d", slots[i]);
		ncases[F_INTERPOSING_SLOT]++;
		interposing_slot_ok(slots[i], ctx);
	}
}

static void
test_interposing_slot_random(void)
{
	libsys_mock_reset();

	for (int i = 0; i < 200000; i++) {
		int slot = (int)(randu32() % INTERPOS_MAX);
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d slot %d", i, slot);
		ncases[F_INTERPOSING_SLOT]++;
		interposing_slot_ok(slot, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* arc4random compat                                                  */
/* ------------------------------------------------------------------ */

static bool
arc4_pair_ok(int func, const char *ctx, void (*port_fn)(void),
    void (*ref_fn)(void))
{
	int before = syslog_total;
	int mid;
	int after;
	int port_delta;
	int ref_delta;

	port_fn();
	mid = syslog_total;
	ref_fn();
	after = syslog_total;

	port_delta = mid - before;
	ref_delta = after - mid;

	if (port_delta != ref_delta) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "syslog port=%d ref=%d",
		    port_delta, ref_delta);
		report(func, ctx, msg);
		return false;
	}
	return true;
}

static bool
arc4_addrandom_pair_ok(const char *ctx, unsigned char *pbuf, int pint,
    unsigned char *rbuf, int rint)
{
	int before = syslog_total;
	int mid;
	int after;
	int port_delta;
	int ref_delta;

	P::__arc4random_addrandom_fbsd11(pbuf, pint);
	mid = syslog_total;
	ref___arc4random_addrandom_fbsd11(rbuf, rint);
	after = syslog_total;

	port_delta = mid - before;
	ref_delta = after - mid;

	if (port_delta != ref_delta) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "syslog port=%d ref=%d",
		    port_delta, ref_delta);
		report(F_ARC4_ADDRANDOM, ctx, msg);
		return false;
	}
	return true;
}

static void
test_arc4_stir_edges(void)
{
	ncases[F_ARC4_STIR]++;
	arc4_pair_ok(F_ARC4_STIR, "first call",
	    +[]() { P::__arc4random_stir_fbsd11(); },
	    +[]() { ref___arc4random_stir_fbsd11(); });

	ncases[F_ARC4_STIR]++;
	arc4_pair_ok(F_ARC4_STIR, "second call",
	    +[]() { P::__arc4random_stir_fbsd11(); },
	    +[]() { ref___arc4random_stir_fbsd11(); });

	ncases[F_ARC4_STIR]++;
	arc4_pair_ok(F_ARC4_STIR, "third call",
	    +[]() { P::__arc4random_stir_fbsd11(); },
	    +[]() { ref___arc4random_stir_fbsd11(); });
}

static void
test_arc4_addrandom_edges(void)
{
	unsigned char pb[16];
	unsigned char rb[16];

	std::memset(pb, 0x7f, sizeof pb);
	std::memset(rb, 0x7f, sizeof rb);

	ncases[F_ARC4_ADDRANDOM]++;
	arc4_addrandom_pair_ok("first call null", nullptr, 0, nullptr, 0);

	ncases[F_ARC4_ADDRANDOM]++;
	arc4_addrandom_pair_ok("second call null", nullptr, 0, nullptr, 0);

	pb[0] = 0x80;
	rb[0] = 0x80;
	ncases[F_ARC4_ADDRANDOM]++;
	arc4_addrandom_pair_ok("high-bit buf", pb, 1, rb, 1);

	pb[0] = 0xff;
	rb[0] = 0xff;
	ncases[F_ARC4_ADDRANDOM]++;
	arc4_addrandom_pair_ok("0xff buf neg len", pb, -1, rb, -1);

	std::memset(pb, 0x80, sizeof pb);
	std::memset(rb, 0x7f, sizeof rb);
	ncases[F_ARC4_ADDRANDOM]++;
	arc4_addrandom_pair_ok("full guard buf", pb, 16, rb, 16);
}

static void
test_arc4_stir_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_ARC4_STIR]++;
		arc4_pair_ok(F_ARC4_STIR, ctx,
		    +[]() { P::__arc4random_stir_fbsd11(); },
		    +[]() { ref___arc4random_stir_fbsd11(); });
	}
}

static void
test_arc4_addrandom_random(void)
{
	unsigned char pb[32];
	unsigned char rb[32];

	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		int len = (int)(randu32() % 33) - 8;
		size_t n = (size_t)(randu32() % 17);

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		std::memset(pb, 0x7f, sizeof pb);
		std::memset(rb, 0x7f, sizeof rb);
		for (size_t j = 0; j < n; j++) {
			pb[j] = (unsigned char)(randu32() & 0xffu);
			rb[j] = pb[j];
		}

		ncases[F_ARC4_ADDRANDOM]++;
		arc4_addrandom_pair_ok(ctx, n > 0 ? pb : nullptr, len,
		    n > 0 ? rb : nullptr, len);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	int exit_code = 0;

	syslog_reset();
	libsys_mock_reset();

	test_dirname_edges();
	test_dirname_random();

	test_interposing_slot_edges();
	test_interposing_slot_random();

	test_arc4_stir_edges();
	test_arc4_addrandom_edges();
	test_arc4_stir_random();
	test_arc4_addrandom_random();

	std::printf("\n%-28s %10s %10s\n", "Function", "Cases", "Failures");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-28s %10llu %10llu\n",
		    fname[f], ncases[f], nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("%-28s %10llu %10llu\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0)
		exit_code = 1;
	return exit_code;
}
