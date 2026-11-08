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

- Model: Claude Sonnet (independent of the GLM-5.3 Flash generator).
- Subset: `state-machine` and `driver-glue` records that passed steps 1 and 2.
- Ask for a verdict plus a short reason (`agree` or `disagree: <reason>`), not a regenerated contract.
- Use a fixed instruction prefix so prompt caching applies.
- Pilot first: a few hundred records to measure real token usage before the full pass.

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

Assumes about 2,820 input and 720 output tokens per record, about 58k records in the subset (roughly 20% of the corpus; the real figure depends on the step 1-2 filter). Standard rates, no discounts.

| Model | Rate (in/out per 1M tokens) | Subset (~58k) | Full corpus (~288k) |
|---|---|---|---|
| Sonnet 5.5 | $2 / $10 | ~$740 | ~$3,700 |

Reasoning tokens bill as output and will push the totals up. Asking for a verdict and short reason instead of a regenerated contract, and caching the instruction prefix, should bring the real cost down. Check whether Sonnet has a batch discount before running.

## 5. Schedule

Planned for Wednesday. Order: steps 1-2 first, bucket failures by `kind`, then step 3, then the Sonnet pilot, then the full step 4 pass, with step 6 tagging written as each pass runs.
