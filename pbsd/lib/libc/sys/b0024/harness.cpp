/*
 * Differential test harness for batch b0024.
 *
 * stat, lstat, mknod and _Fork are thin syscall wrappers: each forwards a fixed
 * argument list to __sys_fstatat, __sys_mknodat or __sys_fork.  The harness
 * supplies recording mocks for those three primitives so that a wrong constant
 * (AT_FDCWD vs something else, 0 vs AT_SYMLINK_NOFOLLOW), a transposed
 * argument, or a different target primitive all change both the return value
 * and every byte the mock writes through caller-supplied buffers.
 */

#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.sys.b0024;

namespace port = pbsd::lib_libc_sys::b0024;

extern "C" {

int ref_stat(const char *path, struct stat *sb);
int ref_lstat(const char *path, struct stat *sb);
int ref_mknod(const char *path, mode_t mode, dev_t dev);
pid_t ref__Fork(void);

}

/* ------------------------------------------------------------------ */
/* Recording syscall mocks (linked by both oracle and port).          */
/* ------------------------------------------------------------------ */

#define	GUARD			0x7f
#define	STAT_PAD		256
#define	PATH_CAP		512
#define	PATH_GUARD_PAD		64

struct FstatatRec {
	int		calls;
	int		dirfd;
	const char	*path;
	const void	*sb_p;
	int		flags;
	int		ret;
};

struct MknodatRec {
	int		calls;
	int		dirfd;
	const char	*path;
	mode_t		mode;
	dev_t		dev;
	int		ret;
};

struct ForkRec {
	int		calls;
	pid_t		ret;
};

static FstatatRec g_fstatat;
static MknodatRec g_mknodat;
static ForkRec g_fork;
static uint32_t g_fork_case_seed;

static void
fstatat_reset(void)
{
	g_fstatat.calls = 0;
	g_fstatat.dirfd = -0x5a5a;
	g_fstatat.path = nullptr;
	g_fstatat.sb_p = nullptr;
	g_fstatat.flags = -0x5a59;
	g_fstatat.ret = -0x5a58;
}

static void
mknodat_reset(void)
{
	g_mknodat.calls = 0;
	g_mknodat.dirfd = -0x6b6b;
	g_mknodat.path = nullptr;
	g_mknodat.mode = static_cast<mode_t>(~0u);
	g_mknodat.dev = static_cast<dev_t>(~0u);
	g_mknodat.ret = -0x6b68;
}

static void
fork_reset(void)
{
	g_fork.calls = 0;
	g_fork.ret = static_cast<pid_t>(-0x7c7c);
}

static unsigned
hash_path(const char *path)
{
	unsigned h = 2166136261u;
	if (path == nullptr)
		return h;
	for (int i = 0; i < PATH_CAP && path[i] != '\0'; i++) {
		h ^= static_cast<unsigned char>(path[i]);
		h *= 16777619u;
	}
	return h;
}

static int
mock_fstatat_body(int dirfd, const char *path, struct stat *sb, int flags)
{
	unsigned h = hash_path(path);
	h ^= static_cast<unsigned>(dirfd) * 2654435761u;
	h ^= static_cast<unsigned>(flags) * 2246822519u;
	h ^= (sb != nullptr) ? 0x9e3779b9u : 0x517cc1b7u;

	if (sb != nullptr) {
		unsigned char *p = reinterpret_cast<unsigned char *>(sb);
		for (size_t i = 0; i < sizeof(struct stat); i++) {
			p[i] = static_cast<unsigned char>(
			    (h >> (i & 7)) ^ (flags * 31u) ^
			    (dirfd * 17u) + static_cast<unsigned>(i) * 7u +
			    0xa5u);
		}
	}

	int ret = static_cast<int>(h ^ (h >> 16));
	if (dirfd != AT_FDCWD)
		ret = -ret - 1;
	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW)
		ret += static_cast<int>(flags);

	g_fstatat.calls++;
	g_fstatat.dirfd = dirfd;
	g_fstatat.path = path;
	g_fstatat.sb_p = sb;
	g_fstatat.flags = flags;
	g_fstatat.ret = ret;
	return ret;
}

