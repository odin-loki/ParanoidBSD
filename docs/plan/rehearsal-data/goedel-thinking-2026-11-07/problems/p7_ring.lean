import Mathlib
def occ (head tail : Nat) : Nat := tail - head
def push (tail : Nat) : Nat := tail + 1
theorem push_keeps_bound : ∀ head tail cap : Nat, head ≤ tail → occ head tail < cap → occ head (push tail) ≤ cap := by
  sorry
