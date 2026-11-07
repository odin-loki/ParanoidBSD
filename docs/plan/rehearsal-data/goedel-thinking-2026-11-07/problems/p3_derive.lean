import Mathlib
def RSub (a b : Nat) : Prop := a &&& b = a
def derive (parent want : Nat) : Nat := parent &&& want
theorem derive_never_widens : ∀ p w : Nat, RSub (derive p w) p := by
  sorry
