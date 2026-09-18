# What the words mean

The harness refuses to treat silence as a clean bill of health.
These words are the reason.

## Three coverage numbers, and they multiply

| Number | Question | This run |
|---|---|---|
| **Class** | Of the *kinds* of defect we enumerated, how many can some instrument here look for? | 45 / 52 COVERED (0.87) on the tools that were actually installed |
| **Code** | Of the *functions that exist*, how many did some instrument return a verdict for? | 284073 / 335855 TOUCHED (0.846) |
| **Resolution** | Of the verdicts that came back, how many are settled — a proof, or a failure whose bucket explains it, or a failure somebody read into `UB_FINDINGS.md`? | 6237 functions PROVED. 423 READ-THESE failures still unread. |

Quoting class coverage alone is the easiest way to lie with this file.
An instrument set that can see every class, run over none of the tree, has found nothing.

## Row strength (one function, best cell)

From [`tools/verify/matrix.py`](../../../tools/verify/matrix.py):

| Strength | Means |
|---|---|
| **PROVED** | A model checker discharged the *checked properties* for every input inside the unwind bound. Not “the function is correct”. Not “it computes the right answer”. |
| **CHECKED** | An engine decided something: `BOUNDED` (holds up to K iterations) or `FAILED` (a counterexample exists). |
| **SCANNED** | Only a translation-unit tool looked. Silence here is not evidence. |
| **UNTOUCHED** | Nobody has a verdict. Split into *never attempted* (needs a run) and *attempted, no answer* (needs a build fix or a bound raised). |

`ERROR`, `TIMEOUT`, `NOFUNC`, `TU-ERROR`, `NOTRUN` are not clean.

## CBMC `FAILED` is not a bug

[`tools/verify/report.py`](../../../tools/verify/report.py) buckets failures before a person reads them:

| Bucket | This run | What to do |
|---|---:|---|
| pointer/memory (missing precondition) | 1481 | Not a bug. The signature was checked with a nondeterministic pointer. |
| static (callers constrain the domain) | 380 | Deferred, not dismissed. |
| float div-by-zero | 88 | IEEE-754 defines it; `lib/msun` depends on it. |
| per-CPU read CBMC cannot model | 132 | The model's null, not the code's. |
| extern-driven | 18 | Unmodelled return, unconstrained. |
| CBMC could not model it | 4 | Tool limit. |
| **EXPORTED, arithmetic — READ THESE** | **337** (25 already in `UB_FINDINGS.md`) | The queue. |
| index out of range — READ THESE | 36 (1 already read) | The queue. |
| static, address taken — READ THESE | 62 | The queue: callers are not all in-file. |

What survives is exported, arithmetic, parameter-driven.
Both confirmed findings already in `docs/security/UB_FINDINGS.md` came out of that bucket.

## Clang / cppcheck / tidy / warnings findings are not bugs either

They are reports. The analyser is interprocedural *within* a translation unit and not *across* one.
A finding at the use is often a fact about a callee in another file that it could not see.
`docs/security/UB_FINDINGS.md` is the list of defects that survived reading, UBSan, and a re-check.
Nothing from *this* run has been through that yet.

## Class verdicts

| Verdict | Means |
|---|---|
| COVERED | Some instrument PROVES or FINDS the class. |
| PARTIAL | The best available is SOME (a named subset only). |
| GAP | Nothing here sees it. |

Out of scope is named, not deleted from the denominator: data races, TOCTOU, wrong answers, missing specs, crypto misuse, side channels, build-config-only defects.
Nothing in this sweep checked those.
