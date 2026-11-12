# Sellable-level verification plan (customer: US Department of Defense and its contractors)

Written 12 Nov 2026. Supersedes the "what is verified" wording of VERIFICATION_PLAN.md; steps 1-4 there are done (see VERIFICATION_REPORT.md).

## 1. The rule: what we may call "proved"

A buyer will check claims, so every function gets exactly one of these labels, and only the first may be called proved.

| Label | Meaning | May be sold as |
|---|---|---|
| **PROVED** | Machine-checked: Lean 4 proof or Frama-C/WP proof accepted by the checker, no `sorry`, no `axiom`/`admit`, no unstated assumption | proved |
| **PROVED-ASSUMING** | As PROVED, but assumes listed preconditions on pointers/memory (PRISM T2 policy) | proved, with the assumption listed |
| **BOUNDED** | Bounded model check or exhaustive test within stated bounds (PRISM T3) | bounded-verified, bound stated |
| **TESTED** | Passed random/edge-value runtime checks (step 3) | tested |
| **REVIEWED** | Passed mechanical checks and an independent second-model review (steps 1, 2, 4) | reviewed, NOT proved |
| **REJECTED / UNCHECKED** | Anything else | not offered |

Never merged in any summary. No count of "verified" without its label.

## 2. Current state (12 Nov 2026, committed)

