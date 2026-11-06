# Plan: fine-tune a Lean prover for PBSD tier T1, in C, C++ and CUDA

Written 6 Nov 2026. **Measured** = run on this PC. **Sourced** = from a page or file named in the text. **Assumption** and **unknown** are open, each with a step that settles it.

**Language rule (Odin, 6 Nov 2026).** Everything we write for this is C, C++ or CUDA. No Python: no PyTorch, `transformers`, PEFT, TRL, unsloth or `datasets`. Shell is allowed as glue only. Lean itself, PRISM and FORGE are existing C++/Lean tools and stay as they are.

## 1. Where we stand, and two findings that come first

**Measured.** `prism prove` with Goedel-Prover-V2-8B (Q4_K `.fpk`) closed `n + 0 = n`, `n * 1 = n`, `a + b = b + a` and a primed-name variant in about 90 s each, and failed `2 * n = n + n` in 10 minutes. Splitting that lemma into three smaller ones for the 8B proved only the trivial one.

1. **The test project had no Mathlib** (measured): `ring`, `norm_num`, `linarith`, `nlinarith` are `unknown tactic`; only `omega` and `simp` work. Goedel was trained on Mathlib-style proofs, so most "unknown tactic" rejections are very probably the environment, not the model, and every baseline so far is a floor.
2. **PRISM leaves no room to think** (sourced: `grammars/lean_proof.gbnf`, `src/prism/ai/proof_search.cpp`, `include/prism/forge_llm.hpp`): the model must reply `{"tactics": [...]}` under a grammar, default `max_new = 768`. Goedel-V2 and Qwen 3.5 are long-chain-of-thought models.

