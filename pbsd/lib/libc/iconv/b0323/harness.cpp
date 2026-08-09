/*
 * Differential test harness for batch b0323: citrus_prop.c
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdbool.h>

import pbsd.lib.libc.iconv.b0323;

namespace port = pbsd::lib_libc_iconv::b0323;

extern "C" {

typedef enum {
	_CITRUS_PROP_BOOL = 0,
	_CITRUS_PROP_STR  = 1,
	_CITRUS_PROP_CHR  = 2,
	_CITRUS_PROP_NUM  = 3,
} _citrus_prop_type_t;

typedef struct {
	_citrus_prop_type_t type;
	union {
		const char *str;
		int chr;
		bool boolean;
		std::uint64_t num;
	} u;
} _citrus_prop_object_t;

typedef struct _citrus_prop_hint_t _citrus_prop_hint_t;

struct _citrus_memory_stream {
	struct {
		void   *r_head;
		size_t  r_size;
	} ms_region;
	size_t ms_pos;
};

void ref__citrus_prop_object_init(_citrus_prop_object_t *, _citrus_prop_type_t);
void ref__citrus_prop_object_uninit(_citrus_prop_object_t *);
int ref__citrus_prop_read_chr_common(struct _citrus_memory_stream *,
    int *, int, int);
int ref__citrus_prop_read_num_common(struct _citrus_memory_stream *,
    std::uint64_t *, int, int);
int ref__citrus_prop_read_chr(struct _citrus_memory_stream *,
    _citrus_prop_object_t *);
int ref__citrus_prop_read_num(struct _citrus_memory_stream *,
    _citrus_prop_object_t *);
int ref__citrus_prop_read_character_common(struct _citrus_memory_stream *,
    int *);
int ref__citrus_prop_read_character(struct _citrus_memory_stream *,
    _citrus_prop_object_t *);
int ref__citrus_prop_read_bool(struct _citrus_memory_stream *,
    _citrus_prop_object_t *);
int ref__citrus_prop_read_str(struct _citrus_memory_stream *,
    _citrus_prop_object_t *);
int ref__citrus_prop_read_symbol(struct _citrus_memory_stream *, char *,
    size_t);
int ref__citrus_prop_parse_element(struct _citrus_memory_stream *,
    const _citrus_prop_hint_t *, void *);
int ref__citrus_prop_parse_variable(const _citrus_prop_hint_t *, void *,
    const void *, size_t);

typedef int (*_citrus_prop_boolean_cb_func_t)(void * __restrict,
    const char *, int);
typedef int (*_citrus_prop_str_cb_func_t)(void * __restrict, const char *,
    const char *);
typedef int (*_citrus_prop_chr_cb_func_t)(void * __restrict, const char *,
    int, int);
typedef int (*_citrus_prop_num_cb_func_t)(void * __restrict, const char *,
    std::uint64_t, std::uint64_t);

struct _citrus_prop_hint_t {
	const char *name;
	_citrus_prop_type_t type;
	union {
		struct { _citrus_prop_boolean_cb_func_t func; } boolean;
		struct { _citrus_prop_str_cb_func_t func; } str;
		struct { _citrus_prop_chr_cb_func_t func; } chr;
		struct { _citrus_prop_num_cb_func_t func; } num;
	} cb;
};

} /* extern "C" */

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t SYM_CAP = 320;
constexpr std::size_t POOL_CAP = 8192;
constexpr long long RANDOM_ITERS = 50000LL;

using PortObj = port::_citrus_prop_object_t;
using PortMs = port::_citrus_memory_stream;

enum FnId : int {
	F_OBJECT_INIT,
	F_OBJECT_UNINIT,
	F_READ_CHR_COMMON,
	F_READ_NUM_COMMON,
	F_READ_CHR,
	F_READ_NUM,
	F_READ_CHARACTER_COMMON,
	F_READ_CHARACTER,
	F_READ_BOOL,
	F_READ_STR,
	F_READ_SYMBOL,
	F_PARSE_ELEMENT,
	F_PARSE_VARIABLE,
	F_COUNT
};

