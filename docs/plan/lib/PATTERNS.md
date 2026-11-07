# Proof patterns that worked (read this before writing proofs)

Mined from real, gate-accepted P0-02 proofs by `tools/mine/mine.sh` (see `MINED.md` for every proof, `mine.db` for the data). Lean 4.9 + Mathlib v4.9.0. Rule of thumb from the data: **try the cheapest shape first** (rfl, then library lemma, then omega, then case split), and call `lean_check` after each try.

## 1. Statements that are definitional: `intro …; rfl`
Equalities that hold by unfolding the definitions (a field is set, a counter is incremented):
```lean
theorem derive_keeps_generation : Spec.derive_keeps_generation := by
  intro t w
  rfl
theorem revoke_bumps_generation : Spec.revoke_bumps_generation := by
  intro e
  exact ⟨rfl, rfl⟩          -- a conjunction of two definitional facts
```
If `rfl` fails, `simp [Spec.<def>]` or `unfold Spec.<def>` first.

## 2. Counters and generations: pull facts out of the hypothesis, then `omega`
Take the equalities from the hypothesis projections with `have … := h.1.1`, state the definitional ones with `:= rfl`, close with `omega`:
```lean
theorem stale_fails_closed : Spec.stale_fails_closed := by
  intro e t ask hst hu
  have he : t.generation = e.generation := hu.1.1
  have hs : t.generation < e.generation := hst
  omega
```
Same for restart/epoch statements: `have h2 : (Spec.restart e).generation = e.generation + 1 := rfl`.

## 3. One library lemma after the intros
`Finset.inter_subset_left`, `Nat.lt_succ_self`, `Finset.mem_biUnion`, `Finset.mem_biUnion.mpr ⟨witness, membership, by simp [map]⟩`. Search with `exact?` in a scratch `lean_check` file before inventing a proof.

## 4. Iff over a union of enum-indexed sets (rights maps, POSIX/NT bits): case split the enum
```lean
constructor
· intro hr
  obtain ⟨b, hbs, hbr⟩ := Finset.mem_biUnion.mp hr
  cases b with
  | rUsr => exact Or.inl hbs
  | wUsr => exact absurd hbr (by simp [Spec.posixRightOf])   -- every bit that maps elsewhere
· intro h
  rcases h with h | h | h
  · exact Finset.mem_biUnion.mpr ⟨Spec.PosixBit.rUsr, h, by simp [Spec.posixRightOf]⟩
```
Long but mechanical: one `cases` arm per constructor; arms for bits that do not map to the right are all `absurd … (by simp [map])`.

## 5. A statement you cannot prove may be false
Typical sign: after the `rfl` facts of shape 2, `omega` proves the **negation** (for example the claim needs `restart e` but says `restart (restart e)`). Then:
1. prove `theorem neg_<name> : ¬ Spec.<name>` in Model.lean (a counterexample, or a general `intro h; …; omega`);
2. `post_message(kind=proof_failure, to=if, body="<name> is false: <counterexample>")` — the IF task reopens, the freeze re-runs, and you resume;
3. do not spend more turns on that statement.

## 6. Workflow that kept gate runs low
`write_file` the whole Model.lean with `sorry` for every proof → `lean_check` → replace the sorries one at a time, `lean_check` after each → `run_gate` only when it compiles with 0 sorry. (With `lean_check` the P0-02 task needed 8 gate runs instead of 38.)

## 7. Things that did not work (so far)
- Calling a prover model: 13 `lean_prove` calls returned nothing; Goedel-8B loops. Write proofs yourself.
- Re-running the gate unchanged: the same failure repeats; change the file.
