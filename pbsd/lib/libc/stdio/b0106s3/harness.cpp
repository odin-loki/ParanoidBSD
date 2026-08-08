/*
 * harness.cpp -- differential test for PBSD batch b0106s3 (vsprintf.c).
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

import pbsd.lib.libc.stdio.b0106s3;

namespace port = pbsd::lib_libc_stdio::b0106s3;

extern "C" {
int ref_vsprintf_l(char *__restrict, locale_t, const char *__restrict,
    va_list);
int ref_vsprintf(char *__restrict, const char *__restrict, va_list);
locale_t pbsd_shim_get_locale(void);
extern locale_t pbsd_shim_locale_c;
extern locale_t pbsd_shim_locale_utf8;
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
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t STR_PRE = 32;
constexpr std::size_t STR_BUF = 4096;
constexpr std::size_t STR_POST = 32;
constexpr std::size_t STR_CAP = STR_PRE + STR_BUF + STR_POST;
constexpr unsigned RAND_ITERS = 200000u;
constexpr int MAXPRINT = 8;

constexpr short SHIM_SWR = 0x0008;
constexpr short SHIM_SSTR = 0x0200;

enum StatId { S_VSPRINTF_L, S_VSPRINTF, NSTAT };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "vsprintf_l", 0, 0, 0 },
	{ "vsprintf",   0, 0, 0 },
};

std::uint64_t rng_state = 0xb0106a3feedfaceULL;

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
		std::printf("  FAIL %-12s %-28s %s\n", g_stat[which].name, label,
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
};

void
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

ShimSnap
shim_snap(const char *buf_base)
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
	s.base_off = pbsd_shim_vfprintf_entry_base -
	    reinterpret_cast<const unsigned char *>(buf_base);
	s.p_off = pbsd_shim_vfprintf_entry_p -
	    reinterpret_cast<const unsigned char *>(buf_base);
	return (s);
}

bool
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
	if (a.base_off != b.base_off) {
		fail_msg(which, label, "entry _bf._base offset mismatch");
		return (false);
	}
	if (a.p_off != b.p_off) {
		fail_msg(which, label, "entry _p offset mismatch");
		return (false);
	}
	return (true);
}

bool
test_vsprintf_l_one(StatId which, const char *label, locale_t locale,
    int set_errno, const char *fmt, ...)
{
	GuardedBuf refb, portb;
	int ref_ret, port_ret;
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
	ref_ret = ref_vsprintf_l(refb.user(), loc_copy_ref, fmt, ap);
	va_end(ap);
	ref_shim = shim_snap(refb.user());

	shim_reset();
	errno = set_errno;
	loc_copy_port = locale;
	va_start(ap, fmt);
	port_ret = port::vsprintf_l(portb.user(), loc_copy_port, fmt, ap);
	va_end(ap);
	port_shim = shim_snap(portb.user());

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
	if (ref_shim.flags != (SHIM_SWR | SHIM_SSTR)) {
		fail_msg(which, label, "unexpected __SWR|__SSTR flags");
		return (false);
	}
	if (ref_shim.w != INT_MAX || ref_shim.size != INT_MAX) {
		fail_msg(which, label, "unexpected INT_MAX window");
		return (false);
	}
	if (ref_shim.base_off != 0 || ref_shim.p_off != 0) {
		fail_msg(which, label, "unexpected initial stream offsets");
		return (false);
	}
	if (ref_shim.serrno != set_errno) {
		fail_msg(which, label, "saved errno not forwarded");
		return (false);
	}
	if (locale == NULL) {
		if (ref_shim.locale != pbsd_shim_get_locale()) {
			fail_msg(which, label, "FIX_LOCALE did not substitute");
			return (false);
		}
	} else if (ref_shim.locale != locale) {
		fail_msg(which, label, "explicit locale not forwarded");
		return (false);
	}
	return (true);
}

bool
test_vsprintf_one(StatId which, const char *label, int set_errno,
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
	ref_ret = ref_vsprintf(refb.user(), fmt, ap);
	va_end(ap);
	ref_shim = shim_snap(refb.user());

	shim_reset();
	errno = set_errno;
	va_start(ap, fmt);
	port_ret = port::vsprintf(portb.user(), fmt, ap);
	va_end(ap);
	port_shim = shim_snap(portb.user());

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
	if (ref_shim.locale != pbsd_shim_get_locale()) {
		fail_msg(which, label, "__get_locale not used");
		return (false);
	}
	return (true);
}

void
handwritten_vsprintf_l(void)
{
	static const char hb[] = "\x80\xff\xfe\x7f";
	static const char zs[] = "z\0hidden";
	char word[16];

	test_vsprintf_l_one(S_VSPRINTF_L, "empty", pbsd_shim_locale_c, 0, "");
	test_vsprintf_l_one(S_VSPRINTF_L, "single_pct", pbsd_shim_locale_c, 0,
	    "%%");
	test_vsprintf_l_one(S_VSPRINTF_L, "literal_a", pbsd_shim_locale_c, 0,
	    "a");
	test_vsprintf_l_one(S_VSPRINTF_L, "int_zero", pbsd_shim_locale_c, 0,
	    "%d", 0);
	test_vsprintf_l_one(S_VSPRINTF_L, "int_one", pbsd_shim_locale_c, 0,
	    "%d", 1);
	test_vsprintf_l_one(S_VSPRINTF_L, "int_neg1", pbsd_shim_locale_c, 0,
	    "%d", -1);
	test_vsprintf_l_one(S_VSPRINTF_L, "int_max", pbsd_shim_locale_c, 0,
	    "%d", INT_MAX);
	test_vsprintf_l_one(S_VSPRINTF_L, "int_min", pbsd_shim_locale_c, 0,
	    "%d", INT_MIN);
	test_vsprintf_l_one(S_VSPRINTF_L, "uint_max", pbsd_shim_locale_c, 0,
	    "%u", UINT_MAX);
	test_vsprintf_l_one(S_VSPRINTF_L, "hex_ff", pbsd_shim_locale_c, 0,
	    "%x", 0xff);
	test_vsprintf_l_one(S_VSPRINTF_L, "char_nul", pbsd_shim_locale_c, 0,
	    "%c", 0);
	test_vsprintf_l_one(S_VSPRINTF_L, "char_7f", pbsd_shim_locale_c, 0,
	    "%c", 0x7f);
	test_vsprintf_l_one(S_VSPRINTF_L, "char_ff", pbsd_shim_locale_c, 0,
	    "%c", (int)0xff);
	test_vsprintf_l_one(S_VSPRINTF_L, "str_empty", pbsd_shim_locale_c, 0,
	    "%s", "");
	test_vsprintf_l_one(S_VSPRINTF_L, "str_hi", pbsd_shim_locale_c, 0,
	    "%s", hb);
	test_vsprintf_l_one(S_VSPRINTF_L, "str_prec", pbsd_shim_locale_c, 0,
	    "%.2s", zs);
	test_vsprintf_l_one(S_VSPRINTF_L, "width_pad", pbsd_shim_locale_c, 0,
	    "%5d", 3);
	test_vsprintf_l_one(S_VSPRINTF_L, "width_zero", pbsd_shim_locale_c, 0,
	    "%05d", 7);
	test_vsprintf_l_one(S_VSPRINTF_L, "long_long", pbsd_shim_locale_c, 0,
	    "%lld", (long long)-123456789);
	test_vsprintf_l_one(S_VSPRINTF_L, "ptr_fmt", pbsd_shim_locale_c, 0,
	    "%p", word);
	test_vsprintf_l_one(S_VSPRINTF_L, "errno_m_einval", pbsd_shim_locale_c,
	    EINVAL, "%m");
	test_vsprintf_l_one(S_VSPRINTF_L, "errno_m_eacces", pbsd_shim_locale_c,
	    EACCES, "%m");
	test_vsprintf_l_one(S_VSPRINTF_L, "errno_m_zero", pbsd_shim_locale_c, 0,
	    "%m");
	test_vsprintf_l_one(S_VSPRINTF_L, "combo", pbsd_shim_locale_c, 0,
	    "%d|%s|%c", 42, "xy", 'Q');
	test_vsprintf_l_one(S_VSPRINTF_L, "utf8_locale", pbsd_shim_locale_utf8,
	    0, "%d %s", 9, "utf8");
	test_vsprintf_l_one(S_VSPRINTF_L, "null_locale", NULL, ENOENT, "%d", 5);
	test_vsprintf_l_one(S_VSPRINTF_L, "null_locale_m", NULL, EEXIST, "%m");
	test_vsprintf_l_one(S_VSPRINTF_L, "long_run", pbsd_shim_locale_c, 0,
	    "%1024d", 1);
}

void
handwritten_vsprintf(void)
{
	static const char hb[] = "\x81\x82\x83";
	static const char zs[] = "tail\0gone";

	test_vsprintf_one(S_VSPRINTF, "empty", 0, "");
	test_vsprintf_one(S_VSPRINTF, "single_pct", 0, "%%");
	test_vsprintf_one(S_VSPRINTF, "literal_z", 0, "z");
	test_vsprintf_one(S_VSPRINTF, "int_zero", 0, "%d", 0);
	test_vsprintf_one(S_VSPRINTF, "int_neg", 0, "%d", -99);
	test_vsprintf_one(S_VSPRINTF, "int_max", 0, "%d", INT_MAX);
	test_vsprintf_one(S_VSPRINTF, "int_min", 0, "%d", INT_MIN);
	test_vsprintf_one(S_VSPRINTF, "uint_hi", 0, "%u", 0xfffffffeu);
	test_vsprintf_one(S_VSPRINTF, "hex", 0, "%#x", 0xabc);
	test_vsprintf_one(S_VSPRINTF, "oct", 0, "%o", 511);
	test_vsprintf_one(S_VSPRINTF, "char_hi", 0, "%c", (int)0x80);
	test_vsprintf_one(S_VSPRINTF, "char_ff", 0, "%c", (int)0xff);
	test_vsprintf_one(S_VSPRINTF, "str_empty", 0, "%s", "");
	test_vsprintf_one(S_VSPRINTF, "str_hi", 0, "%s", hb);
	test_vsprintf_one(S_VSPRINTF, "str_prec", 0, "%.3s", zs);
	test_vsprintf_one(S_VSPRINTF, "width", 0, "%8s", "ab");
	test_vsprintf_one(S_VSPRINTF, "ll", 0, "%lli", (long long)LLONG_MAX);
	test_vsprintf_one(S_VSPRINTF, "errno_m", EIO, "%m");
	test_vsprintf_one(S_VSPRINTF, "combo", 0, "%x+%d", 0x10, 2);
	test_vsprintf_one(S_VSPRINTF, "long_run", 0, "%512s", "pad");
}

void
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

void
random_vsprintf_l(void)
{
	char argstr[32];
	char label[48];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		locale_t loc;
		int set_errno;
		unsigned kind;

		loc = (rnd_u32() & 7u) == 0u ? NULL :
		    ((rnd_u32() & 1u) ? pbsd_shim_locale_c :
		    pbsd_shim_locale_utf8);
		set_errno = (int)(rnd_u32() & 0x7fu);
		kind = rnd_u32() % 9u;
		std::snprintf(label, sizeof(label), "rand_%u", i);

		switch (kind) {
		case 0:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%d",
			    (int)(rnd_u32() ^ (rnd_u32() >> 1)));
			break;
		case 1:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%u", rnd_u32());
			break;
		case 2:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%x", rnd_u32());
			break;
		case 3:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%c", (int)(rnd_u32() & 0xffu));
			break;
		case 4:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%s", argstr);
			break;
		case 5:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%%");
			break;
		case 6:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%m");
			break;
		case 7:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%d %c %s", (int)rnd_u32(),
			    (int)(rnd_u32() & 0xffu), argstr);
			break;
		default:
			(void)test_vsprintf_l_one(S_VSPRINTF_L, label, loc,
			    set_errno, "%u|%x|%d", rnd_u32(), rnd_u32(),
			    (int)rnd_u32());
			break;
		}
	}
}

void
random_vsprintf(void)
{
	char argstr[24];
	char label[48];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		int set_errno;
		unsigned kind;

		set_errno = (int)(rnd_u32() & 0xffu);
		kind = rnd_u32() % 9u;
		std::snprintf(label, sizeof(label), "rand_%u", i);

		switch (kind) {
		case 0:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%d", (int)(rnd_u32() ^ 0x80000000u));
			break;
		case 1:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%u", rnd_u32());
			break;
		case 2:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%x", rnd_u32());
			break;
		case 3:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%c", (int)(rnd_u32() & 0xffu));
			break;
		case 4:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%s", argstr);
			break;
		case 5:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%%");
			break;
		case 6:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%m");
			break;
		case 7:
			fill_random_string(argstr, sizeof(argstr), rnd_u32());
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%x %d %s", rnd_u32(), (int)rnd_u32(), argstr);
			break;
		default:
			(void)test_vsprintf_one(S_VSPRINTF, label, set_errno,
			    "%d %c", (int)rnd_u32(),
			    (int)(rnd_u32() & 0xffu));
			break;
		}
	}
}

} /* namespace */

int
main(void)
{
	long long total_fails = 0;

	if (pbsd_shim_init() < 0) {
		std::fprintf(stderr, "pbsd_shim_init failed\n");
		return (1);
	}

	handwritten_vsprintf_l();
	handwritten_vsprintf();
	random_vsprintf_l();
	random_vsprintf();

	std::printf("function     cases      failures\n");
	std::printf("--------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-12s %10lld %10lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		total_fails += g_stat[i].fails;
	}

	return (total_fails == 0 ? 0 : 1);
}
