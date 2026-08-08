/*
 * harness.cpp -- differential test for PBSD batch b0185.
 *
 * Batch b0185 consists of tseq.c, tcreat3.c, tread2.c, and tverify.c.
 * Every source file defines only main() and depends on Berkeley DB hash-table
 * infrastructure recorded in skipped.txt, so port.cppm exports no functions
 * and oracle.c declares no ref_ functions.
 *
 * There is therefore nothing to compare, and this harness registers zero
 * cases.  It deliberately does not manufacture cases against invented db(3)
 * shims: a comparison of two fabrications is not a differential test of the
 * batch.
 */

#include <cstdio>

import pbsd.lib.libc.db.test.hash.tests.b0185;

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
	std::printf("PBSD batch b0185 differential test\n");
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
	std::printf("tseq.c, tcreat3.c, tread2.c, and tverify.c depend on "
	    "db(3) hash-table integration infrastructure; see skipped.txt.\n");

	return total_fails == 0 ? 0 : 1;
}