const char *const fname[F_COUNT] = {
	"_citrus_prop_object_init",
	"_citrus_prop_object_uninit",
	"_citrus_prop_read_chr_common",
	"_citrus_prop_read_num_common",
	"_citrus_prop_read_chr",
	"_citrus_prop_read_num",
	"_citrus_prop_read_character_common",
	"_citrus_prop_read_character",
	"_citrus_prop_read_bool",
	"_citrus_prop_read_str",
	"_citrus_prop_read_symbol",
	"_citrus_prop_parse_element",
	"_citrus_prop_parse_variable",
};

unsigned long ncase[F_COUNT];
unsigned long nfail[F_COUNT];
unsigned nprinted[F_COUNT];

std::uint64_t rng = 0x0323b0323b0323ULL;

std::uint64_t
rng_next(void)
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

unsigned char
rand_byte(void)
{
	return ((unsigned char)(rng_next() & 0xffu));
}

void
fail(FnId f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 10)
		std::printf("  FAIL %-36s : %s\n", fname[f], why);
}

void
case_bump(FnId f)
{
	ncase[f]++;
}

bool
bufs_equal(const unsigned char *a, const unsigned char *b, std::size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

void
bind_ms(struct _citrus_memory_stream *ms, void *p, std::size_t n)
{
	ms->ms_region.r_head = p;
	ms->ms_region.r_size = n;
	ms->ms_pos = 0;
}

void
bind_ms(PortMs *ms, void *p, std::size_t n)
{
	ms->ms_region.r_head = p;
	ms->ms_region.r_size = n;
	ms->ms_pos = 0;
}

bool
ms_equal(const struct _citrus_memory_stream *a, const PortMs *b)
{
	return (a->ms_pos == b->ms_pos &&
	    a->ms_region.r_size == b->ms_region.r_size);
}

struct Ctx {
	int		cb_ret;
	int		bool_n;
	int		bool_v;
	const char	*bool_name;
	int		str_n;
	char		str_buf[512];
	const char	*str_name;
	int		chr_n;
	int		chr_a;
	int		chr_b;
	const char	*chr_name;
	int		num_n;
	std::uint64_t	num_a;
	std::uint64_t	num_b;
	const char	*num_name;
};

extern "C" int
cb_bool(void *ctx, const char *name, int v)
{
	auto *c = (Ctx *)ctx;

	c->bool_n++;
	c->bool_v = v;
	c->bool_name = name;
	return (c->cb_ret);
}

extern "C" int
cb_str(void *ctx, const char *name, const char *s)
{
	auto *c = (Ctx *)ctx;

	c->str_n++;
	c->str_name = name;
	std::strncpy(c->str_buf, s != nullptr ? s : "", sizeof(c->str_buf) - 1);
	c->str_buf[sizeof(c->str_buf) - 1] = '\0';
	return (c->cb_ret);
}

extern "C" int
cb_chr(void *ctx, const char *name, int a, int b)
{
	auto *c = (Ctx *)ctx;

	c->chr_n++;
	c->chr_name = name;
	c->chr_a = a;
	c->chr_b = b;
	return (c->cb_ret);
}

extern "C" int
cb_num(void *ctx, const char *name, std::uint64_t a, std::uint64_t b)
{
	auto *c = (Ctx *)ctx;

	c->num_n++;
	c->num_name = name;
	c->num_a = a;
	c->num_b = b;
	return (c->cb_ret);
}

void
reset_ctx(Ctx *c)
{
	std::memset(c, 0, sizeof(*c));
}

bool
ctx_equal(const Ctx *a, const Ctx *b)
{
	if (a->cb_ret != b->cb_ret || a->bool_n != b->bool_n ||
	    a->bool_v != b->bool_v || a->str_n != b->str_n ||
	    a->chr_n != b->chr_n || a->chr_a != b->chr_a ||
	    a->chr_b != b->chr_b || a->num_n != b->num_n ||
	    a->num_a != b->num_a || a->num_b != b->num_b)
		return (false);
	if (a->bool_name != b->bool_name || a->str_name != b->str_name ||
	    a->chr_name != b->chr_name || a->num_name != b->num_name)
		return (false);
	return (std::strcmp(a->str_buf, b->str_buf) == 0);
}

void
test_object_init(_citrus_prop_type_t ty)
{
	_citrus_prop_object_t ro{};
	PortObj po{};

	case_bump(F_OBJECT_INIT);
	ref__citrus_prop_object_init(&ro, ty);
	port::_citrus_prop_object_init(&po, (port::_citrus_prop_type_t)ty);
	if (ro.type != (decltype(ro.type))po.type ||
	    std::memcmp(&ro.u, &po.u, sizeof(ro.u)) != 0)
		fail(F_OBJECT_INIT, "object bytes differ");
}

void
test_object_uninit(void)
{
	char *rs = (char *)std::malloc(16);
	char *ps = (char *)std::malloc(16);

	if (rs == nullptr || ps == nullptr)
		return;
	std::strcpy(rs, "release");
	std::strcpy(ps, "release");

	case_bump(F_OBJECT_UNINIT);
	{
		_citrus_prop_object_t ro{};
		ro.type = _CITRUS_PROP_STR;
		ro.u.str = rs;
		ref__citrus_prop_object_uninit(&ro);
	}
	{
		PortObj po{};
		po.type = port::_CITRUS_PROP_STR;
		po.u.str = ps;
		port::_citrus_prop_object_uninit(&po);
	}
}

void
test_read_chr_common(const char *input, int base, int neg, int expect,
    std::size_t expect_pos, bool check = true)
{
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	int rv_r, rv_p, res_r = -999, res_p = -999;

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));

	case_bump(F_READ_CHR_COMMON);
	rv_r = ref__citrus_prop_read_chr_common(&ms_r, &res_r, base, neg);
	rv_p = port::_citrus_prop_read_chr_common(&ms_p, &res_p, base, neg);
	if (rv_r != rv_p || res_r != res_p || ms_r.ms_pos != ms_p.ms_pos)
		fail(F_READ_CHR_COMMON, "chr_common mismatch");
	if (check) {
		if (ms_r.ms_pos != expect_pos)
			fail(F_READ_CHR_COMMON, "unexpected chr_common pos");
		if (res_r != expect)
			fail(F_READ_CHR_COMMON, "unexpected chr_common value");
	}
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_READ_CHR_COMMON, "pool corruption");
}

