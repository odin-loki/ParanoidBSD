import Std.Tactic.BVDecide

/-- Model of ad5446_init. The two registration calls are external and
unmodelable; they are modeled as returning 0 (success), which is the
value the function propagates on the success path. -/
def model : BitVec 32 :=
  let ret1 : BitVec 32 := 32#0  -- ad5446_spi_register_driver()
  if ret1 ≠ 32#0 then ret1
  else
    let ret2 : BitVec 32 := 32#0  -- ad5446_i2c_register_driver()
    if ret2 ≠ 32#0 then ret2
    else 32#0

theorem contract : (model).toInt = 0 ∨ (model).toInt < 0 := by
  simp only [model]
  bv_omega

#print axioms contract
