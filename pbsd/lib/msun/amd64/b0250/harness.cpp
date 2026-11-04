/*
 * Differential test harness for PBSD batch b0250 (amd64 fenv.c).
 */

import pbsd.lib.msun.amd64.b0250;

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_msun_amd64::b0250;

extern "C" {
typedef uint16_t fexcept_t;
typedef struct {
	struct {
		uint32_t __control;
		uint32_t __status;
		uint32_t __tag;
		char __other[16];
	} __x87;
	uint32_t __mxcsr;
} fenv_t;

extern const fenv_t ref___fe_dfl_env;

void ref_test_set_x87_control(uint32_t);
void ref_test_set_x87_status(uint32_t);
void ref_test_set_x87_tag(uint32_t);
void ref_test_set_x87_other(const char *);
void ref_test_set_mxcsr(uint32_t);
void ref_test_reset(void);
uint32_t ref_test_get_x87_control(void);
uint32_t ref_test_get_x87_status(void);
uint32_t ref_test_get_mxcsr(void);

int ref_feclearexcept(int);
int ref_fegetexceptflag(fexcept_t *, int);
int ref_fesetexceptflag(const fexcept_t *, int);
int ref_feraiseexcept(int);
int ref_fetestexcept(int);
int ref_fegetround(void);
int ref_fesetround(int);
int ref_fegetenv(fenv_t *);
int ref_feholdexcept(fenv_t *);
int ref_fesetenv(const fenv_t *);
int ref_feupdateenv(const fenv_t *);
int ref_feenableexcept(int);
int ref_fedisableexcept(int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t GUARD_PAD = 32;
constexpr unsigned MAX_REPORT = 12;
constexpr unsigned RANDOM_ITERS = 200000;

constexpr int FE_INVALID = 0x01;
constexpr int FE_DENORMAL = 0x02;
constexpr int FE_DIVBYZERO = 0x04;
constexpr int FE_OVERFLOW = 0x08;
constexpr int FE_UNDERFLOW = 0x10;
constexpr int FE_INEXACT = 0x20;
constexpr int FE_ALL_EXCEPT = (FE_DIVBYZERO | FE_DENORMAL | FE_INEXACT |
    FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
constexpr int FE_TONEAREST = 0x0000;
constexpr int FE_DOWNWARD = 0x0400;
constexpr int FE_UPWARD = 0x0800;
constexpr int FE_TOWARDZERO = 0x0c00;
constexpr int _ROUND_MASK = (FE_TONEAREST | FE_DOWNWARD |
    FE_UPWARD | FE_TOWARDZERO);

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

Stat st_feclearexcept = { "feclearexcept", 0, 0, 0 };
Stat st_fegetexceptflag = { "fegetexceptflag", 0, 0, 0 };
Stat st_fesetexceptflag = { "fesetexceptflag", 0, 0, 0 };
Stat st_feraiseexcept = { "feraiseexcept", 0, 0, 0 };
Stat st_fetestexcept = { "fetestexcept", 0, 0, 0 };
Stat st_fegetround = { "fegetround", 0, 0, 0 };
Stat st_fesetround = { "fesetround", 0, 0, 0 };
Stat st_fegetenv = { "fegetenv", 0, 0, 0 };
Stat st_feholdexcept = { "feholdexcept", 0, 0, 0 };
Stat st_fesetenv = { "fesetenv", 0, 0, 0 };
Stat st_feupdateenv = { "feupdateenv", 0, 0, 0 };
Stat st_feenableexcept = { "feenableexcept", 0, 0, 0 };
Stat st_fedisableexcept = { "fedisableexcept", 0, 0, 0 };
Stat st___fe_dfl_env = { "__fe_dfl_env", 0, 0, 0 };

uint32_t rng = 0xC0FFEE42u;

uint32_t rand32() { rng = rng * 1103515245u + 12345u; return rng; }

void sync_state(uint32_t control, uint32_t status, uint32_t tag,
    const char *other, uint32_t mxcsr)
{
	ref_test_set_x87_control(control);
	ref_test_set_x87_status(status);
	ref_test_set_x87_tag(tag);
	ref_test_set_x87_other(other);
	ref_test_set_mxcsr(mxcsr);
	port::test_set_x87_control(control);
	port::test_set_x87_status(status);
	port::test_set_x87_tag(tag);
	port::test_set_x87_other(other);
	port::test_set_mxcsr(mxcsr);
}

void prep_env(uint32_t control, uint32_t status, uint32_t mxcsr)
{
	char other[16];
	std::memset(other, 0, sizeof(other));
	sync_state(control, status, 0, other, mxcsr);
}

void report_fail(Stat &st, const char *detail)
{
	st.fails++;
	if (st.reported < MAX_REPORT) {
		std::fprintf(stderr, "FAIL %s: %s\n", st.name, detail);
		st.reported++;
	}
}

template<typename Fn>
void run_pair(Stat &st, Fn body, const char *tag)
{
	st.cases++;
	if (!body())
		report_fail(st, tag);
}

bool state_match(uint32_t pc, uint32_t ps, uint32_t pm)
{
	return (pc == ref_test_get_x87_control() &&
	    ps == ref_test_get_x87_status() &&
	    pm == ref_test_get_mxcsr());
}

void test_feclearexcept_one(int excepts, uint32_t c, uint32_t s, uint32_t m,
    const char *tag)
{
	run_pair(st_feclearexcept, [&]() -> bool {
		int pr, rr;
		prep_env(c, s, m);
		pr = port::feclearexcept(excepts);
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_feclearexcept(excepts);
		return (pr == rr && state_match(pc, ps, pm));
	}, tag);
}

void test_feclearexcept_edges(void)
{
	static const int kEx[] = {
		0, 1, 2, 4, 8, 0x10, 0x20, 0x3f, 0x3e, 0x40, 0x80, 0xff,
		FE_ALL_EXCEPT, FE_ALL_EXCEPT - 1, FE_ALL_EXCEPT + 1, -1
	};
	static const uint32_t kC[] = { 0x037f, 0x027f, 0x0370, 0xffff037f };
	static const uint32_t kS[] = { 0, 0x3f, 0x21, 0xffff0021u };
	static const uint32_t kM[] = { 0x1f80, 0x1f83, 0x1fa0, 0x803f };
	for (int ex : kEx)
		for (uint32_t c : kC)
			for (uint32_t s : kS)
				for (uint32_t m : kM)
					test_feclearexcept_one(ex, c, s, m,
					    "edge");
}

void test_feclearexcept_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_feclearexcept_one((int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu), rand32() & 0xffffu,
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_fegetexceptflag_one(fexcept_t in, int ex, uint32_t c, uint32_t s,
    uint32_t m, const char *tag)
{
	run_pair(st_fegetexceptflag, [&]() -> bool {
		unsigned char ga[sizeof(fexcept_t) + 2 * GUARD_PAD];
		unsigned char gb[sizeof(fexcept_t) + 2 * GUARD_PAD];
		fexcept_t pf, rf;
		int pr, rr;
		std::memset(ga, GUARD, sizeof(ga));
		std::memset(gb, GUARD, sizeof(gb));
		std::memcpy(ga + GUARD_PAD, &in, sizeof(in));
		std::memcpy(gb + GUARD_PAD, &in, sizeof(in));
		prep_env(c, s, m);
		pr = port::fegetexceptflag(&pf, ex);
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_fegetexceptflag(&rf, ex);
		std::memcpy(ga + GUARD_PAD, &pf, sizeof(pf));
		std::memcpy(gb + GUARD_PAD, &rf, sizeof(rf));
		return (pr == rr && std::memcmp(ga, gb, sizeof(ga)) == 0 &&
		    state_match(pc, ps, pm));
	}, tag);
}

void test_fegetexceptflag_edges(void)
{
	static const fexcept_t kIn[] = { 0, 1, 0x3f, 0x80, 0xff };
	static const int kEx[] = { 0, 1, 0x3f, 0x40, 0xff, FE_ALL_EXCEPT, -1 };
	for (fexcept_t in : kIn)
		for (int ex : kEx)
			test_fegetexceptflag_one(in, ex, 0x037f, 0x2a, 0x1f8a,
			    "edge");
}

void test_fegetexceptflag_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fegetexceptflag_one((fexcept_t)rand32(),
		    (int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu), rand32() & 0xffffu,
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_fesetexceptflag_one(fexcept_t flag, int ex, uint32_t c, uint32_t s,
    uint32_t m, const char *tag)
{
	run_pair(st_fesetexceptflag, [&]() -> bool {
		int pr, rr;
		prep_env(c, s, m);
		pr = port::fesetexceptflag(&flag, ex);
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_fesetexceptflag(&flag, ex);
		return (pr == rr && state_match(pc, ps, pm));
	}, tag);
}

void test_fesetexceptflag_edges(void)
{
	static const fexcept_t kF[] = { 0, 1, 0x3f, 0x80, 0xff };
	static const int kEx[] = { 0, 1, 0x3f, 0x40, FE_ALL_EXCEPT };
	for (fexcept_t f : kF)
		for (int ex : kEx)
			test_fesetexceptflag_one(f, ex, 0x037f, 0x15, 0x1f85,
			    "edge");
}

void test_fesetexceptflag_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fesetexceptflag_one((fexcept_t)rand32(),
		    (int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu), rand32() & 0xffffu,
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_feraiseexcept_one(int ex, uint32_t c, uint32_t s, uint32_t m,
    const char *tag)
{
	run_pair(st_feraiseexcept, [&]() -> bool {
		int pr, rr;
		prep_env(c, s, m);
		pr = port::feraiseexcept(ex);
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_feraiseexcept(ex);
		return (pr == rr && state_match(pc, ps, pm));
	}, tag);
}

void test_feraiseexcept_edges(void)
{
	static const int kEx[] = {
		0, 1, 2, 4, 8, 0x10, 0x20, 0x3f, 0x40, 0x80, FE_ALL_EXCEPT, -1
	};
	for (int ex : kEx)
		test_feraiseexcept_one(ex, 0x037f, 0, 0x1f80, "edge");
}

void test_feraiseexcept_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_feraiseexcept_one((int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu), rand32() & 0xffffu,
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_fetestexcept_one(int ex, uint32_t c, uint32_t s, uint32_t m,
    const char *tag)
{
	run_pair(st_fetestexcept, [&]() -> bool {
		int pr, rr;
		prep_env(c, s, m);
		pr = port::fetestexcept(ex);
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_fetestexcept(ex);
		return (pr == rr && state_match(pc, ps, pm));
	}, tag);
}

void test_fetestexcept_edges(void)
{
	static const int kEx[] = { 0, 1, 0x3f, 0x40, 0x80, 0xff, FE_ALL_EXCEPT };
	for (int ex : kEx)
		test_fetestexcept_one(ex, 0x037f, 0x2a, 0x1f8a, "edge");
}

void test_fetestexcept_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fetestexcept_one((int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu), rand32() & 0xffffu,
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_fegetround_one(uint32_t c, const char *tag)
{
	run_pair(st_fegetround, [&]() -> bool {
		prep_env(c, 0, 0x1f80);
		int pr = port::fegetround();
		prep_env(c, 0, 0x1f80);
		int rr = ref_fegetround();
		return (pr == rr);
	}, tag);
}

void test_fegetround_edges(void)
{
	static const uint32_t kC[] = {
		0x037f, 0x027f, 0x077f, 0x0b7f, 0x0f7f, 0xffff037f
	};
	for (uint32_t c : kC)
		test_fegetround_one(c, "edge");
}

void test_fegetround_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fegetround_one(0x0300u | (rand32() & 0x0fffu), "random");
}

void test_fesetround_one(int round, uint32_t c, uint32_t m, const char *tag)
{
	run_pair(st_fesetround, [&]() -> bool {
		int pr, rr;
		prep_env(c, 0, m);
		pr = port::fesetround(round);
		uint32_t pc = port::test_get_x87_control();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, 0, m);
		rr = ref_fesetround(round);
		return (pr == rr && pc == ref_test_get_x87_control() &&
		    pm == ref_test_get_mxcsr());
	}, tag);
}

void test_fesetround_edges(void)
{
	static const int kR[] = {
		FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO,
		0x1000, 0x2000, -1, 0x7fff, _ROUND_MASK + 1, _ROUND_MASK
	};
	for (int r : kR)
		test_fesetround_one(r, 0x037f, 0x1f80, "edge");
}

void test_fesetround_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fesetround_one((int)(rand32() & 0xffff),
		    0x0300u | (rand32() & 0x0fffu),
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_fegetenv_one(uint32_t c, uint32_t s, uint32_t m, const char *tag)
{
	run_pair(st_fegetenv, [&]() -> bool {
		unsigned char ga[sizeof(fenv_t) + 2 * GUARD_PAD];
		unsigned char gb[sizeof(fenv_t) + 2 * GUARD_PAD];
		fenv_t pe, re;
		int pr, rr;
		std::memset(ga, GUARD, sizeof(ga));
		std::memset(gb, GUARD, sizeof(gb));
		std::memset(&pe, 0x7f, sizeof(pe));
		std::memset(&re, 0x7f, sizeof(re));
		prep_env(c, s, m);
		pr = port::fegetenv(reinterpret_cast<port::fenv_t *>(&pe));
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_fegetenv(&re);
		std::memcpy(ga + GUARD_PAD, &pe, sizeof(pe));
		std::memcpy(gb + GUARD_PAD, &re, sizeof(re));
		return (pr == rr && std::memcmp(ga, gb, sizeof(ga)) == 0 &&
		    state_match(pc, ps, pm));
	}, tag);
}

void test_fegetenv_edges(void)
{
	test_fegetenv_one(0x037f, 0, 0x1f80, "edge");
	test_fegetenv_one(0xffff037f, 0xffff0021u, 0x1f83, "edge");
	test_fegetenv_one(0x027f, 0x3f, 0x803f, "edge");
}

void test_fegetenv_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fegetenv_one(0x0300u | (rand32() & 0x0fffu),
		    rand32() & 0xffffu, 0x1f00u | (rand32() & 0x1fffu),
		    "random");
}

void test_feholdexcept_one(uint32_t c, uint32_t s, uint32_t m,
    const char *tag)
{
	run_pair(st_feholdexcept, [&]() -> bool {
		unsigned char ga[sizeof(fenv_t) + 2 * GUARD_PAD];
		unsigned char gb[sizeof(fenv_t) + 2 * GUARD_PAD];
		fenv_t pe, re;
		int pr, rr;
		std::memset(ga, GUARD, sizeof(ga));
		std::memset(gb, GUARD, sizeof(gb));
		std::memset(&pe, 0x7f, sizeof(pe));
		std::memset(&re, 0x7f, sizeof(re));
		prep_env(c, s, m);
		pr = port::feholdexcept(reinterpret_cast<port::fenv_t *>(&pe));
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_feholdexcept(&re);
		std::memcpy(ga + GUARD_PAD, &pe, sizeof(pe));
		std::memcpy(gb + GUARD_PAD, &re, sizeof(re));
		return (pr == rr && std::memcmp(ga, gb, sizeof(ga)) == 0 &&
		    state_match(pc, ps, pm));
	}, tag);
}

void test_feholdexcept_edges(void)
{
	test_feholdexcept_one(0x037f, 0x2a, 0x1f8a, "edge");
	test_feholdexcept_one(0x027f, 0x3f, 0x803f, "edge");
	test_feholdexcept_one(0xffff037f, 0xffff0000u, 0x1f80, "edge");
}

void test_feholdexcept_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_feholdexcept_one(0x0300u | (rand32() & 0x0fffu),
		    rand32() & 0xffffu, 0x1f00u | (rand32() & 0x1fffu),
		    "random");
}

fenv_t make_env(uint32_t control, uint32_t status, uint32_t tag, uint32_t mxcsr)
{
	fenv_t e;
	char other[16];
	std::memset(other, 0, sizeof(other));
	other[14] = (char)0xff;
	other[15] = (char)0xff;
	e.__x87.__control = control;
	e.__x87.__status = status;
	e.__x87.__tag = tag;
	std::memcpy(e.__x87.__other, other, 16);
	e.__mxcsr = mxcsr;
	return e;
}

void test_fesetenv_one(const fenv_t &in, uint32_t c, uint32_t s, uint32_t m,
    const char *tag)
{
	run_pair(st_fesetenv, [&]() -> bool {
		int pr, rr;
		prep_env(c, s, m);
		pr = port::fesetenv(reinterpret_cast<const port::fenv_t *>(&in));
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_fesetenv(&in);
		return (pr == rr && state_match(pc, ps, pm));
	}, tag);
}

void test_fesetenv_edges(void)
{
	test_fesetenv_one(make_env(0xffff037f, 0xffff0000u, 0xffffffffu, 0x1f80),
	    0x037f, 0, 0x1f80, "edge");
	test_fesetenv_one(make_env(0x027f, 0x15, 0, 0x1f83),
	    0x037f, 0x2a, 0x1f8a, "edge");
}

void test_fesetenv_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++) {
		fenv_t e = make_env(rand32(), rand32(), rand32(),
		    0x1f00u | (rand32() & 0x1fffu));
		test_fesetenv_one(e, 0x0300u | (rand32() & 0x0fffu),
		    rand32() & 0xffffu, 0x1f00u | (rand32() & 0x1fffu),
		    "random");
	}
}

void test_feupdateenv_one(const fenv_t &in, uint32_t c, uint32_t s, uint32_t m,
    const char *tag)
{
	run_pair(st_feupdateenv, [&]() -> bool {
		int pr, rr;
		prep_env(c, s, m);
		pr = port::feupdateenv(
		    reinterpret_cast<const port::fenv_t *>(&in));
		uint32_t pc = port::test_get_x87_control();
		uint32_t ps = port::test_get_x87_status();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, s, m);
		rr = ref_feupdateenv(&in);
		return (pr == rr && state_match(pc, ps, pm));
	}, tag);
}

void test_feupdateenv_edges(void)
{
	test_feupdateenv_one(
	    make_env(0xffff037f, 0xffff0000u, 0xffffffffu, 0x1f80),
	    0x037f, 0x2a, 0x1f8a, "edge");
	test_feupdateenv_one(make_env(0x027f, 0, 0, 0x1f80),
	    0x037f, 0x3f, 0x803f, "edge");
}

void test_feupdateenv_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++) {
		fenv_t e = make_env(rand32(), rand32(), rand32(),
		    0x1f00u | (rand32() & 0x1fffu));
		test_feupdateenv_one(e, 0x0300u | (rand32() & 0x0fffu),
		    rand32() & 0xffffu, 0x1f00u | (rand32() & 0x1fffu),
		    "random");
	}
}

