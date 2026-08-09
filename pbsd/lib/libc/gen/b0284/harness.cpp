/*
 * Differential harness for batch b0284 (sig2str, str2sig, __xuname).
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

import pbsd.lib.libc.gen.b0284;

namespace P = pbsd::lib_libc_gen::b0284;

extern "C" {
void harness_reset_mocks(void);
void harness_set_env_s(const char *);
void harness_set_env_r(const char *);
void harness_set_env_v(const char *);
void harness_set_env_m(const char *);
void harness_set_sys_ostype(const char *);
void harness_set_sys_hostname(const char *);
void harness_set_sys_osrelease(const char *);
void harness_set_sys_version(const char *);
void harness_set_sys_machine(const char *);
void harness_set_sysctl_fail(int fail, int err);
void harness_set_sysctl_enomem(int enomem);

int ref_sig2str(int signum, char *str);
int ref_str2sig(const char *str, int *pnum);
int ref___xuname(int namesize, void *namebuf);
}

enum {
	F_SIG2STR,
	F_STR2SIG,
	F_XUNAME,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"sig2str",
	"str2sig",
	"__xuname",
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
		std::printf("FAIL %-12s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

static void
record_ok(int f)
{
	ncases[f]++;
}

static void
record_fail(int f, const char *ctx, const char *detail)
{
	ncases[f]++;
	report(f, ctx, detail);
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xb0284facefeedULL;

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

static int
rand_signum(void)
{
	switch (randu32() % 12) {
	case 0: return 0;
	case 1: return -1;
	case 2: return 1;
	case 3: return 31;
	case 4: return 32;
	case 5: return 33;
	case 6: return 64;
	case 7: return 65;
	case 8: return 95;
	case 9: return 96;
	case 10: return 126;
	case 11: return 127;
	default: return (int)(randu32() % 200);
	}
}

/* ------------------------------------------------------------------ */
/* Guard buffers                                                      */
/* ------------------------------------------------------------------ */

struct GuardBuf {
	static constexpr size_t PRE = 16;
	static constexpr size_t POST = 16;
	unsigned char storage[PRE + 64 + POST];

	void
	init(void)
	{
		std::memset(storage, 0x7f, sizeof storage);
	}

	char *
	win(void)
	{
		return (char *)(storage + PRE);
	}

	size_t
	winsz(void) const
	{
		return 64;
	}

	bool
	guards_intact(void) const
	{
		for (size_t i = 0; i < PRE; i++)
			if (storage[i] != 0x7f)
				return false;
		for (size_t i = PRE + 64; i < sizeof storage; i++)
			if (storage[i] != 0x7f)
				return false;
		return true;
	}

	bool
	eq(const GuardBuf &o) const
	{
		return std::memcmp(storage, o.storage, sizeof storage) == 0;
	}

	void
	copy_from(const GuardBuf &o)
	{
		std::memcpy(storage, o.storage, sizeof storage);
	}
};

struct UnameBuf {
	static constexpr size_t PRE = 16;
	static constexpr size_t FIELDS = 5;
	static constexpr size_t FIELD_MAX = 256;
	static constexpr size_t POST = 16;
	unsigned char storage[PRE + FIELDS * FIELD_MAX + POST];

	void
	init(void)
	{
		std::memset(storage, 0x7f, sizeof storage);
	}

	char *
	base(void)
	{
		return (char *)(storage + PRE);
	}

	size_t
	cap(void) const
	{
		return FIELDS * FIELD_MAX;
	}

	bool
	guards_intact(void) const
	{
		for (size_t i = 0; i < PRE; i++)
			if (storage[i] != 0x7f)
				return false;
		for (size_t i = PRE + cap(); i < sizeof storage; i++)
			if (storage[i] != 0x7f)
				return false;
		return true;
	}

	bool
	eq(const UnameBuf &o) const
	{
		return std::memcmp(storage, o.storage, sizeof storage) == 0;
	}

	void
	copy_from(const UnameBuf &o)
	{
		std::memcpy(storage, o.storage, sizeof storage);
	}
};

static void
setup_default_sysctl(void)
{
	harness_set_sys_ostype("FreeBSD");
	harness_set_sys_hostname("testhost");
	harness_set_sys_osrelease("14.0-RELEASE");
	harness_set_sys_version("14.0-RELEASE\n#1: Mon Jan 1 00:00:00 UTC 2024\n\troot@host:/usr/src\n");
	harness_set_sys_machine("amd64");
	harness_set_sysctl_fail(0, 0);
	harness_set_sysctl_enomem(0);
}

/* ------------------------------------------------------------------ */
/* sig2str                                                            */
/* ------------------------------------------------------------------ */

