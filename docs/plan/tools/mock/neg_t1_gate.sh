#!/usr/bin/env bash
# T1 gate: a mock package with a Lean model against a locked statement, and a TLA+ model for Apalache.
export PATH=$HOME/.elan/bin:$PATH
G="$(cd "$(dirname "$0")/../gates" && pwd)/proof_check.sh"
W=/tmp/t1; rm -rf $W; mkdir -p $W/proofs/P0-02/spec $W/proofs/P0-02/model; cd $W
git init -q; git config user.email m@m; git config user.name m
M=proofs/P0-02/model
cat ~/PRISM/proofs/techniques/lean-toolchain > $M/lean-toolchain
cat > proofs/P0-02/spec/Spec.lean <<'L'
namespace Spec
def add_zero_eq : Prop := ∀ n : Nat, n + 0 = n
end Spec
L
cat > $M/lakefile.toml <<'L'
name = "model"
[[lean_lib]]
name = "Spec"
srcDir = "../spec"
[[lean_lib]]
name = "Model"
L
cat > $M/Model.lean <<'L'
import Spec
namespace Model
theorem add_zero_eq : Spec.add_zero_eq := by
  intro n; simp
end Model
L
H=$(sha256sum proofs/P0-02/spec/Spec.lean | cut -d' ' -f1)
printf '{"proofs/P0-02/spec/Spec.lean":"%s","theorems":{"P0-02":["add_zero_eq"]}}\n' "$H" > proofs/spec-lock.json
git add -A; git commit -q -m base
run() { PBSD_PROOF_TIER=T1 bash $G P0-02-MODEL $W > /tmp/t1.out 2>&1; rc=$?; printf '%-48s rc=%s  %s\n' "$1" $rc "$(grep -E '^(FAIL|infrastructure|PROVED|PBSD-RESULT)' /tmp/t1.out | head -1 | cut -c1-110)"; }
run "Lean model, genuine proof"
grep -v "^$" /tmp/t1.out | head -12 | cut -c1-200
cp $M/Model.lean /tmp/Model.good
printf 'import Spec\ntheorem add_zero_eq : Spec.add_zero_eq := by\n  sorry\n' > $M/Model.lean; run "Lean model with sorry"
printf 'import Spec\naxiom cheat : Spec.add_zero_eq\ntheorem add_zero_eq : Spec.add_zero_eq := cheat\n' > $M/Model.lean; run "Lean model with an axiom"
printf 'import Spec\ntheorem add_zero_eq : Spec.add_zero_eq := by\n  intro n; native_decide\n' > $M/Model.lean; run "Lean model with native_decide"
printf 'import Spec\ntheorem add_zero_eq : True := trivial\n' > $M/Model.lean; run "Lean model proving a different statement"
cp /tmp/Model.good $M/Model.lean
echo 'def add_zero_eq : Prop := True' > proofs/P0-02/spec/Spec.lean; run "locked statement file edited"
git checkout -q -- proofs/P0-02/spec/Spec.lean
tla() { cat > $M/Counter.tla <<T
---- MODULE Counter ----
EXTENDS Integers
VARIABLE
  \* @type: Int;
  x
Init == x = 0
Next == x' = x + 1
Safety == $1
IndInv == $2
IndInit == x \in Int /\ IndInv
====
T
}
tla "x >= 0" "x >= 0"; run "Lean + TLA+ (inductive invariant)"
tla "x >= 3" "x >= -5"; run "TLA+: IndInv does not imply Safety"
tla "x >= 0" "x <= 10"; run "TLA+: IndInv not inductive"