void test_feenableexcept_one(int mask, uint32_t c, uint32_t m, const char *tag)
{
	run_pair(st_feenableexcept, [&]() -> bool {
		int pr, rr;
		prep_env(c, 0, m);
		pr = port::feenableexcept(mask);
		uint32_t pc = port::test_get_x87_control();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, 0, m);
		rr = ref_feenableexcept(mask);
		return (pr == rr && pc == ref_test_get_x87_control() &&
		    pm == ref_test_get_mxcsr());
	}, tag);
}

void test_feenableexcept_edges(void)
{
	static const int kM[] = { 0, 1, 2, 4, 8, 0x10, 0x20, 0x3f, 0x40, 0x80,
	    FE_ALL_EXCEPT, -1 };
	for (int m : kM)
		test_feenableexcept_one(m, 0x037f, 0x1f80, "edge");
	test_feenableexcept_one(0x3f, 0x027f, 0x1f00, "edge");
}

void test_feenableexcept_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_feenableexcept_one((int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu),
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test_fedisableexcept_one(int mask, uint32_t c, uint32_t m,
    const char *tag)
{
	run_pair(st_fedisableexcept, [&]() -> bool {
		int pr, rr;
		prep_env(c, 0, m);
		pr = port::fedisableexcept(mask);
		uint32_t pc = port::test_get_x87_control();
		uint32_t pm = port::test_get_mxcsr();
		prep_env(c, 0, m);
		rr = ref_fedisableexcept(mask);
		return (pr == rr && pc == ref_test_get_x87_control() &&
		    pm == ref_test_get_mxcsr());
	}, tag);
}

void test_fedisableexcept_edges(void)
{
	static const int kM[] = { 0, 1, 2, 4, 8, 0x10, 0x20, 0x3f, 0x40, 0x80,
	    FE_ALL_EXCEPT, -1 };
	for (int m : kM)
		test_fedisableexcept_one(m, 0x037f, 0x1f80, "edge");
	test_fedisableexcept_one(0x3f, 0x027f, 0x1f00, "edge");
}

void test_fedisableexcept_random(void)
{
	for (unsigned i = 0; i < RANDOM_ITERS; i++)
		test_fedisableexcept_one((int)(rand32() & 0xff),
		    0x0300u | (rand32() & 0x0fffu),
		    0x1f00u | (rand32() & 0x1fffu), "random");
}

void test___fe_dfl_env(void)
{
	run_pair(st___fe_dfl_env, [&]() -> bool {
		return (std::memcmp(&port::__fe_dfl_env, &ref___fe_dfl_env,
		    sizeof(fenv_t)) == 0);
	}, "const");
}

void print_table(void)
{
	const Stat *all[] = {
		&st_feclearexcept, &st_fegetexceptflag, &st_fesetexceptflag,
		&st_feraiseexcept, &st_fetestexcept, &st_fegetround,
		&st_fesetround, &st_fegetenv, &st_feholdexcept,
		&st_fesetenv, &st_feupdateenv, &st_feenableexcept,
		&st_fedisableexcept, &st___fe_dfl_env
	};
	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-18s %12s %12s\n", "--------", "-----", "--------");
	for (const Stat *st : all)
		std::printf("%-18s %12llu %12llu\n",
		    st->name, st->cases, st->fails);
}

unsigned long long total_fails(void)
{
	const Stat *all[] = {
		&st_feclearexcept, &st_fegetexceptflag, &st_fesetexceptflag,
		&st_feraiseexcept, &st_fetestexcept, &st_fegetround,
		&st_fesetround, &st_fegetenv, &st_feholdexcept,
		&st_fesetenv, &st_feupdateenv, &st_feenableexcept,
		&st_fedisableexcept, &st___fe_dfl_env
	};
	unsigned long long n = 0;
	for (const Stat *st : all)
		n += st->fails;
	return n;
}

} /* namespace */

int main(void)
{
	test___fe_dfl_env();
	test_feclearexcept_edges();
	test_feclearexcept_random();
	test_fegetexceptflag_edges();
	test_fegetexceptflag_random();
	test_fesetexceptflag_edges();
	test_fesetexceptflag_random();
	test_feraiseexcept_edges();
	test_feraiseexcept_random();
	test_fetestexcept_edges();
	test_fetestexcept_random();
	test_fegetround_edges();
	test_fegetround_random();
	test_fesetround_edges();
	test_fesetround_random();
	test_fegetenv_edges();
	test_fegetenv_random();
	test_feholdexcept_edges();
	test_feholdexcept_random();
	test_fesetenv_edges();
	test_fesetenv_random();
	test_feupdateenv_edges();
	test_feupdateenv_random();
	test_feenableexcept_edges();
	test_feenableexcept_random();
	test_fedisableexcept_edges();
	test_fedisableexcept_random();
	print_table();
	return (total_fails() == 0 ? 0 : 1);
}