static int
mock_mknodat_body(int dirfd, const char *path, mode_t mode, dev_t dev)
{
	unsigned h = hash_path(path);
	h ^= static_cast<unsigned>(dirfd) * 40503u;
	h ^= static_cast<unsigned>(mode) * 12289u;
	h ^= static_cast<unsigned>(dev) * 65521u;

	int ret = static_cast<int>(h ^ (h >> 11));
	if (dirfd != AT_FDCWD)
		ret = -ret - 3;
	ret += static_cast<int>(mode & 0xffu);
	ret ^= static_cast<int>(dev & 0xffffu);

	g_mknodat.calls++;
	g_mknodat.dirfd = dirfd;
	g_mknodat.path = path;
	g_mknodat.mode = mode;
	g_mknodat.dev = dev;
	g_mknodat.ret = ret;
	return ret;
}

static pid_t
mock_fork_body(void)
{
	unsigned seq = 0x31415926u + g_fork_case_seed * 1103515245u;
	seq ^= static_cast<unsigned>(g_fork.calls) * 12345u;
	pid_t ret = static_cast<pid_t>(seq & 0x7fffffffu);
	if (ret <= 0)
		ret = 42;

	g_fork.calls++;
	g_fork.ret = ret;
	return ret;
}

extern "C" int
__sys_fstatat(int dirfd, const char *path, struct stat *sb, int flags)
{
	return mock_fstatat_body(dirfd, path, sb, flags);
}

extern "C" int
__sys_mknodat(int dirfd, const char *path, mode_t mode, dev_t dev)
{
	return mock_mknodat_body(dirfd, path, mode, dev);
}

extern "C" pid_t
__sys_fork(void)
{
	return mock_fork_body();
}

/* ------------------------------------------------------------------ */
/* Per-function statistics                                              */
/* ------------------------------------------------------------------ */

enum Fn {
	FN_STAT,
	FN_LSTAT,
	FN_MKNOD,
	FN_FORK,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"stat",
	"lstat",
	"mknod",
	"_Fork",
};

static long long fn_cases[FN_COUNT];
static long long fn_fails[FN_COUNT];
static int fn_reported[FN_COUNT];

static void
report_fail(Fn fn, long long caseno, const char *what)
{
	if (fn_reported[fn] >= 12)
		return;
	fn_reported[fn]++;
	printf("FAIL[%lld] %s: %s\n", caseno, fn_name[fn], what);
}

static long long
ptr_off(const void *p, const void *base)
{
	if (p == nullptr)
		return -1;
	return static_cast<long long>(
	    reinterpret_cast<intptr_t>(p) - reinterpret_cast<intptr_t>(base));
}

/* ------------------------------------------------------------------ */
/* PRNG                                                                 */
/* ------------------------------------------------------------------ */

static uint64_t g_rng;

static void
rng_seed(uint64_t s)
{
	g_rng = s;
}

static uint32_t
rng_u32(void)
{
	uint64_t x = g_rng;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	g_rng = x;
	return static_cast<uint32_t>(x >> 32);
}

/* ------------------------------------------------------------------ */
/* Path / stat buffer helpers                                           */
/* ------------------------------------------------------------------ */

struct PathBuf {
	unsigned char bytes[PATH_GUARD_PAD + PATH_CAP + PATH_GUARD_PAD];
};

struct StatBuf {
	unsigned char bytes[STAT_PAD + sizeof(struct stat) + STAT_PAD];
};

static void
fill_path_buf(PathBuf &pb, const unsigned char *src, int len)
{
	memset(pb.bytes, GUARD, sizeof(pb.bytes));
	if (len < 0)
		len = 0;
	if (len > PATH_CAP)
		len = PATH_CAP;
	if (src != nullptr && len > 0)
		memcpy(pb.bytes + PATH_GUARD_PAD, src, static_cast<size_t>(len));
	pb.bytes[PATH_GUARD_PAD + len] = '\0';
}

static const char *
path_ptr(PathBuf &pb)
{
	return reinterpret_cast<const char *>(pb.bytes + PATH_GUARD_PAD);
}

