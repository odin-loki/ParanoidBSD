# PBSD C→C++ port — resume plan

Handoff written 2026-08-01. Updated 2026-08-08 after fixing the jammed queue and
restarting with Composer 2.5 + cursor login (not the expired API key).

---

## Where things stand

Working copy is **`~/pbsd` inside WSL (Ubuntu)**, not the Windows folder. It is
on ext4; the Windows checkout is a 9p mount where `git status` alone takes nine
minutes. 1,558 commits, clean tree.

| | count |
|---|---|
| VERIFIED | 268 (236 proven with no agent call, 32 by agent) |
| NEEDS_HUMAN | 4,204 |
| SKIPPED | 25 |
| PENDING | **0** |
| DEFERRED | **0** |

Pending and deferred are both zero, so **rerunning the driver right now does
nothing at all**. The queue must be reset first (Phase 1).

The Windows repo at `C:\Users\odinl\OneDrive\Desktop\Operating System` is behind
the WSL repo and needs a fast-forward at the end.

## What actually happened

The run looks like a catastrophic quality failure: 1,354 of 1,359 agent batches
rejected, then 4,204 deferred files burned through at 1,238 files/hour with zero
recovered. Three seconds per file is not a model thinking about C code.

Grouping the failures by reason gives the real story:

| failures | reason |
|---|---|
| 1,346 | `ActionRequiredError: Increase limits for faster requests` / `Rate limit exceeded` |
| 4,203 | same, during the deferred phase |
| 1 | mutation check (a genuine code verdict) |
| 1 | differential run failed (a genuine code verdict) |

**Two real code failures in the entire run.** Everything else was the Cursor
account hitting its usage limit at 80-way concurrency. The pipeline then did the
worst possible thing: it treated "rate limited" as "this file is too hard",
marked all 4,204 files NEEDS_HUMAN, exhausted their retry budget, and emptied the
queue. That is the bug to fix, and it is in `pbsd.py`, not in the ports.

## What is already done and must not be re-litigated

- **The agent-free mechanical path is sound.** It compiles the original C and the
  C++ module to LLVM IR and compares function bodies, and it self-validates: for
  every file it certifies, it plants a bug at *every* mutation site and requires
  the comparison to reject all of them. It was unsound at first and accepted 55
  of 189 planted bugs; after the fixes it rejects 925 of 925. Do not relax
  `proof_is_sensitive`, `degenerate()`, or the metadata-only IR normalisation to
  make the numbers look better — each of those exists because it caught a real
  wrong port.
- **The parallel architecture is correct.** Workers only compute verdicts; the
  parent process is the sole writer of git, the inventory and the log. Keep it
  that way or the workers will fight over `index.lock`.
- **Throughput of the free path**: whole tree screens in about 6 minutes at 64
  workers.
- `claude-opus-5-thinking-high` is a valid model slug and works.
- WSL is configured with 50 GB RAM / 64 processors via `C:\Users\odinl\.wslconfig`.
  At 80 concurrent agents memory sat at ~22 GB, so memory is not the constraint.
  128 concurrent got OOM-killed before the memory increase.

---

## Phase 0 — unblock the account (user action, blocks everything)

Nothing below matters until this is done. The error `Increase limits for faster
requests` is a billing/usage cap on the Cursor account, not something code can
retry its way past.

- Raise the usage limit in Cursor settings, or wait for the quota to reset.
- Confirm with a single call before launching anything:
  `cursor-agent -p "Reply READY" --workspace ~/pbsd --model composer-2.5 --output-format text --force`
- If that returns `ActionRequiredError`, stop and tell the user. Do not start a
  run; it will just re-jam the queue.

## Phase 1 — reset the jammed queue

4,204 files are parked in NEEDS_HUMAN with their retry budget spent, purely
because of rate limits. Put them back.

- In `~/pbsd`, set every `NEEDS_HUMAN` row in `docs/migration/inventory.csv` back
  to `PENDING`.
- Reset `attempts` to 0 for every entry in `docs/migration/deferred.jsonl`, or
  delete the file — its contents are all rate-limit noise, not real difficulty
  classifications.
- Commit as a distinct step so the reset is reviewable.
- Verify: `python3 pbsd.py --status` shows ~4,204 pending and 268 verified.

## Phase 2 — never let a rate limit become a verdict

This is the core fix. In `pbsd.py`:

- **Classify the failure.** `call_agent` already returns the error text. Detect
  `ActionRequiredError`, `Rate limit exceeded`, and `Increase limits` and return
  a distinct sentinel rather than a generic failure.
