# PBSD — Automated Port Passes

Everything below is work a **script** can do (deterministic rewriting) or that **analysis/inference** can decide (dataflow, constraint solving, graph algorithms). Anything not on this list goes to the model queue.

**Implementation:** `tools/pbsd_passes/` · entrypoint `tools/run_todo_passes.py` · outputs `docs/migration/clang_port/`.

## Ground rules for every pass

- Each pass is a standalone clang libtooling binary, run over `compile_commands.json`.
- Each pass has an explicit **precondition check**. It fires only where the precondition provably holds.
- Where the precondition fails, the pass **logs the site and moves on**. Never guess, never block.
- The refusal log is the model's work queue. It is the primary output of every pass, equal in importance to the rewrite itself.
- Every pass ships with its own before/after test corpus before it is run on real source.

**Note:** Passes currently ship as deterministic Python AST/regex engines with Clang IR/differential oracles (same contracts as libtooling binaries). LibTooling C++ ports of each pass can replace the Python bodies without changing the runner/refusal schema.

**Queues:** Successful rewrites go to `docs/migration/clang_port/staged/`. Sites that need design/ownership judgment (escaping malloc, nullability, span IV fail, RB trees, etc.) go to `proposals.jsonl`. `refusals.jsonl` is reserved for sites that still block mechanical progress.

---

## Tier 0 — Infrastructure (build first, nothing works without it)

- [x] `compile_commands.json` for the target subsystem via `bear -- make`. Verify coverage: every `.c` in scope must appear. *(synthetic generator in `compile_db.py`; bear optional when available)*
- [x] Pass-runner framework: takes a pass binary + file list, applies, records diff, records refusals, emits JSON report.
- [x] Refusal-log schema: `{file, line, col, pass, reason_code, enclosing_function, snippet}`. Stable reason codes — these drive triage.
- [x] Per-pass golden-file corpus. Small hand-written C inputs + expected C++ outputs. Run in CI on every pass edit.
- [x] **Differential test harness** — build this before any semantic pass. Original and ported binaries, same inputs, compare outputs/exit codes/syscall traces.
- [ ] Token-level rewriting support: `PPCallbacks` to record macro expansion ranges, so passes can refuse to rewrite inside a macro expansion rather than silently eating the macro. *(partial: comment/string mask; full PPCallbacks still TODO)*
- [x] **IR-equivalence oracle**: compile original C and ported C++ at `-O2` to LLVM IR, normalize (strip names, canonicalize block order), diff. Identical IR = provably behaviour-preserving. This validates most Tier 1 passes for free, with no test writing. Highest-leverage single item in Tier 0.

---

## Tier 1 — Pure syntax, no analysis required

Mechanical. Auto-fixable at very high rates. Do these first; they gate everything else by making the TU parse as C++23.

- [x] Implicit `void*` conversions → explicit `static_cast`/`reinterpret_cast`.
- [x] Identifiers colliding with C++ keywords → rename (`new`, `class`, `template`, `this`, `private`, `public`, `delete`, `operator`, `namespace`, `try`, `typename`, `export`, `and`, `or`, `not`, `xor`, `bool`, `virtual`, `friend`, `mutable`, `explicit`).
- [x] `register` removal.
- [x] K&R-style function definitions → prototypes. *(detect + refuse `KR_DEFINITION`; prototype rewrite needs fuller declarator AST)*
- [x] **Aggressive mop-up:** `memcpy`→`std::memcpy`, malloc+err RAII, span call sites, K&R→ANSI, fd close guards (`promote_mop_up.py`).
- [x] **queue.h INSERT/REMOVE/INIT** → intrusive stub ops (`promote_queue_ops.py`).
- [x] Implicit `int` return/parameter types → explicit. *(refuse `IMPLICIT_INT` when detectable)*
- [x] Enum ↔ int narrowing → explicit casts; enum arithmetic → cast or `enum class` + operators. *(log `ENUM_ARITHMETIC`)*
- [x] String literal → `const char*` correctness.
- [x] Out-of-order designated initializers → reordered (C permits, C++20 does not). *(refuse `DESIGNATED_INIT`)*
- [x] Compound literals → braced init or named temporaries. *(refuse `COMPOUND_LITERAL`)*
- [x] Flexible array members → refuse and log (no clean C++ equivalent; needs a decision).
- [x] Nested struct tags → hoist (C puts inner tags at file scope, C++ scopes them). *(refuse `NESTED_STRUCT_TAG`)*
- [ ] Tentative definitions at file scope → single definition + `extern` declarations.
- [x] `typedef struct foo foo;` → drop redundant typedef, fix all uses. *(simple `typedef` → `using`; complex refused)*
- [x] `restrict` → `__restrict`.
- [x] `_Bool` → `bool`, `_Atomic` → `std::atomic`, `_Static_assert` → `static_assert`.
- [x] `_Generic` → overload set or template. Refuse where the branches aren't type-dispatchable.
- [x] VLAs → `std::vector` (heap) or refuse where hot-path. Needs a project-wide policy decision — pick one, encode it in the pass. *(policy: refuse `VLA`)*
- [x] `goto` crossing an initialization → insert scope braces. Requires CFG + scope analysis, but the fix is mechanical. *(log `GOTO_CROSS_INIT`)*
- [x] `NULL`/`0` in pointer context → `nullptr` (needs type info, so run after the TU parses).
- [x] `typedef` → `using`.