void
test_read_num_common(const char *input, int base, int neg,
    std::uint64_t expect, std::size_t expect_pos, bool check = true)
{
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	std::uint64_t res_r = 999, res_p = 999;
	int rv_r, rv_p;

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));

	case_bump(F_READ_NUM_COMMON);
	rv_r = ref__citrus_prop_read_num_common(&ms_r, &res_r, base, neg);
	rv_p = port::_citrus_prop_read_num_common(&ms_p, &res_p, base, neg);
	if (rv_r != rv_p || res_r != res_p || ms_r.ms_pos != ms_p.ms_pos)
		fail(F_READ_NUM_COMMON, "num_common mismatch");
	if (check) {
		if (ms_r.ms_pos != expect_pos)
			fail(F_READ_NUM_COMMON, "unexpected num_common pos");
		if (res_r != expect)
			fail(F_READ_NUM_COMMON, "unexpected num_common value");
	}
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_READ_NUM_COMMON, "pool corruption");
}

void
test_read_typed(FnId fid,
    int (*ref_fn)(struct _citrus_memory_stream *, _citrus_prop_object_t *),
    int (*port_fn)(PortMs *, PortObj *),
    const char *input, int expect_rv,
    _citrus_prop_type_t ty, std::uint64_t expect_u, bool check = true)
{
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	_citrus_prop_object_t oro{};
	PortObj opo{};
	int rv_r, rv_p;

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));
	ref__citrus_prop_object_init(&oro, ty);
	port::_citrus_prop_object_init(&opo, (port::_citrus_prop_type_t)ty);

	case_bump(fid);
	rv_r = ref_fn(&ms_r, &oro);
	rv_p = port_fn(&ms_p, &opo);
	if (rv_r != rv_p || !ms_equal(&ms_r, &ms_p))
		fail(fid, "return/pos mismatch");
	if (check && rv_r != expect_rv)
		fail(fid, "unexpected rv");
	if (rv_r == 0 && check) {
		if (ty == _CITRUS_PROP_CHR) {
			if (oro.u.chr != opo.u.chr)
				fail(fid, "chr value mismatch");
			if ((int)oro.u.chr != (int)expect_u)
				fail(fid, "unexpected chr");
		} else if (ty == _CITRUS_PROP_NUM) {
			if (oro.u.num != opo.u.num)
				fail(fid, "num value mismatch");
			if (oro.u.num != expect_u)
				fail(fid, "unexpected num");
		}
	}
	ref__citrus_prop_object_uninit(&oro);
	port::_citrus_prop_object_uninit(&opo);
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(fid, "pool corruption");
}

