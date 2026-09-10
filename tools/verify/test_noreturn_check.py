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


if __name__ == "__main__":
    unittest.main()
