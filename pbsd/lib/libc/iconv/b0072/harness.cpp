/*
 * Differential test harness for batch b0072.
 *
 * iconv_open_into, __iconv and iconv forward to __bsd_* entry points.
 * _citrus_string_hash_func is tested directly against the ref_ oracle.
 * Recording mocks for the __bsd_* primitives are linked by both sides.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.iconv.b0072;

namespace port = pbsd::lib_libc_iconv::b0072;

using port::iconv_allocation_t;
using port::iconv_t;

extern "C" {
int ref_iconv_open_into(const char *, const char *, iconv_allocation_t *);
size_t ref___iconv(iconv_t, char **, size_t *, char **, size_t *,
    __uint32_t, size_t *);
size_t ref_iconv(iconv_t, char **, size_t *, char **, size_t *);
int ref__citrus_string_hash_func(const char *, int);
}

/* ------------------------------------------------------------------ */
/* Constants and recording mocks                                      */
/* ------------------------------------------------------------------ */

#define	GUARD			0x7f
#define	PATH_CAP		256
#define	PATH_GUARD_PAD		32
#define	IN_CAP			128
#define	OUT_CAP			128
#define	IO_GUARD_PAD		32
#define	ALLOC_GUARD_PAD		32
#define	KEY_CAP			128
#define	KEY_GUARD_PAD		16
#define	RANDOM_ITERS		200000LL

struct OpenIntoRec {
	int		calls;
	const char	*from;
	const char	*to;
	iconv_allocation_t *alloc;
	int		ret;
};

struct ExIconvRec {
	int		calls;
	iconv_t		cd;
	char		**inbuf;
	size_t		*inbytesleft;
	char		**outbuf;
	size_t		*outbytesleft;
	__uint32_t	flags;
	size_t		*extra;
	size_t		ret;
};

struct IconvRec {
	int		calls;
	iconv_t		cd;
	char		**inbuf;
	size_t		*inbytesleft;
	char		**outbuf;
	size_t		*outbytesleft;
	size_t		ret;
};

static OpenIntoRec g_open_into;
static ExIconvRec g_ex_iconv;
static IconvRec g_iconv;

static void
open_into_reset(void)
{
	g_open_into.calls = 0;
	g_open_into.from = reinterpret_cast<const char *>(static_cast<intptr_t>(-0x1001));
	g_open_into.to = reinterpret_cast<const char *>(static_cast<intptr_t>(-0x1002));
	g_open_into.alloc = reinterpret_cast<iconv_allocation_t *>(static_cast<intptr_t>(-0x1003));
	g_open_into.ret = -0x1004;
}

static void
ex_iconv_reset(void)
{
	g_ex_iconv.calls = 0;
	g_ex_iconv.cd = reinterpret_cast<iconv_t>(static_cast<intptr_t>(-0x2001));
	g_ex_iconv.inbuf = reinterpret_cast<char **>(static_cast<intptr_t>(-0x2002));
	g_ex_iconv.inbytesleft = reinterpret_cast<size_t *>(static_cast<intptr_t>(-0x2003));
	g_ex_iconv.outbuf = reinterpret_cast<char **>(static_cast<intptr_t>(-0x2004));
	g_ex_iconv.outbytesleft = reinterpret_cast<size_t *>(static_cast<intptr_t>(-0x2005));
	g_ex_iconv.flags = 0xdeadbeefu;
	g_ex_iconv.extra = reinterpret_cast<size_t *>(static_cast<intptr_t>(-0x2006));
	g_ex_iconv.ret = static_cast<size_t>(-1);
}

static void
iconv_reset(void)
{
	g_iconv.calls = 0;
	g_iconv.cd = reinterpret_cast<iconv_t>(static_cast<intptr_t>(-0x3001));
	g_iconv.inbuf = reinterpret_cast<char **>(static_cast<intptr_t>(-0x3002));
	g_iconv.inbytesleft = reinterpret_cast<size_t *>(static_cast<intptr_t>(-0x3003));
	g_iconv.outbuf = reinterpret_cast<char **>(static_cast<intptr_t>(-0x3004));
	g_iconv.outbytesleft = reinterpret_cast<size_t *>(static_cast<intptr_t>(-0x3005));
	g_iconv.ret = static_cast<size_t>(-1);
}

static unsigned
hash_cstr(const char *s)
{
	unsigned h = 2166136261u;

	if (s == nullptr)
		return h;
	for (int i = 0; s[i] != '\0' && i < PATH_CAP; i++) {
		h ^= static_cast<unsigned char>(s[i]);
		h *= 16777619u;
	}
	return h;
}

