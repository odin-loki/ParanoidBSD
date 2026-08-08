/*
 * Differential test harness for batch b0062.
 *
 * iconv_close, iconvctl, __iconv_free_list and iconv_open are thin wrappers
 * that forward to __bsd_* entry points.  Recording mocks for those primitives
 * are linked by both the ref_ oracle and the C++23 port so that a wrong
 * target, transposed argument, or altered return cannot pass unnoticed.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.iconv.b0062;

namespace port = pbsd::lib_libc_iconv::b0062;

extern "C" {
typedef void *iconv_t;

int ref_iconv_close(iconv_t);
int ref_iconvctl(iconv_t, int, void *);
void ref___iconv_free_list(char **, size_t);
iconv_t ref_iconv_open(const char *, const char *);
}

/* ------------------------------------------------------------------ */
/* Recording mocks (linked by both oracle and port).                  */
/* ------------------------------------------------------------------ */

#define	GUARD			0x7f
#define	CTL_PAD			64
#define	CTL_DATA		32
#define	PATH_CAP		256
#define	PATH_GUARD_PAD		32
#define	LIST_CAP		16
#define	STR_CAP			64
#define	STR_GUARD_PAD		16
#define	RANDOM_ITERS		200000LL

struct CloseRec {
	int	calls;
	iconv_t	arg;
	int	ret;
};

struct CtlRec {
	int	calls;
	iconv_t	arg_a;
	int	arg_b;
	const void *arg_c;
	int	ret;
};

struct FreeRec {
	int	calls;
	char	**arg_a;
	size_t	arg_b;
};

struct OpenRec {
	int	calls;
	const char *arg_from;
	const char *arg_to;
	iconv_t	ret;
};

static CloseRec g_close;
static CtlRec g_ctl;
static FreeRec g_free;
static OpenRec g_open;

static void
close_reset(void)
{
	g_close.calls = 0;
	g_close.arg = reinterpret_cast<iconv_t>(static_cast<intptr_t>(-0x1111));
	g_close.ret = -0x1112;
}

static void
ctl_reset(void)
{
	g_ctl.calls = 0;
	g_ctl.arg_a = reinterpret_cast<iconv_t>(static_cast<intptr_t>(-0x2221));
	g_ctl.arg_b = -0x2222;
	g_ctl.arg_c = reinterpret_cast<const void *>(static_cast<intptr_t>(-0x2223));
	g_ctl.ret = -0x2224;
}

static void
free_reset(void)
{
	g_free.calls = 0;
	g_free.arg_a = reinterpret_cast<char **>(
	    static_cast<intptr_t>(-0x3331));
	g_free.arg_b = static_cast<size_t>(-1);
}

static void
open_reset(void)
{
	g_open.calls = 0;
	g_open.arg_from = reinterpret_cast<const char *>(static_cast<intptr_t>(-0x4441));
	g_open.arg_to = reinterpret_cast<const char *>(static_cast<intptr_t>(-0x4442));
	g_open.ret = reinterpret_cast<iconv_t>(static_cast<intptr_t>(-0x4443));
}