static void
case_sig2str(int signum)
{
	GuardBuf bp, br;

	bp.init();
	br.init();
	bp.copy_from(br);

	int rp = P::sig2str(signum, bp.win());
	int rr = ref_sig2str(signum, br.win());

	if (rp != rr) {
		record_fail(F_SIG2STR, "sig2str rv",
		    std::to_string(signum).c_str());
		return;
	}
	if (!bp.guards_intact() || !br.guards_intact()) {
		record_fail(F_SIG2STR, "sig2str guards", "guard corruption");
		return;
	}
	if (!bp.eq(br)) {
		record_fail(F_SIG2STR, "sig2str buf",
		    std::to_string(signum).c_str());
		return;
	}
	record_ok(F_SIG2STR);
}

static void
test_sig2str_edges(void)
{
	static const int edges[] = {
		-100, -1, 0, 1, 2, 9, 15, 31, 32, 33, 34, 63, 64, 65, 66,
		80, 95, 96, 97, 125, 126, 127, 200, 1000, INT_MIN, INT_MAX,
	};

	for (int s : edges)
		case_sig2str(s);
}

static void
test_sig2str_random(void)
{
	for (unsigned i = 0; i < 200000u; i++)
		case_sig2str(rand_signum());
}

/* ------------------------------------------------------------------ */
/* str2sig                                                            */
/* ------------------------------------------------------------------ */

static void
case_str2sig(const char *str)
{
	GuardBuf nip, nir;
	int pnum_p = 0x55555555;
	int pnum_r = 0x55555555;

	nip.init();
	nir.init();

	int rp = P::str2sig(str, &pnum_p);
	int rr = ref_str2sig(str, &pnum_r);

	if (rp != rr) {
		record_fail(F_STR2SIG, "str2sig rv", str != nullptr ? str : "(null)");
		return;
	}
	if (rp == 0 && pnum_p != pnum_r) {
		record_fail(F_STR2SIG, "str2sig pnum", str);
		return;
	}
	if (!nip.guards_intact() || !nir.guards_intact()) {
		record_fail(F_STR2SIG, "str2sig guards", str);
		return;
	}
	record_ok(F_STR2SIG);
}

static char rndbuf[128];

static void
fill_random_cstr(char *buf, size_t maxlen)
{
	size_t n = (size_t)(randu32() % (maxlen > 0 ? maxlen : 1));

	for (size_t i = 0; i < n; i++) {
		unsigned v = randu32() % 256;
		if (v == 0 && (randu32() & 3u) != 0)
			v = 'A' + (randu32() % 26);
		buf[i] = (char)v;
	}
	buf[n] = '\0';
}

static void
test_str2sig_edges(void)
{
	static const char *const edges[] = {
		"",
		"SIGINT",
		"sigint",
		"SIG",
		"INT",
		"HUP",
		"hup",
		"KILL",
		"Signal 0",
		"0",
		"1",
		"126",
		"127",
		"999",
		"RTMIN",
		"rtmin",
		"RTMAX",
		"rtmax",
		"RTMIN+0",
		"RTMIN+1",
		"RTMIN+31",
		"RTMIN-1",
		"RTMAX-0",
		"RTMAX-1",
		"RTMAX-61",
		"RTMAX+1",
		"SIGRTMIN",
		"SIGRTMAX",
		"SIGRTMIN+5",
		"SIGRTMAX-3",
		"RTMIN+",
		"RTMIN-",
		"RTMIN+abc",
		"RTMINX",
		"RTMINN",
		"RTMIX",
		"NOTASIGNAL",
		"\x80\xff",
	};

	for (const char *s : edges)
		case_str2sig(s);
}

static void
test_str2sig_random(void)
{
	static const char *const bases[] = {
		"INT", "HUP", "RTMIN", "RTMAX", "SIGINT", "65", "1", "KILL",
		"RTMIN+5", "RTMAX-10", "SIGRTMIN+1", "SIGRTMAX-2",
	};

	for (unsigned i = 0; i < 200000u; i++) {
		if ((randu32() & 7u) == 0) {
			fill_random_cstr(rndbuf, sizeof(rndbuf) - 1);
			case_str2sig(rndbuf);
		} else {
			const char *b = bases[randu32() % (sizeof(bases) / sizeof(bases[0]))];
			size_t bl = std::strlen(b);
			size_t off = (size_t)(randu32() % 4);
			size_t n = (size_t)(randu32() % 32);
			size_t pos = 0;

			for (size_t j = 0; j < off && pos < sizeof(rndbuf) - 1; j++)
				rndbuf[pos++] = (char)(0x80 + (randu32() % 0x7f));
			for (size_t j = 0; j < bl && pos < sizeof(rndbuf) - 1; j++)
				rndbuf[pos++] = b[j];
			for (size_t j = 0; j < n && pos < sizeof(rndbuf) - 1; j++) {
				unsigned v = randu32() % 256;
				rndbuf[pos++] = (char)v;
			}
			rndbuf[pos] = '\0';
			case_str2sig(rndbuf);
		}
	}
}

