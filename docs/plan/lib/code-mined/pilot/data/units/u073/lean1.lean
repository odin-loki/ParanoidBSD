import Std.Tactic.BVDecide

def model (attr_len expected_len : BitVec 32) : BitVec 32 :=
  if expected_len = attr_len ∨ expected_len = 4294967295#32 ∨ expected_len = 4294967294#32 then
    1#32
  else
    0#32

theorem contract (attr_len expected_len : BitVec 32) :
    model attr_len expected_len = 1 ↔
      (expected_len = attr_len ∨ expected_len.toNat = 4294967295 ∨ expected_len.toNat = 4294967294) := by
  unfold model
  by_cases h : expected_len = attr_len ∨ expected_len = 4294967295#32 ∨ expected_len = 4294967294#32
  · rw [if_pos h]
    constructor
    · intro _; exact h
    · intro _; rfl
  · rw [if_neg h]
    constructor
    · intro hn; exact absurd hn (by decide)
    · intro hh
      rcases hh with hh | hh | hh
      · exact absurd (Or.inl hh) h
      · exact absurd (Or.inr (Or.inl (by omega))) h
      · exact absurd (Or.inr (Or.inr (by omega))) h