**Result after fixing the environment (measured, 6 Nov 2026).** Lean 4.9.0 + Mathlib v4.9.0 (the version Goedel's model card says it was built for), `prism prove`, GPU, a budget of 6 model calls, one run per lemma, with three PRISM/FORGE fixes (below). **Both Goedel-Prover-V2 models proved all six lemmas**, each checked by the Lean kernel, the axiom audit and `lake build`:

| Lemma | 8B (Q4_K) | 32B (Q4_K) |
|---|---|---|
| `2 * n = n + n` | 127 s | 208 s |
| `2ab ≤ a² + b²` (reals) | 137 s | 160 s |
| `n % 2 = 0 ∨ n % 2 = 1` | 128 s | 155 s |
| `2 · Σ i = n(n+1)` (induction) | 212 s | 269 s |
| `abc ≤ (a³+b³+c³)/3` | 219 s | 210 s |
| `n² mod 4 ∈ {0,1}` | 180 s | 282 s |

The same lemma `2 * n = n + n` was a 10-minute failure for both models in a Mathlib-free project. Times include loading the model on every call (about 100 s), so they say little about proving speed. The 32B was not faster or more successful on these, and was 0.9–1.6x slower. **This is a small, easy set from textbook mathematics, not PBSD lemmas, with one run each; it shows the environment was the blocker, not that the models are strong on the real target.**

**Consequence for the plan.** Phase 0's exit test ("if Goedel-8B with Mathlib already solves most of the micro-set, stop here") is met on this set. Training should wait until the PBSD micro-set (phase 0) shows a gap that Goedel-8B/32B cannot close.

**What this means.** Three cheap, training-free steps may close much of the gap and must be done first: Mathlib (phase 0), a reasoning prelude in PRISM's reply format (C++ change), and the existing escalation to Goedel-32B. Under the language rule, **training is a build project, not a configuration**, because FORGE has no training path yet (section 4). It should start only if phase 0 leaves a gap that is worth that effort.

## 2. Data inventory

[Goedel-LM on Hugging Face](https://huggingface.co/Goedel-LM/datasets), sourced from the dataset cards, 6 Nov 2026.

| Dataset | Size | Contents | Licence | Use |
|---|---|---|---|---|
| [`SFT_dataset_v2`](https://huggingface.co/datasets/Goedel-LM/SFT_dataset_v2) | 1,745,010 examples, 6.11 GB | `messages` (2–4 chat turns): the user gives a Lean 4 file (`import Mathlib`, `import Aesop`, `maxHeartbeats 0`, theorem ending in `sorry`); the assistant answers with the proof | Apache-2.0 | main SFT source. **Construction and Lean/Mathlib version not stated** |
| [`RL_dataset_V2`](https://huggingface.co/datasets/Goedel-LM/RL_dataset_V2/blob/main/README.md) | 98,632 examples, 336 MB download | subsets `lean4_completion` (whole proof) and `lean4_revision` (fix a proof from compiler feedback) | Apache-2.0 | revision subset matches PRISM's retry loop |
| [`Goedel-Pset-v1`](https://huggingface.co/datasets/Goedel-LM/Goedel-Pset-v1) | 1,732,594 rows, 828 MB | formal Lean 4 statements + informal statement; **no proofs** | not stated on the card | statements for expert iteration (phase 4) |
| `Lean-workbook-proofs` | 29.8k | proofs for Lean Workbook problems | **not checked** | small clean set |
| [`MathOlympiadBench`](https://huggingface.co/datasets/Goedel-LM/MathOlympiadBench) | 360 problems | formalised olympiad problems (158 IMO, 131 shortlist, 68 regional, 3 puzzles) | **not checked** | **evaluation only** |
| PBSD's own | 12 verified-core packages | locked statements in `proofs/<WP>/spec/`, P0-02 and P1-02 first | ours | the real target; domain eval |

The paper ([arXiv 2508.03613](https://arxiv.org/abs/2508.03613)) says models, code and data are released, but the [repo README](https://github.com/Goedel-LM/Goedel-Prover-V2) documents inference only. **Assume there is no usable training code.**

**Domain gap.** These sets are competition and Lean-Workbook mathematics. PBSD's T1 lemmas are data-structure and state-machine invariants. Expect the public data to give tactic fluency, not domain skill; the domain comes from proofs our own pipeline accepts (phase 6).

## 3. The base model

| | Qwen 3.5 9B | Goedel-Prover-V2-8B |
|---|---|---|
| Source | [model card](https://huggingface.co/Qwen/Qwen3.5-9B) | local `model.safetensors` + `config.json` (measured) |
| Architecture | 32 layers, hidden 4096, **hybrid**: 3 Gated DeltaNet (linear attention) layers per 1 Gated Attention layer; 262,144 context; multimodal; Apache-2.0 | Qwen3, **dense** standard attention, 36 layers, hidden 4096 |
| Already a Lean prover | no | yes (fine-tuned from Qwen3-8B; 83.0% MiniF2F pass@32 per its card) |
| Quantised file here | `qwen3.5-9b-gguf.fpk`, 5.73 GB | `goedel-8b-q4_k.fpk`, 4.93 GB |
| Thinking | on by default; no `/think` switch | long chain of thought |

The base weights for the first are `Qwen/Qwen3.5-9B-Base` (model card). The choice between them is a build-cost question (section 4), not only a quality question.

**This machine (measured).** RTX 3090 24 GB, 64 threads, 47 GB RAM visible to WSL, 619 GB free on the WSL disk, 4.1 TB free on D:. The 3090 is also the GPU for `prism prove` and the PBSD run, so training and the weekend run cannot overlap.

## 4. The stack, and the gap in FORGE

| Job | Tool (language) |
|---|---|
| Download | `curl` (C) from the Hugging Face file URLs |
| Parquet to JSONL | DuckDB command line (C++): `COPY (SELECT ...) TO 'x.jsonl'`. **Not installed; not tried on these files** |
| Tokenise, filter, de-duplicate, remove eval overlap, pack, convert formats | new C++ tool `pbsd_data`, using FORGE's tokenizer |
| Check proofs against Lean | C++ job runner (the runner's `run_job` pattern) driving `lake env lean` |
| Train | FORGE (C++/CUDA), new `forge train` |
| Evaluate | `prism prove` plus a small C++ driver |
| Deploy | FORGE LoRA adapter file, loaded at run time |
| Expert-iteration loop | C++, extending the runner |

**What FORGE already has** (sourced: `graph/include/forge/graph/autodiff.hpp`, `docs/graph.md`, `docs/gpu.md`, `docs/STATUS.md`, `docs/cli.md`):
- Reverse-mode autodiff, `grad(g, loss, wrt)`, that appends the backward graph to the same IR, so it runs on CPU or GPU. Supported ops: add sub mul div neg scale square sqrt rsqrt recip exp log tanh sigmoid silu relu abs, matmul, matmul_t, transpose, reshape, reduce_sum, reduce_mean, logsumexp, softmax, rms_norm. Anything else raises `unsupported`.
- A CUDA runtime with kernels compiled to PTX (no CUDA SDK needed), tested on this RTX 3090; FlashAttention-2 and quantised tensor-core matmul **forward** kernels.
- `forge import` for `qwen3` and `qwen3_5` blueprints, and `Gated DeltaNet` **inference** (`delta_rule`, `causal_conv` state ops).
- LoRA adapters at inference: `add_adapter` loads `<weight>.lora_a` / `.lora_b` (PEFT layout, `W' = W + (alpha/r)·B·A`) from a `.fpk`. **A trained adapter can therefore be deployed with no merge or conversion step.**

**What is missing for training** (checked by absence from the supported-op list and the source):
1. Backward for attention (FlashAttention-2 backward), RoPE, and embedding gather.
2. Input-gradient for a **frozen 4-bit base** matmul (only activation gradients are needed, but no such kernel exists).
3. Backward for `delta_rule` and `causal_conv` (Qwen 3.5 only). This is the hardest piece: the forward keeps the DeltaNet state in registers.
4. AdamW update, LoRA parameter wiring, gradient checkpointing and activation memory planning for 24 GB, loss masking and sequence packing, a training loop and a checkpoint format.

FORGE's own plan (`FORGE_PLAN.md`) lists autodiff for LoRA as a "later phase", so this is planned work that has not been built.

**Testing without Python.** Gradient checks in C++ by finite differences (FORGE already tests `grad` this way), plus a self-consistency check that the training forward pass gives the same loss as the inference path on a fixed batch. There is no reference-gradient file from another framework.

**llama.cpp as an alternative.** I could not confirm its current training support (my search returned only old guides), so it is **unknown**. Check it in a half-day spike before building anything; if its training path is usable on this GPU with a dense Qwen3 model it would save most of the FORGE work, though not for the Qwen 3.5 hybrid layers.

## 5. Evaluation, defined first

One harness for every model: **`prism prove` itself**, with real Lean and Mathlib and a fixed model-call budget, counting a lemma solved only when PRISM reports PROVED (kernel, axiom audit, `lake build`). It measures what we ship, including the grammar and token limit. Always evaluate the **quantised `.fpk`**.

| Set | Size | Role |
|---|---|---|
| PBSD micro-set | the 8 lemmas so far, growing to about 50 from P0-02 and P1-02 | the real target |
| MathOlympiadBench subset | 50–100 | hard, out of domain |
| MiniF2F test | 244 | standard comparison; **source and licence not checked** |

Report solved at 1, 4 and 8 calls, median seconds per lemma, and the failure split. Baselines: Goedel-8B, stock Qwen 3.5 9B, Goedel-32B.

## 6. Phases

**Phase 0 — no training; environment and cheap wins (2–3 days).**
- Mathlib in a Lake workspace pinned to one Lean and Mathlib version, chosen by compiling about 1,000 sampled `SFT_dataset_v2` examples against candidates. Re-run all baselines.
- PRISM (C++): try a reasoning prelude before the constrained JSON, and a larger `max_new`.
- Run the Goedel-32B escalation properly (my first attempt failed on a project-setup mistake, not the model).
- *Exit:* a baseline table. If Goedel-8B with Mathlib already solves most of the PBSD micro-set, **stop here** and revisit training later.

**Phase 1 — data tools in C++ (3–4 days).**
- Download (about 7.3 GB), convert with DuckDB, then `pbsd_data`: length statistics (**unknown**, sets sequence length and cost), de-duplicate, remove eval overlap, verify a sample against the pinned Lean, keep what still compiles.
- Three formats: whole proof as `{"tactics": [...]}`; next step (prefix then remaining tactics); revision (error plus failed proof to fix). A second variant with a short reasoning prelude.

**Phase 2 — spikes before committing (about 1 week).**
- *Spike A:* confirm FORGE's `grad()` trains a tiny two-layer transformer (matmul, rms_norm, silu, softmax only) with AdamW on CPU, loss falling.
- *Spike B:* FlashAttention-2 backward on the 3090 against a finite-difference check.
- *Spike C:* llama.cpp training on a dense Qwen3 model (section 4).
- *Exit:* pick the engine, and decide whether the Qwen 3.5 `delta_rule` backward is in scope.

**Phase 3 — build the trainer, dense model first.**
- Order: attention/RoPE/gather backward, quantised-base input-gradient, AdamW and LoRA wiring, checkpointing, loop, adapter writer. Target the dense Qwen3 architecture, i.e. **Goedel-8B + LoRA**, which exercises everything except DeltaNet and doubles as the control.
- *Exit:* gradient checks pass; a short run lowers loss; the adapter loads in FORGE and changes outputs. Measure tokens per second here and write it into this plan. At an **assumed** 1,500 tokens/s, 100M tokens is about 19 hours and 1B about 8 days; the full SFT set is several billion tokens, so **use a filtered subset**.

**Phase 4 — Qwen 3.5 9B (only if phase 3 works and phase 0 left a gap).**
- Add `delta_rule` and `causal_conv` backward. Train, compare with the Goedel-8B LoRA on the same eval, and keep whichever wins per cost.

**Phase 5 — expert iteration with Lean as the judge.**
- Run the model through `prism prove` on `Goedel-Pset-v1` statements (start with a few thousand), keep every PRISM-accepted proof, add to the mix, retrain. Full RL with a Lean reward is deferred: it normally needs more than one GPU.

**Phase 6 — deploy and PBSD domain loop.**
- Small PRISM change (C++): an env var for the adapter path, passed to FORGE's `add_adapter`. Escalation chain unchanged: this model, then Goedel-32B, then the frontier tier.
- Every proof the pipeline accepts is a verified example in the target domain; fine-tune on them as P0-02 and P1-02 run.

## 7. Risks

| Risk | Why it matters | Mitigation |
|---|---|---|
| **No training path in FORGE** | the largest piece of engineering here | spikes first; dense model before hybrid; llama.cpp check; stop at phase 0 if enough |
| DeltaNet backward is hard | Qwen 3.5 only; state held in registers in the forward kernel | do it last; the dense Goedel-8B LoRA is the fallback |
| No reference gradients | no Python means no PyTorch comparison | finite differences and forward-loss self-consistency |
| Memory on 24 GB | 9B frozen 4-bit base plus activations | gradient checkpointing; measure early (**unknown**) |
| Mathlib version mismatch | public proofs may not compile here | phase 0/1 sample compile; filter |
| Interface mismatch | thinking model forced into 768 tokens of JSON | prelude format; larger `max_new` |
| Contamination | eval sets may overlap training data | remove overlap before training |
| Quantisation loss | bf16 may beat the shipped Q4_K | evaluate only the `.fpk` |
| GPU is shared | weekend run needs the 3090 | schedule training after it |
| Licences | Pset card names none; two cards unchecked | read each before redistributing |
| Out-of-domain data | competition maths is not kernel invariants | phase 6 loop |

## 8. Decisions needed

1. **Order:** phase 0 first and decide again from its table (recommended), or go straight to the trainer.
2. **Engine:** build training in FORGE (recommended; it is yours and already has autodiff and CUDA), or use llama.cpp if spike C works.
3. **Base:** Goedel-8B + LoRA first as the control and trainer test, then Qwen 3.5 9B, or Qwen 3.5 9B directly (needs the DeltaNet backward before any result).
4. **Budget:** a cap on engineering time and on GPU days.
