import Spec
import Mathlib

namespace Model

theorem narrow_subset_parent : Spec.narrow_subset_parent := by
  intro p w
  exact Finset.inter_subset_left

theorem derive_keeps_generation : Spec.derive_keeps_generation := by
  intro t w
  rfl

theorem derive_rights_narrowed : Spec.derive_rights_narrowed := by
  intro t w
  rfl

theorem revoked_use_fails : Spec.revoked_use_fails := by
  intro e t ask h hu
  exact absurd hu.1.2 (by simp [h])

theorem stale_fails_closed : Spec.stale_fails_closed := by
  intro e t ask hst hu
  have he : t.generation = e.generation := hu.1.1
  have hs : t.generation < e.generation := hst
  omega

theorem live_subset_use_ok : Spec.live_subset_use_ok := by
  intro e t ask hl hsub
  exact ⟨⟨hl.left, hl.right⟩, hsub⟩


theorem revoke_bumps_generation : Spec.revoke_bumps_generation := by
  intro e
  exact ⟨rfl, rfl⟩

theorem restart_clears_and_bumps : Spec.restart_clears_and_bumps := by
  intro e
  exact ⟨rfl, rfl⟩

theorem restart_token_stale_fails : Spec.restart_token_stale_fails := by
  intro e
  constructor
  · exact Nat.lt_succ_self (Spec.mkToken e).generation
  · intro hu
    have h : (Spec.mkToken e).generation = (Spec.restart e).generation := hu.1.1
    have h1 : (Spec.mkToken e).generation = e.generation := rfl
    have h2 : (Spec.restart e).generation = e.generation + 1 := rfl
    omega

/-- Helper: no token can be live against an entry twice restarted after
it was minted; the generations book-keeping of `restart` makes the
locked statement `Spec.restart_new_token_live` false. -/
theorem not_live_restarted_twice (e : Spec.Entry) :
    ¬ Spec.tokenLive (Spec.restart (Spec.restart e)) (Spec.mkToken (Spec.restart e)) := by
  intro hl
  have he : (Spec.mkToken (Spec.restart e)).generation
      = (Spec.restart (Spec.restart e)).generation := hl.1
  have h1 : (Spec.mkToken (Spec.restart e)).generation = e.generation + 1 := rfl
  have h2 : (Spec.restart (Spec.restart e)).generation = e.generation + 2 := rfl
  omega

theorem neg_restart_new_token_live : ¬ Spec.restart_new_token_live := by
  intro h
  exact not_live_restarted_twice
      ⟨0, ∅, Spec.CapKind.memory, false⟩ (h ⟨0, ∅, Spec.CapKind.memory, false⟩)

theorem posix_read_iff : Spec.posix_read_iff := by
  intro s
  constructor
  · intro hr
    obtain ⟨b, hbs, hbr⟩ := Finset.mem_biUnion.mp hr
    cases b with
    | rUsr => exact Or.inl hbs
    | rGrp => exact Or.inr (Or.inl hbs)
    | rOth => exact Or.inr (Or.inr hbs)
    | wUsr => exact absurd hbr (by simp [Spec.posixRightOf])
    | wGrp => exact absurd hbr (by simp [Spec.posixRightOf])
    | wOth => exact absurd hbr (by simp [Spec.posixRightOf])
    | xUsr => exact absurd hbr (by simp [Spec.posixRightOf])
    | xGrp => exact absurd hbr (by simp [Spec.posixRightOf])
    | xOth => exact absurd hbr (by simp [Spec.posixRightOf])
  · intro h
    rcases h with h | h | h
    · exact Finset.mem_biUnion.mpr
        ⟨Spec.PosixBit.rUsr, h, by simp [Spec.posixRightOf]⟩
    · exact Finset.mem_biUnion.mpr
        ⟨Spec.PosixBit.rGrp, h, by simp [Spec.posixRightOf]⟩
    · exact Finset.mem_biUnion.mpr
        ⟨Spec.PosixBit.rOth, h, by simp [Spec.posixRightOf]⟩

theorem nt_delete_iff : Spec.nt_delete_iff := by
  intro s
  constructor
  · intro hd
    obtain ⟨b, hbs, hbd⟩ := Finset.mem_biUnion.mp hd
    cases b with
    | genericRead => exact absurd hbd (by simp [Spec.ntRightOf])
    | genericWrite => exact absurd hbd (by simp [Spec.ntRightOf])
    | genericExecute => exact absurd hbd (by simp [Spec.ntRightOf])
    | deleteAccess => exact hbs
    | duplicateAccess => exact absurd hbd (by simp [Spec.ntRightOf])
  · intro h
    exact Finset.mem_biUnion.mpr
      ⟨Spec.NtBit.deleteAccess, h, by simp [Spec.ntRightOf]⟩

theorem android_untrusted_rights_empty : Spec.android_untrusted_rights_empty := by
  intro r hr
  subst hr
  rfl

end Model
