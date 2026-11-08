# Code-Mined Shards: Verification Plan

**Author:** Odin Loch
**Branch:** `claude/dreamy-mahavira-7e0e79`
**Scope:** `docs/plan/lib/code-mined/shards/` (72 shards: `tierB-0000..0008`, `tierC-0000..0062`, about 4,000 records each, about 288k records total)

## 1. Background

The shards are LLM-generated annotations of C source slices from the Linux and FreeBSD trees. Each record is keyed by `file:start-end` and holds, per function: a summary, an ACSL-style `contract`, a `prose_contract`, loop shapes with informal invariants, and sync/ownership/error-convention/model hints. Records covering register or mask headers hold idiom bullets instead of functions, which is expected.

The shards were generated with GLM-5.3 Flash. Nothing in them is machine-checked.

### Sample findings (900 random records, 6 random shards)

- Per record, about 4 functions, 2,820 prompt tokens and 720 completion tokens on average.
- 3,490 functions: 96% have a prose contract, 95% of loops have an invariant, but only 11% have a formal ACSL contract.
- About half of the no-function records are driver files, and 237 of 310 are headers. These are not defects.
- Defects seen in spot checks:
  - Formal `ensures` disagrees with the prose contract (`omap2xxx_cm_wait_module_ready`: prose returns `-EINVAL`, formal contract does not).
  - Formal contract weaker than the prose (`handle_pg_range`).
  - Suspect `ensures result == 0` (`tmr_run`), possibly on a void function.
  - Partial postconditions (`mmcspi_shift_copy` only pins output when `shift == 0`).

Estimated quality: about 6.5/10 as annotation; not usable as proof evidence until verified.

## 2. Goal

Cheaply separate trustworthy records from untrustworthy ones, so that only verified records are used for training, papers or downstream proof work.

## 3. Steps

Run in order from cheapest to most expensive. Each step only processes records that have not already failed an earlier step.

### Step 1: Mechanical checks (all records, free)

Using the `file:start-end` key, extract the source slice and check with a regex or tree-sitter pass:

- Contract uses `result` or `\result` but the function returns `void`.
- Contract names parameters that are not in the signature.
- Prose lists error codes (for example `-EINVAL`) that the formal `ensures` omits.
- Function name does not appear in the slice.
- Formal contract has no `requires` or no `ensures`.

Requires the original source trees (`hbsd/src` and `pbsd-ref/linux`) on the local machine. The repo only holds the shards.

### Step 2: ACSL syntax check (records with formal contracts)

Parse each contract with Frama-C's annotation parser or a stub grammar. A contract that does not parse is wrong.

### Step 3: Runtime assertion fuzzing (pure helpers)

Target kinds: `arith-helper`, `bitmap`, `string-op`, `table-lookup`, and other functions that compile standalone.

- Compile standalone, convert `requires` and `ensures` into `assert`s.
- Fuzz for a few minutes per function (libFuzzer or AFL).
- Any counterexample means the contract is wrong.

### Step 4: Cross-check with a second model (subset)

- Primary checker: DeepSeek V4.1 Flash on Morph LLM (`morph-dsv41flash`). Different model family from the GLM-5.3 Flash generator, so the check stays independent.
- Tiebreaker: Claude Sonnet, run only on records where the primary checker answers `DISAGREE`.
- Subset: `state-machine` and `driver-glue` records that passed steps 1 and 2.
- Ask for plain-text verdicts, one per function: `AGREE` or `DISAGREE: <short reason>`. Do not rely on structured output or ask for a regenerated contract.
- Treat the primary checker's output as a flag, not a verdict: a smaller model will raise some false disagreements, which the Sonnet tiebreak resolves.
- Use a fixed instruction prefix so prompt caching applies.
- Pilot first: a few hundred records to measure real token usage and the disagreement rate before the full pass.

### Step 6: Tag every record

Write the result of each step onto the record. Keep the original shards untouched and write tagged copies alongside them.

```json
"verified": {
  "syntax": "pass|fail|skip",
  "fuzz": "pass|fail|skip",
  "xcheck": "agree|disagree|skip"
}
```

An overall status is derived from these fields. Only records with no `fail` or `disagree` and at least one `pass` or `agree` count as verified.

### Not planned: stratified manual audit

A hand audit of about 150-200 functions split across `kind` would give a per-kind error rate. It is left out for now and can be added after the tagged results are in.

## 4. Cost estimate (Step 4 only)

Assumes about 2,820 input and 720 output tokens per record, about 58k records in the subset (roughly 20% of the corpus; the real figure depends on the step 1-2 filter). Standard rates, no caching.

| Model | Rate (in/out per 1M tokens) | Subset (~58k) | Full corpus (~288k) |
|---|---|---|---|
| DeepSeek V4.1 Flash on Morph (primary) | $0.12 / $0.80 | ~$53 | ~$263 |
| Sonnet 5.5 (tiebreak, disagreements only) | $2 / $10 | depends on disagreement rate | depends on disagreement rate |

Morph's V4.1 Flash rate was $0.15 / $0.60 at launch (2026-09-10) and listed at $0.12 / $0.80 as of 2026-10-17; confirm the live rate before running. Reasoning tokens bill as output and will push totals up. If about 15% of subset records are escalated, Sonnet adds roughly $110 on the subset, for a total near $160. Check whether Sonnet has a batch discount before running.

## 5. Schedule

Planned for Wednesday. Order: steps 1-2 first, bucket failures by `kind`, then step 3, then the V4.1 Flash pilot, then the full step 4 pass with Sonnet tiebreak on disagreements, with step 6 tagging written as each pass runs.