static size_t
mock_transform(iconv_t cd, char **inbuf, size_t *inbytesleft,
    char **outbuf, size_t *outbytesleft, __uint32_t flags, size_t *extra)
{
	unsigned h = static_cast<unsigned>(
	    reinterpret_cast<uintptr_t>(cd) & 0xffffffffu);
	size_t consumed = 0;
	size_t produced = 0;
	size_t in_left = 0;
	size_t out_left = 0;

	h ^= flags * 2654435761u;
	if (extra != nullptr)
		h ^= static_cast<unsigned>(*extra) * 2246822519u;

	if (inbytesleft != nullptr)
		in_left = *inbytesleft;
	if (outbytesleft != nullptr)
		out_left = *outbytesleft;

	if (inbuf != nullptr && *inbuf != nullptr && inbytesleft != nullptr) {
		size_t n = in_left;
		if (n > 16)
			n = 16;
		for (size_t i = 0; i < n; i++) {
			unsigned char c = static_cast<unsigned char>((*inbuf)[i]);
			h ^= c * (static_cast<unsigned>(i) + 1u);
			consumed++;
			if (outbuf != nullptr && *outbuf != nullptr &&
			    outbytesleft != nullptr && produced < out_left) {
				(*outbuf)[produced] = static_cast<char>(
				    (c ^ static_cast<unsigned char>(h >> (i & 7))) +
				    static_cast<unsigned char>((flags >> (i & 3)) & 0xffu));
				produced++;
			}
		}
		*inbuf += consumed;
		*inbytesleft -= consumed;
	}

	if (extra != nullptr)
		*extra = consumed ^ produced ^ static_cast<size_t>(h);

	size_t ret = static_cast<size_t>(h ^ (h >> 11));
	ret += consumed;
	ret += produced << 8;
	ret ^= static_cast<size_t>(reinterpret_cast<uintptr_t>(cd) & 0xffffu);
	return ret;
}

extern "C" int
__bsd_iconv_open_into(const char *a, const char *b, void *c)
{
	unsigned ha = hash_cstr(a);
	unsigned hb = hash_cstr(b);
	int ret = static_cast<int>(ha ^ (hb * 40503u) ^ 0x13579bu);

	if (c != nullptr) {
		auto *alloc = static_cast<iconv_allocation_t *>(c);
		for (int i = 0; i < 64; i++) {
			uintptr_t v = static_cast<uintptr_t>(ha) ^
			    (static_cast<uintptr_t>(hb) << (i & 15)) ^
			    static_cast<uintptr_t>(i * 0x9e3779b9u);
			alloc->spaceholder[i] = reinterpret_cast<void *>(v | 1u);
		}
	}

	g_open_into.calls++;
	g_open_into.from = a;
	g_open_into.to = b;
	g_open_into.alloc = static_cast<iconv_allocation_t *>(c);
	g_open_into.ret = ret;
	return ret;
}

extern "C" size_t
__bsd___iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf,
    size_t *outbytesleft, __uint32_t flags, size_t *extra)
{
	size_t ret = mock_transform(cd, inbuf, inbytesleft, outbuf, outbytesleft,
	    flags, extra);

	g_ex_iconv.calls++;
	g_ex_iconv.cd = cd;
	g_ex_iconv.inbuf = inbuf;
	g_ex_iconv.inbytesleft = inbytesleft;
	g_ex_iconv.outbuf = outbuf;
	g_ex_iconv.outbytesleft = outbytesleft;
	g_ex_iconv.flags = flags;
	g_ex_iconv.extra = extra;
	g_ex_iconv.ret = ret;
	return ret;
}

extern "C" size_t
__bsd_iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf,
    size_t *outbytesleft)
{
	size_t ret = mock_transform(cd, inbuf, inbytesleft, outbuf, outbytesleft,
	    0u, nullptr);

	g_iconv.calls++;
	g_iconv.cd = cd;
	g_iconv.inbuf = inbuf;
	g_iconv.inbytesleft = inbytesleft;
	g_iconv.outbuf = outbuf;
	g_iconv.outbytesleft = outbytesleft;
	g_iconv.ret = ret;
	return ret;
}

/* ------------------------------------------------------------------ */
/* Per-function statistics                                              */
/* ------------------------------------------------------------------ */

enum Fn {
	FN_OPEN_INTO,
	FN_EX_ICONV,
	FN_ICONV,
	FN_HASH,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"iconv_open_into",
	"__iconv",
	"iconv",
	"_citrus_string_hash_func",
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
	std::printf("FAIL[%lld] %s: %s\n", caseno, fn_name[fn], what);
}

