import Mathlib

/-!
# P0-02 capability model -- locked statements (Spec)

This file defines the package model for the capability object model header
(`pbsd/kernel/include/cap/cap.h`) and states, as closed `def <name> : Prop`,
every theorem that `P0-02-MODEL` must prove.  The statements talk only about
P0-02 model.  The statements talk only about
the concrete definitions below (`structure`, `inductive`, `def`); the file
presents no unverifiable ground truth: nothing here is trusted without proof.

Model summary (mirrors the C++ header):

* A rights mask is a finite set of `Right`s; deriving intersects it.
* An `Entry` protects an object: monotonic `generation`, a rights mask and a
  `revoked` flag.  A `Token` is what callers hold: a generation the token was
  issued at plus the rights it was narrowed to.
* `revoke` bumps the generation and marks the entry revoked; `restart` bumps
  the generation and clears the flag -- so every token issued before the event
  is stale and fails closed.
* Mapping tables exist for POSIX mode bits, NT security descriptor bits and
  Android uid ranges.
-/

namespace Spec

/-! ## Rights, kinds and entry/token data -/

/-- The closed rights vocabulary of the capability model (SI-1). -/
inductive Right
  | read | write | execute | grant | duplicate | destroy | map | irq

deriving instance DecidableEq, Repr for Right

/-- Object kinds the entry can protect. -/
inductive CapKind
  | memory | file | socket | device | irq | dmaWindow | process | personality

deriving instance DecidableEq, Repr for CapKind

/-- POSIX mode bit (owner / group / other, R / W / X). -/
inductive PosixBit
  | rUsr | wUsr | xUsr | rGrp | wGrp | xGrp | rOth | wOth | xOth

deriving instance DecidableEq, Repr for PosixBit

/-- NT security descriptor access-mask bit. -/
inductive NtBit
  | genericRead | genericWrite | genericExecute | deleteAccess | duplicateAccess

deriving instance DecidableEq, Repr for NtBit

/-- Android uid range the mapping table covers. -/
inductive AndroidRange
  | userApp | system | root | untrusted

deriving instance DecidableEq, Repr for AndroidRange

/-- Fintype instances for the enums used with `Finset` ( elems are listed,
completeness by case analysis ). -/
instance : Fintype Right where
  elems := {Right.read, Right.write, Right.execute, Right.grant,
            Right.duplicate, Right.destroy, Right.map, Right.irq}
  complete := by
    intro x
    cases x <;> simp

instance : Fintype PosixBit where
  elems := {PosixBit.rUsr, PosixBit.wUsr, PosixBit.xUsr, PosixBit.rGrp,
            PosixBit.wGrp, PosixBit.xGrp, PosixBit.rOth, PosixBit.wOth,
            PosixBit.xOth}
  complete := by
    intro x
    cases x <;> simp

instance : Fintype NtBit where
  elems := {NtBit.genericRead, NtBit.genericWrite, NtBit.genericExecute,
            NtBit.deleteAccess, NtBit.duplicateAccess}
  complete := by
    intro x
    cases x <;> simp

instance : Fintype AndroidRange where
  elems := {AndroidRange.userApp, AndroidRange.system, AndroidRange.root,
            AndroidRange.untrusted}
  complete := by
    intro x
    cases x <;> simp

/-- The authoritative capability entry: protects one object.  `generation` is
monotonic within the entry and increases on `revoke` and `restart`. -/
structure Entry where
  generation : Nat
  rights : Finset Right
  kind : CapKind
  revoked : Bool

/-- What a caller holds: the issue generation and the rights it was narrowed
to at issuance. -/
structure Token where
  generation : Nat
  rights : Finset Right

/-- Mint a token straight from an entry. -/
def mkToken (e : Entry) : Token := ⟨e.generation, e.rights⟩

/-- Deriving (granting) a child only narrows: intersection of masks. -/
def narrowRights (parent ask : Finset Right) : Finset Right := parent ∩ ask

/-- Derive a child token from a parent token (SI-1: only narrows). -/
def derive (parent : Token) (want : Finset Right) : Token :=
  ⟨parent.generation, narrowRights parent.rights want⟩

/-- A token is live iff it was issued at the entry's current generation and
the entry is not revoked. -/
def tokenLive (e : Entry) (t : Token) : Prop :=
  t.generation = e.generation ∧ e.revoked = false

/-- The authorisation decision: live token and the asked rights are a subset
of the token's rights. -/
def useOK (e : Entry) (t : Token) (ask : Finset Right) : Prop :=
  tokenLive e t ∧ ask ⊆ t.rights

/-- Generation relation: a token issued strictly before the entry's current
generation is stale (used after any revoke or restart). -/
def tokenStale (e : Entry) (t : Token) : Prop := t.generation < e.generation

/-- Revoke: bump the generation (all pre-revoke tokens fail closed) and lift
the revocation flag. -/
def revoke (e : Entry) : Entry := ⟨e.generation + 1, e.rights, e.kind, true⟩

/-- Restart: bump the generation (all pre-restart tokens fail closed) and
bring the entry back unrevoked. -/
def restart (e : Entry) : Entry := ⟨e.generation + 1, e.rights, e.kind, false⟩

/-! ## Mapping tables -/