void
test_read_character_common(const char *input, int expect_rv, int expect_ch,
    std::size_t expect_pos, bool check = true)
{
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	int rv_r, rv_p, ch_r = -1, ch_p = -1;

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));

	case_bump(F_READ_CHARACTER_COMMON);
	rv_r = ref__citrus_prop_read_character_common(&ms_r, &ch_r);
	rv_p = port::_citrus_prop_read_character_common(&ms_p, &ch_p);
	if (rv_r != rv_p || ch_r != ch_p || ms_r.ms_pos != ms_p.ms_pos)
		fail(F_READ_CHARACTER_COMMON, "mismatch");
	if (check) {
		if (rv_r != expect_rv || ch_r != expect_ch ||
		    ms_r.ms_pos != expect_pos)
			fail(F_READ_CHARACTER_COMMON, "unexpected result");
	}
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_READ_CHARACTER_COMMON, "pool corruption");
}

void
test_read_bool(const char *input, int expect_rv, int expect_bool,
    bool check = true)
{
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	_citrus_prop_object_t oro{};
	PortObj opo{};
	int rv_r, rv_p;

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));
	ref__citrus_prop_object_init(&oro, _CITRUS_PROP_BOOL);
	port::_citrus_prop_object_init(&opo, port::_CITRUS_PROP_BOOL);

	case_bump(F_READ_BOOL);
	rv_r = ref__citrus_prop_read_bool(&ms_r, &oro);
	rv_p = port::_citrus_prop_read_bool(&ms_p, &opo);
	if (rv_r != rv_p || !ms_equal(&ms_r, &ms_p))
		fail(F_READ_BOOL, "return/pos mismatch");
	if (check && rv_r != expect_rv)
		fail(F_READ_BOOL, "unexpected rv");
	if (rv_r == 0 && check &&
	    ((oro.u.boolean != 0) != (opo.u.boolean != 0) ||
	    (oro.u.boolean != 0) != expect_bool))
		fail(F_READ_BOOL, "bool value mismatch");
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_READ_BOOL, "pool corruption");
}

void
test_read_str(const char *input, int expect_rv, const char *expect_s,
    bool check = true)
{
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	_citrus_prop_object_t oro{};
	PortObj opo{};
	int rv_r, rv_p;

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));
	ref__citrus_prop_object_init(&oro, _CITRUS_PROP_STR);
	port::_citrus_prop_object_init(&opo, port::_CITRUS_PROP_STR);

	case_bump(F_READ_STR);
	rv_r = ref__citrus_prop_read_str(&ms_r, &oro);
	rv_p = port::_citrus_prop_read_str(&ms_p, &opo);
	if (rv_r != rv_p || !ms_equal(&ms_r, &ms_p))
		fail(F_READ_STR, "return/pos mismatch");
	if (check && rv_r != expect_rv) {
		fail(F_READ_STR, "unexpected rv");
	} else if (rv_r == 0) {
		const char *sr = oro.u.str;
		const char *sp = opo.u.str;
		if ((sr == nullptr) != (sp == nullptr) ||
		    (sr != nullptr && sp != nullptr &&
			std::strcmp(sr, sp) != 0))
			fail(F_READ_STR, "str content mismatch");
		if (check && expect_s != nullptr && sr != nullptr &&
		    std::strcmp(sr, expect_s) != 0)
			fail(F_READ_STR, "unexpected str");
	}
	ref__citrus_prop_object_uninit(&oro);
	port::_citrus_prop_object_uninit(&opo);
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_READ_STR, "pool corruption");
}