static long long
ptr_off(const void *p, const void *base)
{
	if (p == nullptr)
		return -1;
	return static_cast<long long>(
	    reinterpret_cast<intptr_t>(p) -
	    reinterpret_cast<intptr_t>(base));
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
/* Buffer helpers                                                       */
/* ------------------------------------------------------------------ */

struct PathBuf {
	unsigned char bytes[PATH_GUARD_PAD + PATH_CAP + PATH_GUARD_PAD];
};

struct AllocSetup {
	unsigned char guard_before[ALLOC_GUARD_PAD];
	iconv_allocation_t alloc;
	unsigned char guard_after[ALLOC_GUARD_PAD];
};

struct IoSetup {
	unsigned char in_bytes[IO_GUARD_PAD + IN_CAP + IO_GUARD_PAD];
	unsigned char out_bytes[IO_GUARD_PAD + OUT_CAP + IO_GUARD_PAD];
	char *in_ptr;
	char *out_ptr;
	size_t in_left;
	size_t out_left;
	size_t extra;
};

struct KeyBuf {
	unsigned char bytes[KEY_GUARD_PAD + KEY_CAP + KEY_GUARD_PAD];
};

static void
fill_path_buf(PathBuf &pb, const unsigned char *src, int len)
{
	std::memset(pb.bytes, GUARD, sizeof(pb.bytes));
	if (len < 0)
		len = 0;
	if (len > PATH_CAP)
		len = PATH_CAP;
	if (src != nullptr && len > 0)
		std::memcpy(pb.bytes + PATH_GUARD_PAD, src,
		    static_cast<size_t>(len));
	pb.bytes[PATH_GUARD_PAD + len] = '\0';
}

static const char *
path_ptr(const PathBuf &pb)
{
	return reinterpret_cast<const char *>(pb.bytes + PATH_GUARD_PAD);
}

static void
init_alloc_setup(AllocSetup &as)
{
	std::memset(&as, 0, sizeof(as));
	std::memset(as.guard_before, GUARD, sizeof(as.guard_before));
	std::memset(as.guard_after, GUARD, sizeof(as.guard_after));
}

static void
init_io_setup(IoSetup &io, const unsigned char *in_src, int in_len,
    int out_cap, size_t in_left_init, size_t out_left_init)
{
	std::memset(&io, 0, sizeof(io));
	std::memset(io.in_bytes, GUARD, sizeof(io.in_bytes));
	std::memset(io.out_bytes, GUARD, sizeof(io.out_bytes));
	if (in_len < 0)
		in_len = 0;
	if (in_len > IN_CAP)
		in_len = IN_CAP;
	if (out_cap < 0)
		out_cap = 0;
	if (out_cap > OUT_CAP)
		out_cap = OUT_CAP;
	if (in_src != nullptr && in_len > 0) {
		std::memcpy(io.in_bytes + IO_GUARD_PAD, in_src,
		    static_cast<size_t>(in_len));
	}
	io.in_ptr = reinterpret_cast<char *>(io.in_bytes + IO_GUARD_PAD);
	io.out_ptr = reinterpret_cast<char *>(io.out_bytes + IO_GUARD_PAD);
	io.in_left = in_left_init;
	if (io.in_left > static_cast<size_t>(in_len))
		io.in_left = static_cast<size_t>(in_len);
	io.out_left = out_left_init;
	if (io.out_left > static_cast<size_t>(out_cap))
		io.out_left = static_cast<size_t>(out_cap);
	io.extra = static_cast<size_t>(0xfeedfaceu);
}

static void
fill_key_buf(KeyBuf &kb, const unsigned char *src, int len)
{
	std::memset(kb.bytes, GUARD, sizeof(kb.bytes));
	if (len < 0)
		len = 0;
	if (len > KEY_CAP)
		len = KEY_CAP;
	if (src != nullptr && len > 0)
		std::memcpy(kb.bytes + KEY_GUARD_PAD, src,
		    static_cast<size_t>(len));
	kb.bytes[KEY_GUARD_PAD + len] = '\0';
}

static const char *
key_ptr(const KeyBuf &kb)
{
	return reinterpret_cast<const char *>(kb.bytes + KEY_GUARD_PAD);
}

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
	{ 'A', 'B', 'C', 0 },
	{ 0x80, 0xff, 0x7f, 0x00, 0xfe, 0 },
};

static const int EDGE_PATH_LENS[] = {
	0, 1, 3, 1, 1, 3, 3, 5
};
static const int N_EDGE_PATHS =
    static_cast<int>(sizeof(EDGE_PATHS) / sizeof(EDGE_PATHS[0]));

static const intptr_t EDGE_PTRS[] = {
	0, 1, -1, 0x7f, 0x80, 0xff, 0x100, 0x7fffffff, -0x7fffffff,
	0xdeadbeef, -0x100,
};
static const int N_EDGE_PTRS =
    static_cast<int>(sizeof(EDGE_PTRS) / sizeof(EDGE_PTRS[0]));

