import Mathlib
def RSub (a b : Nat) : Prop := a &&& b = a
theorem rights_subset_refl : ∀ a : Nat, RSub a a := by
  sorry
