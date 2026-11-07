import Mathlib
def RSub (a b : Nat) : Prop := a &&& b = a
theorem rights_subset_trans : ∀ a b c : Nat, RSub a b → RSub b c → RSub a c := by
  sorry