static const unsigned char EDGE_IN_DATA[][16] = {
	{ 0 },
	{ 'x' },
	{ 0x80 },
	{ 0xff },
	{ 'A', 'B', 'C', 'D' },
	{ 0x7f, 0x80, 0xff, 0x00, 0xfe },
	{ 'a', '\0', 'b', 'c' },
};
static const int EDGE_IN_LENS[] = { 0, 1, 1, 1, 4, 5, 4 };
static const int N_EDGE_IN =
    static_cast<int>(sizeof(EDGE_IN_DATA) / sizeof(EDGE_IN_DATA[0]));

static const unsigned char EDGE_KEYS[][16] = {
	{ 0 },
	{ 'a', 0 },
	{ 'A', 'B', 'C', 0 },
	{ 0x80, 0xff, 0 },
	{ 'z', 'Z', '9', '_', 0 },
	{ 0x7f, 0x80, 0xfe, 0xff, 0 },
};
static const int EDGE_KEY_LENS[] = { 0, 1, 3, 2, 4, 4 };
static const int N_EDGE_KEYS =
    static_cast<int>(sizeof(EDGE_KEYS) / sizeof(EDGE_KEYS[0]));

static const int HASH_SIZES[] = {
	1, 2, 3, 4, 5, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256,
	1023, 1024, 4096, -1, -2, -128, -256, INT_MIN, INT_MAX
};
static const int N_HASH_SIZES =
    static_cast<int>(sizeof(HASH_SIZES) / sizeof(HASH_SIZES[0]));

/* ------------------------------------------------------------------ */
/* iconv_open_into                                                      */
/* ------------------------------------------------------------------ */