void
test_read_symbol(const char *input, std::size_t cap, int expect_rv,
    const char *expect_name, std::size_t expect_pos, bool check = true)
{
	unsigned char buf_r[SYM_CAP];
	unsigned char buf_p[SYM_CAP];
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	int rv_r, rv_p;

	std::memset(buf_r, GUARD, sizeof(buf_r));
	std::memset(buf_p, GUARD, sizeof(buf_p));
	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));

	case_bump(F_READ_SYMBOL);
	rv_r = ref__citrus_prop_read_symbol(&ms_r, (char *)buf_r, cap);
	rv_p = port::_citrus_prop_read_symbol(&ms_p, (char *)buf_p, cap);
	if (rv_r != rv_p || ms_r.ms_pos != ms_p.ms_pos)
		fail(F_READ_SYMBOL, "return/pos mismatch");
	if (check && rv_r != expect_rv)
		fail(F_READ_SYMBOL, "unexpected rv");
	if (!bufs_equal(buf_r, buf_p, sizeof(buf_r)))
		fail(F_READ_SYMBOL, "symbol buffer mismatch");
	if (check && expect_name != nullptr &&
	    std::strcmp((char *)buf_r, expect_name) != 0)
		fail(F_READ_SYMBOL, "unexpected symbol");
	if (check && ms_r.ms_pos != expect_pos)
		fail(F_READ_SYMBOL, "unexpected pos");
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_READ_SYMBOL, "pool corruption");
}

void
test_parse_element(const char *input, int expect_rv, bool check = true)
{
	_citrus_prop_hint_t hints[5];
	port::_citrus_prop_hint_t phints[5];
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	struct _citrus_memory_stream ms_r{};
	PortMs ms_p{};
	Ctx cr{}, cp{};
	int rv_r, rv_p;

	hints[0] = { "alpha", _CITRUS_PROP_BOOL, { .boolean = { cb_bool } } };
	hints[1] = { "beta", _CITRUS_PROP_STR, { .str = { cb_str } } };
	hints[2] = { "gamma", _CITRUS_PROP_CHR, { .chr = { cb_chr } } };
	hints[3] = { "delta", _CITRUS_PROP_NUM, { .num = { cb_num } } };
	hints[4] = { nullptr, _CITRUS_PROP_BOOL, {} };
	phints[0] = { "alpha", port::_CITRUS_PROP_BOOL,
	    { .boolean = { cb_bool } } };
	phints[1] = { "beta", port::_CITRUS_PROP_STR, { .str = { cb_str } } };
	phints[2] = { "gamma", port::_CITRUS_PROP_CHR, { .chr = { cb_chr } } };
	phints[3] = { "delta", port::_CITRUS_PROP_NUM, { .num = { cb_num } } };
	phints[4] = { nullptr, port::_CITRUS_PROP_BOOL, {} };

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, std::strlen(input));
	std::memcpy(pool_p, input, std::strlen(input));
	bind_ms(&ms_r, pool_r, std::strlen(input));
	bind_ms(&ms_p, pool_p, std::strlen(input));
	reset_ctx(&cr);
	reset_ctx(&cp);

	case_bump(F_PARSE_ELEMENT);
	rv_r = ref__citrus_prop_parse_element(&ms_r, hints, &cr);
	rv_p = port::_citrus_prop_parse_element(&ms_p, phints, &cp);
	if (rv_r != rv_p || !ms_equal(&ms_r, &ms_p))
		fail(F_PARSE_ELEMENT, "return/pos mismatch");
	if (check && rv_r != expect_rv)
		fail(F_PARSE_ELEMENT, "unexpected rv");
	if (rv_r == 0 && !ctx_equal(&cr, &cp))
		fail(F_PARSE_ELEMENT, "callback state mismatch");
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_PARSE_ELEMENT, "pool corruption");
}

