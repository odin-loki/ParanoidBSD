import Std.Tactic.BVDecide

def model (v : BitVec 32) : BitVec 32 :=
  (v &&& 0xffff#32) <<< 2

theorem contract (v : BitVec 32) :
    model v == ((v &&& 0xffff#32) <<< 2) := by
  simp only [model]
  bv_decide

#print axioms contract