static unsigned
hash_bytes(const unsigned char *p, int len)
{
	unsigned h = 2166136261u;

	if (p == nullptr)
		return h;
	if (len < 0)
		len = 0;
	for (int i = 0; i < len; i++) {
		h ^= p[i];
		h *= 16777619u;
	}
	return h;
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

extern "C" int
__bsd_iconv_close(iconv_t a)
{
	unsigned h = hash_bytes(reinterpret_cast<const unsigned char *>(&a),
	    static_cast<int>(sizeof(a)));
	int ret = static_cast<int>(h ^ (h >> 16) ^ 0x5a5a5a5au);

	g_close.calls++;
	g_close.arg = a;
	g_close.ret = ret;
	return ret;
}

extern "C" int
__bsd_iconvctl(iconv_t a, int b, void *c)
{
	unsigned h = hash_bytes(reinterpret_cast<const unsigned char *>(&a),
	    static_cast<int>(sizeof(a)));
	h ^= static_cast<unsigned>(b) * 2654435761u;
	h ^= (c != nullptr) ? 0x9e3779b9u : 0x517cc1b7u;

	if (c != nullptr) {
		unsigned char *p = static_cast<unsigned char *>(c);
		for (int i = 0; i < CTL_DATA; i++) {
			p[i] = static_cast<unsigned char>(
			    (h >> (i & 7)) ^ (static_cast<unsigned>(b) * 31u) ^
			    (static_cast<unsigned>(reinterpret_cast<intptr_t>(a) &
				0xffu) * 17u) + static_cast<unsigned>(i) * 7u +
			    0xa5u);
		}
	}

	int ret = static_cast<int>(h ^ (h >> 11));
	ret += b;
	ret ^= static_cast<int>(reinterpret_cast<intptr_t>(a) & 0xffff);

	g_ctl.calls++;
	g_ctl.arg_a = a;
	g_ctl.arg_b = b;
	g_ctl.arg_c = c;
	g_ctl.ret = ret;
	return ret;
}

extern "C" void
__bsd___iconv_free_list(char **a, size_t b)
{
	g_free.calls++;
	g_free.arg_a = a;
	g_free.arg_b = b;

	for (size_t i = 0; i < b; i++) {
		if (a == nullptr)
			break;
		if (a[i] == nullptr)
			continue;
		unsigned char *p = reinterpret_cast<unsigned char *>(a[i]);
		p[0] = static_cast<unsigned char>(0x80u | (i & 0x7fu));
		p[1] = static_cast<unsigned char>((b >> (i & 3)) & 0xffu);
		p[2] = static_cast<unsigned char>((i * 17u + b) & 0xffu);
	}
}

extern "C" iconv_t
__bsd_iconv_open(const char *a, const char *b)
{
	unsigned ha = hash_cstr(a);
	unsigned hb = hash_cstr(b);
	uintptr_t h = static_cast<uintptr_t>(ha ^ (hb * 40503u) ^ 0x13579bu);

	if (a != nullptr) {
		for (int i = 0; a[i] != '\0' && i < PATH_CAP; i++)
			h ^= static_cast<uintptr_t>(static_cast<unsigned char>(a[i]))
			    << (static_cast<unsigned>(i) % 24);
	}
	if (b != nullptr) {
		for (int i = 0; b[i] != '\0' && i < PATH_CAP; i++)
			h ^= static_cast<uintptr_t>(static_cast<unsigned char>(b[i]))
			    << (static_cast<unsigned>(i + 3) % 24);
	}
	if (h == 0)
		h = 1;
	iconv_t ret = reinterpret_cast<iconv_t>(h | 1u);

	g_open.calls++;
	g_open.arg_from = a;
	g_open.arg_to = b;
	g_open.ret = ret;
	return ret;
}

/* ------------------------------------------------------------------ */
/* Per-function statistics                                              */
/* ------------------------------------------------------------------ */

enum Fn {
	FN_CLOSE,
	FN_CTL,
	FN_FREE,
	FN_OPEN,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"iconv_close",
	"iconvctl",
	"__iconv_free_list",
	"iconv_open",
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

struct CtlBuf {
	unsigned char bytes[CTL_PAD + CTL_DATA + CTL_PAD];
};

struct ListSetup {
	char *slots[LIST_CAP];
	unsigned char str_storage[LIST_CAP][STR_GUARD_PAD + STR_CAP + STR_GUARD_PAD];
	unsigned char list_guard[PATH_GUARD_PAD];
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

static void *
ctl_ptr(CtlBuf &cb, int off)
{
	return cb.bytes + CTL_PAD + off;
}

static void
fill_ctl_buf(CtlBuf &cb, uint32_t seed)
{
	std::memset(cb.bytes, GUARD, sizeof(cb.bytes));
	uint32_t h = seed;
	for (int i = 0; i < CTL_DATA; i++) {
		h = h * 1103515245u + 12345u;
		cb.bytes[CTL_PAD + i] = static_cast<unsigned char>(h >> 16);
	}
}

static void
init_list_setup(ListSetup &ls, const unsigned char *const *src,
    const int *lens, int count)
{
	std::memset(&ls, 0, sizeof(ls));
	std::memset(ls.list_guard, GUARD, sizeof(ls.list_guard));
	for (int i = 0; i < LIST_CAP; i++)
		ls.slots[i] = nullptr;
	if (count < 0)
		count = 0;
	if (count > LIST_CAP)
		count = LIST_CAP;
	for (int i = 0; i < count; i++) {
		std::memset(ls.str_storage[i], GUARD,
		    sizeof(ls.str_storage[i]));
		int len = lens[i];
		if (len < 0)
			len = 0;
		if (len > STR_CAP)
			len = STR_CAP;
		if (src[i] != nullptr && len > 0) {
			std::memcpy(ls.str_storage[i] + STR_GUARD_PAD, src[i],
			    static_cast<size_t>(len));
		}
		ls.str_storage[i][STR_GUARD_PAD + len] = '\0';
		ls.slots[i] = reinterpret_cast<char *>(ls.str_storage[i] +
		    STR_GUARD_PAD);
	}
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
	{ 'a', '\0', 'b', 0 },
	{ 0x80, 0xff, 0x7f, 0x00, 0xfe, 0 },
};

static const int EDGE_PATH_LENS[] = {
	0, 1, 3, 1, 1, 3, 3, 5
};
static const int N_EDGE_PATHS =
    static_cast<int>(sizeof(EDGE_PATHS) / sizeof(EDGE_PATHS[0]));

static const intptr_t EDGE_PTRS[] = {
	0,
	1,
	-1,
	0x7f,
	0x80,
	0xff,
	0x100,
	0x7fffffff,
	-0x7fffffff,
	0xdeadbeef,
	-0x100,
};

static const int N_EDGE_PTRS =
    static_cast<int>(sizeof(EDGE_PTRS) / sizeof(EDGE_PTRS[0]));

/* ------------------------------------------------------------------ */
/* iconv_close                                                          */
/* ------------------------------------------------------------------ */

static void
run_close_case(iconv_t handle, uint32_t tag)
{
	Fn fn = FN_CLOSE;
	CloseRec rec_port, rec_ref;
	int ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ret_port = port::iconv_close(handle);
	rec_port = g_close;

	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ret_ref = ref_iconv_close(handle);
	rec_ref = g_close;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.arg != rec_ref.arg) {
		report_fail(fn, n, "iconv_t argument");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (g_ctl.calls != 0 || g_free.calls != 0 || g_open.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_close_cases(void)
{
	for (int i = 0; i < N_EDGE_PTRS; i++) {
		iconv_t h = reinterpret_cast<iconv_t>(EDGE_PTRS[i]);
		run_close_case(h, static_cast<uint32_t>(0x6000u + i));
	}
}

static void
random_close_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		uintptr_t v = static_cast<uintptr_t>(rng_u32());
		v ^= static_cast<uintptr_t>(rng_u32()) << 16;
		v ^= static_cast<uintptr_t>(rng_u32()) << 32;
		iconv_t h = reinterpret_cast<iconv_t>(v);
		run_close_case(h, rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* iconvctl                                                             */
/* ------------------------------------------------------------------ */

static void
run_ctl_case(iconv_t handle, int req, CtlBuf &port_buf, CtlBuf &ref_buf,
    void *port_c, void *ref_c, uint32_t tag)
{
	Fn fn = FN_CTL;
	CtlRec rec_port, rec_ref;
	int ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ret_port = port::iconvctl(handle, req, port_c);
	rec_port = g_ctl;

	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ret_ref = ref_iconvctl(handle, req, ref_c);
	rec_ref = g_ctl;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.arg_a != rec_ref.arg_a) {
		report_fail(fn, n, "iconv_t argument");
		bad = 1;
	}
	if (rec_port.arg_b != rec_ref.arg_b) {
		report_fail(fn, n, "request argument");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (port_c != nullptr && ref_c != nullptr) {
		if (ptr_off(rec_port.arg_c, port_buf.bytes) !=
		    ptr_off(rec_ref.arg_c, ref_buf.bytes)) {
			report_fail(fn, n, "void pointer offset");
			bad = 1;
		}
	} else if (rec_port.arg_c != rec_ref.arg_c) {
		report_fail(fn, n, "void pointer null mismatch");
		bad = 1;
	}
	if (std::memcmp(port_buf.bytes, ref_buf.bytes, sizeof(port_buf.bytes)) !=
	    0) {
		report_fail(fn, n, "ctl guard buffer");
		bad = 1;
	}
	if (g_close.calls != 0 || g_free.calls != 0 || g_open.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_ctl_cases(void)
{
	for (int pi = 0; pi < N_EDGE_PTRS; pi++) {
		for (int ei = 0; ei < NEDGE; ei++) {
			for (int null_c = 0; null_c < 2; null_c++) {
				for (int off = 0; off < 8; off++) {
					iconv_t h = reinterpret_cast<iconv_t>(
					    EDGE_PTRS[pi]);
					CtlBuf pa, pb;
					fill_ctl_buf(pa,
					    static_cast<uint32_t>(pi * 97u + ei));
					fill_ctl_buf(pb,
					    static_cast<uint32_t>(pi * 97u + ei));
					void *pc = null_c ? nullptr :
					    ctl_ptr(pa, off);
					void *rc = null_c ? nullptr :
					    ctl_ptr(pb, off);
					run_ctl_case(h, EDGE_INTS[ei], pa, pb,
					    pc, rc,
					    static_cast<uint32_t>(pi * 1009u +
						ei * 17u + off));
				}
			}
		}
	}
}

static void
random_ctl_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		uintptr_t v = static_cast<uintptr_t>(rng_u32());
		v ^= static_cast<uintptr_t>(rng_u32()) << 17;
		iconv_t h = reinterpret_cast<iconv_t>(v);
		int req = static_cast<int>(rng_u32());
		if ((rng_u32() & 3u) == 0u)
			req = EDGE_INTS[rng_u32() % static_cast<unsigned>(NEDGE)];
		CtlBuf pa, pb;
		uint32_t seed = rng_u32();
		fill_ctl_buf(pa, seed);
		fill_ctl_buf(pb, seed);
		bool use_c = (rng_u32() & 7u) != 0u;
		int off = static_cast<int>(rng_u32() % CTL_PAD);
		void *pc = use_c ? ctl_ptr(pa, off) : nullptr;
		void *rc = use_c ? ctl_ptr(pb, off) : nullptr;
		run_ctl_case(h, req, pa, pb, pc, rc, rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* __iconv_free_list                                                    */
/* ------------------------------------------------------------------ */

static void
run_free_case(ListSetup &port_ls, ListSetup &ref_ls, size_t count,
    uint32_t tag)
{
	Fn fn = FN_FREE;
	FreeRec rec_port, rec_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	port::__iconv_free_list(port_ls.slots, count);
	rec_port = g_free;

	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ref___iconv_free_list(ref_ls.slots, count);
	rec_ref = g_free;

	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (rec_port.arg_b != rec_ref.arg_b) {
		report_fail(fn, n, "count argument");
		bad = 1;
	}
	if (rec_port.arg_a != port_ls.slots) {
		report_fail(fn, n, "port list pointer");
		bad = 1;
	}
	if (rec_ref.arg_a != ref_ls.slots) {
		report_fail(fn, n, "ref list pointer");
		bad = 1;
	}
	for (int i = 0; i < LIST_CAP; i++) {
		if (std::memcmp(port_ls.str_storage[i], ref_ls.str_storage[i],
		    sizeof(port_ls.str_storage[i])) != 0) {
			report_fail(fn, n, "string guard buffer");
			bad = 1;
			break;
		}
	}
	if (std::memcmp(port_ls.list_guard, ref_ls.list_guard,
	    sizeof(port_ls.list_guard)) != 0) {
		report_fail(fn, n, "list guard");
		bad = 1;
	}
	if (g_close.calls != 0 || g_ctl.calls != 0 || g_open.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
edge_free_cases(void)
{
	static const unsigned char *one[] = {
		reinterpret_cast<const unsigned char *>("a"),
	};
	static const int one_len[] = { 1 };

	static const unsigned char *empty[] = {
		reinterpret_cast<const unsigned char *>(""),
	};
	static const int empty_len[] = { 0 };

	static const unsigned char *hi[] = {
		reinterpret_cast<const unsigned char *>("\x80\xff"),
	};
	static const int hi_len[] = { 2 };

	static const unsigned char *multi[] = {
		reinterpret_cast<const unsigned char *>(""),
		reinterpret_cast<const unsigned char *>("x"),
		nullptr,
		reinterpret_cast<const unsigned char *>("\xff"),
	};
	static const int multi_len[] = { 0, 1, 0, 1 };

	for (size_t count = 0; count <= static_cast<size_t>(LIST_CAP); count++) {
		ListSetup pa, pb;
		init_list_setup(pa, one, one_len, 1);
		init_list_setup(pb, one, one_len, 1);
		run_free_case(pa, pb, count, static_cast<uint32_t>(count));

		init_list_setup(pa, empty, empty_len, 1);
		init_list_setup(pb, empty, empty_len, 1);
		run_free_case(pa, pb, count, static_cast<uint32_t>(0x1000u + count));

		init_list_setup(pa, hi, hi_len, 1);
		init_list_setup(pb, hi, hi_len, 1);
		run_free_case(pa, pb, count, static_cast<uint32_t>(0x2000u + count));

		init_list_setup(pa, multi, multi_len, 4);
		init_list_setup(pb, multi, multi_len, 4);
		run_free_case(pa, pb, count, static_cast<uint32_t>(0x3000u + count));
	}
}

static void
random_free_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		int count = static_cast<int>(rng_u32() % (LIST_CAP + 1));
		unsigned char tmp[LIST_CAP][STR_CAP];
		const unsigned char *src[LIST_CAP];
		int lens[LIST_CAP];
		for (int i = 0; i < count; i++) {
			int len = static_cast<int>(rng_u32() % (STR_CAP + 1));
			for (int j = 0; j < len; j++) {
				uint32_t r = rng_u32();
				if ((r & 7u) == 0u)
					tmp[i][j] = '\0';
				else if ((r & 3u) == 0u)
					tmp[i][j] = static_cast<unsigned char>(
					    0x80u | (r & 0x7fu));
				else
					tmp[i][j] = static_cast<unsigned char>(
					    r & 0xffu);
			}
			lens[i] = len;
			if ((rng_u32() & 15u) == 0u) {
				src[i] = nullptr;
				lens[i] = 0;
			} else {
				src[i] = tmp[i];
			}
		}
		ListSetup pa, pb;
		init_list_setup(pa, src, lens, count);
		init_list_setup(pb, src, lens, count);
		run_free_case(pa, pb, static_cast<size_t>(count), rng_u32());
	}
}

/* ------------------------------------------------------------------ */
/* iconv_open                                                           */
/* ------------------------------------------------------------------ */

static void
run_open_case(const char *from, const char *to, uint32_t tag)
{
	Fn fn = FN_OPEN;
	OpenRec rec_port, rec_ref;
	iconv_t ret_port, ret_ref;
	long long n = fn_cases[fn]++;
	int bad = 0;

	(void)tag;
	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ret_port = port::iconv_open(from, to);
	rec_port = g_open;

	close_reset();
	ctl_reset();
	free_reset();
	open_reset();

	ret_ref = ref_iconv_open(from, to);
	rec_ref = g_open;

	if (ret_port != ret_ref) {
		report_fail(fn, n, "return value");
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls || rec_port.calls != 1) {
		report_fail(fn, n, "dispatch count");
		bad = 1;
	}
	if (ptr_off(rec_port.arg_from, from) != ptr_off(rec_ref.arg_from, from)) {
		report_fail(fn, n, "from pointer offset");
		bad = 1;
	}
	if (ptr_off(rec_port.arg_to, to) != ptr_off(rec_ref.arg_to, to)) {
		report_fail(fn, n, "to pointer offset");
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report_fail(fn, n, "recorded ret");
		bad = 1;
	}
	if (g_close.calls != 0 || g_ctl.calls != 0 || g_free.calls != 0) {
		report_fail(fn, n, "wrong primitive invoked");
		bad = 1;
	}

	if (bad)
		fn_fails[fn]++;
}

static void
run_open_pair(PathBuf &port_from, PathBuf &port_to, PathBuf &ref_from,
    PathBuf &ref_to, uint32_t tag)
{
	run_open_case(path_ptr(port_from), path_ptr(port_to), tag);
	(void)ref_from;
	(void)ref_to;
}

static void
edge_open_cases(void)
{
	for (int fi = 0; fi < N_EDGE_PATHS; fi++) {
		for (int ti = 0; ti < N_EDGE_PATHS; ti++) {
			PathBuf pa, pb, pc, pd;
			fill_path_buf(pa, EDGE_PATHS[fi], EDGE_PATH_LENS[fi]);
			fill_path_buf(pb, EDGE_PATHS[ti], EDGE_PATH_LENS[ti]);
			fill_path_buf(pc, EDGE_PATHS[fi], EDGE_PATH_LENS[fi]);
			fill_path_buf(pd, EDGE_PATHS[ti], EDGE_PATH_LENS[ti]);
			run_open_pair(pa, pb, pc, pd,
			    static_cast<uint32_t>(fi * 1009u + ti));
		}
	}

	for (int i = 0; i < NEDGE; i++) {
		PathBuf pa, pb, pc, pd;
		unsigned char ch = static_cast<unsigned char>(EDGE_INTS[i]);
		fill_path_buf(pa, &ch, 1);
		fill_path_buf(pb, &ch, 1);
		fill_path_buf(pc, &ch, 1);
		fill_path_buf(pd, &ch, 1);
		run_open_pair(pa, pb, pc, pd, static_cast<uint32_t>(0xface0000u + i));
	}
}

static void
random_open_sweep(long long iters)
{
	for (long long n = 0; n < iters; n++) {
		PathBuf pa, pb, pc, pd;
		int len_a = static_cast<int>(rng_u32() % (PATH_CAP + 1));
		int len_b = static_cast<int>(rng_u32() % (PATH_CAP + 1));
		unsigned char tmp_a[PATH_CAP];
		unsigned char tmp_b[PATH_CAP];
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
		fill_path_buf(pc, tmp_a, len_a);
		fill_path_buf(pd, tmp_b, len_b);
		run_open_pair(pa, pb, pc, pd, rng_u32());
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	rng_seed(0x0062006200620062ULL);

	edge_close_cases();
	edge_ctl_cases();
	edge_free_cases();
	edge_open_cases();

	random_close_sweep(RANDOM_ITERS);
	random_ctl_sweep(RANDOM_ITERS);
	random_free_sweep(RANDOM_ITERS);
	random_open_sweep(RANDOM_ITERS);

	std::printf("\n%-24s %12s %12s %s\n", "FUNCTION", "CASES", "FAILURES",
	    "RESULT");
	std::printf("%-24s %12s %12s %s\n", "------------------------",
	    "------------", "------------", "------");

	long long total_cases = 0;
	long long total_fail = 0;

	for (int i = 0; i < FN_COUNT; i++) {
		std::printf("%-24s %12lld %12lld %s\n", fn_name[i], fn_cases[i],
		    fn_fails[i], fn_fails[i] == 0 ? "PASS" : "FAIL");
		total_cases += fn_cases[i];
		total_fail += fn_fails[i];
	}

	std::printf("%-24s %12lld %12lld %s\n", "TOTAL", total_cases, total_fail,
	    total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
