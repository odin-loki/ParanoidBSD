#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""esbmc_driver.py's parsing, against a mock that emits ESBMC's shapes.

THIS IS THE ONLY THING THAT HAS EVER RUN. The real binary is in no
distribution's package set and is not on PyPI, so the driver was written
against ESBMC's documented CLI and output format and has never met it.
What that leaves testable is the half that is ours: given ESBMC's
output, does this driver reach the right verdict?

So mock/esbmc emits each documented output shape and this drives the
real driver over it. A green run here means the PARSER is right about
the format it was told about. It means nothing whatever about whether
the FLAGS are right - only `esbmc_driver.py --selftest' on a machine
with the binary can say that, and the cases below cannot substitute
for it.

The cases that matter most are the ones where a wrong reading would be
a FALSE CLAIM rather than a missing one:

  * PROVED-UNBOUNDED must need the inductive step or the forward
    condition IN THE OUTPUT. Awarding it for --mode kinduction plus
    VERIFICATION SUCCESSFUL would turn "ran out of k" into a theorem.
  * a clean bounded run must NOT be PROVED unless something says the
    bound was policed. ESBMC's unwinding-assertion default is this
    driver's largest uncertainty and this is where being wrong about
    it is made cheap.
  * falsification is base-case bug hunting. Its SUCCESSFUL is not a
    proof, whatever the word suggests.
  * an absent binary is NOTRUN, is written to the JSONL, and exits
    non-zero. A scope nobody checked must never look like a clean one.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
MOCK = HERE / "testdata" / "esbmc_mock" / "esbmc"
DRIVER = HERE / "esbmc_driver.py"

os.environ["PBSD_ESBMC"] = str(MOCK)
import esbmc_driver as D  # noqa: E402


def task(fn, mode="bounded", **kw):
    t = {
        "file": "probe.c", "src": "probe.c", "function": fn,
        "tier": "ub", "mode": mode, "unwind": 16, "max_k": 32,
        "k_step": 0, "context_bound": 2, "concurrency_checks": False,
        "unwind_assert": "auto", "solver": "default", "timeout": 20,
        "esbmc_timeout": 0, "memlimit": "", "mem_mb": 0,
        "class": "SCALAR", "linkage": "exported",
    }
    t.update(kw)
    return t


class Bounded(unittest.TestCase):
    def test_clean_without_evidence_is_bounded_not_proved(self):
        # The whole unwinding-assertion uncertainty, in one case. ESBMC is
        # BELIEVED to generate unwinding assertions by default. If that is
        # wrong, a loop that wanted more than --unwind iterations comes
        # back SUCCESSFUL, and calling it PROVED is a false theorem about
        # all inputs. Under `auto' the driver declines instead.
        r = D.verify_one(task("ok_bounded"))
        self.assertEqual(r["status"], "BOUNDED")
        self.assertTrue(r["unwind_assert_unconfirmed"])

    def test_clean_with_evidence_is_proved(self):
        r = D.verify_one(task("ok_bounded_evidence"))
        self.assertEqual(r["status"], "PROVED")
        self.assertNotIn("unwind_assert_unconfirmed", r)

    def test_asking_for_the_flag_is_enough_for_proved(self):
        # --unwind-assert on passes --unwinding-assertions. If that
        # spelling does not exist, ESBMC errors and the record is ERROR,
        # which is loud. So asking is safe and silence is not.
        r = D.verify_one(task("ok_bounded", unwind_assert="on"))
        self.assertEqual(r["status"], "PROVED")

    def test_off_never_proves(self):
        r = D.verify_one(task("ok_bounded_evidence", unwind_assert="off"))
        self.assertEqual(r["status"], "BOUNDED")

    def test_only_an_unwinding_assertion_failed_is_bounded(self):
        # ESBMC reports the bound being too small as VERIFICATION FAILED
        # whose one violated property is the unwinding assertion. CBMC
        # reports the same fact the other way round. Same meaning, and it
        # is not a defect.
        r = D.verify_one(task("unwind_hit"))
        self.assertEqual(r["status"], "BOUNDED")
        self.assertTrue(r["unwind_hit"])
        self.assertEqual(r.get("failures"), None)

    def test_a_real_violation_is_failed(self):
        r = D.verify_one(task("overflow_fail"))
        self.assertEqual(r["status"], "FAILED")
        self.assertEqual(len(r["failures"]), 1)
        self.assertIn("arithmetic overflow on add", r["failures"][0]["desc"])
        self.assertIn('!overflow("+", a, b)', r["failures"][0]["expr"])

    def test_a_defect_beats_a_bound(self):
        r = D.verify_one(task("mixed_fail"))
        self.assertEqual(r["status"], "FAILED")
        self.assertTrue(r["unwind_hit"])
        # The unwinding assertion is NOT in the failure list: it is not a
        # defect and report.py would bucket it as one.
        self.assertTrue(all("unwinding" not in f["desc"]
                            for f in r["failures"]))

    def test_failed_with_nothing_parseable_is_error(self):
        # Not a finding and not clean. A record nobody can act on is the
        # same as no record, so it is ERROR with the output kept.
        r = D.verify_one(task("failed_noprops"))
        self.assertEqual(r["status"], "ERROR")
        self.assertIn("detail", r)

    def test_vcc_counts_are_recorded(self):
        r = D.verify_one(task("overflow_fail"))
        self.assertEqual(r["vcc_generated"], 14)
        self.assertEqual(r["vcc_remaining"], 9)


