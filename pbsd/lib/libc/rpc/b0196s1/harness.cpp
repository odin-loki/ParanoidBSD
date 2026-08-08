/*
 * Batch b0196s1 differential test.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.rpc.b0196s1;

extern "C" {
struct ref_rpc_createerr_blob {
	int cf_stat;
	struct { int re_status; } cf_error;
};
extern struct ref_rpc_createerr_blob ref_rpc_createerr;
struct rpc_createerr *ref___rpc_createerr(void);

void mock_reset_b0196s1(void);

extern int mock_thr_main_result;
extern int mock_thr_once_result;
extern int mock_thr_keycreate_result;
extern int mock_thr_setspecific_result;
extern int mock_thr_getspecific_null;
extern int mock_malloc_fail;
extern int mock_thr_setspecific_fail_on;
}

namespace port = pbsd::lib_libc_rpc::b0196s1;

namespace {

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "__rpc_createerr", 0, 0 },
};

unsigned long long reported = 0;
const unsigned long long report_limit = 30;

std::uint64_t rng_state = 0x243F6A8885A308D3ULL;

std::uint64_t
next_u64()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

void
check_rpc_createerr(const char *origin)
{
	tbl[0].cases++;

	mock_reset_b0196s1();
	if (origin[0] == 'm')
		mock_thr_main_result = 1;
	if (origin[0] == 'o')
		mock_thr_once_result = -1;
	if (origin[0] == 'p')
		mock_thr_once_result = 1;
	if (origin[0] == 'k')
		mock_thr_keycreate_result = -1;
	if (origin[0] == 'j')
		mock_thr_keycreate_result = 1;
	if (origin[0] == 's')
		mock_thr_setspecific_fail_on = 1;
	if (origin[0] == 'r')
		mock_thr_setspecific_result = 1;
	if (origin[0] == 'n')
		mock_thr_setspecific_result = -1;
	if (origin[0] == 'a')
		mock_malloc_fail = 1;
	if (origin[0] == 't') {
		mock_thr_main_result = 0;
		mock_thr_getspecific_null = 1;
	}
	if (origin[0] == 'u') {
		mock_thr_main_result = 0;
		mock_thr_getspecific_null = 1;
		port::__rpc_createerr();
		ref___rpc_createerr();
		mock_thr_getspecific_null = 0;
	}

	port::rpc_createerr *pgot = port::__rpc_createerr();
	port::rpc_createerr *rwant =
	    reinterpret_cast<port::rpc_createerr *>(ref___rpc_createerr());

	bool ok = true;
	if (pgot == nullptr || rwant == nullptr) {
		ok = (pgot == nullptr && rwant == nullptr);
	} else {
		const bool pg_global = pgot == &port::port_rpc_createerr;
		const bool rw_global =
		    rwant == reinterpret_cast<port::rpc_createerr *>(
			&port::port_rpc_createerr);
		ok = pg_global == rw_global;

		if (!pg_global && !rw_global) {
			ok = ok && pgot->cf_stat ==
			    static_cast<enum clnt_stat>(0) &&
			    rwant->cf_stat ==
			    static_cast<enum clnt_stat>(0) &&
			    pgot->cf_error.re_status == 0 &&
			    rwant->cf_error.re_status == 0;
		}

		enum clnt_stat ps = pgot->cf_stat;
		enum clnt_stat rs = rwant->cf_stat;
		pgot->cf_stat = static_cast<enum clnt_stat>(42);
		rwant->cf_stat = static_cast<enum clnt_stat>(43);
		ok = ok && pgot->cf_stat == static_cast<enum clnt_stat>(42) &&
		    rwant->cf_stat == static_cast<enum clnt_stat>(43);
		pgot->cf_stat = ps;
		rwant->cf_stat = rs;
	}

	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL __rpc_createerr [%s]\n", origin);
		}
	}
}

void
edge_cases()
{
	check_rpc_createerr("main");
	check_rpc_createerr("once");
	check_rpc_createerr("once_pos");
	check_rpc_createerr("key");
	check_rpc_createerr("key_pos");
	check_rpc_createerr("setspec");
	check_rpc_createerr("setspec_ret");
	check_rpc_createerr("setspec_neg");
	check_rpc_createerr("alloc");
	check_rpc_createerr("tls");
	check_rpc_createerr("reuse");

	for (int m = 0; m <= 1; m++) {
		for (int o = -1; o <= 1; o++) {
			for (int k = -1; k <= 1; k++) {
				tbl[0].cases++;
				mock_reset_b0196s1();
				mock_thr_main_result = m;
				mock_thr_once_result = o;
				mock_thr_keycreate_result = k;
				mock_thr_getspecific_null = 1;

				port::rpc_createerr *pgot = port::__rpc_createerr();
				struct rpc_createerr *rwant = ref___rpc_createerr();
				const bool pg_global =
				    pgot == &port::port_rpc_createerr;
				const bool rw_global =
				    rwant == &ref_rpc_createerr;
				bool ok = pg_global == rw_global;
				if (!ok)
					tbl[0].failures++;
			}
		}
	}
}

void
random_sweep()
{
	const unsigned long long iterations = 200000;
	static const char origins[] = {
		'm', 'o', 'p', 'k', 'j', 's', 'r', 'n', 'a', 't', 'u'
	};

	for (unsigned long long i = 0; i < iterations; i++) {
		const char origin = origins[static_cast<unsigned>(
		    next_u64() % (sizeof(origins) - 1))];
		check_rpc_createerr(&origin);
	}
}

} /* namespace */

int
main()
{
	edge_cases();
	random_sweep();

	std::printf("\n=== b0196s1 differential test results ===\n");
	unsigned long long total_fail = 0;
	for (const stats &s : tbl) {
		std::printf("%-16s  cases=%llu  failures=%llu\n",
		    s.name, s.cases, s.failures);
		total_fail += s.failures;
	}
	std::printf("TOTAL failures: %llu\n", total_fail);
	return total_fail == 0 ? 0 : 1;
}
