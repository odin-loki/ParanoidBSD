/*
 * harness.cpp -- differential test for PBSD batch b0123.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

import pbsd.sbin.ipf.libipf.b0123;

namespace port = pbsd::sbin_ipf_libipf::b0123;

#define USE_INET6

#define AF_UNSPEC	0
#define AF_INET		2
#define AF_INET6	28

typedef port::mb_t port_mb_t;

struct mbuf;
typedef struct mbuf ref_mb_t;

extern "C" const char *ref_familyname(int family);
extern "C" int ref_ftov(int version);
extern "C" int ref_vtof(int version);
extern "C" void ref_freembt(ref_mb_t *m);

extern "C" void __real_free(void *);

static long free_wrap_count;

extern "C" void
__wrap_free(void *p)
{
	free_wrap_count++;
	__real_free(p);
}

static const int MAX_REPORT = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_familyname = { "familyname", 0, 0, 0 };
static Stat st_ftov = { "ftov", 0, 0, 0 };
static Stat st_vtof = { "vtof", 0, 0, 0 };
static Stat st_freembt = { "freembt", 0, 0, 0 };

static std::uint64_t rng_state;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline int
rnd_int(void)
{
	return (int)(rnd() & 0xffffffffu);
}

/* ------------------------------------------------------------------------ */
/* familyname                                                                */
/* ------------------------------------------------------------------------ */

static void
familyname_run(int family, const char *tag)
{
	const char *pa = port::familyname(family);
	const char *pb = ref_familyname(family);
	int bad = 0;

	if (pa == nullptr || pb == nullptr) {
		bad = (pa != pb);
	} else if (std::strcmp(pa, pb) != 0) {
		bad = 1;
	}

	st_familyname.cases++;
	if (bad) {
		st_familyname.fails++;
		if (st_familyname.reported < MAX_REPORT) {
			st_familyname.reported++;
			std::printf(
			    "  FAIL familyname [%s] family=%d port=\"%s\" ref=\"%s\"\n",
			    tag, family, pa ? pa : "(null)", pb ? pb : "(null)");
		}
	}
}

static const int fn_edge[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	27,
	28,
	29,
	30,
	-1,
	-2,
	INT_MAX,
	INT_MIN,
	0x7f,
	0x80,
	0xff,
	0x100,
	0x7fffffff,
	(int)0x80000000u,
	(int)0xfffffffeu,
	(int)0xffffffffu,
};

static void
familyname_edge(void)
{
	std::size_t n = sizeof(fn_edge) / sizeof(fn_edge[0]);

	for (std::size_t i = 0; i < n; i++)
		familyname_run(fn_edge[i], "edge");
}

static void
familyname_random(long count)
{
	for (long i = 0; i < count; i++)
		familyname_run(rnd_int(), "rand");
}

/* ------------------------------------------------------------------------ */
/* ftov                                                                      */
/* ------------------------------------------------------------------------ */

static void
ftov_run(int version, const char *tag)
{
	int pa = port::ftov(version);
	int pb = ref_ftov(version);

	st_ftov.cases++;
	if (pa != pb) {
		st_ftov.fails++;
		if (st_ftov.reported < MAX_REPORT) {
			st_ftov.reported++;
			std::printf("  FAIL ftov [%s] version=%d port=%d ref=%d\n",
			    tag, version, pa, pb);
		}
	}
}

static const int ftov_edges[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	27,
	28,
	29,
	30,
	-1,
	-2,
	INT_MAX,
	INT_MIN,
	0x7f,
	0x80,
	0xff,
	0x100,
	0x7fffffff,
	(int)0x80000000u,
	(int)0xfffffffeu,
	(int)0xffffffffu,
};

static void
ftov_edge(void)
{
	std::size_t n = sizeof(ftov_edges) / sizeof(ftov_edges[0]);

	for (std::size_t i = 0; i < n; i++)
		ftov_run(ftov_edges[i], "edge");
}

static void
ftov_random(long count)
{
	for (long i = 0; i < count; i++)
		ftov_run(rnd_int(), "rand");
}

/* ------------------------------------------------------------------------ */
/* vtof                                                                      */
/* ------------------------------------------------------------------------ */

