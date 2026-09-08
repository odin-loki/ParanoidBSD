#!/usr/bin/env python3
"""Every shape in the fixture is reported, or is not, on purpose.

The half that matters is the negative half. A leak detector that reports
a hundred contracts is worse than none, because the hundred are what
stops anybody reading the one. So the fixture is mostly functions that
must NOT be reported, each named for the reason.

The live half re-runs the tool over the three files whose findings were
read by hand, and asserts the answers that reading gave.
"""

import subprocess
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))

import lock_balance as lb                                  # noqa: E402

FIXTURE = HERE / "testdata" / "lock_balance_shapes.c"


def lines_of(path: Path) -> set:
    return {ln for ln, _n, _a, _h in lb.check(path)}


class Fixture(unittest.TestCase):
    def test_the_two_leaks_and_nothing_else(self):
        got = lines_of(FIXTURE)
        text = FIXTURE.read_text().splitlines()
        want = {i + 1 for i, ln in enumerate(text)
                if ln.strip() in ("return (2);", "return (1);")
                and "LEAK" not in ln}
        # the two the fixture's comments call LEAK, by their own lines
        self.assertEqual(len(got), 2, sorted(got))
        for ln in got:
            self.assertTrue(text[ln - 1].strip().startswith("return"))
        self.assertLessEqual(got, want)

    def test_deleting_an_unlock_is_caught(self):
        """The gate has to fail before it is worth anything.

        Delete the release inside `if (seq == 0)', not the one before
        the last return: taking away the LAST one makes the function
        fall off the end holding the lock, which the tool reads - on
        purpose - as a contract, and it then reports nothing at all.
        """
        text = FIXTURE.read_text().splitlines()
        i = next(k for k, ln in enumerate(text)
                 if ln.strip() == "mtx_unlock(&lk);"
                 and text[k + 1].strip() == "return (0);")
        broken = HERE / "testdata" / "_lock_balance_broken.c"
        broken.write_text("\n".join(text[:i] + text[i + 1:]) + "\n")
        try:
            self.assertEqual(len(lines_of(broken)), 3)
        finally:
            broken.unlink()


class Tree(unittest.TestCase):
    """Read by hand, one at a time, and these are the answers."""

    # Reported, read, confirmed, and FIXED in the same commit that added
    # this file's tool. They are kept by name rather than deleted: the
    # tool is only worth its output, and a regression that puts one of
    # these back has to be visible as this test going red rather than as
    # a line reappearing in a list nobody diffs.
    FIXED = {
        "sys/netipsec/ipsec.c": "ipsec_chkreplay(), th == 0 arm",
        "sys/dev/drm2/drm_bufs.c": "drm_get_resource_start() and _len()",
        "sys/arm/allwinner/aw_mmc.c": "aw_mmc_cam_request(), EBUSY",
        "sys/arm64/nvidia/tegra210/max77620_gpio.c": "one of five arms",
        "sys/arm/nvidia/drm2/tegra_bo.c": "tegra_bo_init_pager()",
        "sys/powerpc/mpc85xx/fsl_espi.c": "fsl_espi_transfer()",
        "sys/powerpc/pseries/phyp_vscsi.c": "vscsi_attach(), M_NOWAIT",
        "sys/dev/sound/pci/ich.c": "ichchan_init(), default: arm",
        "sys/netpfil/ipfw/ip_fw_table.c": "find_table_entry()",
    }
    # Reported, and read, and NOT leaks. Kept so that a change which
    # "fixes" one of them has to say what it did.
    KNOWN_FALSE = {
        "sys/dev/cxgbe/iw_cxgbe/cm.c": 1146,        # solisten_dequeue unlocks
        "sys/kern/vfs_mount.c": 2319,               # dounmount_cleanup does
        "sys/kern/kern_proc.c": 454,                # _pfind returns it locked
    }

    def test_the_fixed_ones_stay_fixed(self):
        for rel, why in self.FIXED.items():
            src = ROOT / "hbsd/src" / rel
            if not src.is_file():
                self.skipTest(f"{rel} is not in this tree")
            with self.subTest(rel):
                self.assertEqual(lines_of(src), set(), why)

    def test_the_three_that_are_not_leaks_are_still_reported(self):
        """Not because they should be fixed - because the tool should not
        quietly stop looking at the shapes they are made of."""
        for rel, line in self.KNOWN_FALSE.items():
            src = ROOT / "hbsd/src" / rel
            if not src.is_file():
                self.skipTest(f"{rel} is not in this tree")
            with self.subTest(rel):
                self.assertIn(line, lines_of(src))

    def test_the_tool_runs_on_a_directory(self):
        out = subprocess.run(
            [sys.executable, str(HERE / "lock_balance.py"),
             str(ROOT / "hbsd/src/sys/kern")],
            capture_output=True, text=True, check=True).stdout
        self.assertIn("kern_proc.c:454", out)
        self.assertIn("return(s) in", out.splitlines()[-1])


if __name__ == "__main__":
    unittest.main()
