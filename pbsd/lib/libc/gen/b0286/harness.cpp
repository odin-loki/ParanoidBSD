/*
 * Differential harness for batch b0286 (confstr, statvfs, fstatvfs,
 * __timezone_compat).
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/statvfs.h>

import pbsd.lib.libc.gen.b0286;

namespace P = pbsd::lib_libc_gen::b0286;

extern "C" {
size_t ref_confstr(int name, char *buf, size_t len);
int ref_fstatvfs(int fd, struct statvfs *result);
int ref_statvfs(const char *path, struct statvfs *result);
char *ref___timezone_compat(int zone, int dst);
}

enum {
	F_CONFSTR,
	F_FSTATVFS,
	F_STATVFS,
	F_TIMEZONE,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"confstr",
	"fstatvfs",
	"statvfs",
	"__timezone_compat",
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

static std::uint64_t rng_state = 0xb0286facefeedULL;

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
	static constexpr size_t WIN = 4096;
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
	copy_cstr(const char *s)
	{
		size_t n = s != nullptr ? std::strlen(s) : 0;

		if (n >= WIN)
			n = WIN - 1;
		std::memcpy(win(), s, n);
		win()[n] = '\0';
	}

	bool
	identical(const GuardBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof bytes) == 0;
	}
};

struct GuardStatvfs {
	static constexpr size_t PRE = 32;
	static constexpr size_t POST = 32;
	unsigned char pre[PRE];
	struct statvfs sv;
	unsigned char post[POST];

	void
	init(void)
	{
		std::memset(pre, 0x7f, sizeof pre);
		std::memset(&sv, 0x7f, sizeof sv);
		std::memset(post, 0x7f, sizeof post);
	}

	bool
	identical(const GuardStatvfs &o) const
	{
		return std::memcmp(pre, o.pre, sizeof pre) == 0 &&
		    std::memcmp(&sv, &o.sv, sizeof sv) == 0 &&
		    std::memcmp(post, o.post, sizeof post) == 0;
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
/* Environment helpers                                                */
/* ------------------------------------------------------------------ */

static char *saved_tzname;
static bool had_tzname;

static void
save_tzname(void)
{
	const char *v = std::getenv("TZNAME");

	had_tzname = v != nullptr;
	saved_tzname = had_tzname ? std::strdup(v) : nullptr;
}

static void
restore_tzname(void)
{
	if (had_tzname)
		(void)setenv("TZNAME", saved_tzname, 1);
	else
		(void)unsetenv("TZNAME");
}

static void
set_tzname_value(const char *v)
{
	if (v != nullptr)
		(void)setenv("TZNAME", v, 1);
	else
		(void)unsetenv("TZNAME");
}

/* ------------------------------------------------------------------ */
/* confstr                                                            */
/* ------------------------------------------------------------------ */

static void
check_confstr(int name, char *buf_p, char *buf_r, size_t len, const char *ctx)
{
	size_t p, r;
	int e0 = errno;
	int pe, re;

	errno = 0;
	p = P::confstr(name, buf_p, len);
	pe = errno;
	errno = 0;
	r = ref_confstr(name, buf_r, len);
	re = errno;

	ncases[F_CONFSTR]++;
	if (p != r) {
		char msg[96];

		std::snprintf(msg, sizeof msg, "ret port=%zu ref=%zu name=%d",
		    p, r, name);
		report(F_CONFSTR, ctx, msg);
	}
	if (pe != re) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "errno port=%d ref=%d", pe, re);
		report(F_CONFSTR, ctx, msg);
	}
	if (buf_p != nullptr && buf_r != nullptr &&
	    std::memcmp(buf_p, buf_r, len > 0 ? len : 0) != 0) {
		/* compare up to guard window when len may truncate */
		size_t cmp = len;

		if (cmp > GuardBuf::WIN)
			cmp = GuardBuf::WIN;
		if (std::memcmp(buf_p, buf_r, cmp) != 0)
			report(F_CONFSTR, ctx, "buf prefix mismatch");
	}
	errno = e0;
}

static void
confstr_case(int name, size_t len, bool use_buf, const char *ctx)
{
	GuardBuf gp, gr;

	gp.init();
	gr.init();
	if (use_buf) {
		for (size_t i = 0; i < gp.winsz(); i++) {
			gp.win()[i] = (char)(0x80 | (i & 0x7f));
			gr.win()[i] = gp.win()[i];
		}
	}
	check_confstr(name, use_buf ? gp.win() : nullptr,
	    use_buf ? gr.win() : nullptr, len, ctx);
	if (use_buf && !gp.identical(gr))
		report(F_CONFSTR, ctx, "guard buffer mismatch");
}

