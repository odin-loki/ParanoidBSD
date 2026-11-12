import Std.Tactic.BVDecide

def model (__A : BitVec 32) : BitVec 32 := __A

theorem contract : ∀ (__A : BitVec 32), model __A == __A := by
  intro __A
  simp only [model]
  bv_decide

#print axioms contract