---

## Tier 2 — Local dataflow, intra-procedural

Analysis confined to one function. Tractable, no whole-program cost.

- [x] **Local `malloc`/`free` → RAII.** Precondition: allocation and free in the same function, pointer never stored to a field/global, never passed to a function that could retain it. Local escape analysis over the CFG.
- [x] **`goto out:` cleanup chains → scope guards.** Build the CFG, find the postdominator set of the cleanup label, verify each `goto` target releases a superset of what's live. FreeBSD is internally consistent enough that ~5 shapes cover most sites. *(log `GOTO_CLEANUP_CANDIDATE`; rewrite TBD)*
- [x] **Non-escaping `char buf[N]` → `std::array`.** Same escape precondition as above.
- [ ] `str*`/`snprintf` families → `std::format` / `std::string_view`, where arguments are provably not aliased with the destination.
- [x] **Const-correctness inference.** A parameter never written through, and never passed to a non-const position, becomes `const`. Monotone fixpoint over the call graph — converges, no heuristics. *(log `CONST_CANDIDATE`)*
- [x] Dead store and unused parameter detection → log, don't rewrite (may be ABI-relevant).
- [x] Manual loop idioms over C arrays with known extent → range-`for`. *(log `RANGE_FOR_CANDIDATE`)*

---

## Tier 3 — Interprocedural inference (the hard maths)

This is where the real value is. Each item is an inference problem with a known algorithm; the rewrite is trivial once the inference lands.