class KInduction(unittest.TestCase):
    def test_inductive_step_is_unbounded(self):
        # The reason this driver exists. The inductive step closes the
        # property for EVERY iteration count, so there is no bound in the
        # claim - strictly stronger than cbmc_driver.py's PROVED.
        r = D.verify_one(task("kind_inductive", mode="kinduction"))
        self.assertEqual(r["status"], "PROVED-UNBOUNDED")
        self.assertEqual(r["closed_by"], "inductive-step")
        self.assertEqual(r["k"], 2)

    def test_forward_condition_is_unbounded(self):
        r = D.verify_one(task("kind_forward", mode="kinduction"))
        self.assertEqual(r["status"], "PROVED-UNBOUNDED")
        self.assertEqual(r["closed_by"], "forward-condition")
        self.assertEqual(r["k"], 3)

    def test_base_case_alone_is_never_unbounded(self):
        # "Solution found by the base case" is BMC at depth k. It found a
        # bug; it proved nothing.
        r = D.verify_one(task("kind_base_fail", mode="kinduction"))
        self.assertEqual(r["status"], "FAILED")
        self.assertEqual(r["closed_by"], "base-case")

    def test_running_out_of_k_is_unknown(self):
        r = D.verify_one(task("kind_exhausted", mode="kinduction"))
        self.assertEqual(r["status"], "UNKNOWN")
        self.assertIn("detail", r)

    def test_clean_without_a_reason_is_not_unbounded(self):
        # The case that would be easiest to get wrong: SUCCESSFUL under
        # --k-induction. Awarding PROVED-UNBOUNDED on the strength of the
        # MODE would mean the driver's biggest claim rested on a flag
        # having been passed, not on anything ESBMC said.
        r = D.verify_one(task("kind_clean_no_reason", mode="kinduction"))
        self.assertEqual(r["status"], "BOUNDED")


class Incremental(unittest.TestCase):
    def test_forward_condition_is_unbounded(self):
        r = D.verify_one(task("incr_forward", mode="incremental"))
        self.assertEqual(r["status"], "PROVED-UNBOUNDED")
        self.assertEqual(r["closed_by"], "forward-condition")

    def test_unknown(self):
        r = D.verify_one(task("incr_unknown", mode="incremental"))
        self.assertEqual(r["status"], "UNKNOWN")


class Falsification(unittest.TestCase):
    def test_clean_falsification_is_never_a_proof(self):
        r = D.verify_one(task("fals_clean", mode="falsification"))
        self.assertEqual(r["status"], "BOUNDED")

    def test_it_still_finds_bugs(self):
        r = D.verify_one(task("overflow_fail", mode="falsification"))
        self.assertEqual(r["status"], "FAILED")


