import Lake
open Lake DSL

-- Package for proofs/<WP>/model: the Lean model proved against the locked statements in ../spec.
-- Spec is the locked statement library (written by the -IF task, hash-pinned at P0-FREEZE); Model proves it.
-- Lean 4.9.0 + Mathlib v4.9.0 is the environment Goedel-Prover-V2 was built for (tools/cloud/lean-mathlib.sh).
package «model»

require mathlib from git "https://github.com/leanprover-community/mathlib4" @ "v4.9.0"

lean_lib Spec where
  srcDir := "../spec"

@[default_target]
lean_lib Model