static void
test_confstr_edges(void)
{
	static const int names[] = {
		_CS_PATH,
		_CS_POSIX_V6_ILP32_OFF32_CFLAGS,
		_CS_POSIX_V6_ILP32_OFF32_LDFLAGS,
		_CS_POSIX_V6_ILP32_OFF32_LIBS,
		_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,
		_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,
		_CS_POSIX_V6_ILP32_OFFBIG_LIBS,
		_CS_POSIX_V6_LP64_OFF64_CFLAGS,
		_CS_POSIX_V6_LP64_OFF64_LDFLAGS,
		_CS_POSIX_V6_LP64_OFF64_LIBS,
		_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,
		_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,
		_CS_POSIX_V6_LPBIG_OFFBIG_LIBS,
		_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS,
		-1,
		99999,
		0x7fffffff,
	};

	for (size_t i = 0; i < sizeof names / sizeof names[0]; i++) {
		char ctx[64];

		std::snprintf(ctx, sizeof ctx, "edge name=%d len=0", names[i]);
		confstr_case(names[i], 0, false, ctx);
		std::snprintf(ctx, sizeof ctx, "edge name=%d len=0 buf", names[i]);
		confstr_case(names[i], 0, true, ctx);
		std::snprintf(ctx, sizeof ctx, "edge name=%d len=1", names[i]);
		confstr_case(names[i], 1, true, ctx);
		std::snprintf(ctx, sizeof ctx, "edge name=%d len=2", names[i]);
		confstr_case(names[i], 2, true, ctx);
		std::snprintf(ctx, sizeof ctx, "edge name=%d len=256", names[i]);
		confstr_case(names[i], 256, true, ctx);
		std::snprintf(ctx, sizeof ctx, "edge name=%d len=4096", names[i]);
		confstr_case(names[i], 4096, true, ctx);
	}
}

static void
test_confstr_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		int name;
		size_t len;
		bool use_buf;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		if ((randu32() % 8u) == 0u)
			name = (int)(randu32() % 32u);
		else if ((randu32() % 8u) == 1u)
			name = (int)_CS_PATH;
		else if ((randu32() % 8u) == 2u)
			name = (int)_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS;
		else if ((randu32() % 8u) == 3u)
			name = (int)_CS_POSIX_V6_LP64_OFF64_CFLAGS;
		else if ((randu32() % 8u) == 4u)
			name = (int)_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS;
		else
			name = (int)(randu32() % 20000u) - 5000;
		len = (size_t)(randu32() % 512u);
		use_buf = (randu32() & 7u) != 0u;
		confstr_case(name, len, use_buf, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* statvfs / fstatvfs                                                 */
/* ------------------------------------------------------------------ */

static bool
statvfs_bufs_equal(const GuardStatvfs &p, const GuardStatvfs &r)
{
	return std::memcmp(&p.sv, &r.sv, sizeof(struct statvfs)) == 0;
}

static void
check_statvfs(const char *path, GuardStatvfs &gp, GuardStatvfs &gr,
    const char *ctx)
{
	int p, r;
	int e0 = errno;
	int pe, re;

	errno = 0;
	p = P::statvfs(path, &gp.sv);
	pe = errno;
	errno = 0;
	r = ref_statvfs(path, &gr.sv);
	re = errno;

	ncases[F_STATVFS]++;
	if (p != r) {
		char msg[96];

		std::snprintf(msg, sizeof msg, "ret port=%d ref=%d path=%s", p, r,
		    path != nullptr ? path : "(null)");
		report(F_STATVFS, ctx, msg);
	}
	if (pe != re) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "errno port=%d ref=%d", pe, re);
		report(F_STATVFS, ctx, msg);
	}
	if (p == 0 && r == 0 && !statvfs_bufs_equal(gp, gr))
		report(F_STATVFS, ctx, "statvfs struct mismatch");
	if (!gp.identical(gr))
		report(F_STATVFS, ctx, "guard mismatch");
	errno = e0;
}