class Concurrency(unittest.TestCase):
    def test_a_data_race_is_a_failure(self):
        # The one defect class this harness cannot reach at all today.
        r = D.verify_one(task("race_fail", mode="concurrency",
                              concurrency_checks=True))
        self.assertEqual(r["status"], "FAILED")
        self.assertIn("data race on shared_counter", r["failures"][0]["desc"])

    def test_the_context_bound_is_on_the_command_line(self):
        cmd = D.build_cmd(task("race_fail", mode="concurrency",
                               context_bound=3, concurrency_checks=True))
        self.assertIn("--context-bound", cmd)
        self.assertEqual(cmd[cmd.index("--context-bound") + 1], "3")
        self.assertIn("--data-races-check", cmd)

    def test_concurrency_checks_are_opt_in(self):
        cmd = D.build_cmd(task("race_fail", mode="concurrency"))
        self.assertNotIn("--data-races-check", cmd)


class NotAnAnswer(unittest.TestCase):
    def test_parse_error(self):
        r = D.verify_one(task("parse_error"))
        self.assertEqual(r["status"], "ERROR")
        # The why-line is hoisted to the head of the window, because
        # report.py reads the first lines of `detail' looking for a
        # reason and a truncation window does not contain one.
        head = r["detail"].splitlines()[0]
        self.assertTrue("PARSING ERROR" in head or "fatal error" in head,
                        f"why-line not hoisted: {head!r}")

    def test_conversion_error(self):
        r = D.verify_one(task("conversion_error"))
        self.assertEqual(r["status"], "ERROR")
        self.assertIn("CONVERSION ERROR", r["detail"])

    def test_function_not_in_this_unit(self):
        r = D.verify_one(task("nofunc"))
        self.assertEqual(r["status"], "NOFUNC")

    def test_out_of_memory_keeps_its_reason(self):
        # Half of run 33's CBMC ERRORs were the memory bound, and they were
        # landing in report.py's "no reason recorded" bucket, which reads
        # as "we do not know" when we do. Same trap, same remedy.
        r = D.verify_one(task("oom"))
        self.assertEqual(r["status"], "ERROR")
        self.assertIn("Out of memory", r["detail"].splitlines()[0])

    def test_timeout(self):
        r = D.verify_one(task("hang", timeout=2))
        self.assertEqual(r["status"], "TIMEOUT")


class AbsentBinaryIsNeverClean(unittest.TestCase):
    """The premise of the whole repository, in the form most likely to
    go unnoticed: a missing tool costs no time and prints no error."""

    def setUp(self):
        self._old = os.environ.get("PBSD_ESBMC")
        os.environ["PBSD_ESBMC"] = "/nonexistent/esbmc"

    def tearDown(self):
        os.environ["PBSD_ESBMC"] = self._old or str(MOCK)

    def test_available_says_no(self):
        self.assertFalse(D.esbmc_available())

    def test_every_status_becomes_notrun(self):
        for mode in ("bounded", "kinduction", "incremental",
                     "falsification", "concurrency"):
            r = D.verify_one(task("ok_bounded", mode=mode))
            self.assertEqual(r["status"], "NOTRUN", mode)
            self.assertIn("NOTHING was checked", r["detail"])

    def test_selftest_does_not_pass(self):
        p = subprocess.run([sys.executable, str(DRIVER), "--selftest"],
                           capture_output=True, text=True, timeout=120)
        self.assertNotEqual(p.returncode, 0)
        self.assertIn("NOT FOUND", p.stdout)