static void
run_open_into_case(const char *from, const char *to, AllocSetup &port_as,
    AllocSetup &ref_as, bool null_alloc, uint32_t tag)
{
	Fn fn = FN_OPEN_INTO;
	OpenIntoRec rec_port, rec_ref;
	iconv_allocation_t *port_c;
	iconv_allocation_t *ref_c;
	int ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	port_c = null_alloc ? nullptr : &port_as.alloc;
	ref_c = null_alloc ? nullptr : &ref_as.alloc;

	open_into_reset();
	ex_iconv_reset();
	iconv_reset();

	ret_port = port::iconv_open_into(from, to, port_c);
	rec_port = g_open_into;

	open_into_reset();
	ex_iconv_reset();
	iconv_reset();

	ret_ref = ref_iconv_open_into(from, to, ref_c);
	rec_ref = g_open_into;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (ptr_off(rec_port.from, from) != ptr_off(rec_ref.from, from)) {
		report_fail(fn, n, "from pointer offset");
		bad = 1;
	}
	if (ptr_off(rec_port.to, to) != ptr_off(rec_ref.to, to)) {
		report_fail(fn, n, "to pointer offset");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (!null_alloc) {
		if (ptr_off(rec_port.alloc, &port_as.alloc) !=
		    ptr_off(rec_ref.alloc, &ref_as.alloc)) {
			report_fail(fn, n, "alloc pointer offset");
			bad = 1;
		}
		if (std::memcmp(&port_as.alloc, &ref_as.alloc,
		    sizeof(iconv_allocation_t)) != 0) {
			report_fail(fn, n, "allocation contents");
			bad = 1;
		}
		if (std::memcmp(port_as.guard_before, ref_as.guard_before,
		    sizeof(port_as.guard_before)) != 0 ||
		    std::memcmp(port_as.guard_after, ref_as.guard_after,
		    sizeof(port_as.guard_after)) != 0) {
			report_fail(fn, n, "alloc guard bytes");
			bad = 1;
		}
	} else if (rec_port.alloc != rec_ref.alloc) {
		report_fail(fn, n, "alloc null mismatch");
		bad = 1;
	}
	if (g_ex_iconv.calls != 0 || g_iconv.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_open_into_cases(void)
{
	for (int fi = 0; fi < N_EDGE_PATHS; fi++) {
		for (int ti = 0; ti < N_EDGE_PATHS; ti++) {
			for (int null_alloc = 0; null_alloc < 2; null_alloc++) {
				PathBuf pa, pb;
				AllocSetup as_p, as_r;
				fill_path_buf(pa, EDGE_PATHS[fi], EDGE_PATH_LENS[fi]);
				fill_path_buf(pb, EDGE_PATHS[ti], EDGE_PATH_LENS[ti]);
				init_alloc_setup(as_p);
				init_alloc_setup(as_r);
				run_open_into_case(path_ptr(pa), path_ptr(pb),
				    as_p, as_r, null_alloc != 0,
				    static_cast<uint32_t>(fi * 1009u + ti));
			}
		}
	}
}

static void
random_open_into_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		PathBuf pa, pb;
		AllocSetup as_p, as_r;
		unsigned char tmp_a[PATH_CAP];
		unsigned char tmp_b[PATH_CAP];
		int len_a = static_cast<int>(rng_u32() % (PATH_CAP + 1));
		int len_b = static_cast<int>(rng_u32() % (PATH_CAP + 1));
		for (int i = 0; i < len_a; i++) {
			uint32_t r = rng_u32();
			if ((r & 7u) == 0u)
				tmp_a[i] = '\0';
			else if ((r & 3u) == 0u)
				tmp_a[i] = static_cast<unsigned char>(
				    0x80u | (r & 0x7fu));
			else
				tmp_a[i] = static_cast<unsigned char>(r & 0xffu);
		}
		for (int i = 0; i < len_b; i++) {
			uint32_t r = rng_u32();
			if ((r & 7u) == 0u)
				tmp_b[i] = '\0';
			else if ((r & 3u) == 0u)
				tmp_b[i] = static_cast<unsigned char>(
				    0x80u | (r & 0x7fu));
			else
				tmp_b[i] = static_cast<unsigned char>(r & 0xffu);
		}
		fill_path_buf(pa, tmp_a, len_a);
		fill_path_buf(pb, tmp_b, len_b);
		init_alloc_setup(as_p);
		init_alloc_setup(as_r);
		run_open_into_case(path_ptr(pa), path_ptr(pb), as_p, as_r,
		    (rng_u32() & 15u) == 0u, rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* __iconv                                                              */
/* ------------------------------------------------------------------ */

static void
run_ex_iconv_case(iconv_t cd, IoSetup &port_io, IoSetup &ref_io,
    __uint32_t flags, bool null_in, bool null_out, bool null_extra, uint32_t tag)
{
	Fn fn = FN_EX_ICONV;
	ExIconvRec rec_port, rec_ref;
	char *port_in, *ref_in, *port_out, *ref_out;
	size_t port_in_left, ref_in_left, port_out_left, ref_out_left;
	size_t port_extra, ref_extra;
	size_t *port_extra_p, *ref_extra_p;
	size_t ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	port_in = null_in ? nullptr : port_io.in_ptr;
	ref_in = null_in ? nullptr : ref_io.in_ptr;
	port_out = null_out ? nullptr : port_io.out_ptr;
	ref_out = null_out ? nullptr : ref_io.out_ptr;
	port_in_left = port_io.in_left;
	ref_in_left = ref_io.in_left;
	port_out_left = port_io.out_left;
	ref_out_left = ref_io.out_left;
	port_extra = port_io.extra;
	ref_extra = ref_io.extra;
	port_extra_p = null_extra ? nullptr : &port_extra;
	ref_extra_p = null_extra ? nullptr : &ref_extra;

	open_into_reset();
	ex_iconv_reset();
	iconv_reset();

	ret_port = port::__iconv(cd,
	    null_in ? nullptr : &port_in,
	    null_in ? nullptr : &port_in_left,
	    null_out ? nullptr : &port_out,
	    null_out ? nullptr : &port_out_left,
	    flags, port_extra_p);
	rec_port = g_ex_iconv;

	open_into_reset();
	ex_iconv_reset();
	iconv_reset();

	ret_ref = ref___iconv(cd,
	    null_in ? nullptr : &ref_in,
	    null_in ? nullptr : &ref_in_left,
	    null_out ? nullptr : &ref_out,
	    null_out ? nullptr : &ref_out_left,
	    flags, ref_extra_p);
	rec_ref = g_ex_iconv;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.cd != rec_ref.cd) {
		report_fail(fn, n, "iconv_t argument");
		bad = 1;
	}
	if (rec_port.flags != rec_ref.flags) {
		report_fail(fn, n, "flags argument");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (!null_in) {
		if (ptr_off(rec_port.inbuf, &port_io.in_ptr) !=
		    ptr_off(rec_ref.inbuf, &ref_io.in_ptr)) {
			report_fail(fn, n, "inbuf pointer offset");
			bad = 1;
		}
		if (ptr_off(rec_port.inbytesleft, &port_in_left) !=
		    ptr_off(rec_ref.inbytesleft, &ref_in_left)) {
			report_fail(fn, n, "inbytesleft pointer offset");
			bad = 1;
		}
		if (port_in_left != ref_in_left) {
			report_fail(fn, n, "inbytesleft value");
			bad = 1;
		}
		if (ptr_off(port_in, port_io.in_bytes) !=
		    ptr_off(ref_in, ref_io.in_bytes)) {
			report_fail(fn, n, "in pointer advance");
			bad = 1;
		}
	}
	if (!null_out) {
		if (ptr_off(rec_port.outbuf, &port_io.out_ptr) !=
		    ptr_off(rec_ref.outbuf, &ref_io.out_ptr)) {
			report_fail(fn, n, "outbuf pointer offset");
			bad = 1;
		}
		if (ptr_off(rec_port.outbytesleft, &port_out_left) !=
		    ptr_off(rec_ref.outbytesleft, &ref_out_left)) {
			report_fail(fn, n, "outbytesleft pointer offset");
			bad = 1;
		}
		if (port_out_left != ref_out_left) {
			report_fail(fn, n, "outbytesleft value");
			bad = 1;
		}
		if (ptr_off(port_out, port_io.out_bytes) !=
		    ptr_off(ref_out, ref_io.out_bytes)) {
			report_fail(fn, n, "out pointer advance");
			bad = 1;
		}
	}
	if (!null_extra) {
		if (ptr_off(rec_port.extra, &port_extra) !=
		    ptr_off(rec_ref.extra, &ref_extra)) {
			report_fail(fn, n, "extra pointer offset");
			bad = 1;
		}
		if (port_extra != ref_extra) {
			report_fail(fn, n, "extra value");
			bad = 1;
		}
	}
	if (std::memcmp(port_io.in_bytes, ref_io.in_bytes,
	    sizeof(port_io.in_bytes)) != 0) {
		report_fail(fn, n, "input guard buffer");
		bad = 1;
	}
	if (std::memcmp(port_io.out_bytes, ref_io.out_bytes,
	    sizeof(port_io.out_bytes)) != 0) {
		report_fail(fn, n, "output guard buffer");
		bad = 1;
	}
	if (g_open_into.calls != 0 || g_iconv.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_ex_iconv_cases(void)
{
	for (int pi = 0; pi < N_EDGE_PTRS; pi++) {
		for (int ii = 0; ii < N_EDGE_IN; ii++) {
			for (int ei = 0; ei < NEDGE; ei++) {
				for (int null_in = 0; null_in < 2; null_in++) {
					for (int null_out = 0; null_out < 2;
					    null_out++) {
						for (int null_extra = 0;
						    null_extra < 2; null_extra++) {
							IoSetup pa, pb;
							iconv_t cd =
							    reinterpret_cast<iconv_t>(
								EDGE_PTRS[pi]);
							init_io_setup(pa,
							    EDGE_IN_DATA[ii],
							    EDGE_IN_LENS[ii],
							    OUT_CAP,
							    static_cast<size_t>(
								EDGE_IN_LENS[ii]),
							    static_cast<size_t>(
								OUT_CAP));
							init_io_setup(pb,
							    EDGE_IN_DATA[ii],
							    EDGE_IN_LENS[ii],
							    OUT_CAP,
							    static_cast<size_t>(
								EDGE_IN_LENS[ii]),
							    static_cast<size_t>(
								OUT_CAP));
							run_ex_iconv_case(cd, pa, pb,
							    static_cast<__uint32_t>(
								EDGE_INTS[ei]),
							    null_in != 0,
							    null_out != 0,
							    null_extra != 0,
							    static_cast<uint32_t>(
								pi * 10007u + ii *
								101u + ei));
						}
					}
				}
			}
		}
	}
}

static void
random_ex_iconv_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		IoSetup pa, pb;
		unsigned char tmp[IN_CAP];
		int in_len = static_cast<int>(rng_u32() % (IN_CAP + 1));
		int out_cap = static_cast<int>(rng_u32() % (OUT_CAP + 1));
		for (int i = 0; i < in_len; i++) {
			uint32_t r = rng_u32();
			if ((r & 7u) == 0u)
				tmp[i] = '\0';
			else if ((r & 3u) == 0u)
				tmp[i] = static_cast<unsigned char>(
				    0x80u | (r & 0x7fu));
			else
				tmp[i] = static_cast<unsigned char>(r & 0xffu);
		}
		size_t in_left = static_cast<size_t>(rng_u32() % (in_len + 1));
		size_t out_left = static_cast<size_t>(rng_u32() % (out_cap + 1));
		init_io_setup(pa, tmp, in_len, out_cap, in_left, out_left);
		init_io_setup(pb, tmp, in_len, out_cap, in_left, out_left);
		uintptr_t v = static_cast<uintptr_t>(rng_u32());
		v ^= static_cast<uintptr_t>(rng_u32()) << 17;
		iconv_t cd = reinterpret_cast<iconv_t>(v);
		__uint32_t flags = rng_u32();
		if ((rng_u32() & 7u) == 0u)
			flags = static_cast<__uint32_t>(
			    EDGE_INTS[rng_u32() % static_cast<unsigned>(NEDGE)]);
		run_ex_iconv_case(cd, pa, pb, flags,
		    (rng_u32() & 15u) == 0u,
		    (rng_u32() & 15u) == 0u,
		    (rng_u32() & 15u) == 0u,
		    rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* iconv                                                                */
/* ------------------------------------------------------------------ */

static void
run_iconv_case(iconv_t cd, IoSetup &port_io, IoSetup &ref_io,
    bool null_in, bool null_out, uint32_t tag)
{
	Fn fn = FN_ICONV;
	IconvRec rec_port, rec_ref;
	char *port_in, *ref_in, *port_out, *ref_out;
	size_t port_in_left, ref_in_left, port_out_left, ref_out_left;
	size_t ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	port_in = null_in ? nullptr : port_io.in_ptr;
	ref_in = null_in ? nullptr : ref_io.in_ptr;
	port_out = null_out ? nullptr : port_io.out_ptr;
	ref_out = null_out ? nullptr : ref_io.out_ptr;
	port_in_left = port_io.in_left;
	ref_in_left = ref_io.in_left;
	port_out_left = port_io.out_left;
	ref_out_left = ref_io.out_left;

	open_into_reset();
	ex_iconv_reset();
	iconv_reset();

	ret_port = port::iconv(cd,
	    null_in ? nullptr : &port_in,
	    null_in ? nullptr : &port_in_left,
	    null_out ? nullptr : &port_out,
	    null_out ? nullptr : &port_out_left);
	rec_port = g_iconv;

	open_into_reset();
	ex_iconv_reset();
	iconv_reset();

	ret_ref = ref_iconv(cd,
	    null_in ? nullptr : &ref_in,
	    null_in ? nullptr : &ref_in_left,
	    null_out ? nullptr : &ref_out,
	    null_out ? nullptr : &ref_out_left);
	rec_ref = g_iconv;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.cd != rec_ref.cd) {
		report_fail(fn, n, "iconv_t argument");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (!null_in) {
		if (ptr_off(rec_port.inbuf, &port_io.in_ptr) !=
		    ptr_off(rec_ref.inbuf, &ref_io.in_ptr)) {
			report_fail(fn, n, "inbuf pointer offset");
			bad = 1;
		}
		if (port_in_left != ref_in_left) {
			report_fail(fn, n, "inbytesleft value");
			bad = 1;
		}
		if (ptr_off(port_in, port_io.in_bytes) !=
		    ptr_off(ref_in, ref_io.in_bytes)) {
			report_fail(fn, n, "in pointer advance");
			bad = 1;
		}
	}
	if (!null_out) {
		if (ptr_off(rec_port.outbuf, &port_io.out_ptr) !=
		    ptr_off(rec_ref.outbuf, &ref_io.out_ptr)) {
			report_fail(fn, n, "outbuf pointer offset");
			bad = 1;
		}
		if (port_out_left != ref_out_left) {
			report_fail(fn, n, "outbytesleft value");
			bad = 1;
		}
		if (ptr_off(port_out, port_io.out_bytes) !=
		    ptr_off(ref_out, ref_io.out_bytes)) {
			report_fail(fn, n, "out pointer advance");
			bad = 1;
		}
	}
	if (std::memcmp(port_io.in_bytes, ref_io.in_bytes,
	    sizeof(port_io.in_bytes)) != 0) {
		report_fail(fn, n, "input guard buffer");
		bad = 1;
	}
	if (std::memcmp(port_io.out_bytes, ref_io.out_bytes,
	    sizeof(port_io.out_bytes)) != 0) {
		report_fail(fn, n, "output guard buffer");
		bad = 1;
	}
	if (g_open_into.calls != 0 || g_ex_iconv.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_iconv_cases(void)
{
	for (int pi = 0; pi < N_EDGE_PTRS; pi++) {
		for (int ii = 0; ii < N_EDGE_IN; ii++) {
			for (int null_in = 0; null_in < 2; null_in++) {
				for (int null_out = 0; null_out < 2; null_out++) {
					IoSetup pa, pb;
					iconv_t cd = reinterpret_cast<iconv_t>(
					    EDGE_PTRS[pi]);
					init_io_setup(pa, EDGE_IN_DATA[ii],
					    EDGE_IN_LENS[ii], OUT_CAP,
					    static_cast<size_t>(EDGE_IN_LENS[ii]),
					    static_cast<size_t>(OUT_CAP));
					init_io_setup(pb, EDGE_IN_DATA[ii],
					    EDGE_IN_LENS[ii], OUT_CAP,
					    static_cast<size_t>(EDGE_IN_LENS[ii]),
					    static_cast<size_t>(OUT_CAP));
					run_iconv_case(cd, pa, pb,
					    null_in != 0, null_out != 0,
					    static_cast<uint32_t>(pi * 7919u + ii));
				}
			}
		}
	}
}

static void
random_iconv_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		IoSetup pa, pb;
		unsigned char tmp[IN_CAP];
		int in_len = static_cast<int>(rng_u32() % (IN_CAP + 1));
		int out_cap = static_cast<int>(rng_u32() % (OUT_CAP + 1));
		for (int i = 0; i < in_len; i++) {
			uint32_t r = rng_u32();
			if ((r & 7u) == 0u)
				tmp[i] = '\0';
			else if ((r & 3u) == 0u)
				tmp[i] = static_cast<unsigned char>(
				    0x80u | (r & 0x7fu));
			else
				tmp[i] = static_cast<unsigned char>(r & 0xffu);
		}
		size_t in_left = static_cast<size_t>(rng_u32() % (in_len + 1));
		size_t out_left = static_cast<size_t>(rng_u32() % (out_cap + 1));
		init_io_setup(pa, tmp, in_len, out_cap, in_left, out_left);
		init_io_setup(pb, tmp, in_len, out_cap, in_left, out_left);
		uintptr_t v = static_cast<uintptr_t>(rng_u32());
		v ^= static_cast<uintptr_t>(rng_u32()) << 19;
		iconv_t cd = reinterpret_cast<iconv_t>(v);
		run_iconv_case(cd, pa, pb,
		    (rng_u32() & 15u) == 0u,
		    (rng_u32() & 15u) == 0u,
		    rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* _citrus_string_hash_func                                             */
/* ------------------------------------------------------------------ */

static void
run_hash_case(const char *key, int hashsize, uint32_t tag)
{
	Fn fn = FN_HASH;
	int ret_port, ret_ref;
	long long n;
	int bad = 0;

	(void)tag;

	if (hashsize == 0)
		return;

	n = fn_cases[fn]++;

	ret_port = port::_citrus_string_hash_func(key, hashsize);
	ret_ref = ref__citrus_string_hash_func(key, hashsize);

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_hash_cases(void)
{
	for (int ki = 0; ki < N_EDGE_KEYS; ki++) {
		for (int hi = 0; hi < N_HASH_SIZES; hi++) {
			KeyBuf kb;
			fill_key_buf(kb, EDGE_KEYS[ki], EDGE_KEY_LENS[ki]);
			run_hash_case(key_ptr(kb), HASH_SIZES[hi],
			    static_cast<uint32_t>(ki * 1009u + hi));
		}
	}

	for (int ei = 0; ei < NEDGE; ei++) {
		KeyBuf kb;
		unsigned char ch = static_cast<unsigned char>(EDGE_INTS[ei]);
		fill_key_buf(kb, &ch, 1);
		run_hash_case(key_ptr(kb), EDGE_INTS[ei],
		    static_cast<uint32_t>(0xface0000u + ei));
	}
}

static void
random_hash_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		KeyBuf kb;
		unsigned char tmp[KEY_CAP];
		int len = static_cast<int>(rng_u32() % (KEY_CAP + 1));
		for (int i = 0; i < len; i++) {
			uint32_t r = rng_u32();
			if ((r & 7u) == 0u)
				tmp[i] = '\0';
			else if ((r & 3u) == 0u)
				tmp[i] = static_cast<unsigned char>(
				    0x80u | (r & 0x7fu));
			else if ((r & 5u) == 0u)
				tmp[i] = static_cast<unsigned char>(
				    'A' + (r % 26u));
			else
				tmp[i] = static_cast<unsigned char>(r & 0xffu);
		}
		fill_key_buf(kb, tmp, len);
		int hashsize;
		if ((rng_u32() & 7u) == 0u)
			hashsize = EDGE_INTS[rng_u32() % static_cast<unsigned>(NEDGE)];
		else if ((rng_u32() & 3u) == 0u)
			hashsize = -static_cast<int>(rng_u32() % 4096u + 1u);
		else
			hashsize = static_cast<int>(rng_u32() % 8192u + 1u);
		if (hashsize == 0)
			hashsize = 1;
		run_hash_case(key_ptr(kb), hashsize, rng_u32());
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	rng_seed(0x0072007200720072ULL);

	edge_open_into_cases();
	edge_ex_iconv_cases();
	edge_iconv_cases();
	edge_hash_cases();

	random_open_into_sweep(RANDOM_ITERS);
	random_ex_iconv_sweep(RANDOM_ITERS);
	random_iconv_sweep(RANDOM_ITERS);
	random_hash_sweep(RANDOM_ITERS);

	std::printf("\n%-28s %12s %12s %s\n", "FUNCTION", "CASES", "FAILURES",
	    "RESULT");
	std::printf("%-28s %12s %12s %s\n", "----------------------------",
	    "------------", "------------", "------");

	long long total_cases = 0;
	long long total_fail = 0;

	for (int i = 0; i < FN_COUNT; i++) {
		std::printf("%-28s %12lld %12lld %s\n", fn_name[i], fn_cases[i],
		    fn_fails[i], fn_fails[i] == 0 ? "PASS" : "FAIL");
		total_cases += fn_cases[i];
		total_fail += fn_fails[i];
	}

	std::printf("%-28s %12lld %12lld %s\n", "TOTAL", total_cases, total_fail,
	    total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