void
test_parse_variable(const char *input, std::size_t len, int expect_rv,
    bool check = true)
{
	_citrus_prop_hint_t hints[5];
	port::_citrus_prop_hint_t phints[5];
	unsigned char pool_r[POOL_CAP];
	unsigned char pool_p[POOL_CAP];
	Ctx cr{}, cp{};
	int rv_r, rv_p;

	hints[0] = { "alpha", _CITRUS_PROP_BOOL, { .boolean = { cb_bool } } };
	hints[1] = { "beta", _CITRUS_PROP_STR, { .str = { cb_str } } };
	hints[2] = { "gamma", _CITRUS_PROP_CHR, { .chr = { cb_chr } } };
	hints[3] = { "delta", _CITRUS_PROP_NUM, { .num = { cb_num } } };
	hints[4] = { nullptr, _CITRUS_PROP_BOOL, {} };
	phints[0] = { "alpha", port::_CITRUS_PROP_BOOL,
	    { .boolean = { cb_bool } } };
	phints[1] = { "beta", port::_CITRUS_PROP_STR, { .str = { cb_str } } };
	phints[2] = { "gamma", port::_CITRUS_PROP_CHR, { .chr = { cb_chr } } };
	phints[3] = { "delta", port::_CITRUS_PROP_NUM, { .num = { cb_num } } };
	phints[4] = { nullptr, port::_CITRUS_PROP_BOOL, {} };

	std::memset(pool_r, GUARD, sizeof(pool_r));
	std::memset(pool_p, GUARD, sizeof(pool_p));
	std::memcpy(pool_r, input, len);
	std::memcpy(pool_p, input, len);
	reset_ctx(&cr);
	reset_ctx(&cp);

	case_bump(F_PARSE_VARIABLE);
	rv_r = ref__citrus_prop_parse_variable(hints, &cr, pool_r, len);
	rv_p = port::_citrus_prop_parse_variable(phints, &cp, pool_p, len);
	if (rv_r != rv_p)
		fail(F_PARSE_VARIABLE, "return mismatch");
	if (check && rv_r != expect_rv)
		fail(F_PARSE_VARIABLE, "unexpected rv");
	if (rv_r == 0 && !ctx_equal(&cr, &cp))
		fail(F_PARSE_VARIABLE, "callback state mismatch");
	if (!bufs_equal(pool_r, pool_p, sizeof(pool_r)))
		fail(F_PARSE_VARIABLE, "pool corruption");
}