class FlagsAreNeverSilentlyWeakened(unittest.TestCase):
    """ESBMC enables bounds, pointer and div-by-zero BY DEFAULT and takes
    --no-* to switch them off. That is the opposite polarity from CBMC,
    and a check disabled by a flag nobody reads is invisible in a result.
    tools/pbsd_agent/esbmc_check.py passes --no-bounds-check on every run
    it has ever made; these guard against that landing here."""

    def test_the_ub_tier_passes_no_negative_check_flag(self):
        cmd = D.build_cmd(task("ok_bounded"))
        for bad in D.FORBIDDEN_FLAGS:
            self.assertNotIn(bad, cmd)

    def test_the_ub_tier_is_the_positive_checks(self):
        cmd = D.build_cmd(task("ok_bounded"))
        for f in ("--overflow-check", "--ub-shift-check",
                  "--memory-leak-check"):
            self.assertIn(f, cmd)
        # Advisory checks are a separate run, exactly as in cbmc_driver.py.
        self.assertNotIn("--unsigned-overflow-check", cmd)
        self.assertNotIn("--nan-check", cmd)

    def test_the_advisory_tier_is_the_advisory_checks(self):
        cmd = D.build_cmd(task("ok_bounded", tier="advisory"))
        self.assertIn("--unsigned-overflow-check", cmd)
        self.assertIn("--nan-check", cmd)
        self.assertNotIn("--overflow-check", cmd)

    def test_check_flags_refuses_a_disabled_check(self):
        with self.assertRaises(ValueError):
            D.check_flags(["esbmc", "x.c", "--no-pointer-check"])
        with self.assertRaises(ValueError):
            D.check_flags(["esbmc", "x.c", "--no-assertions"])

    def test_extra_cannot_smuggle_one_in(self):
        r = D.verify_one(task("ok_bounded", extra=["--no-bounds-check"]))
        self.assertEqual(r["status"], "ERROR")
        self.assertIn("refusing to run", r["detail"])

    def test_every_flag_the_driver_can_pass_has_a_confidence(self):
        # A flag with no entry in FLAG_CONFIDENCE is a flag --selftest
        # will not probe, which is how an unverified spelling gets into a
        # verdict without anyone being told.
        seen = set()
        for mode in ("bounded", "kinduction", "incremental",
                     "falsification", "concurrency"):
            for ua in ("auto", "on", "off"):
                for tier in ("ub", "advisory"):
                    for solver in D.SOLVER_FLAG:
                        cmd = D.build_cmd(task(
                            "x", mode=mode, unwind_assert=ua, tier=tier,
                            solver=solver, k_step=1, memlimit="2g",
                            esbmc_timeout=30, concurrency_checks=True))
                        seen.update(a for a in cmd[1:] if a.startswith("-"))
        missing = sorted(seen - set(D.FLAG_CONFIDENCE))
        self.assertEqual(missing, [], f"undocumented flags: {missing}")

    def test_the_lock_order_check_is_not_reachable_by_accident(self):
        # Its confidence is "low" and nothing turns it on. If that
        # changes, this fails and someone has to say so on purpose.
        cmd = D.build_cmd(task("x", mode="concurrency",
                               concurrency_checks=True))
        self.assertNotIn("--lock-order-check", cmd)


class ReportCompatibility(unittest.TestCase):
    """report.py buckets a record without knowing which engine wrote it,
    so an ESBMC failure has to look like a CBMC one where report.py
    reads it - or the ESBMC findings quietly fall out of the triage."""

    def test_desc_matches_report_pys_line_regex(self):
        r = D.verify_one(task("overflow_fail"))
        d = r["failures"][0]["desc"]
        # report.py: re.match(r"line (\d+) (.*)", d["desc"])
        m = re.match(r"line (\d+) (.*)", d)
        self.assertIsNotNone(m, d)
        self.assertEqual(int(m.group(1)), 7)
        # report.py's kinds(): re.sub(r"^line \d+ ", "", desc)
        self.assertEqual(re.sub(r"^line \d+ ", "", d),
                         "arithmetic overflow on add")

    def test_every_record_names_its_engine(self):
        # cbmc_driver.py's records have no engine field because there was
        # only ever one engine. report.py takes several .jsonl on one
        # command line, so a merged file exists the first time anyone runs
        # both, and a merged file that cannot tell them apart reports the
        # union as if one tool had said it.
        r = D.verify_one(task("ok_bounded"))
        self.assertEqual(r["engine"], "esbmc")
        self.assertEqual(r["mode"], "bounded")
        self.assertEqual(r["v"], D.RESULT_VERSION)

    def test_report_pys_status_order_does_not_hide_the_new_ones(self):
        # report.py prints `order + [k for k in status if k not in order]',
        # so PROVED-UNBOUNDED, UNKNOWN and NOTRUN appear even though it
        # has never heard of them. They appear UNEXPLAINED, which is the
        # follow-up this driver cannot make from here.
        order = ["PROVED", "PROVED-ASSUMING", "BOUNDED", "FAILED",
                 "TIMEOUT", "ERROR", "NOFUNC"]
        for s in ("PROVED-UNBOUNDED", "UNKNOWN", "NOTRUN"):
            self.assertNotIn(s, order)