static void
fill_stat_buf(StatBuf &sb, uint32_t seed)
{
	memset(sb.bytes, GUARD, sizeof(sb.bytes));
	uint32_t h = seed;
	for (size_t i = 0; i < sizeof(struct stat); i++) {
		h = h * 1103515245u + 12345u;
		sb.bytes[STAT_PAD + i] = static_cast<unsigned char>(h >> 16);
	}
}

static struct stat *
stat_ptr(StatBuf &sb, int off)
{
	return reinterpret_cast<struct stat *>(sb.bytes + STAT_PAD + off);
}

static const int STAT_OFFS[] = {
	0,
	4,
	16,
	static_cast<int>(STAT_PAD - sizeof(struct stat))
};
static const int N_STAT_OFFS = 4;

static const int EDGE_INTS[] = {
	INT_MIN, INT_MIN + 1, -65536, -256, -128, -2, -1, 0, 1, 2, 3, 7,
	0x7f, 0x80, 0xff, 0x100, 65535, 65536, INT_MAX - 1, INT_MAX
};
static const int NEDGE = static_cast<int>(sizeof(EDGE_INTS) / sizeof(EDGE_INTS[0]));

static const unsigned char EDGE_PATHS[][8] = {
	{ 0 },
	{ 'a', 0 },
	{ '\0', '\0', '\0', 0 },
	{ 0x80, 0 },
	{ 0xff, 0 },
	{ 'a', 'b', 'c', 0 },
	{ 'a', '\0', 'b', 0 },
	{ 0x80, 0xff, 0x7f, 0x00, 0xfe, 0 },
};

static const int EDGE_PATH_LENS[] = {
	0, 1, 3, 1, 1, 3, 3, 5
};
static const int N_EDGE_PATHS =
    static_cast<int>(sizeof(EDGE_PATHS) / sizeof(EDGE_PATHS[0]));

/* ------------------------------------------------------------------ */
/* stat / lstat cases                                                   */
/* ------------------------------------------------------------------ */

static void
run_fstatat_case(Fn fn, const char *path, StatBuf &port_sb, StatBuf &ref_sb,
    struct stat *port_sp, struct stat *ref_sp, uint32_t tag)
{
	FstatatRec rec_port, rec_ref;
	int ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	fstatat_reset();
	fork_reset();
	mknodat_reset();

	if (fn == FN_STAT)
		ret_port = port::stat(path, port_sp);
	else
		ret_port = port::lstat(path, port_sp);
	rec_port = g_fstatat;

	fstatat_reset();
	fork_reset();
	mknodat_reset();

