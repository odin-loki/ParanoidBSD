/*
 * harness.cpp -- differential test for PBSD batch b0005.
 *
 * Batch b0005 consists of the single source file
 * sys/security/mac_none/mac_none.c, which defines no functions: its entire
 * body is an empty `struct mac_policy_ops' initializer and the kernel-only
 * MAC_POLICY_SET() registration macro.  Both were recorded in skipped.txt
 * instead of being ported, so port.cppm exports no functions and oracle.c
 * declares no ref_ functions.
 *
 * There is therefore nothing to compare, and this harness registers zero
 * cases.  It deliberately does not manufacture cases against invented data
 * structures: a comparison of two fabrications is not a differential test of
 * the batch, and reporting it as passing coverage would be a false positive.
 */

#include <cstdio>

import pbsd.sys.security.mac.none.b0005;

namespace {

struct Stat {
	const char *name;
	long cases;
	long fails;
};

/*
 * No ported function means no row: the table below is intentionally empty.
 */
const Stat *const stats_begin = nullptr;
const Stat *const stats_end = nullptr;

} /* namespace */

int main()
{
	std::printf("PBSD batch b0005 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	std::printf("%-32s %12s %12s %10s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;
	long functions = 0;
	for (const Stat *s = stats_begin; s != stats_end; ++s) {
		total_cases += s->cases;
		total_fails += s->fails;
		++functions;
		std::printf("%-32s %12ld %12ld %10s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "PASS" : "FAIL");
	}
	if (functions == 0) {
		std::printf("%-32s %12s %12s %10s\n", "(no functions in batch)",
		    "-", "-", "-");
	}
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	std::printf("\n%ld function(s) ported, %ld case(s) run, "
	    "%ld failure(s).\n", functions, total_cases, total_fails);
	std::printf("mac_none.c defines no functions; see skipped.txt.\n");

	return total_fails == 0 ? 0 : 1;
}