static void
vtof_run(int version, const char *tag)
{
	int pa = port::vtof(version);
	int pb = ref_vtof(version);

	st_vtof.cases++;
	if (pa != pb) {
		st_vtof.fails++;
		if (st_vtof.reported < MAX_REPORT) {
			st_vtof.reported++;
			std::printf("  FAIL vtof [%s] version=%d port=%d ref=%d\n",
			    tag, version, pa, pb);
		}
	}
}

static const int vtof_edges[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	-1,
	-2,
	INT_MAX,
	INT_MIN,
	0x7f,
	0x80,
	0xff,
	0x100,
	0x7fffffff,
	(int)0x80000000u,
	(int)0xfffffffeu,
	(int)0xffffffffu,
};

static void
vtof_edge(void)
{
	std::size_t n = sizeof(vtof_edges) / sizeof(vtof_edges[0]);

	for (std::size_t i = 0; i < n; i++)
		vtof_run(vtof_edges[i], "edge");
}

static void
vtof_random(long count)
{
	for (long i = 0; i < count; i++)
		vtof_run(rnd_int(), "rand");
}

/* ------------------------------------------------------------------------ */
/* freembt                                                                   */
/* ------------------------------------------------------------------------ */

static void
freembt_run_ptr(void *p, const char *tag)
{
	long port_frees;
	long ref_frees;

	free_wrap_count = 0;
	port::freembt(static_cast<port_mb_t *>(p));
	port_frees = free_wrap_count;

	free_wrap_count = 0;
	ref_freembt(static_cast<ref_mb_t *>(p));
	ref_frees = free_wrap_count;

	st_freembt.cases++;
	if (port_frees != ref_frees) {
		st_freembt.fails++;
		if (st_freembt.reported < MAX_REPORT) {
			st_freembt.reported++;
			std::printf(
			    "  FAIL freembt [%s] ptr=%p port_frees=%ld ref_frees=%ld\n",
			    tag, (void *)p, port_frees, ref_frees);
		}
	}
}

static void
freembt_run_size(std::size_t sz, const char *tag)
{
	void *pa = std::malloc(sz == 0 ? 1 : sz);
	void *pb = std::malloc(sz == 0 ? 1 : sz);

	if (pa == nullptr || pb == nullptr) {
		std::fprintf(stderr, "harness OOM in freembt [%s]\n", tag);
		std::exit(2);
	}
	if (sz > 0) {
		std::memset(pa, 0xaa, sz);
		std::memset(pb, 0xaa, sz);
	}

	long port_frees;
	long ref_frees;

	free_wrap_count = 0;
	port::freembt(static_cast<port_mb_t *>(pa));
	port_frees = free_wrap_count;

	free_wrap_count = 0;
	ref_freembt(static_cast<ref_mb_t *>(pb));
	ref_frees = free_wrap_count;

	st_freembt.cases++;
	if (port_frees != ref_frees) {
		st_freembt.fails++;
		if (st_freembt.reported < MAX_REPORT) {
			st_freembt.reported++;
			std::printf(
			    "  FAIL freembt [%s] sz=%zu port_frees=%ld ref_frees=%ld\n",
			    tag, sz, port_frees, ref_frees);
		}
	}
}

static void
freembt_edge(void)
{
	freembt_run_ptr(nullptr, "null");
	freembt_run_size(1, "sz1");
	freembt_run_size(16, "sz16");
	freembt_run_size(4096, "sz4096");
}

static void
freembt_random(long count)
{
	for (long i = 0; i < count; i++) {
		unsigned mode = (unsigned)(rnd() % 8u);
		if (mode == 0) {
			freembt_run_ptr(nullptr, "rand-null");
		} else {
			std::size_t sz = (std::size_t)(rnd() % 8192u) + 1u;
			freembt_run_size(sz, "rand");
		}
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	rng_state = 0x0123a123ULL;
	familyname_edge();
	familyname_random(200000);

	rng_state = 0x0123b123ULL;
	ftov_edge();
	ftov_random(200000);

	rng_state = 0x0123c123ULL;
	vtof_edge();
	vtof_random(200000);

	rng_state = 0x0123d123ULL;
	freembt_edge();
	freembt_random(200000);

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	const Stat *all[] = {
		&st_familyname, &st_ftov, &st_vtof, &st_freembt
	};
	long total_fail = 0;
	long total_case = 0;

	for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		std::printf("%-12s %12ld %12ld   %s\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->fails;
		total_case += all[i]->cases;
	}
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");
	std::printf("%-12s %12ld %12ld   %s\n", "TOTAL", total_case,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
