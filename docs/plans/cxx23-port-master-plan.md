# PBSD — HBSD → C++23 Port Master Plan

**Author:** Odin Loch  
**Base:** HardenedBSD 15-STABLE → ParanoidBSD (PBSD)  
**Language:** ISO C++23 for all new and rewritten code  
**Automation doctrine:** Prefer deterministic Clang tooling over LLM judgment  
**Status:** Active — supersedes `pbsd-master-plan.md`, `bsd-port-playbook.md`, `yggdrasil-parallel-algorithm-plan.md`, `paranoidbsd-security-catalogue.md`, and `pbsd-cpp23-conversion-execution-plan.md`  
**Date:** July 2026

---

## 0. What this document is

One plan for porting HardenedBSD C into PBSD C++23. It merges architecture, concurrency envelopes, security-native defaults, the per-file pipeline, and the **Clang-first automation layer**.

| Concern | Where it lives now |
|---|---|
| Architecture, phases, UDA, BIFROST, desktop | §1–§4, §10 |
| Concurrency envelopes (A/B/C) | §5 |
| Security-native defaults | §6 |
| Per-file port pipeline | §7 |
| Clang / deterministic tooling | §8–§9 |
| Queue, defaults, orchestration | §11–§12 |
| Risks & licensing | §13–§14 |

Existing repo artifacts (`docs/specs/*`, `docs/migration/*`, `pbsd/`, `tools/`) remain authoritative for frozen specs and wave status. This plan governs *how* conversion proceeds.

---

## 1. Goal & non-negotiables

**Goal:** Replace owned HBSD C (kernel + userland + driver framework) with C++23 modules under `pbsd/`, differentially verified against HBSD originals, with HardenedBSD mitigations retained and PBSD capability/handle discipline woven in.

**Non-negotiables**

1. **Preserve every upstream header.** New PBSD work is AGPL-3.0-or-later; ported files keep the copyright notice, conditions and disclaimer they inherited, and that header governs the file. Attribute derived ports in [`NOTICES.md`](../../NOTICES.md) / [`PROVENANCE.md`](../PROVENANCE.md); the per-path map is [`LICENSING.md`](../../LICENSING.md). A rewrite pass that drops a header is a licence violation, and `tools/check_licences.py` fails on it.
2. **No file is `DONE` until differential verification passes.** Compile-only = `UNVERIFIED`.
3. **Behavior observed beats behavior assumed.** Instrument originals before rewriting ambiguous UB.
4. **Deterministic tooling first.** AST queries, clang-tidy fixits, include graphs, and inventory scripts run before any generative rewrite.
5. **Never silently “fix” bugs during port.** Log separately; port faithfully so diffs stay meaningful.
6. **Kernel freestanding ABI:** new kernel C++ uses `-fno-exceptions -fno-rtti` (`docs/specs/KERNEL_CXX_ABI.md`).

---

## 2. Architecture (merged)

```
┌─────────────────────────────────────────────────────────────────┐
│  PBSD PRESENTATION — KDE Plasma 6 / Wayland / Aero theme        │
├─────────────────────────────────────────────────────────────────┤
│  PERSONALITY — POSIX · linuxulator · BIFROST (Phase 6+)         │
├─────────────────────────────────────────────────────────────────┤
│  PBSD SERVICE LAYER (C++23)                                     │
│  UDA · capability revocation tree · static analyser · tools     │
├─────────────────────────────────────────────────────────────────┤
│  HARDENEDBSD KERNEL (C → progressive C++23)                     │
│  PaX · Capsicum · KASLR · SafeStack · CFI · ZFS · net stack     │
└─────────────────────────────────────────────────────────────────┘
```

**Decisions (frozen)**

| Decision | Choice |
|---|---|
| Base | HardenedBSD 15-STABLE |
| Desktop | KDE Plasma 6.x, Wayland-native |
| Language | C++23 modules for new/rewritten code |
| FS | ZFS retained (CDDL module; design-ref for native CoW later) |
| Security core | Capsicum → revocation tree; HBSD mitigations kept |
| Handles | `UniqueHandle` / `BorrowedHandle` / `SharedHandle` only |
| Driver path | UDA (descriptor + class engine) layered on FreeBSD driver model |

---

## 3. What HBSD already gives you

ASLR · SafeStack · CFI · W^X · PaX · RELRO/PIE · KASLR · hardened malloc · Capsicum · ZFS · jails · DTrace · Clang/LLVM toolchain · ports tree.

