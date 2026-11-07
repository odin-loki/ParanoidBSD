import Mathlib
structure Tok where
  id : Nat
  gen : Nat
def live (cur : Nat) (t : Tok) : Prop := t.gen = cur
theorem bump_invalidates_all : ∀ (cur : Nat) (t : Tok), live cur t → ¬ live (cur + 1) t := by
  sorry