class EndToEnd(unittest.TestCase):
    """The driver as a process: a plan, a classes file, a .jsonl out."""

    def _tree(self, td):
        td = Path(td)
        (td / "pre" / "lib" / "x").mkdir(parents=True)
        # --preprocessed is the path that works without FreeBSD headers.
        (td / "pre" / "lib" / "x" / "a.c.i").write_text("int f(void){return 0;}\n")
        plan = {"records": [{"path": "lib/x/a.c",
                             "functions": ["ok_bounded", "overflow_fail",
                                           "kind_inductive", "nofunc"]}]}
        classes = {"lib/x/a.c": {"ok": True, "gb": "unused-by-esbmc",
                                 "functions": {"ok_bounded": "SCALAR",
                                               "overflow_fail": "SCALAR",
                                               "kind_inductive": "SCALAR",
                                               "nofunc": "SCALAR"},
                                 "linkage": {"ok_bounded": "exported"}}}
        (td / "plan.json").write_text(json.dumps(plan))
        (td / "classes.json").write_text(json.dumps(classes))
        return td

    def _run(self, td, *extra, esbmc=None):
        env = dict(os.environ)
        env["PBSD_ESBMC"] = esbmc or str(MOCK)
        return subprocess.run(
            [sys.executable, str(DRIVER),
             "--plan", str(td / "plan.json"),
             "--classes", str(td / "classes.json"),
             "--preprocessed", str(td / "pre"),
             "--out", str(td / "out.jsonl"), "--jobs", "2",
             "--timeout", "20", *extra],
            capture_output=True, text=True, env=env, timeout=180)

    def test_a_whole_run_writes_one_record_per_function(self):
        with tempfile.TemporaryDirectory() as t:
            td = self._tree(t)
            p = self._run(td, "--mode", "kinduction")
            self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
            recs = [json.loads(l) for l in
                    (td / "out.jsonl").read_text().splitlines() if l.strip()]
            got = {r["function"]: r["status"] for r in recs}
            self.assertEqual(got, {
                "ok_bounded": "BOUNDED",          # SUCCESSFUL, no reason
                "overflow_fail": "FAILED",
                "kind_inductive": "PROVED-UNBOUNDED",
                "nofunc": "NOFUNC",
            })
            self.assertIn("PROVED-UNBOUNDED means", p.stdout)

    def test_an_absent_binary_exits_nonzero_and_records_notrun(self):
        with tempfile.TemporaryDirectory() as t:
            td = self._tree(t)
            p = self._run(td, esbmc="/nonexistent/esbmc")
            self.assertEqual(p.returncode, 1, p.stdout)
            recs = [json.loads(l) for l in
                    (td / "out.jsonl").read_text().splitlines() if l.strip()]
            self.assertEqual({r["status"] for r in recs}, {"NOTRUN"})
            self.assertEqual(len(recs), 4)
            self.assertIn("never checked", p.stdout)

    def test_resume_does_not_inherit_a_missing_binary(self):
        # The nastiest resume bug available here: run once with no esbmc,
        # then again on a machine that has it, and --resume skips every
        # function the first run failed to check. The absence would
        # become permanent and invisible.
        with tempfile.TemporaryDirectory() as t:
            td = self._tree(t)
            self._run(td, esbmc="/nonexistent/esbmc")
            p = self._run(td, "--resume")
            self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
            recs = [json.loads(l) for l in
                    (td / "out.jsonl").read_text().splitlines() if l.strip()]
            real = [r for r in recs if r["status"] != "NOTRUN"]
            self.assertEqual(len(real), 4, [r["status"] for r in recs])

    def test_resume_does_not_cross_modes(self):
        # A BOUNDED from --mode falsification is not an answer to what
        # --mode kinduction would conclude.
        with tempfile.TemporaryDirectory() as t:
            td = self._tree(t)
            self._run(td, "--mode", "falsification")
            p = self._run(td, "--mode", "kinduction", "--resume")
            self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
            recs = [json.loads(l) for l in
                    (td / "out.jsonl").read_text().splitlines() if l.strip()]
            kind = [r for r in recs if r["mode"] == "kinduction"]
            self.assertEqual(len(kind), 4)

    def test_a_rerun_without_resume_does_not_double_count(self):
        with tempfile.TemporaryDirectory() as t:
            td = self._tree(t)
            self._run(td)
            self._run(td)
            recs = (td / "out.jsonl").read_text().splitlines()
            self.assertEqual(len([l for l in recs if l.strip()]), 4)

    def test_retry_status_drops_error_and_rechecks(self):
        # --resume would skip ERROR, which is how a corrected driver
        # would never re-run the 10868-ERROR sweep. --retry-status
        # ERROR drops those rows so the file does not hold two
        # generations, then checks them again.
        with tempfile.TemporaryDirectory() as t:
            td = self._tree(t)
            rec = {
                "v": D.RESULT_VERSION, "engine": "esbmc", "mode": "bounded",
                "file": "lib/x/a.c", "function": "ok_bounded",
                "status": "ERROR", "tier": "ub", "detail": "unrecognised option '-xc'",
            }
            (td / "out.jsonl").write_text(json.dumps(rec) + "\n")
            p = self._run(td, "--resume", "--retry-status", "ERROR")
            self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
            recs = [json.loads(l) for l in
                    (td / "out.jsonl").read_text().splitlines() if l.strip()]
            by_fn = [r for r in recs if r["function"] == "ok_bounded"]
            self.assertEqual(len(by_fn), 1, by_fn)
            self.assertNotEqual(by_fn[0]["status"], "ERROR")
            self.assertIn("retried", p.stdout)

    def test_selftest_against_the_mock_probes_every_flag(self):
        p = subprocess.run([sys.executable, str(DRIVER), "--selftest"],
                           capture_output=True, text=True, timeout=120,
                           env=dict(os.environ, PBSD_ESBMC=str(MOCK)))
        self.assertIn("flags this driver can pass", p.stdout)
        self.assertIn("checks CBMC makes that this driver does NOT",
                      p.stdout)
        self.assertIn("--pointer-overflow-check", p.stdout)
        # The mock's --help carries every flag the driver can pass, so a
        # spelling that drifts out of the mock is reported here rather
        # than discovered on a real binary.
        self.assertNotIn("NOT IN --help", p.stdout,
                         "a flag the driver passes is not in the mock's "
                         "--help; fix the mock or the spelling")


