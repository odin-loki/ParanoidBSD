import Std.Tactic.BVDecide

/-- Model of `random_base`.

The calls to the (external, nondeterministic) function `rng_below` are modeled
by the parameters `r8`, `r40` and `r37`, the results of `rng_below(8)`,
`rng_below(40)` and `rng_below(37)` respectively.  By the contract of
`rng_below(n)` they satisfy `0 <= r < n`, which is recorded in `contract`.

The `switch` is translated as a case analysis on the value of `rng_below(8)`;
`(int)rng_below(40) - 2` is signed 32-bit subtraction (wrapping), and the
`default` branch returns `(int)rng_below(37)`. -/
def model (r8 r40 r37 : BitVec 32) : BitVec 32 :=
  match r8.toNat with
  | 0 => 0#32
  | 1 => 10#32
  | 2 => 16#32
  | 3 => 8#32
  | 4 => 2#32
  | 5 => 36#32
  | 6 => r40 - 2#32
  | _ => r37

/-- Contract of `random_base`:
`requires true; ensures -2 <= \result <= 37`.

The hypotheses `h8`, `h40`, `h37` are the contract of `rng_below(n)`
(`0 <= \result < n`) instantiated at the three call sites. -/
theorem contract (r8 r40 r37 : BitVec 32)
    (h8 : r8.toNat < 8) (h40 : r40.toNat < 40) (h37 : r37.toNat < 37) :
    -2 ≤ (model r8 r40 r37).toInt ∧ (model r8 r40 r37).toInt ≤ 37 := by
  unfold model
  split <;> (first | bv_omega | omega | decide)