- 1,135,593 mined functions. Tags after step 4: verified 415,923 (this is REVIEWED at best), rejected 173,356, unchecked 546,314.
- 74,181 functions repaired (41,068 records); re-verification steps 1-3 in progress; Frama-C: 24,646 of 25,544 repaired contracts parse and type.
- 0 functions are PROVED yet by the label above (the earlier PRISM/Lean work is on the tester's tasks, not on these mined annotations).

## 3. Stages

Each stage ends with a committed data snapshot and an updated count table. Resume-safe, runs in its own window, no results only in /tmp.

### S1. Finish re-verification of repairs
Steps 1-3 + Frama-C on all 74,181 repaired functions; retry the 484 failed repairs and 897 failed step-4 calls. Exit: every repair has a tag fixed / still-rejected / dropped.

### S2. Repair loop until the rejected count stops falling
Repeat: DeepSeek review of repaired records (step 4 prompt) -> GLM repair of the disagreements and mechanical failures (reasoning effort raised on later rounds) -> re-verify. Stop when a round fixes under 2% of what is left. Whatever remains is dropped and listed, not hidden.

### S3. Choose what to prove (scope for sale)
Rank functions by value to the product: state machines, parsers, allocators, refcount, ring buffers, bitmaps, lock helpers, arith/bit helpers, error paths. Proved-tier target list is built from REVIEWED functions of these kinds, pure/scalar first (cheapest to prove, strongest claim). Size the list to the credit available, quoted per tier before spending.

### S4. Prove
Per function, PRISM T2 policy: scalar -> single-line ACSL, `PROVED`; pointer -> `\valid(p + (0..N-1))` assumption, `PROVED-ASSUMING`; not provable in 2 tries -> T3 BOUNDED. Lean proofs written by GLM with `lean_check` and the gate. Every proof is replayed by an independent checker run (fresh process, pinned toolchain), scanned for `sorry`/`axiom`/`admit`/`native_decide` abuse, and the statement is compared with the contract so the proof cannot be of something weaker. Failed attempts feed the repair loop (S2).

### S5. Independent audit of the claims (the sale-critical stage)
- Proof-statement audit: sample at least 400 PROVED records stratified by kind; a second model plus a human-readable diff checks that the proved statement equals the stated contract, and that the contract matches the C code (the weak-but-true cases found in the tiebreak: 12 of 60 contracts were weaker than the prose).
- Mutation test: break the C code (flip a comparison, drop an error return) for a sample of PROVED functions; the proof or the harness must fail. Report the kill rate.
- Reproducibility: one script that rebuilds every claim from a clean checkout (toolchain versions pinned, hashes of every input file recorded).
- Known gaps listed: what the proofs do NOT cover (hardware, concurrency, compiler, undefined behaviour not modelled, assumptions in PROVED-ASSUMING).

### S6. Package
Assurance case (claims, evidence, assumptions, gaps), per-function ledger (label, tool, toolchain, hash, assumptions), the count table, rerun instructions, and a short summary a contracting officer can read. Licensing check on mined Linux (GPL-2.0), HBSD/FreeBSD (BSD) and KDE code before anything is sold: legal review item, not a technical one.

## 4. Quality gates (a stage is not finished until these hold)

1. Every number in a report is regenerated from committed data by script, not typed by hand.
2. Zero `sorry`/`axiom`/`admit` in the PROVED tier, checked by script on every proof.
3. Mutation kill rate and audit error rate reported, with the failures listed.
4. Two clean runs of the rebuild script give identical results.
5. Everything committed and pushed at the end of each stage.

## 5. Credit and cost

Spent so far: about $263 of the $295 credit (DeepSeek step 4 $236, GLM repair $28). More credit is coming; each stage gets a price quote before it starts. Rough rates: DeepSeek review about $0.0016-0.0023 per record, GLM repair about $0.0004 per record; proving cost per function is to be measured on a 200-function pilot at the start of S4, then extrapolated.

## 6. Order and dependencies

S1 -> S2 -> S3 (can start once S1 is done) -> S4 -> S5 -> S6. S5's audit design and the rebuild script can be written while S2 and S4 run. Pilot of S4 (200 functions) comes first so the price and the achievable PROVED count are known before the main run.

## 7. Risks

- Contracts true but weaker than the prose: fix by the stricter reviewer rule (weaker = defect) before the S4 target list is final.
- Reviewer bias: DeepSeek and GLM share blind spots; the human-readable audit in S5 is the backstop.
- Proof of a weaker statement than the claim: addressed by the statement comparison in S4 and the audit in S5.
- Credit runs out mid-stage: every runner stops cleanly and resumes; stage exits are by count, not by money.

## 8. Estimate to prove "the whole thing" (12 Nov 2026; extrapolated from two measured tasks, to be replaced by the 200-function pilot)

Measured: IF task about 5-20 min and $0.03-0.05; MODEL task 48 min and $0.77 (345 requests, 26 M input tokens, 71% cache hits). One PRISM prove about 118 s of CPU on the PC. These are tester tasks, not mined functions, so the per-function price is a range, not a measurement.

| Scope | Functions | Cost at $0.05 (simple) | at $0.20 (middle) | at $0.77 (heavy) |
|---|---|---|---|---|
| Core kinds (arith, bit, bitmap, allocator, refcount, ring-buffer, list, lock, parser, error-path, init-teardown, other) | about 32,000 | $1,600 | $6,400 | $25,000 |
| All reviewed + repaired (415,923 + 67,865) | about 484,000 | $24,000 | $97,000 | $373,000 |

Time: the PC runs one PRISM prove at a time at about 118 s, so 32,000 functions is about 44 days on the PC alone, and 484,000 is about 18 months. With the GCP burst workers (about 100 in parallel, billing not yet enabled) 32,000 is about 10 days and 484,000 about 6 months. Model calls alone (100 parallel, 10 min average) would be 600 functions an hour.

Not every function can be PROVED: driver glue (322,000 of the reviewed set) depends on hardware and concurrency that the proofs cannot model, so most of it ends BOUNDED or TESTED. A realistic sellable proved tier is the core kinds (about 32,000) plus whatever of the rest passes the T2 policy; the pilot sets the proved share.

Decision rule: run the 200-function pilot (about $10-150), then quote the core tier with its measured price before spending.

## 9. S4 pilot result (12 Nov 2026, measured; replaces the extrapolation in section 8 for the proving cost)

Full report: [pilot/PILOT_REPORT.md](pilot/PILOT_REPORT.md); data and scripts in `pilot/`. 200 core-kind functions (+ 60 loop-bearing): route Frama-C WP, 29 of 200 proved (15 PROVED, 14 PROVED-ASSUMING; all replayed clean), 149 did not build because the mined code needs real kernel headers; GLM proved nothing (0 of 24 loop/assert attempts; 6 Lean theorems, none acceptable as PROVED). Model spend USD 0.55 of the USD 40 cap; about 10 s of CPU and USD 0 of model per proved function. The cost per function in section 8 ($0.05-$0.77) does not apply: the proving cost is compute, not tokens. Open items before the main run are in the report, section 7 (headers per translation unit first).