/* ------------------------------------------------------------------ */
/* __xuname                                                           */
/* ------------------------------------------------------------------ */

static void
case_xuname(int namesize, void (*setup)(void))
{
	UnameBuf bp, br;
	int save_errno = errno;

	harness_reset_mocks();
	setup_default_sysctl();
	if (setup != nullptr)
		setup();

	bp.init();
	br.init();
	bp.copy_from(br);

	errno = 0x42;
	int rp = P::__xuname(namesize, bp.base());
	int save_errno_after_p = errno;
	errno = 0x42;
	int rr = ref___xuname(namesize, br.base());
	int save_errno_after_r = errno;

	if (rp != rr) {
		record_fail(F_XUNAME, "rv",
		    (std::string("namesize=") + std::to_string(namesize)).c_str());
		return;
	}
	if (save_errno_after_p != save_errno_after_r) {
		record_fail(F_XUNAME, "errno",
		    (std::string("namesize=") + std::to_string(namesize)).c_str());
		return;
	}
	if (!bp.guards_intact() || !br.guards_intact()) {
		record_fail(F_XUNAME, "guards", "guard corruption");
		return;
	}
	if (!bp.eq(br)) {
		record_fail(F_XUNAME, "buf",
		    (std::string("namesize=") + std::to_string(namesize)).c_str());
		return;
	}
	(void)save_errno;
	record_ok(F_XUNAME);
}

static void
test_xuname_edges(void)
{
	case_xuname(1, nullptr);
	case_xuname(2, nullptr);
	case_xuname(8, nullptr);
	case_xuname(16, nullptr);
	case_xuname(64, nullptr);
	case_xuname(256, nullptr);

	case_xuname(16, []() { harness_set_env_s("Linux"); });
	case_xuname(16, []() { harness_set_env_r("6.6.0"); });
	case_xuname(16, []() { harness_set_env_v("custom"); });
	case_xuname(16, []() { harness_set_env_m("x86_64"); });
	case_xuname(16, []() {
		harness_set_env_s("A");
		harness_set_env_r("B");
		harness_set_env_v("C");
		harness_set_env_m("D");
	});

	case_xuname(8, []() {
		harness_set_sys_version("a\nb\tc\n");
	});

	case_xuname(4, []() {
		harness_set_sys_version("\n\n\n");
	});

	case_xuname(3, []() {
		harness_set_sys_version("\t\nX");
	});

	case_xuname(16, []() {
		harness_set_sysctl_fail(1, EIO);
	});

	case_xuname(16, []() {
		harness_set_sysctl_enomem(1);
	});

	case_xuname(16, []() {
		harness_set_sys_ostype(
		    "VeryLongOperatingSystemNameThatExceedsBuffer");
	});

	case_xuname(1, []() {
		harness_set_sys_hostname("host");
	});
}

static unsigned g_xuname_rand_mode;
static char g_xuname_rand_ver[64];

static void
random_xuname_setup(void)
{
	if (g_xuname_rand_mode & 1u)
		harness_set_env_s("env_s");
	if (g_xuname_rand_mode & 2u)
		harness_set_env_r("env_r");
	if (g_xuname_rand_mode & 4u)
		harness_set_env_v("env_v");
	if (g_xuname_rand_mode & 8u)
		harness_set_env_m("env_m");
	if ((g_xuname_rand_mode & 3u) == 0)
		harness_set_sysctl_fail(1, EACCES);
	if ((g_xuname_rand_mode & 5u) == 0)
		harness_set_sysctl_enomem(1);
	harness_set_sys_version(g_xuname_rand_ver);
}

static void
test_xuname_random(void)
{
	for (unsigned i = 0; i < 200000u; i++) {
		int namesize = 1 + (int)(randu32() % 64);

		g_xuname_rand_mode = randu32() % 16;
		fill_random_cstr(g_xuname_rand_ver, sizeof(g_xuname_rand_ver) - 1);
		case_xuname(namesize, random_xuname_setup);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	harness_reset_mocks();
	setup_default_sysctl();

	test_sig2str_edges();
	test_sig2str_random();
	test_str2sig_edges();
	test_str2sig_random();
	test_xuname_edges();
	test_xuname_random();

	std::printf("%-12s %10s %10s\n", "function", "cases", "failures");
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	for (int i = 0; i < NFUNC; i++) {
		std::printf("%-12s %10llu %10llu\n",
		    fname[i], ncases[i], nfails[i]);
		total_cases += ncases[i];
		total_fails += nfails[i];
	}
	std::printf("%-12s %10llu %10llu\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
