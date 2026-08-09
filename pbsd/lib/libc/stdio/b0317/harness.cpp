/*
 * harness.cpp -- differential test for PBSD batch b0317.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>

import pbsd.lib.libc.stdio.b0317;

namespace port = pbsd::lib_libc_stdio::b0317;

extern "C" {
char *ref_tempnam(const char *, const char *);
int ref_vsnprintf_l(char *__restrict, size_t, locale_t, const char *__restrict,
    va_list);
int ref_vsnprintf(char *__restrict, size_t, const char *__restrict, va_list);
int ref___printf_arginfo_quote(const printf_info *, size_t, int *);
int ref___printf_render_quote(__printf_io *, const printf_info *,
    const void *const *);

extern unsigned char tempnam_guard_before[32];
extern char tempnam_alloc_buf[];
extern unsigned char tempnam_guard_after[32];
extern int mock_malloc_fail;
extern int mock_malloc_calls;
extern int mock_free_calls;
extern char *mock_secure_getenv_val;
extern int mock_secure_getenv_calls;
extern unsigned mock_mktemp_calls;
extern int mock_mktemp_succeed_at;
extern char *mock_mktemp_last_name;

extern locale_t pbsd_shim_locale_c;
extern locale_t pbsd_shim_locale_utf8;
locale_t pbsd_shim_get_locale(void);
int pbsd_shim_init(void);
extern locale_t pbsd_shim_vfprintf_locale;
extern int pbsd_shim_vfprintf_serrno;
extern unsigned long long pbsd_shim_vfprintf_calls;
extern short pbsd_shim_vfprintf_entry_flags;
extern short pbsd_shim_vfprintf_entry_file;
extern int pbsd_shim_vfprintf_entry_r;
extern int pbsd_shim_vfprintf_entry_w;
extern int pbsd_shim_vfprintf_entry_size;
extern int pbsd_shim_vfprintf_entry_lbfsize;
extern int pbsd_shim_vfprintf_entry_orientation;
extern unsigned char *pbsd_shim_vfprintf_entry_base;
extern unsigned char *pbsd_shim_vfprintf_entry_p;

extern int mock_printf_out_ret;
extern int mock_printf_out_calls;
extern __printf_io *mock_printf_out_last_io;
extern const printf_info *mock_printf_out_last_pi;
extern const char *mock_printf_out_last_buf;
extern int mock_printf_out_last_len;
extern int mock_printf_flush_calls;
extern __printf_io *mock_printf_flush_last_io;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t STR_PRE = 32;
constexpr std::size_t STR_BUF = 4096;
constexpr std::size_t STR_POST = 32;
constexpr std::size_t STR_CAP = STR_PRE + STR_BUF + STR_POST;
constexpr unsigned RAND_ITERS = 50000u;
constexpr int MAXPRINT = 8;
constexpr short SHIM_SWR = 0x0008;
constexpr short SHIM_SSTR = 0x0200;

enum StatId {
	S_TEMPNAM,
	S_VSNPRINTF_L,
	S_VSNPRINTF,
	S_ARGINFO_QUOTE,
	S_RENDER_QUOTE,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "tempnam",                0, 0, 0 },
	{ "vsnprintf_l",            0, 0, 0 },
	{ "vsnprintf",              0, 0, 0 },
	{ "__printf_arginfo_quote", 0, 0, 0 },
	{ "__printf_render_quote",  0, 0, 0 },
};

std::uint64_t rng_state = 0xb0317feedfaceULL;

std::uint64_t
rnd_u64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned
rnd_u32(void)
{
	return (unsigned)(rnd_u64() & 0xffffffffu);
}

void
fail_msg(StatId which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].printed++ < MAXPRINT)
		std::printf("  FAIL %-24s %-28s %s\n", g_stat[which].name, label,
		    detail);
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

struct GuardedBuf {
	unsigned char bytes[STR_CAP];

	void fill_guard(void)
	{
		std::memset(bytes, GUARD, STR_CAP);
	}

	char *user(void)
	{
		return reinterpret_cast<char *>(bytes + STR_PRE);
	}

	bool eq(const GuardedBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, STR_CAP) == 0;
	}
};

struct TempnamArena {
	unsigned char before[32];
	char data[1024];
	unsigned char after[32];
};

static void
tempnam_fill_guard(void)
{
	std::memset(tempnam_guard_before, GUARD, sizeof(tempnam_guard_before));
	std::memset(tempnam_alloc_buf, GUARD, 1024);
	std::memset(tempnam_guard_after, GUARD, sizeof(tempnam_guard_after));
}

static bool
tempnam_arena_eq(void)
{
	unsigned i;

	for (i = 0; i < sizeof(tempnam_guard_before); i++) {
		if (tempnam_guard_before[i] != GUARD)
			return (false);
	}
	for (i = 0; i < sizeof(tempnam_guard_after); i++) {
		if (tempnam_guard_after[i] != GUARD)
			return (false);
	}
	return (true);
}

struct TempnamSnap {
	char *ret;
	int errno_after;
	int malloc_calls;
	int free_calls;
	int getenv_calls;
	unsigned mktemp_calls;
	char alloc_buf[1024];
};

static void
tempnam_reset(int malloc_fail, char *tmpdir, int succeed_at, int set_errno)
{
	mock_malloc_fail = malloc_fail;
	mock_malloc_calls = 0;
	mock_free_calls = 0;
	mock_secure_getenv_val = tmpdir;
	mock_secure_getenv_calls = 0;
	mock_mktemp_calls = 0;
	mock_mktemp_succeed_at = succeed_at;
	mock_mktemp_last_name = nullptr;
	tempnam_fill_guard();
	errno = set_errno;
}

static TempnamSnap
tempnam_snap(char *ret)
{
	TempnamSnap s;

	s.ret = ret;
	s.errno_after = errno;
	s.malloc_calls = mock_malloc_calls;
	s.free_calls = mock_free_calls;
	s.getenv_calls = mock_secure_getenv_calls;
	s.mktemp_calls = mock_mktemp_calls;
	std::memcpy(s.alloc_buf, tempnam_alloc_buf, sizeof(s.alloc_buf));
	return (s);
}

static bool
test_tempnam_one(StatId which, const char *label, const char *dir,
    const char *pfx, int malloc_fail, char *tmpdir, int succeed_at,
    int set_errno)
{
	char *ret_p, *ret_r;
	TempnamSnap snap_p, snap_r;
	ptrdiff_t off_p, off_r;

	case_inc(which);

	tempnam_reset(malloc_fail, tmpdir, succeed_at, set_errno);
	ret_p = port::tempnam(dir, pfx);
	snap_p = tempnam_snap(ret_p);

	tempnam_reset(malloc_fail, tmpdir, succeed_at, set_errno);
	ret_r = ref_tempnam(dir, pfx);
	snap_r = tempnam_snap(ret_r);

	if ((ret_p == nullptr) != (ret_r == nullptr)) {
		fail_msg(which, label, "null return mismatch");
		return (false);
	}
	off_p = ret_p ? (ret_p - tempnam_alloc_buf) : 0;
	off_r = ret_r ? (ret_r - tempnam_alloc_buf) : 0;
	if (off_p != off_r) {
		fail_msg(which, label, "return offset mismatch");
		return (false);
	}
	if (snap_p.errno_after != snap_r.errno_after) {
		fail_msg(which, label, "errno mismatch");
		return (false);
	}
	if (snap_p.malloc_calls != snap_r.malloc_calls) {
		fail_msg(which, label, "malloc call count mismatch");
		return (false);
	}
	if (snap_p.free_calls != snap_r.free_calls) {
		fail_msg(which, label, "free call count mismatch");
		return (false);
	}
	if (snap_p.getenv_calls != snap_r.getenv_calls) {
		fail_msg(which, label, "getenv call count mismatch");
		return (false);
	}
	if (snap_p.mktemp_calls != snap_r.mktemp_calls) {
		fail_msg(which, label, "mktemp call count mismatch");
		return (false);
	}
	if (std::memcmp(snap_p.alloc_buf, snap_r.alloc_buf,
	    sizeof(snap_p.alloc_buf)) != 0) {
		fail_msg(which, label, "alloc buffer mismatch");
		return (false);
	}
	if (!tempnam_arena_eq()) {
		fail_msg(which, label, "tempnam guard corruption");
		return (false);
	}
	return (true);
}

static void
handwritten_tempnam(void)
{
	test_tempnam_one(S_TEMPNAM, "malloc_fail", "/var/tmp", "pfx", 1,
	    nullptr, -1, EEXIST);
	test_tempnam_one(S_TEMPNAM, "tmpdir_ok", nullptr, nullptr, 0,
	    (char *)"/envtmp", 0, 0);
	test_tempnam_one(S_TEMPNAM, "tmpdir_slash", nullptr, "aa", 0,
	    (char *)"/env/", 0, 0);
	test_tempnam_one(S_TEMPNAM, "dir_ok", "/local/", "bb", 0, nullptr, 1,
	    0);
	test_tempnam_one(S_TEMPNAM, "dir_noslash", "/local", "cc", 0, nullptr,
	    1, 0);
	test_tempnam_one(S_TEMPNAM, "ptmpdir", nullptr, "dd", 0, nullptr, 2,
	    0);
	test_tempnam_one(S_TEMPNAM, "path_tmp", nullptr, "ee", 0, nullptr, 3,
	    0);
	test_tempnam_one(S_TEMPNAM, "all_fail", "/x", "ff", 0, (char *)"/y",
	    -1, EACCES);
	test_tempnam_one(S_TEMPNAM, "null_dir", nullptr, "gg", 0, nullptr, 2,
	    0);
	test_tempnam_one(S_TEMPNAM, "hi_pfx", "/tmp/", "\x80\xff", 0, nullptr,
	    1, 0);
}

static void
random_tempnam(void)
{
	char tmpdir[64];
	char dir[64];
	char pfx[16];
	char label[48];
	unsigned i;

	for (i = 0; i < RAND_ITERS; i++) {
		int malloc_fail, succeed_at, set_errno, use_tmpdir, use_dir;
		unsigned tlen, dlen, plen, j;

		malloc_fail = (int)(rnd_u32() % 16u == 0u);
		succeed_at = (int)(rnd_u32() % 8u) - 1;
		set_errno = (int)(rnd_u32() & 0xffu);
		use_tmpdir = (int)(rnd_u32() & 1u);
		use_dir = (int)(rnd_u32() & 1u);

		tlen = rnd_u32() % (sizeof(tmpdir) - 1u);
		for (j = 0; j < tlen; j++)
			tmpdir[j] = (char)(unsigned char)((rnd_u32() & 0x7fu) +
			    (rnd_u32() & 1u ? 0x80 : 0));
		tmpdir[tlen] = '\0';
		if (tlen > 0 && (rnd_u32() & 3u) == 0u)
			tmpdir[tlen - 1] = '/';

		dlen = rnd_u32() % (sizeof(dir) - 1u);
		for (j = 0; j < dlen; j++)
			dir[j] = (char)(unsigned char)(rnd_u32() & 0xffu);
		dir[dlen] = '\0';
		if (dlen > 0 && (rnd_u32() & 3u) == 0u)
			dir[dlen - 1] = '/';

		plen = rnd_u32() % (sizeof(pfx) - 1u);
		for (j = 0; j < plen; j++)
			pfx[j] = (char)(unsigned char)(rnd_u32() & 0xffu);
		pfx[plen] = '\0';

		std::snprintf(label, sizeof(label), "rand_%u", i);
		(void)test_tempnam_one(S_TEMPNAM, label,
		    use_dir ? dir : nullptr,
		    (rnd_u32() & 3u) == 0u ? nullptr : pfx,
		    malloc_fail, use_tmpdir ? tmpdir : nullptr, succeed_at,
		    set_errno);
	}
}

struct ShimSnap {
	locale_t locale;
	int serrno;
	unsigned long long calls;
	short flags;
	short file;
	int r;
	int w;
	int size;
	int lbfsize;
	int orientation;
	std::ptrdiff_t base_off;
	std::ptrdiff_t p_off;
	bool used_dummy;
};

static void
shim_reset(void)
{
	pbsd_shim_vfprintf_locale = (locale_t)0;
	pbsd_shim_vfprintf_serrno = -9999;
	pbsd_shim_vfprintf_calls = 0;
	pbsd_shim_vfprintf_entry_flags = -1;
	pbsd_shim_vfprintf_entry_file = -2;
	pbsd_shim_vfprintf_entry_r = -3;
	pbsd_shim_vfprintf_entry_w = -4;
	pbsd_shim_vfprintf_entry_size = -5;
	pbsd_shim_vfprintf_entry_lbfsize = -6;
	pbsd_shim_vfprintf_entry_orientation = -7;
	pbsd_shim_vfprintf_entry_base = (unsigned char *)-8;
	pbsd_shim_vfprintf_entry_p = (unsigned char *)-9;
}

static ShimSnap
shim_snap(const char *buf_base, size_t n)
{
	ShimSnap s;

	s.locale = pbsd_shim_vfprintf_locale;
	s.serrno = pbsd_shim_vfprintf_serrno;
	s.calls = pbsd_shim_vfprintf_calls;
	s.flags = pbsd_shim_vfprintf_entry_flags;
	s.file = pbsd_shim_vfprintf_entry_file;
	s.r = pbsd_shim_vfprintf_entry_r;
	s.w = pbsd_shim_vfprintf_entry_w;
	s.size = pbsd_shim_vfprintf_entry_size;
	s.lbfsize = pbsd_shim_vfprintf_entry_lbfsize;
	s.orientation = pbsd_shim_vfprintf_entry_orientation;
	s.used_dummy = (n != 0 && (n - 1) == 0) || (n == 1);
	if (!s.used_dummy) {
		s.base_off = pbsd_shim_vfprintf_entry_base -
		    reinterpret_cast<const unsigned char *>(buf_base);
		s.p_off = pbsd_shim_vfprintf_entry_p -
		    reinterpret_cast<const unsigned char *>(buf_base);
	} else {
		s.base_off = -1;
		s.p_off = -1;
	}
	return (s);
}

static bool
shim_eq(const ShimSnap &a, const ShimSnap &b, const char *label, StatId which)
{
	if (a.locale != b.locale) {
		fail_msg(which, label, "vfprintf locale mismatch");
		return (false);
	}
	if (a.serrno != b.serrno) {
		fail_msg(which, label, "vfprintf serrno mismatch");
		return (false);
	}
	if (a.calls != b.calls) {
		fail_msg(which, label, "vfprintf call count mismatch");
		return (false);
	}
	if (a.flags != b.flags) {
		fail_msg(which, label, "entry flags mismatch");
		return (false);
	}
	if (a.file != b.file) {
		fail_msg(which, label, "entry _file mismatch");
		return (false);
	}
	if (a.r != b.r) {
		fail_msg(which, label, "entry _r mismatch");
		return (false);
	}
	if (a.w != b.w) {
		fail_msg(which, label, "entry _w mismatch");
		return (false);
	}
	if (a.size != b.size) {
		fail_msg(which, label, "entry _bf._size mismatch");
		return (false);
	}
	if (a.lbfsize != b.lbfsize) {
		fail_msg(which, label, "entry _lbfsize mismatch");
		return (false);
	}
	if (a.orientation != b.orientation) {
		fail_msg(which, label, "entry _orientation mismatch");
		return (false);
	}
	if (!a.used_dummy && !b.used_dummy) {
		if (a.base_off != b.base_off) {
			fail_msg(which, label, "entry _bf._base offset mismatch");
			return (false);
		}
		if (a.p_off != b.p_off) {
			fail_msg(which, label, "entry _p offset mismatch");
			return (false);
		}
	}
	return (true);
}

static bool
test_vsnprintf_l_one(StatId which, const char *label, size_t n,
    locale_t locale, int set_errno, const char *fmt, ...)
{
	GuardedBuf refb, portb;
	int ref_ret, port_ret, ref_errno, port_errno;
	ShimSnap ref_shim, port_shim;
	locale_t loc_copy_ref, loc_copy_port;
	va_list ap;

	case_inc(which);

	refb.fill_guard();
	portb.fill_guard();

	shim_reset();
	errno = set_errno;
	loc_copy_ref = locale;
	va_start(ap, fmt);
	ref_ret = ref_vsnprintf_l(refb.user(), n, loc_copy_ref, fmt, ap);
	va_end(ap);
	ref_errno = errno;
	ref_shim = shim_snap(refb.user(), n);

	shim_reset();
	errno = set_errno;
	loc_copy_port = locale;
	va_start(ap, fmt);
	port_ret = port::vsnprintf_l(portb.user(), n, loc_copy_port, fmt, ap);
	va_end(ap);
	port_errno = errno;
	port_shim = shim_snap(portb.user(), n);

	if (ref_ret != port_ret) {
		fail_msg(which, label, "return value mismatch");
		return (false);
	}
	if (ref_errno != port_errno) {
		fail_msg(which, label, "errno mismatch");
		return (false);
	}
	if (!refb.eq(portb)) {
		fail_msg(which, label, "buffer mismatch");
		return (false);
	}
	if (!shim_eq(ref_shim, port_shim, label, which))
		return (false);
	if (n > (size_t)INT_MAX + 1u) {
		if (ref_ret != EOF || ref_errno != EOVERFLOW) {
			fail_msg(which, label, "overflow path");
			return (false);
		}
	} else if (ref_shim.calls > 0) {
		if (ref_shim.flags != (SHIM_SWR | SHIM_SSTR)) {
			fail_msg(which, label, "unexpected flags");
			return (false);
		}
	}
	return (true);
}

static bool
test_vsnprintf_one(StatId which, const char *label, size_t n, int set_errno,
    const char *fmt, ...)
{
	GuardedBuf refb, portb;
	int ref_ret, port_ret;
	ShimSnap ref_shim, port_shim;
	va_list ap;

	case_inc(which);

	refb.fill_guard();
	portb.fill_guard();

	shim_reset();
	errno = set_errno;
	va_start(ap, fmt);
	ref_ret = ref_vsnprintf(refb.user(), n, fmt, ap);
	va_end(ap);
	ref_shim = shim_snap(refb.user(), n);

	shim_reset();
	errno = set_errno;
	va_start(ap, fmt);
	port_ret = port::vsnprintf(portb.user(), n, fmt, ap);
	va_end(ap);
	port_shim = shim_snap(portb.user(), n);

	if (ref_ret != port_ret) {
		fail_msg(which, label, "return value mismatch");
		return (false);
	}
	if (!refb.eq(portb)) {
		fail_msg(which, label, "buffer mismatch");
		return (false);
	}
	if (!shim_eq(ref_shim, port_shim, label, which))
		return (false);
	if (ref_shim.calls > 0 &&
	    ref_shim.locale != pbsd_shim_get_locale()) {
		fail_msg(which, label, "__get_locale not used");
		return (false);
	}
	return (true);
}

static void
handwritten_vsnprintf_l(void)
{
	static const char hb[] = "\x80\xff\xfe\x7f";
	static const char zs[] = "z\0hidden";

	test_vsnprintf_l_one(S_VSNPRINTF_L, "n0", 0, pbsd_shim_locale_c, 0, "%d",
	    1);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "n1", 1, pbsd_shim_locale_c, 0,
	    "%d", 9);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "n2", 2, pbsd_shim_locale_c, 0,
	    "%d", 42);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "n3", 3, pbsd_shim_locale_c, 0,
	    "%s", "ab");
	test_vsnprintf_l_one(S_VSNPRINTF_L, "empty", 16, pbsd_shim_locale_c, 0,
	    "");
	test_vsnprintf_l_one(S_VSNPRINTF_L, "single_pct", 8, pbsd_shim_locale_c,
	    0, "%%");
	test_vsnprintf_l_one(S_VSNPRINTF_L, "int_max", 32, pbsd_shim_locale_c,
	    0, "%d", INT_MAX);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "int_min", 32, pbsd_shim_locale_c,
	    0, "%d", INT_MIN);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "char_ff", 8, pbsd_shim_locale_c,
	    0, "%c", (int)0xff);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "str_hi", 64, pbsd_shim_locale_c,
	    0, "%s", hb);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "str_prec", 16, pbsd_shim_locale_c,
	    0, "%.2s", zs);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "errno_m", 64, pbsd_shim_locale_c,
	    EINVAL, "%m");
	test_vsnprintf_l_one(S_VSNPRINTF_L, "null_locale", 16, NULL, EEXIST,
	    "%d", 5);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "overflow",
	    (size_t)INT_MAX + 2u, pbsd_shim_locale_c, 0, "%d", 1);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "intmax_edge",
	    (size_t)INT_MAX + 1u, pbsd_shim_locale_c, 0, "%d", 2);
	test_vsnprintf_l_one(S_VSNPRINTF_L, "tiny_n", 4, pbsd_shim_locale_c, 0,
	    "%03d", 7);
}

static void
handwritten_vsnprintf(void)
{
	static const char hb[] = "\x81\x82\x83";

	test_vsnprintf_one(S_VSNPRINTF, "n0", 0, 0, "%d", 0);
	test_vsnprintf_one(S_VSNPRINTF, "n1", 1, 0, "%d", 1);
	test_vsnprintf_one(S_VSNPRINTF, "n2", 2, 0, "%c", 'x');
	test_vsnprintf_one(S_VSNPRINTF, "empty", 8, 0, "");
	test_vsnprintf_one(S_VSNPRINTF, "int", 16, 0, "%d", -99);
	test_vsnprintf_one(S_VSNPRINTF, "hi", 32, 0, "%s", hb);
	test_vsnprintf_one(S_VSNPRINTF, "overflow", (size_t)INT_MAX + 2u, 0,
	    "%d", 3);
}

static void
fill_random_string(char *dst, std::size_t cap, unsigned seed)
{
	std::size_t n, i;

	n = seed % (cap > 0 ? cap : 1);
	if (cap > 0 && n == cap)
		n = cap - 1;
	for (i = 0; i < n; i++)
		dst[i] = (char)(unsigned char)((seed >> (i & 7)) ^ (i * 37u));
	dst[n] = '\0';
}

static void
random_vsnprintf_l(void)
{
	char argstr[32];
	char label[48];
	unsigned i;

	for (i = 0; i < RAND_ITERS; i++) {
		locale_t loc;
		size_t n;
		int set_errno;
		unsigned kind;

		n = (size_t)(rnd_u32() % 4096u);
		if ((rnd_u32() & 0xffu) == 0u)
			n = (size_t)INT_MAX + (rnd_u32() % 4u);
		loc = (rnd_u32() & 7u) == 0u ? NULL :
		    ((rnd_u32() & 1u) ? pbsd_shim_locale_c :
		    pbsd_shim_locale_utf8);
		set_errno = (int)(rnd_u32() & 0x7fu);
		kind = rnd_u32() % 9u;
		std::snprintf(label, sizeof(label), "rand_%u", i);

		switch (kind) {
		case 0:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n, loc,
			    set_errno, "%d",
			    (int)(rnd_u32() ^ (rnd_u32() >> 1)));
			break;
		case 1:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%u", rnd_u32());
			break;
		case 2:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%x", rnd_u32());
			break;
		case 3:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%c", (int)(rnd_u32() & 0xffu));
			break;
		case 4:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%s", argstr);
			break;
		case 5:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%%");
			break;
		case 6:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%m");
			break;
		case 7:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%d %c %s", (int)rnd_u32(),
			    (int)(rnd_u32() & 0xffu), argstr);
			break;
		default:
			(void)test_vsnprintf_l_one(S_VSNPRINTF_L, label, n,
			    loc, set_errno, "%u|%x|%d", rnd_u32(), rnd_u32(),
			    (int)rnd_u32());
			break;
		}
	}
}

static void
random_vsnprintf(void)
{
	char argstr[24];
	char label[48];
	unsigned i;

	for (i = 0; i < RAND_ITERS; i++) {
		size_t n;
		int set_errno;
		unsigned kind;

		n = (size_t)(rnd_u32() % 2048u);
		if ((rnd_u32() & 0x1ffu) == 0u)
			n = (size_t)INT_MAX + (rnd_u32() % 4u);
		set_errno = (int)(rnd_u32() & 0xffu);
		kind = rnd_u32() % 9u;
		std::snprintf(label, sizeof(label), "rand_%u", i);

		switch (kind) {
		case 0:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%d",
			    (int)(rnd_u32() ^ 0x80000000u));
			break;
		case 1:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%u", rnd_u32());
			break;
		case 2:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%x", rnd_u32());
			break;
		case 3:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%c", (int)(rnd_u32() & 0xffu));
			break;
		case 4:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%s", argstr);
			break;
		case 5:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%%");
			break;
		case 6:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%m");
			break;
		case 7:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%x %d %s", rnd_u32(), (int)rnd_u32(),
			    argstr);
			break;
		default:
			(void)test_vsnprintf_one(S_VSNPRINTF, label, n,
			    set_errno, "%d %c", (int)rnd_u32(),
			    (int)(rnd_u32() & 0xffu));
			break;
		}
	}
}

static bool
test_arginfo_quote_one(StatId which, const char *label, int prec, int n)
{
	printf_info pi = {};
	int argt_p[4], argt_r[4];
	int ret_p, ret_r;
	int i;

	for (i = 0; i < 4; i++) {
		argt_p[i] = 0x55;
		argt_r[i] = 0x55;
	}
	pi.prec = prec;

	case_inc(which);

	ret_p = port::__printf_arginfo_quote(&pi, (size_t)n, argt_p);
	ret_r = ref___printf_arginfo_quote(&pi, (size_t)n, argt_r);

	if (ret_p != ret_r) {
		fail_msg(which, label, "return mismatch");
		return (false);
	}
	if (argt_p[0] != argt_r[0]) {
		fail_msg(which, label, "argt[0] mismatch");
		return (false);
	}
	return (true);
}

static void
handwritten_arginfo_quote(void)
{
	test_arginfo_quote_one(S_ARGINFO_QUOTE, "basic", -1, 1);
	test_arginfo_quote_one(S_ARGINFO_QUOTE, "n2", 0, 2);
	test_arginfo_quote_one(S_ARGINFO_QUOTE, "max", INT_MAX, 1);
}

static void
random_arginfo_quote(void)
{
	unsigned i;

	for (i = 0; i < RAND_ITERS; i++) {
		char label[48];

		std::snprintf(label, sizeof(label), "rand_%u", i);
		(void)test_arginfo_quote_one(S_ARGINFO_QUOTE, label,
		    (int)rnd_u32(), 1 + (int)(rnd_u32() % 4u));
	}
}

struct RenderSnap {
	int ret;
	int printf_out_calls;
	int printf_out_len;
	int printf_flush_calls;
};

static void
mock_reset_render(void)
{
	mock_printf_out_ret = 0;
	mock_printf_out_calls = 0;
	mock_printf_out_last_len = 0;
	mock_printf_flush_calls = 0;
}

static RenderSnap
render_snap(int ret)
{
	RenderSnap s;

	s.ret = ret;
	s.printf_out_calls = mock_printf_out_calls;
	s.printf_out_len = mock_printf_out_last_len;
	s.printf_flush_calls = mock_printf_flush_calls;
	return (s);
}

static bool
test_render_quote_one(StatId which, const char *label, const char *str,
    int out_ret)
{
	printf_info pi = {};
	__printf_io io = {};
	const void *arg_p[1];
	const void *arg_r[1];
	RenderSnap snap_p, snap_r;
	int ret_p, ret_r;

	case_inc(which);

	mock_reset_render();
	mock_printf_out_ret = out_ret;
	arg_p[0] = &str;
	ret_p = port::__printf_render_quote(&io, &pi, arg_p);
	snap_p = render_snap(ret_p);

	mock_reset_render();
	mock_printf_out_ret = out_ret;
	arg_r[0] = &str;
	ret_r = ref___printf_render_quote(&io, &pi, arg_r);
	snap_r = render_snap(ret_r);

	if (snap_p.ret != snap_r.ret) {
		fail_msg(which, label, "return mismatch");
		return (false);
	}
	if (snap_p.printf_out_calls != snap_r.printf_out_calls) {
		fail_msg(which, label, "out call count mismatch");
		return (false);
	}
	if (snap_p.printf_flush_calls != snap_r.printf_flush_calls) {
		fail_msg(which, label, "flush call count mismatch");
		return (false);
	}
	return (true);
}

static void
test_render_quote_null(StatId which)
{
	printf_info pi = {};
	__printf_io io = {};
	const void *arg_p[1];
	const void *arg_r[1];
	char *nullp = nullptr;
	int ret_p, ret_r;

	case_inc(which);

	mock_reset_render();
	mock_printf_out_ret = 8;
	arg_p[0] = &nullp;
	ret_p = port::__printf_render_quote(&io, &pi, arg_p);

	mock_reset_render();
	mock_printf_out_ret = 8;
	arg_r[0] = &nullp;
	ret_r = ref___printf_render_quote(&io, &pi, arg_r);

	if (ret_p != ret_r) {
		fail_msg(which, "null", "return mismatch");
		return;
	}
	if (mock_printf_out_calls != 1 || mock_printf_out_last_len != 8) {
		fail_msg(which, "null", "out args");
		return;
	}
}

static void
handwritten_render_quote(void)
{
	static const char empty[] = "";
	static const char plain[] = "hello";
	static const char backslash[] = "a\\b";
	static const char newline[] = "a\nb";
	static const char cr[] = "a\rb";
	static const char tab[] = "a\tb";
	static const char space[] = "a b";
	static const char quote[] = "a\"b";
	static const char vtab[] = "a\vb";
	static const char mixed[] = "a\\n\"\t x\fb";
	static const char hi[] = "\x80\xff";

	test_render_quote_null(S_RENDER_QUOTE);
	test_render_quote_one(S_RENDER_QUOTE, "empty", empty, 2);
	test_render_quote_one(S_RENDER_QUOTE, "plain", plain, 5);
	test_render_quote_one(S_RENDER_QUOTE, "backslash", backslash, 1);
	test_render_quote_one(S_RENDER_QUOTE, "newline", newline, 1);
	test_render_quote_one(S_RENDER_QUOTE, "cr", cr, 1);
	test_render_quote_one(S_RENDER_QUOTE, "tab", tab, 1);
	test_render_quote_one(S_RENDER_QUOTE, "space", space, 1);
	test_render_quote_one(S_RENDER_QUOTE, "quote", quote, 1);
	test_render_quote_one(S_RENDER_QUOTE, "vtab", vtab, 1);
	test_render_quote_one(S_RENDER_QUOTE, "mixed", mixed, 1);
	test_render_quote_one(S_RENDER_QUOTE, "hi", hi, 3);
}

static void
random_render_quote(void)
{
	char randbuf[128];
	char label[48];
	unsigned i;

	for (i = 0; i < RAND_ITERS; i++) {
		int slen = (int)(rnd_u32() % 64u);
		int out_ret = (int)(rnd_u32() % 16u);
		int j;

		for (j = 0; j < slen; j++) {
			unsigned pick = rnd_u32() % 20u;

			switch (pick) {
			case 0:
				randbuf[j] = '\\';
				break;
			case 1:
				randbuf[j] = '\n';
				break;
			case 2:
				randbuf[j] = '\r';
				break;
			case 3:
				randbuf[j] = '\t';
				break;
			case 4:
				randbuf[j] = ' ';
				break;
			case 5:
				randbuf[j] = '"';
				break;
			case 6:
				randbuf[j] = '\v';
				break;
			case 7:
				randbuf[j] = '\f';
				break;
			case 8:
				randbuf[j] = '\0';
				slen = j;
				goto done_fill;
			default:
				randbuf[j] = (char)(unsigned char)(
				    rnd_u32() & 0xffu);
				break;
			}
		}
done_fill:
		randbuf[slen] = '\0';
		std::snprintf(label, sizeof(label), "rand_%u", i);
		(void)test_render_quote_one(S_RENDER_QUOTE, label, randbuf,
		    out_ret);
	}
}

} /* namespace */

int
main(void)
{
	long long total_fails = 0;
	int i;

	if (pbsd_shim_init() < 0) {
		std::fprintf(stderr, "pbsd_shim_init failed\n");
		return (1);
	}

	handwritten_tempnam();
	random_tempnam();
	handwritten_vsnprintf_l();
	handwritten_vsnprintf();
	random_vsnprintf_l();
	random_vsnprintf();
	handwritten_arginfo_quote();
	random_arginfo_quote();
	handwritten_render_quote();
	random_render_quote();

	std::printf("function                   cases      failures\n");
	std::printf("------------------------------------------------\n");
	for (i = 0; i < NSTAT; i++) {
		std::printf("%-26s %10lld %10lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		total_fails += g_stat[i].fails;
	}

	return (total_fails == 0 ? 0 : 1);
}