PBSD adds: ownership analyser (L1–L4), revocation tree, UDA capability-scoped MMIO/IRQ, BIFROST VM-as-capability, atomic update/rollback, STRIDE threat model + measured boot.

---

## 4. Conversion waves (aligned with repo inventory)

| Wave | Scope | Notes |
|---|---|---|
| **0** | Specs, handles, analyser L1+L3, dual-link kmod, inventory | Done (see `WAVE_STATUS.md`) |
| **1** | Nucleus modules (`pbsd/core`, handles, rights, lineage) | Ongoing hand ports |
| **2** | `lib` / `bin` / `sbin` / `usr.bin` | Leaves-first userland |
| **3** | KDE (already C++; ABI/theme bridges) | Light rewrite |
| **4** | `sys/kern`, `sys/vm`, security | Envelope A/B/C heavy |
| **5** | `sys/dev`, CAM → UDA feed | Descriptor engines |
| **6** | net / geom / fs (excl. wholesale ZFS rewrite) | Epoch reads pay off |
| **7** | stand / arch | Freestanding |
| **8** | BIFROST / vmm | New C++23 |
| **9** | Contrib leftovers + **owned-C purge gate** | `wave_purge_c_check.py --gate` |

Inventory source of truth: `docs/migration/c_inventory.csv` (~28k rows).

---

## 5. Concurrency envelopes (Yggdrasil doctrine)

Keep algorithm logic; replace only the concurrency envelope.

| Env | When | C++23 shape |
|---|---|---|
| **A** | Per-CPU / per-thread state | `PerCpu<T>` + MPSC mailbox slow path |
| **B** | Read-heavy pointer graphs | `epoch::Guard` / `epoch::Ptr<T>`; retire on free |
| **C** | Write-heavy keyed lookups (default if unsure) | `Sharded<T, HashFn>`, shards = `next_pow2(4×ncpu)`, =1 embedded |

**Phase 0 substrate (gate everything):** epoch EBR → buf_ring MPSC → `PerCpu`/`Sharded` → counter(9) → seqc → deterministic replay harness.

**Explicit non-ports:** splay trees; lockmgr (→ sx); rmlocks (→ epoch); libc regex backtracker (→ linear VM); softdep parallelization; Giant-era shims.

Every ported algorithm: differential vs original (single-threaded) → TSan stress → shard=1 size check.

---

## 6. Security-native defaults (applied while touching a file)

Deny-by-default capabilities · assume-breach compartments · mechanism-in-kernel / policy-at-edge · measured & reproducible · fail closed.

**Auto-apply by file class (no per-file debate)**

| Class | Apply |
|---|---|
| Kernel core / allocator / scheduler | Zero-on-alloc/free; saturating refcounts; RO-after-init |
| Ownership | Closed handle set only |
| Parsers / untrusted input | `std::span`; restartable fault domain where possible |
| Drivers / UDA | Capability-scoped MMIO/IRQ only |
| Shared state | `[[clang::guarded_by]]` + `-Wthread-safety` |
| ISR-reachable | `[[pbsd::isr]]`; no blocking (L4) |
| All rewritten TUs | stack-protector-strong, trivial-auto-var-init=zero, no raw new/delete |

Full catalogue (boot RoT, MAC/MLS, PQ crypto, TEMPEST, profiles workstation/server/embedded/hardened/classified) remains design reference under `docs/security/` — bake `[native]` items in Phase 0/1; profile-gate the rest.

---

## 7. Per-file pipeline (Stages A–H)

One tightly-coupled cluster per unit. Exit criteria must pass (or Part 4 defaults in the execution appendix) before advancing.

| Stage | Name | Exit |
|---|---|---|
| **A** | Intake, risk tier, envelope triage | Map + tier + envelope recorded |
| **B** | Static spec (control-flow, not names) | Spec + UB flags |
| **C** | Dynamic ground-truth (ASan/UBSan/fuzz) | Spec reconciled to observation |
| **D** | Tests *before* port (must pass on original) | Suite green on C |
| **E** | Envelope wrap only | Algorithm untouched |
| **F** | C++23 impl + security weave + handles | Clean compile + sanitizers |
| **G** | Differential + fuzz oracle | Zero divergence |
| **H** | Port Record + STATUS + queue advance | Disk state updated |

**Risk tiers:** 1 = concurrency/locks/HW/syscalls/MM; 2 = parsers/untrusted; 3 = self-contained. Never downgrade without logged justification.

**Marking:** `DONE` only after G. Else `UNVERIFIED` / `NEEDS-REVIEW`.

---

## 8. Clang-first automation (the Opus thesis, operationalized)