	if (fn == FN_STAT)
		ret_ref = ref_stat(path, ref_sp);
	else
		ret_ref = ref_lstat(path, ref_sp);
	rec_ref = g_fstatat;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.dirfd != rec_ref.dirfd) {
		report_fail(fn, n, "dirfd");
		bad = 1;
	}
	if (rec_port.dirfd != AT_FDCWD) {
		report_fail(fn, n, "dirfd not AT_FDCWD");
		bad = 1;
	}
	if (rec_port.flags != rec_ref.flags) {
		report_fail(fn, n, "flags");
		bad = 1;
	}
	if (fn == FN_STAT && rec_port.flags != 0) {
		report_fail(fn, n, "stat flags not 0");
		bad = 1;
	}
	if (fn == FN_LSTAT && rec_port.flags != AT_SYMLINK_NOFOLLOW) {
		report_fail(fn, n, "lstat flags not AT_SYMLINK_NOFOLLOW");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (ptr_off(rec_port.path, path) != ptr_off(rec_ref.path, path)) {
		report_fail(fn, n, "path pointer offset");
		bad = 1;
	}
	if (port_sp != nullptr && ref_sp != nullptr) {
		if (ptr_off(rec_port.sb_p, port_sb.bytes) !=
		    ptr_off(rec_ref.sb_p, ref_sb.bytes)) {
			report_fail(fn, n, "stat pointer offset");
			bad = 1;
		}
	} else if (rec_port.sb_p != rec_ref.sb_p) {
		report_fail(fn, n, "stat pointer null mismatch");
		bad = 1;
	}
	if (memcmp(port_sb.bytes, ref_sb.bytes, sizeof(port_sb.bytes)) != 0) {
		report_fail(fn, n, "stat guard buffer");
		bad = 1;
	}
	if (g_mknodat.calls != 0 || g_fork.calls != 0) {
		report_fail(fn, n, "wrong syscall invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
run_stat_pair(Fn fn, PathBuf &port_path, PathBuf &ref_path, bool use_sb,
    int stat_off, uint32_t tag)
{
	StatBuf port_sb, ref_sb;
	fill_stat_buf(port_sb, tag);
	fill_stat_buf(ref_sb, tag);

	struct stat *port_sp = use_sb ? stat_ptr(port_sb, stat_off) : nullptr;
	struct stat *ref_sp = use_sb ? stat_ptr(ref_sb, stat_off) : nullptr;

	run_fstatat_case(fn, path_ptr(port_path), port_sb, ref_sb, port_sp,
	    ref_sp, tag);
}

static void
edge_stat_cases(Fn fn)
{
	for (int pi = 0; pi < N_EDGE_PATHS; pi++) {
		for (int so = 0; so < N_STAT_OFFS; so++) {
			for (int null_sb = 0; null_sb < 2; null_sb++) {
				PathBuf pa, pb;
				fill_path_buf(pa, EDGE_PATHS[pi],
				    EDGE_PATH_LENS[pi]);
				fill_path_buf(pb, EDGE_PATHS[pi],
				    EDGE_PATH_LENS[pi]);
				run_stat_pair(fn, pa, pb, null_sb == 0,
				    STAT_OFFS[so],
				    static_cast<uint32_t>(pi * 7919u + so * 97u +
					fn * 13u + 1u));
			}
		}
	}

	for (int i = 0; i < NEDGE; i++) {
		PathBuf pa, pb;
		unsigned char ch = static_cast<unsigned char>(EDGE_INTS[i]);
		fill_path_buf(pa, &ch, 1);
		fill_path_buf(pb, &ch, 1);
		run_stat_pair(fn, pa, pb, true, STAT_OFFS[i % N_STAT_OFFS],
		    static_cast<uint32_t>(0xface0000u + i));
	}
}

static void
random_stat_sweep(Fn fn, long long iters)
{
	for (long long n = 0; n < iters; n++) {
		PathBuf pa, pb;
		int len = static_cast<int>(rng_u32() % (PATH_CAP + 1));
		unsigned char tmp[PATH_CAP];
		for (int i = 0; i < len; i++) {
			uint32_t r = rng_u32();
			if ((r & 7u) == 0u)
				tmp[i] = '\0';
			else if ((r & 3u) == 0u)
				tmp[i] = static_cast<unsigned char>(
				    0x80u | (r & 0x7fu));
			else
				tmp[i] = static_cast<unsigned char>(r & 0xffu);
		}
		fill_path_buf(pa, tmp, len);
		fill_path_buf(pb, tmp, len);

		bool use_sb = (rng_u32() & 15u) != 0u;
		int off = static_cast<int>(rng_u32() % STAT_PAD);
		if (off > STAT_PAD)
			off = STAT_PAD;
		run_stat_pair(fn, pa, pb, use_sb, off, rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* mknod cases                                                          */
/* ------------------------------------------------------------------ */

static void
run_mknod_case(const char *path, mode_t mode, dev_t dev, uint32_t tag)
{
	Fn fn = FN_MKNOD;
	MknodatRec rec_port, rec_ref;
	int ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	fstatat_reset();
	mknodat_reset();
	fork_reset();

	ret_port = port::mknod(path, mode, dev);
	rec_port = g_mknodat;

	fstatat_reset();
	mknodat_reset();
	fork_reset();

	ret_ref = ref_mknod(path, mode, dev);
	rec_ref = g_mknodat;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.dirfd != rec_ref.dirfd || rec_port.dirfd != AT_FDCWD) {
		report_fail(fn, n, "dirfd");
		bad = 1;
	}
	if (rec_port.mode != rec_ref.mode) {
		report_fail(fn, n, "mode");
		bad = 1;
	}
	if (rec_port.dev != rec_ref.dev) {
		report_fail(fn, n, "dev");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (ptr_off(rec_port.path, path) != ptr_off(rec_ref.path, path)) {
		report_fail(fn, n, "path pointer offset");
		bad = 1;
	}
	if (g_fstatat.calls != 0 || g_fork.calls != 0) {
		report_fail(fn, n, "wrong syscall invoked");
		bad = 1;
	}
	(void)tag;

	if (bad)
		fn_fails[fn]++;
}

static void
edge_mknod_cases(void)
{
	for (int pi = 0; pi < N_EDGE_PATHS; pi++) {
		for (int i = 0; i < NEDGE; i++) {
			for (int j = 0; j < NEDGE; j++) {
				PathBuf pa, pb;
				fill_path_buf(pa, EDGE_PATHS[pi],
				    EDGE_PATH_LENS[pi]);
				fill_path_buf(pb, EDGE_PATHS[pi],
				    EDGE_PATH_LENS[pi]);
				run_mknod_case(path_ptr(pa),
				    static_cast<mode_t>(EDGE_INTS[i]),
				    static_cast<dev_t>(EDGE_INTS[j]),
				    static_cast<uint32_t>(pi * 100003u + i * 17u +
					j));
			}
		}
	}
}

static void
random_mknod_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		PathBuf pa, pb;
		int len = static_cast<int>(rng_u32() % (PATH_CAP + 1));
		unsigned char tmp[PATH_CAP];
		for (int i = 0; i < len; i++)
			tmp[i] = static_cast<unsigned char>(rng_u32() & 0xffu);
		fill_path_buf(pa, tmp, len);
		fill_path_buf(pb, tmp, len);

		mode_t mode = static_cast<mode_t>(rng_u32());
		dev_t dev = static_cast<dev_t>(rng_u32());
		if ((rng_u32() & 3u) == 0u)
			mode = static_cast<mode_t>(rng_u32() & 0xffu);
		if ((rng_u32() & 3u) == 0u)
			dev = static_cast<dev_t>(rng_u32() & 0xffffu);

		run_mknod_case(path_ptr(pa), mode, dev, rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* _Fork cases                                                          */
/* ------------------------------------------------------------------ */

static void
run_fork_case(uint32_t tag)
{
	Fn fn = FN_FORK;
	ForkRec rec_port, rec_ref;
	pid_t ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	g_fork_case_seed = tag;

	fstatat_reset();
	mknodat_reset();
	fork_reset();

	ret_port = port::_Fork();
	rec_port = g_fork;

	fstatat_reset();
	mknodat_reset();
	fork_reset();

	ret_ref = ref__Fork();
	rec_ref = g_fork;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (g_fstatat.calls != 0 || g_mknodat.calls != 0) {
		report_fail(fn, n, "wrong syscall invoked");
		bad = 1;
	}
	(void)tag;

	if (bad)
		fn_fails[fn]++;
}

static void
random_fork_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++)
		run_fork_case(rng_u32());
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	rng_seed(0x0024002400240024ULL);

	edge_stat_cases(FN_STAT);
	edge_stat_cases(FN_LSTAT);
	edge_mknod_cases();
	for (int i = 0; i < 64; i++)
		run_fork_case(static_cast<uint32_t>(i * 1234567u));

	random_stat_sweep(FN_STAT, 200000);
	random_stat_sweep(FN_LSTAT, 200000);
	random_mknod_sweep(200000);
	random_fork_sweep(200000);

	printf("\n%-24s %12s %12s %s\n", "FUNCTION", "CASES", "FAILURES",
	    "RESULT");
	printf("%-24s %12s %12s %s\n", "------------------------",
	    "------------", "------------", "------");

	long long total_cases = 0;
	long long total_fail = 0;

	for (int i = 0; i < FN_COUNT; i++) {
		printf("%-24s %12lld %12lld %s\n", fn_name[i], fn_cases[i],
		    fn_fails[i], fn_fails[i] == 0 ? "PASS" : "FAIL");
		total_cases += fn_cases[i];
		total_fail += fn_fails[i];
	}

	printf("%-24s %12lld %12lld %s\n", "TOTAL", total_cases, total_fail,
	    total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
