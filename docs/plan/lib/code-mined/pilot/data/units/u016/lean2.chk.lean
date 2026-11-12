import Std.Tactic.BVDecide

/-- Model of the external function `rng_below(n)`: it returns an unsigned
value supplied by the caller; its contract `0 <= \result < n` is captured as
hypotheses in the theorem below.  `random_base` itself takes no parameters,
so `model` takes the three values that `rng_below` returns during one
execution (for arguments 8, 40 and 37 respectively). -/
def model (r8 r40 r37 : BitVec 32) : BitVec 32 :=
  if r8 = 0#32 then 0#32
  else if r8 = 1#32 then 10#32
  else if r8 = 2#32 then 16#32
  else if r8 = 3#32 then 8#32
  else if r8 = 4#32 then 2#32
  else if r8 = 5#32 then 36#32
  else if r8 = 6#32 then r40 - 2#32
  else r37#32

/-- Contract: `requires true; ensures -2 <= \result <= 37`.
The hypotheses state the contract of `rng_below(n)`: `0 <= \result < n`
(for the three call sites with n = 8, 40, 37). -/
theorem contract (r8 r40 r37 : BitVec 32)
    (h8 : r8.toNat < 8) (h40 : r40.toNat < 40) (h37 : r37.toNat < 37) :
    -2 ≤ (model r8 r40 r37).toInt ∧ (model r8 r40 r37).toInt ≤ 37 := by
  unfold model
  repeat split
  all_goals bv_omega

#print axioms contract
