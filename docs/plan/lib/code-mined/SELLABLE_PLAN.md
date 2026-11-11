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