class ClangEThenDotI(unittest.TestCase):
    """The path that actually produces proofs: clang -E, then ESBMC
    on the .i, never -xc on ESBMC's argv."""

    def test_compiler_driver_flags_never_reach_esbmc(self):
        cmd = D.build_cmd(task("ok_bounded", cflags=[
            "-xc", "-std=gnu17", "--target=x86_64-unknown-freebsd15.0",
            "-Wno-everything", "-I/foo", "-D__FreeBSD__=15",
        ]))
        joined = " ".join(cmd)
        self.assertNotIn("-xc", cmd)
        self.assertNotIn("-std=gnu17", cmd)
        self.assertNotIn("--target=x86_64-unknown-freebsd15.0", joined)
        self.assertIn("-I/foo", cmd)
        self.assertIn("-D__FreeBSD__=15", cmd)

    def test_preprocess_cmd_is_clang_e_with_those_flags(self):
        cmd = D.preprocess_cmd("a.c", "a.c.i", ["-xc", "-std=gnu17", "-I/h"])
        self.assertEqual(cmd[1], "-E")
        self.assertIn("-xc", cmd)
        self.assertIn("-o", cmd)
        self.assertIn("a.c.i", cmd)

    def test_pointer_success_is_proved_assuming_not_proved(self):
        r = D.verify_one(task("ok_bounded_evidence", **{"class": "POINTER"}))
        self.assertEqual(r["status"], "PROVED-ASSUMING")
        self.assertIn("min-null-tree-depth", r.get("assuming", ""))

    def test_kinduction_pointer_is_still_assuming(self):
        r = D.verify_one(task("kind_inductive", mode="kinduction",
                              **{"class": "POINTER"}))
        self.assertEqual(r["status"], "PROVED-ASSUMING")
        self.assertEqual(r.get("closed_by"), "inductive-step")

    def test_scalar_kinduction_is_still_unbounded(self):
        r = D.verify_one(task("kind_inductive", mode="kinduction"))
        self.assertEqual(r["status"], "PROVED-UNBOUNDED")


if __name__ == "__main__":
    unittest.main(verbosity=2)