static void
check_fstatvfs(int fd, GuardStatvfs &gp, GuardStatvfs &gr, const char *ctx)
{
	int p, r;
	int e0 = errno;
	int pe, re;

	errno = 0;
	p = P::fstatvfs(fd, &gp.sv);
	pe = errno;
	errno = 0;
	r = ref_fstatvfs(fd, &gr.sv);
	re = errno;

	ncases[F_FSTATVFS]++;
	if (p != r) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "ret port=%d ref=%d fd=%d", p, r,
		    fd);
		report(F_FSTATVFS, ctx, msg);
	}
	if (pe != re) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "errno port=%d ref=%d", pe, re);
		report(F_FSTATVFS, ctx, msg);
	}
	if (p == 0 && r == 0 && !statvfs_bufs_equal(gp, gr))
		report(F_FSTATVFS, ctx, "statvfs struct mismatch");
	if (!gp.identical(gr))
		report(F_FSTATVFS, ctx, "guard mismatch");
	errno = e0;
}

static void
test_statvfs_edges(void)
{
	static const char *paths[] = {
		"/",
		"/tmp",
		"/proc",
		"/nonexistent_pbsd_b0286_path",
		"",
	};

	for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
		GuardStatvfs gp, gr;
		char ctx[64];

		std::snprintf(ctx, sizeof ctx, "edge path \"%s\"", paths[i]);
		gp.init();
		gr.init();
		check_statvfs(paths[i], gp, gr, ctx);
	}

	{
		GuardStatvfs gp, gr;
		static const unsigned char hb[] = { '/', 0x80, 't', 'm', 'p', 0xff };

		gp.init();
		gr.init();
		GuardBuf path_p, path_r;

		path_p.init();
		path_r.init();
		std::memcpy(path_p.win(), hb, sizeof hb);
		path_p.win()[sizeof hb] = '\0';
		std::memcpy(path_r.win(), hb, sizeof hb);
		check_statvfs(path_p.win(), gp, gr, "high-bit path");
	}

	ncases[F_STATVFS]++;
	{
		GuardStatvfs gp, gr;

		gp.init();
		gr.init();
		check_statvfs(nullptr, gp, gr, "null path");
	}
}

static void
test_fstatvfs_edges(void)
{
	int fd_tmp = open("/tmp", O_RDONLY | O_DIRECTORY);
	int fd_bad = -1;
	char ctx[64];

	if (fd_tmp >= 0) {
		GuardStatvfs gp, gr;

		gp.init();
		gr.init();
		check_fstatvfs(fd_tmp, gp, gr, "edge fd /tmp");
		close(fd_tmp);
	}

	{
		GuardStatvfs gp, gr;

		gp.init();
		gr.init();
		check_fstatvfs(fd_bad, gp, gr, "edge fd -1");
	}

	{
		GuardStatvfs gp, gr;
		int fd = open("/proc/self", O_RDONLY);

		if (fd >= 0) {
			gp.init();
			gr.init();
			std::snprintf(ctx, sizeof ctx, "edge fd %d", fd);
			check_fstatvfs(fd, gp, gr, ctx);
			close(fd);
		}
	}
}

static const char *
pick_path(std::uint32_t r)
{
	static const char *pool[] = {
		"/",
		"/tmp",
		"/proc",
		"/proc/self",
		"/etc",
		"/var",
		"/nonexistent_pbsd_b0286",
		"/dev",
		"/home",
	};

	if ((r % 16u) == 0u)
		return nullptr;
	return pool[r % (sizeof pool / sizeof pool[0])];
}

static void
test_statvfs_random(void)
{
	for (int i = 0; i < 200000; i++) {
		GuardStatvfs gp, gr;
		char ctx[48];
		const char *path;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		gp.init();
		gr.init();
		path = pick_path(randu32());
		check_statvfs(path, gp, gr, ctx);
	}
}

static void
test_fstatvfs_random(void)
{
	for (int i = 0; i < 200000; i++) {
		GuardStatvfs gp, gr;
		char ctx[48];
		int fd;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		gp.init();
		gr.init();
		if ((randu32() % 8u) == 0u) {
			fd = -1;
		} else if ((randu32() % 4u) == 0u) {
			fd = open("/tmp", O_RDONLY | O_DIRECTORY);
		} else if ((randu32() % 4u) == 1u) {
			fd = open("/proc/self", O_RDONLY);
		} else {
			fd = (int)(randu32() % 4096);
		}
		check_fstatvfs(fd, gp, gr, ctx);
		if (fd >= 0 &&
		    (fd == open("/tmp", O_RDONLY | O_DIRECTORY) ||
		    fd == open("/proc/self", O_RDONLY)))
			close(fd);
		if (fd >= 0 && (randu32() % 4u) != 3u)
			close(fd);
	}
}