Mechanical C→C++ of a BSD tree is **not** one clang-tidy check. It is a **pipeline of deterministic tools** that shrink the human/LLM surface to the residual semantic port.

### 8.1 Tool map

| Tool | Role in PBSD port |
|---|---|
| **clang-tidy** | Fixits on C++ and on C compiled as C++ (`modernize-*`, ownership bans, thread-safety); L1 gate via `.clang-tidy` |
| **clang-query** | AST matchers: `malloc`/`free`, `goto`, `void*`, K&R leftovers, `TAILQ_*`, global mutable state → risk/envelope hints |
| **LibTooling / clang plugins** | Custom L2 ownership + L4 ISR checks (`pbsd-ownership`) |
| **clang-scan-deps** | Precise include/module dependency graphs for queue ordering (leaves first) |
| **clang -emit-ast / -ast-dump** | Spec extraction aids; flag union punning, VLAs, flexible arrays |
| **clang-format** | Canonical style on every rewritten TU |
| **clang-apply-replacements** | Batch apply tidy fixits from a compilation database |
| **compile_commands.json** | Single source of truth for all Clang tools (`cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) |

### 8.2 What is safe to automate 100%

1. Inventory + wave assignment (`inventory_c_sources.py`)
2. Module stub generation (`convert_c_batch.py`)
3. AST pattern census → risk tier / envelope suggestions
4. clang-tidy modernize fixits on already-C++ and dual-link TUs
5. Include/dep graph → `queue.json` leaf-first order
6. Format + rename scaffolding (`.c` → `.cpp` companion with `extern "C"` bridge where ABI must hold)
7. Purge gate metrics (`wave_purge_c_check.py`)
8. CI analyser L1/L3 flags

### 8.3 What stays semi-automated (tooling + review)

1. Behavioral spec (B) — assisted by AST dumps + call graphs
2. Envelope application (E) — templates applied mechanically; classification may be `NEEDS-REVIEW`
3. Handle rewriting of ownership edges — matcher finds candidates; human/stronger model confirms
4. Differential harness wiring per subsystem

### 8.4 What is never “clang-tidy alone”

Scheduler/allocator semantics · Capsicum lineage · UDA descriptor engines · BIFROST · anything with intentional UB reliance until Stage C logs exist.

### 8.5 Driver scripts

Primary (implements [todo-passes.md](todo-passes.md) Tier 0–4 passes):

```bash
python3 tools/run_todo_passes.py --corpus-only
python3 tools/run_todo_passes.py --scope bin,usr.bin,sbin --all-passes --no-ir --no-diff
python3 tools/run_todo_passes.py --file hbsd/src/usr.bin/true/true.c
```

Census / tidy companion:

```bash
python3 tools/clang_cxx23_port.py --phase all
```

Outputs land under `docs/migration/clang_port/` (`staged/`, `refusals.jsonl`, `todo_pass_report.md`).

---

## 9. Other deterministic tooling (beyond Clang)

| Tooling | Purpose |
|---|---|
| **ASan / UBSan / TSan / KCSAN-analogue** | Stage C/G oracles |
| **libFuzzer / AFL++ / syzkaller** | Divergence + crash oracles |
| **RapidCheck** | Property tests from Stage B invariants |
| **Record/replay differential runner** | Byte-exact / syscall-trace compare |
| **loom-style deterministic scheduler** | Exhaustive interleavings for Envelope B |
| **TLA+ / SPIN** | Epoch state machine only (Phase 0 substrate) |
| **clang-diff / git diff --word-diff** | Structural AST diff of ports vs originals |
| **Compile-time contracts** | `static_assert`, concepts on intrusive containers |
| **IWYU / include-cleaner** | Dead include purge post-port |
| **Reproducible builds + SBOM** | Supply-chain gate |
| **`c_inventory.csv` + purge gate** | Zero owned-C exit for PBSD 0.1 |

---

## 10. Project phases (OS + conversion interlocking)

| Phase | Focus | Exit |
|---|---|---|
| **0** | Docs, handles, envelopes, analyser L1+L3, QEMU/CI, Clang port script green | Smoke Stage H empty-file; clang report generated |
| **1** | Working HBSD+KDE desktop; handle hierarchy live | Boots; L2 analyser on new code |
| **2** | UDA v1 (virtio-blk/net descriptors) | Real chip or virtio driven by descriptor |
| **3** | Capsicum → revocation tree; L4 ISR | Threat model matches built system |
| **4** | Userland C++23 replacements (≥10 utils) | Analyser-clean differentials |
| **5** | Scheduler/allocator C++23; profiles | Three profile configs |
| **6** | BIFROST Type-1 + Windows guest | VM as capability object |
| **7** | Native compositor (optional KWin replace) | Aero fidelity |
| **8** | Atomic pkg/update (`ygg update`) | Rollback works |
| **9** | PBSD 0.1 | Zero owned C; reproducible; PROVENANCE complete |

**Sizing:** Phase 0–2 in 6–18 months solo+AI; full vision multi-year. Conversion Phase 0–2 (kern/vm + net/fs differentially verified) is the meaningful year-one bar.

---

## 11. Queue construction & non-blocking defaults

1. Walk tree → include/call graph (`clang-scan-deps` + inventory).
2. **Leaves first** within subsystem.
3. Within depth: risk **3→2→1** (cheap wins first), except Phase 0 foundations always first.
4. Subsystem order: kern → vm → net → fs (non-ZFS) → drivers/UDA → libc → utilities.
5. Rebuild queue at each subsystem boundary.

**If stuck:** apply defaults — Envelope C if unsure; skip missing fuzzer; Tier 3 smoke if no invariants; never halt the queue. Log `NEEDS-REVIEW.md`. Only hard stops: differential runner broken globally, or original source missing.

State on disk: `STATUS.md`, `queue.json`, `records/*`, `NEEDS-REVIEW.md`, `GOTCHAS.md`.

---

## 12. Handle hierarchy (closed set)

```cpp
template<typename T> class UniqueHandle;    // move-only owner
template<typename T> class BorrowedHandle;  // non-owning, no_escape
template<typename T> class SharedHandle;    // rare, audited

enum class CapabilityRights : uint32_t {
  Read = 1, Write = 2, Execute = 4, Grant = 8, Duplicate = 16, Destroy = 32
}; // rights only narrow on grant
```

Revoking a handle invalidates its lineage.

Analyser layers: L1 tidy/syntactic · L2 ownership CFG · L3 thread-safety · L4 ISR context (`docs/specs/ANALYSER.md`).

---

## 13. Licensing ledger

Authoritative version: [`LICENSING.md`](../../LICENSING.md). Summary:

- **PBSD original work:** `AGPL-3.0-or-later`, plus a commercial tier
- **`hbsd/`, `kde/`:** vendored, excluded, never relicensed
- **BSD-2/3, ISC, MIT, 0BSD:** direct derivation + retained headers → [`NOTICES.md`](../../NOTICES.md)
- **BSD-4-Clause (22 files):** the advertising clause bars them from an AGPL binary; excluded and ratcheted — §4.2.1
- **GPL-2.0-or-later / LGPL-2.1-or-later (KDE):** compatible via the "or later" upgrade and GPL-3.0 §13. `GPL-2.0-only` must never enter the tree
- **CDDL (ZFS/DTrace):** design reference or load unmodified module; never statically combined with AGPL code
- **Vendored (jemalloc, zstd, LZ4, crypto):** vendor upstream
- **CK epoch:** design ref; fresh C++23 EBR preferred for model-checking

---

## 14. Risk register (top)

| Risk | Mitigation |
|---|---|
| Underestimated rewrite | HBSD stays bootable; waves progressive |
| clang-tidy false confidence | Never mark DONE without differential |
| Envelope misclassification | Default C + NEEDS-REVIEW |
| UDA vs real HW quirks | virtio first; bytecode quirks |
| Analyser L2 cost | Ship L1+L3 immediately |
| Owned-C purge forever deferred | CI gate from Wave 9 scaffolding |

---

## 15. Immediate next actions

1. Run `python3 tools/clang_cxx23_port.py --phase all` (WSL Clang 18).
2. Review `docs/migration/clang_port/REPORT.md` — triage high-signal AST hits into `queue.json`.
3. Keep wave builds green (`pbsd/_build_wave.sh`) while draining leaves.
4. Expand LibTooling plugin checks as handle usage grows.
5. Delete superseded planning docs once this file is accepted (done when this lands).

---

## Appendix A — Port Record (minimum fields)

Classification · risk tier · envelope · dependencies · Stage B/C/D/E/F/G evidence · handle types · security-native items · deliberate ABI changes · status.

## Appendix B — Directory layout (conversion)

```
pbsd/           # C++23 nucleus + ports/
hbsd/           # unmodified / dual-link reference tree
docs/migration/ # inventory, CONVERTED, clang_port/
docs/specs/     # frozen SPECs
tools/          # inventory, convert, clang_cxx23_port, purge gate
```
