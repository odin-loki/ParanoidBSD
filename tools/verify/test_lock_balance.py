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

    LEAKS = {
        "sys/netipsec/ipsec.c": 1357,
        "sys/dev/sound/pci/ich.c": 411,
        "sys/netpfil/ipfw/ip_fw_table.c": 1079,
        "sys/arm/allwinner/aw_mmc.c": 312,
        "sys/arm/nvidia/drm2/tegra_bo.c": 165,
        "sys/arm64/nvidia/tegra210/max77620_gpio.c": 563,
        "sys/powerpc/mpc85xx/fsl_espi.c": 350,
        "sys/powerpc/pseries/phyp_vscsi.c": 341,
    }
    # two in one file, so they do not fit the dict above
    PAIRS = [("sys/dev/drm2/drm_bufs.c", 85), ("sys/dev/drm2/drm_bufs.c", 102)]
    # Reported, and read, and not leaks. Kept so that a change which
    # "fixes" one of them has to say what it did to the ten above.
    KNOWN_FALSE = {
        "sys/dev/cxgbe/iw_cxgbe/cm.c": 1146,        # solisten_dequeue unlocks
        "sys/kern/vfs_mount.c": 2319,               # dounmount_cleanup does
        "sys/kern/kern_proc.c": 454,                # _pfind returns it locked
    }

    def test_each_named_leak_is_still_reported(self):
        for rel, line in self.LEAKS.items():
            src = ROOT / "hbsd/src" / rel
            if not src.is_file():
                self.skipTest(f"{rel} is not in this tree")
            with self.subTest(rel):
                self.assertIn(line, lines_of(src))

    def test_the_pair_in_one_file(self):
        src = ROOT / "hbsd/src" / self.PAIRS[0][0]
        if not src.is_file():
            self.skipTest("drm_bufs.c is not in this tree")
        got = lines_of(src)
        for _rel, line in self.PAIRS:
            self.assertIn(line, got)

    def test_the_tool_runs_on_a_directory(self):
        out = subprocess.run(
            [sys.executable, str(HERE / "lock_balance.py"),
             str(ROOT / "hbsd/src/sys/netipsec")],
            capture_output=True, text=True, check=True).stdout
        self.assertIn("ipsec.c:1357", out)
        self.assertIn("return(s) in", out.splitlines()[-1])


if __name__ == "__main__":
    unittest.main()
