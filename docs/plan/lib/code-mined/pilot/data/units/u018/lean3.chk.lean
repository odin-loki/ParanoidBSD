import Std.Tactic.BVDecide

def model (rate_n_flags : BitVec 32) : BitVec 32 :=
  let type := rate_n_flags &&& ((3#32) <<< 23)
  let ltf_gi := rate_n_flags &&& ((7#32) <<< 20)
  if type = ((0#32) <<< 23) ∨ type = ((1#32) <<< 23) then
    (if ltf_gi = 4#32 then 1#32 else 0#32)
  else 0#32

theorem contract (rate_n_flags : BitVec 32) :
    model rate_n_flags =
      (if ((rate_n_flags &&& ((3#32) <<< 23)) = ((0#32) <<< 23) ∨
           (rate_n_flags &&& ((3#32) <<< 23)) = ((1#32) <<< 23)) ∧
          (rate_n_flags &&& ((7#32) <<< 20)) = 4#32
        then 1#32 else 0#32) := by
  simp only [model]
  bv_decide

#print axioms contract
