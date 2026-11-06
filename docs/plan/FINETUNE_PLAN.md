# Plan: fine-tune Qwen 3.5 9B for Lean proofs (PBSD tier T1)

Written 6 Nov 2026. Everything marked **measured** was run on this PC; **sourced** comes from a page named in the text; **assumption** and **unknown** are open and have a step that settles them.

## 1. Goal and the honest starting point

**Goal.** A local prover that `prism prove` can call (`PRISM_PROVER_FPK`) to close the Lean 4 lemmas of the 12 verified-core package models (T1), on the RTX 3090, with no API cost.

**What exists (measured, 6 Nov 2026).** `prism prove` with Goedel-Prover-V2-8B (Q4_K `.fpk`) closed `n + 0 = n`, `n * 1 = n`, `a + b = b + a` and a primed-name variant in about 90 s each, and failed `2 * n = n + n` in 10 minutes. Decomposing that lemma into three smaller ones for the 8B proved only the trivial one.

**Two findings that change the plan before any training starts.**

1. **The test project had no Mathlib** (measured): `ring`, `norm_num`, `linarith` and `nlinarith` are all `unknown tactic`; only `omega` and `simp` work. Goedel was trained on Mathlib-style proofs, so most of its rejections ("unknown tactic") are very probably the environment, not the model. Every baseline so far is therefore a floor. **Step 0 is a Mathlib workspace and a re-run, before anything is judged.**
2. **PRISM's interface leaves no room to think** (sourced from `grammars/lean_proof.gbnf`, `src/prism/ai/proof_search.cpp`, `include/prism/forge_llm.hpp`): the model must answer with `{"tactics": [...]}` under a grammar, with `max_new = 768` tokens by default. Goedel-V2 and Qwen 3.5 are long-chain-of-thought models. So the fine-tune must either teach a short, direct tactic format, or PRISM must allow a reasoning prelude before the constrained JSON. Both are evaluated below.

**Recommendation, stated plainly.** Fine-tuning Qwen 3.5 9B is reasonable, but it should earn its GPU time. Goedel-V2-8B reached 83.0% on MiniF2F at pass@32 (its model card) with expert iteration and reinforcement learning at a scale a single 3090 does not match. The cheapest strong candidate is a **control**: Goedel-8B plus a small LoRA on the PBSD-shaped format and domain. Run it beside the Qwen 3.5 9B pilot, and let the same eval decide.

## 2. Data inventory

