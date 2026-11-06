#!/usr/bin/env bash
# Lean 4.9.0 + Mathlib v4.9.0 for `prism prove` (T1 model lemmas). Run as your normal user, after elan is installed.
# Why this version: the Goedel-Prover-V2 model card says the models were built and evaluated with Lean 4 version 4.9 and the
# corresponding Mathlib. Without Mathlib, tactics such as ring, norm_num, linarith and nlinarith are "unknown tactic" and the
# model's proofs are rejected (measured 6 Nov 2026: the same lemma failed for 10 minutes without Mathlib, 127 s with it).
# Needs PRISM with the "link .lake/packages" fix (odin-loki/PRISM prove-link-lake-packages): its scratch copy otherwise
# has no Mathlib and the pre-search `lake build` times out.
#   tools/cloud/lean-mathlib.sh [dir]        default dir: ~/lean49 (about 4.5 GB, the Mathlib build cache is downloaded)
set -euo pipefail
export PATH="$HOME/.elan/bin:$PATH"
W="${1:-$HOME/lean49}"; mkdir -p "$W"; cd "$W"
echo "leanprover/lean4:v4.9.0" > lean-toolchain
cat > lakefile.lean <<'L'
import Lake
open Lake DSL

package «lean49»

require mathlib from git "https://github.com/leanprover-community/mathlib4" @ "v4.9.0"

@[default_target]
lean_lib T
L
[ -f T.lean ] || printf 'import Mathlib\nopen BigOperators Real Nat Topology Rat\n\ntheorem example_lemma (n : ℕ) : n + 0 = n := by sorry\n' > T.lean
elan toolchain install leanprover/lean4:v4.9.0
lake update
lake exe cache get
lake build T
echo "ready: cd $W && PRISM_DEVICE=cuda PRISM_PROVER_FPK=... prism prove T.lean <theorem> --allow-exec --write"
