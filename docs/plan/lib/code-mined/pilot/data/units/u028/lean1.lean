import Std.Tactic.BVDecide

def model (mmcr1 : BitVec 64) (pmc : BitVec 32) : BitVec 64 :=
  mmcr1 &&& 0x200000000000#64

theorem contract (mmcr1 : BitVec 64) (pmc : BitVec 32) :
    (model mmcr1 pmc).toNat = (mmcr1 &&& 0x200000000000#64).toNat := by
  simp only [model]
