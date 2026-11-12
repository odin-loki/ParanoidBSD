import Std.Tactic.BVDecide

/-!
Model of `ad5446_init` (file ad5446.c).

The body performs two external calls (`ad5446_spi_register_driver`,
`ad5446_i2c_register_driver`) whose results are not integer computations
of this function.  They are modelled as the extra parameters `spi_ret`
and `i2c_ret` of `model`, and the kernel convention for such registration
routines (return `0` on success, a negative error code otherwise) is
recorded as the hypotheses `hspi` / `hi2c`, mirroring the callee
contracts that the ACSL analysis of this function relies on.
-/

/-- `int ret; ret = ad5446_spi_register_driver(); if (ret) return ret;
     ret = ad5446_i2c_register_driver(); if (ret) { ...unregister...; return ret; }
     return 0;` -/
def model (spi_ret i2c_ret : BitVec 32) : BitVec 32 :=
  if spi_ret ≠ 0 then spi_ret
  else if i2c_ret ≠ 0 then i2c_ret
  else 0

/-- `requires true; ensures (\result == 0 || \result < 0);` -/
theorem contract (spi_ret i2c_ret : BitVec 32)
    (hspi : spi_ret = 0 ∨ spi_ret.toInt < 0)
    (hi2c : i2c_ret = 0 ∨ i2c_ret.toInt < 0) :
    model spi_ret i2c_ret = 0 ∨ (model spi_ret i2c_ret).toInt < 0 := by
  unfold model
  split
  · exact hspi
  · split
    · exact hi2c
    · exact Or.inl rfl