All from [Goedel-LM on Hugging Face](https://huggingface.co/Goedel-LM/datasets) (sourced from the dataset cards, 6 Nov 2026).

| Dataset | Size | Contents | Licence | Use here |
|---|---|---|---|---|
| [`SFT_dataset_v2`](https://huggingface.co/datasets/Goedel-LM/SFT_dataset_v2) | 1,745,010 examples, 6.11 GB | `messages` (2–4 chat turns): the user gives a Lean 4 file (`import Mathlib`, `import Aesop`, `maxHeartbeats 0`, theorem ending in `sorry`), the assistant answers with the proof | Apache-2.0 | main SFT source; **construction and Lean/Mathlib version not stated** |
| [`RL_dataset_V2`](https://huggingface.co/datasets/Goedel-LM/RL_dataset_V2/blob/main/README.md) | 98,632 examples, 336 MB download | subsets `lean4_completion` (whole proof) and `lean4_revision` (fix a proof from compiler feedback) | Apache-2.0 | the revision subset matches PRISM's retry loop; completion for the pilot |
| [`Goedel-Pset-v1`](https://huggingface.co/datasets/Goedel-LM/Goedel-Pset-v1) | 1,732,594 rows, 828 MB | formal Lean 4 statements + informal statement; **no proofs** | not stated on the card | statements for expert iteration (section 5, phase 4) |
| `Lean-workbook-proofs` | 29.8k | proofs for Lean Workbook problems | **not checked** | small clean set; check licence and format |
| [`MathOlympiadBench`](https://huggingface.co/datasets/Goedel-LM/MathOlympiadBench) | 360 problems | formalised olympiad problems (158 IMO, 131 shortlist, 68 regional, 3 puzzles) | **not checked** | **evaluation only** |
| PBSD's own | 12 verified-core packages | locked theorem statements (`proofs/<WP>/spec/`), P0-02 and P1-02 first | ours | the real target and the domain eval (section 4) |

The paper is [arXiv 2508.03613](https://arxiv.org/abs/2508.03613); its abstract says models, code and data are released, but the [repo README](https://github.com/Goedel-LM/Goedel-Prover-V2) documents inference only, so **assume the training code is not usable and write our own**.

**Domain gap.** These sets are competition and Lean-Workbook mathematics. PBSD's T1 lemmas are data-structure and state-machine invariants (allocator, page tables, capabilities, rings). Expect the public data to give tactic fluency, not domain skill. The domain comes from the verified proofs our own pipeline accepts (phase 6).

## 3. Base model facts (Qwen 3.5 9B)

[Official model card](https://huggingface.co/Qwen/Qwen3.5-9B) (sourced): 9B parameters (listed as 10B in BF16), 32 layers, hidden size 4096, a **hybrid** of Gated DeltaNet (linear attention) and Gated Attention in a 3:1 pattern, 262,144 context, Apache-2.0, multimodal, multi-token-prediction trained. The base weights are `Qwen/Qwen3.5-9B-Base`. Thinking is on by default and Qwen 3.5 has no `/think` switch (it uses `enable_thinking`). It needs a very recent `transformers`.

**Unknown, and the first thing the pilot must settle:** whether PEFT, TRL and the 4-bit libraries support this hybrid architecture without a slow fallback for the DeltaNet layers (the model card does not say; a linear-attention kernel library such as flash-linear-attention is likely needed). If training Qwen 3.5 9B turns out to be several times slower than a standard transformer, that alone can decide the base-model question.

**This machine (measured).** RTX 3090 24 GB, 64 threads, 47 GB RAM visible to WSL, 619 GB free on the WSL disk, 4.1 TB free on D:. **No PyTorch is installed.** The 3090 is the same GPU `prism prove` and the PBSD run use, so training and the weekend run cannot overlap.

## 4. Evaluation, defined first

One harness, used for every model: **`prism prove` itself**, with the real Lean and Mathlib, a fixed budget, counting a lemma solved only when PRISM reports PROVED (kernel, axiom audit, `lake build`). This measures the thing we ship, including the grammar and the 768-token limit.

| Set | Size | Role |
|---|---|---|
| PBSD micro-set | start with the 8 lemmas used so far, grow to about 50 from P0-02 and P1-02 | the real target |
| MathOlympiadBench subset | 50–100 | hard, out-of-domain |
| MiniF2F test | 244 | standard comparison with published numbers; **licence and source not checked** |

Report per model: solved at 1, 4 and 8 model calls, median wall-clock per lemma, and the failure split (`unknown tactic`, kernel error, no progress, duplicate). Always evaluate the **quantised `.fpk`**, not the bf16 weights, because that is what runs.

Baselines on the same harness: Goedel-8B Q4_K, stock Qwen 3.5 9B, and Goedel-32B (the existing escalation target).

## 5. Phases

**Phase 0 — environment and baseline (1–2 days).**
- Add Mathlib to a Lake workspace pinned to one Lean and Mathlib version. Pick the version by compiling a random sample of about 1,000 `SFT_dataset_v2` examples against the candidates; the card does not say which one the data was checked with.
- Re-run the Goedel-8B baseline with Mathlib, including `2 * n = n + n` and the decomposed variants.
- *Exit:* a baseline table. If Goedel-8B already solves most of the PBSD micro-set with Mathlib, the fine-tune targets the hard remainder, and the case for it shrinks.

**Phase 1 — data engineering (2–3 days).**
- Download the three datasets (about 7.3 GB). Measure the token length distribution (**unknown**; it sets sequence length and cost).
- Drop examples that exceed the sequence budget, de-duplicate, and **remove anything overlapping the eval sets**.
- Verify a sample against the pinned Lean and Mathlib; keep only the proofs that still compile, and record the pass rate.
- Build three training formats:
  1. *Whole proof* in PRISM's reply format: the proof's top-level tactic lines as `{"tactics": [...]}`.
  2. *Next step*: a proof prefix plus the remaining tactics (matches PRISM's best-first search; PRISM supplies the Lean error and goal state).
  3. *Revision*: from `lean4_revision`, error plus failed proof to the fix.
- Keep a **second format with a short reasoning prelude** to test the thinking question empirically.
- Convert to the Qwen 3.5 chat template; strip thinking from history as the card requires.

**Phase 2 — pilot (2–3 days of GPU).**
- Install PyTorch, a recent `transformers`, PEFT, TRL, bitsandbytes and the linear-attention kernels. Time the first 100 steps and **write the measured tokens per second here** before choosing a data size.
- QLoRA (rank 64 to start) on about 50k examples, Qwen 3.5 9B post-trained versus `-Base`, direct-tactic versus short-prelude format. Control: Goedel-8B + LoRA on the same data.
- *Gate:* continue only if the pilot beats its own baseline on the PBSD micro-set and does not regress on MiniF2F.

**Phase 3 — main SFT (days, set by phase 2).**
- Scale the data to the budget. Cost is `tokens / measured tokens-per-second`; at an **assumed** 1,500 tokens/s on QLoRA, 100M tokens is about 19 hours and 1B tokens about 8 days. The full SFT set, if it averages a few thousand tokens per example, would be several billion tokens, so **use a filtered subset, not all 1.75M**.
- Train in stages and run the eval every checkpoint.

**Phase 4 — expert iteration with Lean as the judge (ongoing).**
- Run the model through `prism prove` on `Goedel-Pset-v1` statements (start with a few thousand), keep every PRISM-accepted proof, add them to the training mix, retrain. This is cheap rejection-sampling fine-tuning and needs no RL infrastructure.
- Full reinforcement learning with a Lean reward (as Goedel did) is **deferred**: it normally needs multiple GPUs, and one 3090 is shared with inference.

**Phase 5 — deploy (1 day).**
- Merge the LoRA, convert to a FORGE `.fpk` (Q4_K, as the existing Goedel and Qwen files were), and evaluate the quantised file. Whether FORGE's converter takes a merged fine-tune unchanged is **unknown**: test it with the pilot checkpoint, not at the end.
- Set `PRISM_PROVER_FPK`. Escalation chain stays: this model, then Goedel-32B, then the frontier tier (the runner's `escalate_tier`).

**Phase 6 — PBSD domain loop.**
- Every proof the pipeline accepts is a Lean-verified example in the target domain. Collect them as the P0-02 and P1-02 packages run, and fine-tune on them. This is where the domain gap closes.

## 6. Risks

| Risk | Why it matters | Mitigation |
|---|---|---|
| Hybrid-attention training is slow or unsupported | no 4-bit/PEFT path may exist for Gated DeltaNet yet | settle in the phase 2 timing run; fall back to the dense Goedel-8B (Qwen3, standard attention) |
| Mathlib version mismatch | proofs that compiled for the authors may not compile here | phase 0 sample compile; filter |
| Interface mismatch (no room to think) | a thinking model forced into 768 tokens of JSON underperforms | direct-tactic format; test the prelude variant; raise `max_new` in PRISM if it wins |
| Contamination | eval sets may overlap the training data | de-duplicate against eval sets before training |
| Quantisation loss | the bf16 model may beat the shipped Q4_K | evaluate only the `.fpk` |
| Compute is shared | the weekend PBSD run needs the 3090 | schedule training after it, or on another machine |
| Licence | the Pset card names no licence; two other cards not checked | read each before redistributing any derived weights or data |
| Public data is out of domain | competition maths is not kernel invariants | phase 6 flywheel |

## 7. Decisions needed

1. **Base:** `Qwen3.5-9B-Base` or the post-trained model. Recommendation: post-trained if phase 2 shows it keeps instruction following with LoRA, otherwise Base.
2. **Format:** direct tactics (recommended start) or a short reasoning prelude. The pilot tests both.
3. **Budget:** a cap on GPU days for phases 2 and 3, and whether to run the Goedel-8B control alongside.