void
handwritten_cases(void)
{
	test_object_init(_CITRUS_PROP_BOOL);
	test_object_init(_CITRUS_PROP_STR);
	test_object_init(_CITRUS_PROP_CHR);
	test_object_init(_CITRUS_PROP_NUM);
	test_object_uninit();

	test_read_chr_common("0", 10, 0, 0, 1);
	test_read_chr_common("255", 10, 0, 255, 3);
	test_read_chr_common("FF", 16, 0, 255, 2);
	test_read_chr_common("377", 8, 0, 255, 3);
	test_read_chr_common("99G", 10, 0, 99, 2);
	test_read_chr_common("", 10, 0, 0, 0);
	test_read_chr_common("80", 10, 1, -80, 2);

	test_read_num_common("0", 10, 0, 0, 1);
	test_read_num_common("18446744073709551615", 10, 0, UINT64_MAX, 20);
	test_read_num_common("FFFFFFFFFFFFFFFF", 16, 0, UINT64_MAX, 16);
	test_read_num_common("123Z", 10, 0, 123, 3);
	test_read_num_common("", 10, 0, 0, 0);

	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "42", 0, _CITRUS_PROP_CHR, 42);
	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "-7", 0, _CITRUS_PROP_CHR, (std::uint64_t)-7);
	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "+15", 0, _CITRUS_PROP_CHR, 15);
	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "0x2a", 0, _CITRUS_PROP_CHR, 42);
	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "010", 0, _CITRUS_PROP_CHR, 8);
	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "0x", 0, _CITRUS_PROP_CHR, 0);
	test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
	    port::_citrus_prop_read_chr, "abc", EINVAL, _CITRUS_PROP_CHR, 0);

	test_read_typed(F_READ_NUM, ref__citrus_prop_read_num,
	    port::_citrus_prop_read_num, "1000", 0, _CITRUS_PROP_NUM, 1000);
	test_read_typed(F_READ_NUM, ref__citrus_prop_read_num,
	    port::_citrus_prop_read_num, "0xdeadbeef", 0, _CITRUS_PROP_NUM,
	    0xdeadbeefULL);
	test_read_typed(F_READ_NUM, ref__citrus_prop_read_num,
	    port::_citrus_prop_read_num, "-5", 0, _CITRUS_PROP_NUM,
	    (std::uint64_t)-5);
	test_read_typed(F_READ_NUM, ref__citrus_prop_read_num,
	    port::_citrus_prop_read_num, "077", 0, _CITRUS_PROP_NUM, 63);

	test_read_character_common("a", 0, 'a', 1);
	test_read_character_common("\\n", 0, '\n', 2);
	test_read_character_common("\\x41", 0, 'A', 4);
	test_read_character_common("\\0", 0, 0, 2);
	test_read_character_common("\\377", 0, 255, 5);
	test_read_character_common("\\q", 0, 'q', 2);
	test_read_character_common("\\7", 0, 7, 2);
	test_read_character_common("", 0, EOF, 0);

	test_read_typed(F_READ_CHARACTER, ref__citrus_prop_read_character,
	    port::_citrus_prop_read_character, "'A'", 0, _CITRUS_PROP_CHR, 'A');
	test_read_typed(F_READ_CHARACTER, ref__citrus_prop_read_character,
	    port::_citrus_prop_read_character, "65", 0, _CITRUS_PROP_CHR, 65);
	test_read_typed(F_READ_CHARACTER, ref__citrus_prop_read_character,
	    port::_citrus_prop_read_character, "'\\n'", 0, _CITRUS_PROP_CHR,
	    '\n');
	test_read_typed(F_READ_CHARACTER, ref__citrus_prop_read_character,
	    port::_citrus_prop_read_character, "'x", EINVAL, _CITRUS_PROP_CHR, 0);

	test_read_bool("true", 0, 1);
	test_read_bool("TRUE", 0, 1);
	test_read_bool("false", 0, 0);
	test_read_bool("FALSE", 0, 0);
	test_read_bool("tru", EINVAL, 0);
	test_read_bool("falsX", EINVAL, 0);
	test_read_bool(" tree", EINVAL, 0);

	test_read_str("\"hello\"", 0, "hello");
	test_read_str("'hi'", 0, "hi");
	test_read_str("plain;", 0, "plain");
	test_read_str("\"a\\x41b\"", 0, "aAb");
	test_read_str("", 0, "");
	test_read_str("\"\\xff\"", 0, "\xff");
	test_read_str("noend", 0, "noend");

	test_read_symbol("foo_bar", 32, 0, "foo_bar", 7);
	test_read_symbol("A", 32, 0, "A", 1);
	test_read_symbol("", 32, 0, "", 0);
	test_read_symbol("bad-name", 32, 0, "bad", 3);
	test_read_symbol("x", 1, 0, "x", 1);
	test_read_symbol("ab", 1, EINVAL, "", 1);

	test_parse_element("alpha=true;", 0);
	test_parse_element("beta=\"str\";", 0);
	test_parse_element("gamma='a';", 0);
	test_parse_element("delta=10-20;", 0);
	test_parse_element("delta=0x10,0x20;", 0);
	test_parse_element("unknown=1;", EINVAL);
	test_parse_element("alpha=maybe;", EINVAL);
	test_parse_element("gamma='a", EINVAL);

	test_parse_variable("", 0, 0);
	test_parse_variable("alpha=true;", 11, 0);
	test_parse_variable("  beta = \"x\" ; gamma=1-2; delta=3;", 34, 0);
	test_parse_variable("alpha=true,beta=\"y\";", 22, 0);
	test_parse_variable("nope=1;", 7, EINVAL);
}

