# Proof library and mined patterns

- `mine.db`: SQLite database of every theorem from the rehearsal runs mined so far (`theorem`: run, name, kind locked/helper/neg, statement, proof, shape, tactics; `spec`: the locked statements).
- `MINED.md`: generated report (shapes, tactic frequency, every proof). Regenerate with `tools/mine/mine.sh --report`.
- `PATTERNS.md`: the curated playbook the prover role reads before it starts. Edit by hand when a pattern is confirmed by a later run.

Add a run: `tools/mine/mine.sh <run-id> <Model.lean> [Spec.lean]` then `tools/mine/mine.sh --report`; commit `mine.db`, `MINED.md` and any change to `PATTERNS.md`.

Plan (not built yet, in order): (1) mine every rehearsal automatically at the end of its phase; (2) key each proof by the shape of its statement (the definitions it mentions, normalised) so the runner can offer "a proof of a similar statement from an earlier run" to the agent; (3) promote lemmas that recur across packages into a shared, hash-locked `PbsdLib` Lean library imported by `Spec` and `Model`; (4) a statement-template layer (or small macro DSL) for the recurring statement forms (fail-closed, never-widens, bumps-generation) so statements are true by construction. Every cached proof is re-checked by the Lean kernel on every use.
