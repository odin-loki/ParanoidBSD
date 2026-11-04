/*
 * harness.cpp -- differential test for PBSD batch b0107.
 *
 * Batch b0107 consists of fgetwc.c, snprintf.c, and gets_s.c.  Every
 * function in those files depends on FreeBSD/HardenedBSD libc internals
 * recorded in skipped.txt, so port.cppm exports no functions and oracle.c
 * declares no ref_ functions.
 *
 * There is therefore nothing to compare, and this harness registers zero
 * cases.  It deliberately does not manufacture cases against invented
 * FILE layouts or vfprintf shims: a comparison of two fabrications is not
 * a differential test of the batch.
 */

#include <cstdio>

import pbsd.lib.libc.stdio.b0107;

namespace {

struct Stat {
	const char *name;
	long cases;
	long fails;
};

const Stat *const stats_begin = nullptr;
const Stat *const stats_end = nullptr;

} /* namespace */

int
main(void)
{
	std::printf("PBSD batch b0107 differential test\n");
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
	std::printf("fgetwc.c, snprintf.c, and gets_s.c depend on libc "
	    "internals; see skipped.txt.\n");

	return total_fails == 0 ? 0 : 1;
}