void
fill_random_input(unsigned char *buf, std::size_t cap, std::size_t *out_len)
{
	std::size_t n = (std::size_t)(rng_next() % (cap - 4)) + 1;
	std::size_t i;

	for (i = 0; i < n; i++)
		buf[i] = rand_byte();
	buf[n] = '\0';
	*out_len = n;
}

void
random_sweep(void)
{
	unsigned char buf[POOL_CAP];
	std::size_t len;
	char numbuf[64];

	for (long long i = 0; i < RANDOM_ITERS; i++) {
		switch ((int)(rng_next() % 11)) {
		case 0:
			test_object_init(
			    (_citrus_prop_type_t)(rng_next() % 4));
			break;
		case 1: {
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_chr_common((char *)buf,
			    (int)(rng_next() % 15) + 2,
			    (int)(rng_next() & 1), 0, 0, false);
			break;
		}
		case 2: {
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_num_common((char *)buf,
			    (int)(rng_next() % 15) + 2,
			    (int)(rng_next() & 1), 0, 0, false);
			break;
		}
		case 3:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_typed(F_READ_CHR, ref__citrus_prop_read_chr,
			    port::_citrus_prop_read_chr, (char *)buf, 0,
			    _CITRUS_PROP_CHR, 0, false);
			break;
		case 4:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_typed(F_READ_NUM, ref__citrus_prop_read_num,
			    port::_citrus_prop_read_num, (char *)buf, 0,
			    _CITRUS_PROP_NUM, 0, false);
			break;
		case 5:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_character_common((char *)buf, 0, 0, 0, false);
			break;
		case 6:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_typed(F_READ_CHARACTER,
			    ref__citrus_prop_read_character,
			    port::_citrus_prop_read_character, (char *)buf, 0,
			    _CITRUS_PROP_CHR, 0, false);
			break;
		case 7:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_bool((char *)buf, 0, 0, false);
			break;
		case 8:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_str((char *)buf, 0, nullptr, false);
			break;
		case 9:
			fill_random_input(buf, sizeof(buf) - 1, &len);
			test_read_symbol((char *)buf,
			    (std::size_t)((rng_next() % 64) + 1), 0, nullptr,
			    0, false);
			break;
		default: {
			const char *names[] = { "alpha", "beta", "gamma",
			    "delta" };
			const char *ops[] = { "=", ":" };
			const char *n = names[rng_next() % 4];
			const char *op = ops[rng_next() % 2];
			int kind = (int)(rng_next() % 4);

			if (kind == 0)
				std::snprintf((char *)buf, sizeof(buf),
				    "%s%s%s;", n, op,
				    (rng_next() & 1) ? "true" : "false");
			else if (kind == 1)
				std::snprintf((char *)buf, sizeof(buf),
				    "%s%s\"%c%c\";", n, op,
				    (char)('a' + (rng_next() % 26)),
				    (char)rand_byte());
			else if (kind == 2)
				std::snprintf((char *)buf, sizeof(buf),
				    "%s%s'%c';", n, op, (char)rand_byte());
			else {
				std::snprintf(numbuf, sizeof(numbuf), "%llu",
				    (unsigned long long)(rng_next() %
				    1000000ULL));
				std::snprintf((char *)buf, sizeof(buf),
				    "%s%s%s-%s;", n, op, numbuf, numbuf);
			}
			len = std::strlen((char *)buf);
			if ((rng_next() & 3) == 0)
				test_parse_element((char *)buf, 0, false);
			else
				test_parse_variable((char *)buf, len, 0, false);
			break;
		}
		}
	}
}

} /* namespace */

int
main(void)
{
	handwritten_cases();
	random_sweep();

	std::printf("\n%-36s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-36s %10lu %10lu\n", fname[i], ncase[i],
		    nfail[i]);

	unsigned long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fail += nfail[i];
	return (total_fail == 0 ? 0 : 1);
}
