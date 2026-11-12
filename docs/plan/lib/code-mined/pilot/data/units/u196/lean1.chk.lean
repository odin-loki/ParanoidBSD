import Std.Tactic.BVDecide

/-- `__is_raid10_offset(layout)` : `(layout & (1 << 16)) != 0` -/
/-- `__raid10_far_copies(layout)` : `(layout >> 8) & 0xFF` -/
/-- `__is_raid10_far` returns a C `bool`, modeled as `BitVec 32` (1 = true, 0 = false). -/
def model (layout : BitVec 32) : BitVec 32 :=
  let isOffset : Bool := (layout &&& (1#32 <<< 16)) != 0#32
  let farCopies : BitVec 32 := (layout.sshiftRight 8) &&& 0xFF#32
  if !isOffset && farCopies > (1#32) then 1#32 else 0#32

theorem contract (layout : BitVec 32) :
    (model layout == 1#32) ↔
      ((layout &&& (1#32 <<< 16)) == 0#32 ∧
       ((layout.sshiftRight 8) &&& 0xFF#32) > 1#32) := by
  simp only [model]
  bv_decide

#print axioms contract
