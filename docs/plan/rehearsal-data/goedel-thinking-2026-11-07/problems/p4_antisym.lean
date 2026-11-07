import Mathlib
def RSub (a b : Nat) : Prop := a &&& b = a
theorem rights_subset_antisymm : ∀ a b : Nat, RSub a b → RSub b a → a = b := by
  sorry