- **Never mark a file REJECTED, DEFERRED or NEEDS_HUMAN on a rate limit.** Leave
  it PENDING so the next round retries it.
- **Add a circuit breaker.** Track consecutive rate-limit results across workers
  (a shared counter in the parent). After, say, 10 in a row, stop submitting new
  work, sleep several minutes, and resume at lower concurrency. Burning 4,000
  queue entries in an hour against a wall must be impossible.
- **Adaptive concurrency.** Start at 4. Raise it while batches succeed; halve it
  on rate limits, floor 1. The right number is a property of the account's quota
  and cannot be guessed — 80 was far too high.
- The supervisor `~/pbsd_driver.sh` already loops rounds until nothing is
  pending; keep it, but make it back off between rounds rather than sleeping 5s.

## Phase 3 — use a cheap model for the bulk, escalate only on failure

Agent calls are now the scarce resource, so stop spending Opus on easy files.

- Add a two-tier model policy: attempt every batch with a cheap fast model first
  (`composer-2.5`), and retry only the batches it fails with
  `claude-opus-5-thinking-high`.
- `MODEL` is a module constant and `--model` already threads through
  `attempt_batch` and `run_deferred_phase`, so this is a small change: add a
  second constant and one escalation step in `attempt_batch`.
- The gate is model-agnostic and unchanged — a cheap model cannot lower quality
  here, it can only fail the gate and escalate. This is safe *because* the gate
  is sound.

## Phase 4 — grow the free path (highest leverage, no quota cost)

Only 236 of 4,419 files are proven with no agent call. The dominant blocker,
from the last full mechanical run:

| files | blocker |
|---|---|
| 3,008 | oracle would not compile to IR |
| 568 | port would not compile as C++ |
| 191 | proof rejected (gate correctly refused) |
| 126 | no function definitions |
| 126 | proof unvalidated |

The 3,008 are overwhelmingly `sys/` kernel sources, which cannot compile
standalone because the FreeBSD kernel build generates headers a source checkout
does not contain. Worth fixing because every file moved here is a file that
costs no quota and needs no model:

- Generate empty `opt_*.h` stubs on demand (a minimal kernel config genuinely has
  them empty) into the scratch include dir built by `include_root()`.
- Generate the `*_if.h` interface headers from the `.m` files with
  `sys/tools/makeobjops.awk`, which is how the real build produces them.
- Re-run `python3 pbsd.py --mechanical-only --jobs 64` (about 6 minutes) and
  measure. Diagnose by category the way the earlier passes did — compile one
  file, read the actual compiler error, fix the specific cause, re-measure. Do
  not guess at flags.

Every file proven here is removed from the agent queue permanently.

## Phase 5 — run it

- Launch the supervisor detached so it survives the shell:
  `setsid nohup ~/pbsd_driver.sh >/dev/null 2>&1 </dev/null &`
- Watch `~/pbsd_run.log`. The health check is the **failure mix**, not the pass
  rate: if `ActionRequiredError` reappears, the circuit breaker is not working —
  stop and fix it rather than letting it run.
- Progress is committed per batch, so a crash or an OOM kill resumes rather than
  repeats.

## Phase 6 — verify and publish

- Confirm no port is a stub. Grep the generated `port.cppm` files for `TODO`,
  `FIXME`, `not implemented`, `placeholder`, `stub` — and check each hit against
  the original C, because FreeBSD sources contain those words themselves. All
  four hits in the last audit were false positives (two `TODO`s preserved
  verbatim from the original, two uses of "stub" in prose about libc syscall
  stubs).
- Confirm every VERIFIED file has an evidence file in
  `docs/migration/artifacts/` recording how it was proven.
- Fast-forward the Windows repo from the WSL one, then push from Windows — the
  Windows side holds the SSH agent, and the WSL key is passphrase-protected and
  will hang a non-interactive push.

---

## Traps worth knowing

- Run everything from `~/pbsd` in WSL. Never work in the Windows checkout.
- `cursor-agent` lives at `~/.local/bin/cursor-agent` and is **not** on the
  non-interactive PATH. Export it in every script.
- Scripts copied from Windows arrive with CRLF endings and bash will fail on the
  `\r`. Strip with `sed -i 's/\r$//'`.
- The mechanical path needs `errno.h`, `math.h` and `fenv.h`, which the FreeBSD
  build generates and a checkout does not have; `include_root()` symlinks them.
- `MECH_SITE_LIMIT` deliberately refuses to certify a file with more than 400
  mutation sites rather than probe it partially. Partial probing is what let the
  one surviving bug through.