/* ------------------------------------------------------------------ */
/* __timezone_compat                                                  */
/* ------------------------------------------------------------------ */

static void
check_timezone(int zone, int dst, const char *ctx)
{
	char *p, *r;
	const char *tzbase_p, *tzbase_r;
	int e0 = errno;

	ncases[F_TIMEZONE]++;
	errno = 0;
	p = P::__timezone_compat(zone, dst);
	tzbase_p = std::getenv("TZNAME");
	errno = 0;
	r = ref___timezone_compat(zone, dst);
	tzbase_r = std::getenv("TZNAME");

	if (p == nullptr && r == nullptr)
		goto done;
	if (p == nullptr || r == nullptr) {
		report(F_TIMEZONE, ctx, "null mismatch");
		goto done;
	}

	if (tzbase_p != nullptr && tzbase_r != nullptr &&
	    (p >= tzbase_p && p < tzbase_p + std::strlen(tzbase_p) + 1) &&
	    (r >= tzbase_r && r < tzbase_r + std::strlen(tzbase_r) + 1)) {
		if (!ptr_offset_ok(p, tzbase_p, r, tzbase_r)) {
			char msg[96];

			std::snprintf(msg, sizeof msg,
			    "TZNAME offset port=%td ref=%td",
			    p - tzbase_p, r - tzbase_r);
			report(F_TIMEZONE, ctx, msg);
		}
	} else if (std::strcmp(p, r) != 0) {
		char msg[160];

		std::snprintf(msg, sizeof msg, "port=\"%s\" ref=\"%s\"", p, r);
		report(F_TIMEZONE, ctx, msg);
	}

done:
	errno = e0;
}

static void
test_timezone_edges(void)
{
	static const int zones[] = {
		-60, -120, 240, 300, 360, 420, 480, 0, -600, -570, -480,
		1, -1, 90, -90, 180, -180, 59, -59, 1439, -1439,
		7 * 60, 8 * 60 + 1, -8 * 60,
	};

	save_tzname();
	(void)unsetenv("TZNAME");

	for (size_t i = 0; i < sizeof zones / sizeof zones[0]; i++) {
		for (int dst = 0; dst <= 1; dst++) {
			char ctx[64];

			std::snprintf(ctx, sizeof ctx, "edge zone=%d dst=%d noenv",
			    zones[i], dst);
			check_timezone(zones[i], dst, ctx);
		}
	}

	set_tzname_value("PST");
	check_timezone(999, 0, "env single PST dst=0");
	check_timezone(999, 1, "env single PST dst=1");

	set_tzname_value("PST,PDT");
	check_timezone(999, 0, "env PST,PDT dst=0");
	check_timezone(999, 1, "env PST,PDT dst=1");

	set_tzname_value("\x80\xff,P\x80");
	check_timezone(0, 0, "env high-bit dst=0");
	check_timezone(0, 1, "env high-bit dst=1");

	set_tzname_value("A");
	check_timezone(0, 0, "env single char");

	set_tzname_value(",B");
	check_timezone(0, 0, "env leading comma dst=0");
	check_timezone(0, 1, "env leading comma dst=1");

	restore_tzname();
}

static void
test_timezone_random(void)
{
	save_tzname();

	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		char tzbuf[64];
		int zone, dst;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		zone = (int)(randu32() % 3000u) - 1500;
		dst = (int)(randu32() & 1u);

		if ((randu32() % 4u) == 0u) {
			(void)unsetenv("TZNAME");
		} else {
			size_t n = (size_t)(randu32() % (sizeof tzbuf - 1));
			size_t j;

			for (j = 0; j < n; j++)
				tzbuf[j] = (char)(1 + (randu32() % 254u));
			tzbuf[n] = '\0';
			if ((randu32() % 5u) == 0u && n > 0) {
				size_t comma = (size_t)(randu32() % n);

				tzbuf[comma] = ',';
			}
			set_tzname_value(tzbuf);
		}
		check_timezone(zone, dst, ctx);
	}

	restore_tzname();
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

	test_confstr_edges();
	test_confstr_random();
	test_statvfs_edges();
	test_statvfs_random();
	test_fstatvfs_edges();
	test_fstatvfs_random();
	test_timezone_edges();
	test_timezone_random();

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
