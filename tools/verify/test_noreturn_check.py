"""What noreturn_check.py must and must not conclude.

Every QUIET case here is one the lint got wrong at some point while it
was being written; each is kept so it cannot get it wrong again.
"""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import noreturn_check as nc

FIXTURE = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "testdata", "noreturn", "cases.c")
NS_FIXTURE = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                          "testdata", "noreturn", "libc", "gen", "ns.c")


class NoreturnCheck(unittest.TestCase):
    def setUp(self):
        nc._HDR_CACHE.clear()
        self.names = {name for _, name, _ in nc.scan(FIXTURE)}

    def test_reports_a_plain_exit(self):
        self.assertIn("report_plain", self.names)

    def test_reports_through_one_hop(self):
        self.assertIn("report_chained", self.names)

    def test_quiet_when_the_body_has_a_return(self):
        self.assertNotIn("quiet_has_return", self.names)

    def test_quiet_when_the_exit_is_conditional(self):
        self.assertNotIn("quiet_conditional", self.names)

    def test_quiet_on_underscored_noreturn(self):
        self.assertNotIn("quiet_declared", self.names)

    def test_quiet_on_dead2(self):
        self.assertNotIn("quiet_dead2", self.names)

    def test_quiet_on_main(self):
        self.assertNotIn("main", self.names)

    def test_quiet_when_the_last_statement_is_a_block(self):
        self.assertNotIn("quiet_ends_in_block", self.names)

    def test_reports_a_one_line_definition(self):
        self.assertIn("oneline_report", self.names)

    def test_quiet_on_a_declared_one_line_definition(self):
        self.assertNotIn("oneline_quiet", self.names)

    def test_reports_past_a_preprocessor_line(self):
        self.assertIn("report_after_endif", self.names)

    def test_reports_past_a_trailing_va_end(self):
        self.assertIn("report_va_end", self.names)

    def test_quiet_when_the_call_before_va_end_returns(self):
        self.assertNotIn("quiet_va_end", self.names)

    # A SEED name the FILE ITSELF defines is judged by its body. sbin/
    # restore's panic() returns; the kernel's panic(9) does not, and the
    # seed cannot tell them apart by name.
    def test_a_local_seed_name_that_returns_is_not_trusted(self):
        self.assertNotIn("quiet_ends_in_local_panic", self.names)

    def test_a_local_seed_name_is_still_not_itself_reported(self):
        """Dropping the seed for PROPAGATION must not also make the
        definition reportable: the first attempt did both, and lib/
        gained ten findings that were libc's own exit(), abort() and
        errx() -- all genuinely noreturn, all declared so in headers
        headers_for() cannot reach. panic() is the seeded name here;
        exiting_panic() and local_abort() are not, and SHOULD be
        reported, which is the rule working as intended."""
        self.assertNotIn("panic", self.names)
        self.assertIn("exiting_panic", self.names)
        self.assertIn("local_abort", self.names)

    def test_a_local_seed_name_that_exits_is_put_back_by_propagation(self):
        self.assertIn("report_ends_in_local_exiting_panic", self.names)

    def test_a_libc_private_name_is_judged_by_its_public_one(self):
        """lib/libc/gen/err.c defines _err(); <err.h> declares err()
        __dead2; namespace.h turns the second into the first before any
        libc source sees it. Reading only the private name made _err()
        the sole --guards candidate in all of lib, and it was a false
        positive -- the attribute is there, under the erased name."""
        nc._HDR_CACHE.clear()
        nc._NS_CACHE.clear()
        names = {name for _, name, _ in nc.scan(NS_FIXTURE)}
        self.assertNotIn("_err", names)
        # Finding an alias is not itself the excuse: quit() is renamed
        # the same way and is not declared noreturn anywhere.
        self.assertIn("_quit", names)
        # And a private name with no alias at all is untouched.
        self.assertIn("_privately_exits", names)

    def test_builtin_trap_does_not_return(self):
        """rtld's abort() is raise() then __builtin_trap(). Without the
        builtin in SEED, dropping the local name lost __assert() -- a
        true positive traded away for a false-positive fix."""
        self.assertIn("report_ends_in_trapping_abort", self.names)


if __name__ == "__main__":
    unittest.main()
