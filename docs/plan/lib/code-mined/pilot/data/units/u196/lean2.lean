import Std.Tactic.BVDecide

def model (layout : BitVec 32) : BitVec 32 :=
  let offsetFlag : BitVec 32 := layout &&& 0x10000#32
  let farCopies : BitVec 32 := layout.sshiftRight 8 &&& 0xFF#32
  if offsetFlag == 0#32 && (1#32).slt farCopies then 1#32 else 0#32

theorem contract (layout : BitVec 32) :
    (model layout = 1#32) ↔
      ((layout &&& 0x10000#32 = 0#32) ∧
        (1#32).slt (layout.sshiftRight 8 &&& 0xFF#32)) := by
  simp only [model]
  bv_decide