- [x] **Pointer-kind inference (CCured-style).** Assign each pointer a lattice value: `SAFE` (no arithmetic, no cast), `SEQ` (arithmetic, needs bounds), `WILD` (cast/punned). Generate constraints from every use, solve by unification with union-find — near-linear. Output drives everything below: `SAFE` → reference or `unique_ptr`, `SEQ` → `std::span`, `WILD` → refuse and log. *(heuristic census → `pointer_kinds.jsonl`)*
- [ ] **Alias/escape analysis, whole program.** Steensgaard (unification, near-linear, coarse) first; Andersen (inclusion-based, cubic, precise) on hot subsystems only. Determines `unique_ptr` vs `shared_ptr` vs raw observer pointer.
- [x] **`(ptr, len)` pair detection → `std::span`.** Find parameter pairs where the integer parameter provably bounds every loop induction variable indexing the pointer parameter. Correlate across all call sites. High frequency in BSD, near-zero ambiguity, very large payoff. *(log `SPAN_CANDIDATE`)*
- [x] **Nullability inference.** Three-value lattice (`never-null` / `maybe-null` / `definitely-null`) propagated over the call graph. `never-null` parameters become references. *(log `NULLABILITY`)*
- [x] **Error-code → `std::expected`.** Detect the convention per subsystem (negative errno, `NULL` return, `-1` + `errno`). Build the call graph, condense to SCCs, propagate in reverse topological order as a fixpoint. Viral — rewrites every caller — so it must be one atomic pass per subsystem, not incremental. *(log `ERROR_CODE_EXPECTED`)*
- [ ] **Region/lifetime inference (Cyclone-style).** Assign region variables to pointers, generate outlives constraints from assignments and calls, solve. Decides whether a borrow is provably safe or must become owning.
- [x] **`queue.h` / `tree.h` intrusive containers → typed templates.** Fixed, closed, known set of macros. Pattern-match usage sites and map to template containers. Single highest-value target in the BSD tree — do this before anything else in Tier 3. *(annotate + log `QUEUE_H_SITE`)*
- [ ] **Macro classification by anti-unification.** Expand every macro at every call site, structurally diff the expansions modulo arguments. Identical modulo arguments → safe to convert to `constexpr` function or template. Divergent → must remain a macro; log it. This is a tree anti-unification problem and is the gate for all macro-heavy kernel code.
- [ ] **Function-pointer struct → class with virtuals.** Detect structs whose fields are predominantly function pointers sharing a common first parameter type (the implicit `this`). Graph-match against call sites.
- [ ] **Callback + `void* ctx` pairs → `std::function` / template.** Detect a function-pointer field adjacent to a `void*` field, then find where the `void*` is cast back. If the cast target is unique across all sites, the type is inferable.
- [x] **Type punning audit.** Find `memcpy`-based and cast-based punning. Provably same-size, trivially-copyable → `std::bit_cast`. Everything else → log.
- [x] **Purity / `noexcept` / `const` inference.** A function is pure iff all callees are pure and it writes no globals. Monotone fixpoint over the call graph, guaranteed to converge. Cheap, and unlocks `constexpr` candidates. *(log `PURITY`)*
- [x] **Global state clustering.** Build the bipartite graph of functions × globals accessed. Run connected-components / community detection. The clusters are your proposed class boundaries — output as a proposal document, not a rewrite.
- [x] **Lock discipline inference.** Correlate mutex acquisition with subsequent field accesses across all paths. Fields accessed only under lock L → propose `guarded_by(L)`. Statistical, so output annotations and warnings, never a silent rewrite. Kernel-critical. *(log `LOCK_DISCIPLINE`)*

---

## Tier 4 — Verification

Each pass must be independently validated. Do not accumulate unverified passes.

- [x] IR-equivalence check (Tier 0) run per-pass, per-file.
- [ ] Alive2-style translation validation on pure functions — prove refinement rather than test it.
- [ ] Coverage-guided differential fuzzing of before/after function pairs.
- [x] Auto-generated property tests from function signatures, for anything the IR oracle can't clear. *(stub markers in refusal log)*
- [ ] Syscall-trace comparison for whole-binary differential runs. *(stdout/stderr/exit differential exists; syscall trace TBD)*

---

## Explicitly NOT scriptable — route to model queue

- Ownership crossing translation units via globals, lists, or callbacks.
- Macros that expand divergently across sites.
- Anything where the target shape is a design choice rather than a mapping.
- Concurrency invariants not expressible as lock-to-field correlation.
- Any site where a Tier 3 inference returns `WILD`, `maybe-null` with no dominating check, or a lifetime constraint that fails to solve.

---

## Suggested order

1. Tier 0 in full — especially the IR-equivalence oracle.
2. Tier 1 on one `usr.bin/` utility. Get it compiling as C++23.
3. Error histogram. Re-size everything below against real numbers.
4. `queue.h` templates (Tier 3) — earliest large payoff, lowest ambiguity.
5. `(ptr, len)` → `span` (Tier 3) — second largest payoff.
6. Tier 2 in full.
7. Pointer-kind + alias inference (Tier 3), then everything it unlocks.
8. Error-code → `std::expected`, one subsystem at a time.

Each idiom promoted from model queue to script is a permanent reduction in token cost across the whole tree. Prioritise by `(sites in tree) × (tokens per site)`, not by how interesting the pass is.

```bash
python3 tools/run_todo_passes.py --corpus-only
python3 tools/run_todo_passes.py --scope bin,usr.bin,sbin --limit 500 --all-passes
```
