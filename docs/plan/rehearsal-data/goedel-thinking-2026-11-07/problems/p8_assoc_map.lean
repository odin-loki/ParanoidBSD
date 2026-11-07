import Mathlib
def narrow (a b : Nat) : Nat := a &&& b
theorem narrow_assoc_comm : ∀ a b c : Nat, narrow (narrow a b) c = narrow (narrow c b) a := by
  sorry