/-- POSIX mode bit to right: read, write and execute bits (any class) land on
the matching right; the other bits grant nothing. -/
def posixRightOf : PosixBit → Finset Right
  | PosixBit.rUsr => {Right.read}
  | PosixBit.rGrp => {Right.read}
  | PosixBit.rOth => {Right.read}
  | PosixBit.wUsr => {Right.write}
  | PosixBit.wGrp => {Right.write}
  | PosixBit.wOth => {Right.write}
  | PosixBit.xUsr => {Right.execute}
  | PosixBit.xGrp => {Right.execute}
  | PosixBit.xOth => {Right.execute}

/-- POSIX mode bits (as a set) collapse onto a rights mask: no right is
created from thin air, the mask is only the union of what the bits name. -/
def rightsForPosix (s : Finset PosixBit) : Finset Right := s.biUnion posixRightOf

/-- NT access bit to right. -/
def ntRightOf : NtBit → Finset Right
  | NtBit.genericRead => {Right.read}
  | NtBit.genericWrite => {Right.write}
  | NtBit.genericExecute => {Right.execute}
  | NtBit.deleteAccess => {Right.destroy}
  | NtBit.duplicateAccess => {Right.duplicate}

/-- NT security descriptor access bits collapse onto a rights mask. -/
def rightsForNT (s : Finset NtBit) : Finset Right := s.biUnion ntRightOf

/-- Android uid range mapping: which object kind a freshly minted capability
protects. -/
def androidKindOf : AndroidRange → CapKind
  | AndroidRange.userApp => CapKind.process
  | AndroidRange.system => CapKind.file
  | AndroidRange.root => CapKind.memory
  | AndroidRange.untrusted => CapKind.socket

/-- Android uid range mapping: which rights a freshly minted capability
carries. -/
def androidRightsOf : AndroidRange → Finset Right
  | AndroidRange.userApp => {Right.read, Right.write}
  | AndroidRange.system => Finset.univ
  | AndroidRange.root => Finset.univ
  | AndroidRange.untrusted => ∅

/-! ## Locked statements (proved by P0-02-MODEL as `Model.<name>`) -/

/-- SI-1: narrowing can only produce a subset of the parent mask. -/
def narrow_subset_parent : Prop :=
  ∀ (p w : Finset Right), narrowRights p w ⊆ p

/-- A derived token keeps its parent's issue generation. -/
def derive_keeps_generation : Prop :=
  ∀ (t : Token) (w : Finset Right), (derive t w).generation = t.generation

/-- A derived token's rights are exactly the narrowed parent mask (and so
never wider than the parent's, SI-1). -/
def derive_rights_narrowed : Prop :=
  ∀ (t : Token) (w : Finset Right), (derive t w).rights = narrowRights t.rights w

/-- A revoked entry refuses every token, however fresh. -/
def revoked_use_fails : Prop :=
  ∀ (e : Entry) (t : Token) (ask : Finset Right), e.revoked → ¬ useOK e t ask

/-- After a revoke or restart every token issued before the event is stale
and fails closed (no authorisation through an old token). -/
def stale_fails_closed : Prop :=
  ∀ (e : Entry) (t : Token) (ask : Finset Right), tokenStale e t → ¬ useOK e t ask

/-- A live token carrying at least the asked rights authorises exactly that
ask (soundness -- the model grants nothing beyond a subset check). -/
def live_subset_use_ok : Prop :=
  ∀ (e : Entry) (t : Token) (ask : Finset Right),
      tokenLive e t → ask ⊆ t.rights → useOK e t ask

/-- `revoke` bumps the generation and marks the entry revoked. -/
def revoke_bumps_generation : Prop :=
  ∀ (e : Entry), (revoke e).generation = e.generation + 1 ∧ (revoke e).revoked = true

/-- `restart` bumps the generation and clears the revocation flag. -/
def restart_clears_and_bumps : Prop :=
  ∀ (e : Entry), (restart e).generation = e.generation + 1 ∧ (restart e).revoked = false

/-- A token minted straight off an entry is stale against the entry after a
restart, so it cannot authorise: exactly the fault-tolerance requirement. -/
def restart_token_stale_fails : Prop :=
  ∀ (e : Entry), tokenStale (restart e) (mkToken e)
      ∧ ¬ useOK (restart e) (mkToken e) Finset.univ

/-- But a token minted after the restart (at the new generation) is live
again: restart is a recovery, not a permanent deny. -/
def restart_new_token_live : Prop :=
  ∀ (e : Entry), tokenLive (restart (restart e)) (mkToken (restart e))

/-- The POSIX mapping's read right comes from precisely the read bits. -/
def posix_read_iff : Prop :=
  ∀ (s : Finset PosixBit), Right.read ∈ rightsForPosix s
      ↔ (PosixBit.rUsr ∈ s ∨ PosixBit.rGrp ∈ s ∨ PosixBit.rOth ∈ s)

/-- The NT mapping turns `deleteAccess` into the destroy right and nothing
else does. -/
def nt_delete_iff : Prop :=
  ∀ (s : Finset NtBit), Right.destroy ∈ rightsForNT s ↔ NtBit.deleteAccess ∈ s

/-- The Android mapping gives the untrusted uid range no rights at all. -/
def android_untrusted_rights_empty : Prop :=
  ∀ (r : AndroidRange), r = AndroidRange.untrusted → androidRightsOf r = ∅

end Spec
