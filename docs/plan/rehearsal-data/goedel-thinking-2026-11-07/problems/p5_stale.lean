import Mathlib
structure Tok where
  id : Nat
  gen : Nat
def live (cur : Nat) (t : Tok) : Prop := t.gen = cur
theorem stale_fails_closed : ∀ (cur : Nat) (t : Tok), t.gen ≠ cur → ¬ live cur t := by
  sorry
