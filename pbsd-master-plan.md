# PBSD — Master Project Plan
**Author:** Odin Loch  
**Base:** HardenedBSD 15-STABLE + KDE Plasma 6.6.x  
**Strategy:** Start with maximum working code, rewrite progressively toward PBSD design goals  
**Language:** C++23 for all new/rewritten code  
**Status:** Planning document — July 2026

---

## 1. What You Are Taking On — Code Volume Reality Check

### Source trees you will clone and work against

| Component | Approx SLOC | What it is | Your touch level |
|---|---|---|---|
| **HardenedBSD src** | ~10–12M lines | Kernel + full BSD userland + HardenedBSD security patches | Deep — rewrite subsystems progressively |
| **HardenedBSD ports** | ~8M lines (port definitions) | Package build recipes | Light — use as-is, add your own ports |
| **KDE Plasma 6** | ~3–4M lines | Compositor, shell, window manager, panels | Deep — theme, customise, eventually rewrite compositor |
| **KDE Frameworks 6** | ~6–8M lines | Core libraries Qt6 is built on | Medium — use as-is, extend |
| **KDE Applications** | ~8–10M lines | Full app suite | Light initially — use as-is |
| **Qt 6** | ~7–8M lines | GUI toolkit | Light — use as-is |
| **Your new code** | ~2–5M lines (target) | Static analyser, UDA framework, C++23 rewrites, BIFROST hypervisor | 100% yours |
| **Total codebase** | ~35–45M lines | Everything in the build tree | — |

### What you will actually touch, phased

- **Phase 0–1:** Read and understand ~500K lines (kernel core, HardenedBSD patches, KDE compositor)
- **Phase 2–3:** Modify ~200–500K lines (UDA framework layered in, C++23 userland tools begin)
- **Phase 4–5:** Rewrite ~1–2M lines (scheduler, allocator, capability extensions, compositor)
- **Phase 6–9:** Write ~1–3M lines net-new (BIFROST hypervisor, Windows 7 theme layer, full C++23 subsystem rewrites)

**Honest sizing:** This is a 5–10 year full-time project to complete all phases. Phase 0–2 (working hardened desktop with UDA framework started) is achievable in 6–18 months solo with AI assistance.

---

## 2. Decision Log — Merged Plan

| Decision | Choice | Reason |
|---|---|---|
| Base OS | HardenedBSD 15-STABLE | Exploit mitigations already shipped (ASLR, SafeStack, CFI, W^X, Capsicum); BSD-2-Clause throughout; proven kernel; ZFS native |
| Desktop | KDE Plasma 6.6.x on FreeBSD ports | Actively maintained FreeBSD port; C++/QML (consistent with C++23 goal); most flexible theming of any major DE |
| Display server | Wayland (KDE 6.7 = last X11; 6.8 = Wayland-only, October 2026) | X11 support ending; plan for Wayland-native from the start |
| Language | Pure C++23 for all new and rewritten code | Consistent with PBSD design; no language boundary complexity; C++23 modules for new subsystems |
| Kernel language | Existing HardenedBSD C kernel retained initially; C++23 added for new subsystems with `-fno-exceptions -fno-rtti` | Pragmatic — don't rewrite what works before proving the design |
| Security model | HardenedBSD PaX mitigations + Capsicum extended toward PBSD capability-revocation-tree model | Build on what's there rather than replacing a working security model |
| Filesystem | ZFS (ships with HardenedBSD/FreeBSD, CDDL — note: isolated as a kernel module per FreeBSD's own model) | World-class; not your problem to build |
| Network stack | FreeBSD's TCP/IP stack (retain) | One of FreeBSD's genuine strengths; don't replace early |
| Windows interop | BIFROST — Type-1 VM hosting licensed Windows guest (Phase 6) | No Windows code touched; user brings their own license |
| Driver strategy | Universal Driver Architecture (UDA) layered onto FreeBSD's existing driver model | Turns driver-breadth problem into descriptor-writing problem |
| Look and feel | Windows 7 Aero aesthetic via KDE Plasma theming layer | KWin effects, Plasma themes, widget styles — zero kernel coupling |
| Licensing | Proprietary (initially) | Consistent with IP-for-defence-sales model; BSD base permits this |
| Provenance | PROVENANCE.md per subsystem, specs cite only public standards | Clean legal trail for all new code |
| Name | PBSD (OS) · BIFROST (VM/interop) | PBSD project identity; BIFROST for VM/interop |

---

## 3. What HardenedBSD Gives You For Free

HardenedBSD ships the following out of the box — these are **not things you need to build**:

**Exploit mitigations (already in kernel):**
- ASLR (stack, mmap, exec, VDSO)
- SafeStack (shadow stack against stack-smashing)
- CFI (control-flow integrity, LLVM-based)
- W^X enforcement (no page simultaneously writable and executable)
- PaX PAGEEXEC / MPROTECT
- Non-executable stack
- RELRO + BIND_NOW (read-only relocations)
- PIE enforced by default
- LibreSSL replacing OpenSSL (smaller attack surface)

**Capability framework:**
- Capsicum (capability-based sandboxing, process and file descriptor level) — you extend this, not replace it

**Kernel hardening:**
- Kernel stack protection
- KASLR
- Hardened malloc (`hbsd_malloc`)
- Zero-initialisation of kernel memory before use

**Platform:**
- ZFS (snapshots, checksums, COW — the best filesystem for your server profile)
- Jails (lightweight OS-level virtualisation — first-class containers)
- DTrace (production-grade kernel tracing)
- LLVM/Clang as the default toolchain (directly compatible with your C++23 and static analyser plan)
- pkg (binary package management)
- Ports tree (~35,000 packages)

---

## 4. Architecture — Merged PBSD + HardenedBSD

```
┌─────────────────────────────────────────────────────────────────┐
│  PBSD PRESENTATION LAYER                                   │
│  KDE Plasma 6 · Windows 7 Aero theme · KWin Wayland compositor │
│  (C++/QML — your theming and eventual compositor rewrite)       │
├─────────────────────────────────────────────────────────────────┤
│  PBSD PERSONALITY LAYERS                                   │
│  POSIX full (FreeBSD libc — retained)                          │
│  Linux-binary-compat (FreeBSD linuxulator — retained/extended) │
│  BIFROST VM interop bridge (Phase 6 — new C++23)               │
├─────────────────────────────────────────────────────────────────┤
│  PBSD SERVICE LAYER (C++23 rewrites, progressive)         │
│  Universal Driver Architecture (UDA) — new, layered on below   │
│  Capability revocation tree (Capsicum extension — new C++23)   │
│  Static analyser (Clang-based, CI-gated — new C++23)          │
│  C++23 userland tools (progressive replacement of BSD utils)   │
├─────────────────────────────────────────────────────────────────┤
│  HARDENEDBSD KERNEL (C — retain, extend, selectively rewrite)  │
│  PaX mitigations · Capsicum · KASLR · SafeStack · CFI          │
│  ZFS · FreeBSD network stack · FreeBSD driver model            │
│  (Scheduler rewrite — Phase 5; allocator rewrite — Phase 5)   │
├─────────────────────────────────────────────────────────────────┤
│  HARDWARE                                                        │
│  x86-64 (primary) · aarch64 (secondary) · embedded (Phase 5+) │
└─────────────────────────────────────────────────────────────────┘
```

---

## 5. Security Features (Consolidated)

**Inherited from HardenedBSD (shipping now):**
ASLR · SafeStack · CFI · W^X · PaX PAGEEXEC/MPROTECT · non-executable stack · RELRO · PIE-by-default · KASLR · hardened malloc · kernel stack protection · LibreSSL · Capsicum capability sandboxing · zero-initialisation of kernel memory

**Added by PBSD (your work, phased):**
- Custom static analyser (Clang-based; ownership/lifetime dataflow over `UniqueHandle`/`BorrowedHandle`/`SharedHandle`; Clang Thread Safety Analysis; ISR-context checks; CI-gated from Phase 0)
- Capability revocation tree (extending Capsicum: rights-only-narrow-on-grant; per-process revocation lineage; whole-lineage containment of a compromised process in one operation)
- Universal Driver Architecture: drivers receive capability-scoped MMIO/IRQ access to their specific device only — never ambient hardware access
- BIFROST VM isolation: Windows guest is a capability-gated kernel object; Windows code never touches the native kernel
- Atomic update/rollback: updates either fully apply or fully revert (no inconsistent-half-applied-state attack surface)
- Formal threat model (STRIDE per subsystem) — Phase 0 document, validated against real implementation at Phase 4
- Verified/measured boot (Secure Boot chain) — Phase 4

---

## 6. Universal Driver Architecture (UDA)

**Core split:** generic class engine (one per device class) + declarative hardware descriptor (one per chip). Adding a new chip = writing a descriptor, not driver code.

**Components:**
- Descriptor schema (register offsets, bit fields, command sequences, init order, interrupt behaviour) — clean-room by construction; transcribes public datasheet facts into your own format
- Generic engines: block, network, display, input, sensor
- Constrained stack-machine bytecode interpreter for init/quirk sequences
- Bus enumeration → descriptor lookup (PCI/USB vendor:device → descriptor database)
- Capability scoping: the engine holds MMIO/IRQ capability; a descriptor cannot grant itself rights

**Build order within UDA:**
1. Descriptor schema frozen as a spec
2. Block engine + QEMU virtio-blk descriptor (proves concept on open-spec virtual hardware first)
3. Net + console engines, same approach
4. First real chip descriptor from a datasheet (once the framework is proven)

---

## 7. Handle & Capability Type Hierarchy (C++23)

```cpp
// All kernel resource ownership routes through this closed type set.
// The static analyser reasons fully about these types and nothing else.

template<typename T>
class UniqueHandle;       // [[kernel::owns]] — move-only, single owner

template<typename T>
class BorrowedHandle;     // [[kernel::borrows]] [[kernel::no_escape]] — non-owning view

template<typename T>
class SharedHandle;       // refcounted, deliberately rare, individually audited

// Rights bitmask — only narrows on duplication/grant, never widens
enum class CapabilityRights : uint32_t {
    Read      = 1 << 0,
    Write     = 1 << 1,
    Execute   = 1 << 2,
    Grant     = 1 << 3,
    Duplicate = 1 << 4,
    Destroy   = 1 << 5,
};
```

Revocation tree: revoking a handle invalidates everything derived from it. One call to contain an entire compromised process's capability lineage.

---

## 8. Static Analyser Design

Built on Clang LibTooling + Clang CFG. Four layers:

| Layer | What it checks | Build effort |
|---|---|---|
| L1 — Syntactic | Ban raw `new`/`delete`; ban raw pointer fields in kernel types; require `std::span` | Days |
| L2 — Ownership dataflow | Track `UniqueHandle`/`BorrowedHandle`/`SharedHandle` through owned→moved→borrowed→released states; flag use-after-move, double-release, exclusive-borrow-while-aliased | Weeks–months |
| L3 — Lock discipline | Clang Thread Safety Analysis (`-Wthread-safety`) with `[[clang::guarded_by]]` annotations — use existing Clang feature, don't rebuild | Hours to enable |
| L4 — Kernel context | ISR-context rules (no blocking in interrupt-tagged functions); reuses L2 CFG infrastructure | Days once L2 exists |

CI gate: all layers must pass clean on every commit. No merge without a green analyser pass, from the first commit.

Runtime backstop: ASan + UBSan + TSan in the QEMU test loop.

---

## 9. BIFROST — VM & Windows Interop (Phase 6)

- Type-1 design: PBSD kernel IS the hypervisor host (Intel VT-x/AMD-V + VT-d/IOMMU)
- `VmHandle` slots into the capability hierarchy — VM creation/destruction is a capability-gated operation
- vCPUs as schedulable entities in the existing scheduler
- Paravirtualised devices via virtio (open OASIS standard — clean-room compatible by construction)
- Windows guest drivers written against Microsoft's public WDK — same legitimate path as VMware Tools, VirtualBox Guest Additions
- GPU passthrough (SR-IOV or paravirtualised GPU channel) for workstation profile — scoped separately, harder problem

---

## 10. Project Plan

### Phase 0 — Foundations (Weeks 1–4)
**Goal:** Everything needed to work cleanly, before touching any source.

- [ ] Clone HardenedBSD src, ports, KDE Plasma (see download script)
- [ ] Set up Cursor workspace with source tree (see `.cursor/` config below)
- [ ] Set up QEMU + GDB stub + CI (GitHub Actions or local) — green on an empty test
- [ ] Write and freeze `SPEC.md` contract template
- [ ] Write and freeze Glossary (PBSD naming, handle types, profile names)
- [ ] Write Security Invariants doc (properties that must hold across every contract)
- [ ] Write `PROVENANCE.md` policy + initial entries for every inherited subsystem
- [ ] Write Threat Model skeleton (STRIDE per subsystem — driver boundary and BIFROST are priority)
- [ ] Write Coding Standards doc (C++23 conventions, `[[kernel::*]]` annotation usage, module boundary rules)
- [ ] Write ABI/versioning policy
- [ ] Configure Clang static analyser Layer 1 (syntactic bans) as a CI check
- [ ] Configure Clang Thread Safety Analysis (`-Wthread-safety`) — Layer 3

**Exit criteria:** CI green, all Phase 0 docs exist and are frozen, static analyser Layers 1+3 running.

---

### Phase 1 — Working Hardened Desktop (Weeks 4–12)
**Goal:** HardenedBSD 15-STABLE + KDE Plasma 6.6.x booting, running, stable.

- [ ] Build HardenedBSD 15-STABLE from source in QEMU (validates build environment)
- [ ] Install KDE Plasma 6.6.x from ports — confirm Wayland compositor running
- [ ] Apply Windows 7 Aero KDE theme (KWin effects, Plasma theme, widget style, taskbar layout)
- [ ] Validate all HardenedBSD hardening features are active (PaX, CFI, SafeStack, Capsicum)
- [ ] Set up real-hardware test target (x86-64 workstation or laptop)
- [ ] Begin `UniqueHandle`/`BorrowedHandle`/`SharedHandle` type hierarchy implementation (C++23)
- [ ] Wire static analyser Layer 2 (ownership dataflow) — initial version, scoped to handle types

**Exit criteria:** Boots to a Windows-7-looking KDE desktop on QEMU and real hardware. Handle type hierarchy compiles. Analyser Layer 2 running on new code.

---

### Phase 2 — Universal Driver Architecture v1 (Weeks 12–24)
**Goal:** UDA descriptor-engine split proven against virtual hardware.

- [ ] Write and freeze base driver contract (lifecycle, DMA request, interrupt delivery)
- [ ] Write and freeze UDA descriptor schema
- [ ] Implement block engine (C++23, layered alongside FreeBSD's existing driver model)
- [ ] Write virtio-blk descriptor (from OASIS virtio spec — PROVENANCE entry required)
- [ ] Prove: QEMU virtio-blk driven entirely by descriptor, zero chip-specific code
- [ ] Implement net engine (same pattern)
- [ ] Write virtio-net descriptor
- [ ] Write first real hardware descriptor from a chipset datasheet (e.g. a common NIC or AHCI controller)

**Exit criteria:** At least one real chip driven by UDA descriptor, not by a conventional driver.

---

### Phase 3 — Capability Extension & Security Hardening (Weeks 20–36)
**Goal:** Extend Capsicum toward PBSD's capability-revocation-tree model; validate threat model.

- [ ] Implement capability revocation tree on top of Capsicum (C++23 kernel extension)
- [ ] Implement rights-only-narrow-on-grant enforcement
- [ ] Write per-process revocation lineage data structure
- [ ] Validate threat model against the real Phase 1–2 implementation (update STRIDE doc)
- [ ] Enable W^X, verified boot chain, full CFI on build
- [ ] Complete static analyser Layer 4 (ISR-context checks)

**Exit criteria:** Threat model document reflects actual built system, not design aspirations. Capability revocation tree operational.

---

### Phase 4 — C++23 Userland Replacement Begins (Weeks 24–52+)
**Goal:** Progressive replacement of BSD C userland tools with C++23 rewrites. Ongoing throughout later phases.

- [ ] Identify target subsystems (shell utilities, system daemons — start with lowest-risk, highest-visibility)
- [ ] Write SPEC.md for each tool before rewriting it
- [ ] Rewrite and validate against the existing BSD tool as a reference (black-box behaviour matching — legally clean, you're matching observable behaviour, not reading source)
- [ ] Each rewrite must pass static analyser gate

**Exit criteria:** At least 10 BSD userland utilities replaced with C++23 implementations passing analyser gate.

---

### Phase 5 — Server & Workstation Profiles, Scheduler & Allocator (Weeks 36–78+)
**Goal:** Profile-based build configuration; kernel scheduler and allocator rewrite.

- [ ] Implement profile build system (embedded-lite, server, workstation build configs)
- [ ] Write scheduler SPEC.md (three variants: RTOS-style, throughput, interactive)
- [ ] Implement C++23 scheduler (kernel module, three-variant design, `-fno-exceptions -fno-rtti`)
- [ ] Write allocator SPEC.md (three variants: slab/fixed-pool, NUMA-aware, general-purpose)
- [ ] Implement C++23 allocator
- [ ] Begin aarch64/embedded profile work (UDA pays off here — descriptor library grows)

**Exit criteria:** Scheduler and allocator replaced with C++23 implementations. Three profile build configs working.

---

### Phase 6 — BIFROST VM & Windows Interop (Weeks 52–104+)
**Goal:** Type-1 hypervisor, VM as capability object, Windows guest running.

- [ ] Write BIFROST SPEC.md and threat model addendum
- [ ] Implement VT-x/AMD-V virtualisation layer (C++23, kernel module)
- [ ] Implement `VmHandle` in capability hierarchy
- [ ] Implement virtio paravirtualised transport (bridge protocol design)
- [ ] Write Windows guest drivers against public WDK (PROVENANCE: cite WDK documentation)
- [ ] Confirm Windows guest boots and interop bridge (shared filesystem, network passthrough) works

**Exit criteria:** Windows runs as a capability-gated guest. Interop bridge functional.

---

### Phase 7 — Compositor Rewrite (Weeks 78–130+)
**Goal:** Replace KWin with a native PBSD compositor matching Windows 7 Aero fidelity.

- [ ] Write compositor SPEC.md
- [ ] Implement Wayland compositor in C++23 (Aero glass, blur, transparency, shadow depth)
- [ ] Validate against KWin behaviour as a black-box reference
- [ ] Window 7 taskbar, start menu, notification area in native compositor

**Exit criteria:** KWin replaced by native compositor. Windows 7 desktop experience indistinguishable from KWin-based Phase 1 version.

---

### Phase 8 — Atomic Update & Packaging (Weeks 52+, parallel)
**Goal:** Content-addressed packages, atomic update/rollback.

- [ ] Design native package format (content-addressed, Izaac-hash-backed integrity verification)
- [ ] Implement atomic update mechanism (transactional, rollback on failure)
- [ ] Port PBSD build system to produce installable packages

**Exit criteria:** `ygg update` applies atomically, rolls back on failure.

---

### Phase 9 — PBSD 0.1 Release
**Goal:** First tagged, distributable release.

- [ ] All Phase 0–4 complete and stable
- [ ] Real-hardware boot on at least two target machines
- [ ] Threat model up to date
- [ ] `LICENSE`, `NOTICE`, `PROVENANCE.md` all complete
- [ ] Release build reproducible (same inputs → same bits)

---

## 11. Documentation Checklist

| Document | Status | Priority |
|---|---|---|
| `SPEC.md` template | ⬜ Not started | Phase 0 |
| Glossary | ⬜ Not started | Phase 0 |
| Security Invariants | ⬜ Not started | Phase 0 |
| `PROVENANCE.md` policy | ⬜ Not started | Phase 0 |
| Threat Model (STRIDE) | ⬜ Not started | Phase 0 |
| Coding Standards / Style Guide | ⬜ Not started | Phase 0 |
| ABI/Versioning Policy | ⬜ Not started | Phase 0 |
| Contributor Provenance Declaration | ⬜ Not started | Phase 0 |
| Toolchain/Environment Setup | ⬜ Not started | Phase 0 |
| Module Index (SPEC.md catalogue) | ⬜ Not started | Phase 1 |
| Risk Register | ⬜ Not started | Phase 1 |
| Roadmap/Milestones (this doc) | ✅ This document | — |
| Base Driver Contract SPEC.md | ⬜ Not started | Phase 2 |
| UDA Descriptor Schema SPEC.md | ⬜ Not started | Phase 2 |
| Kernel Contract SPECs (scheduler, allocator, IPC, VM, filesystem) | ⬜ Not started | Phase 5 |
| BIFROST SPEC.md | ⬜ Not started | Phase 6 |
| Compositor SPEC.md | ⬜ Not started | Phase 7 |
| `LICENSE` + `NOTICE` | ⬜ Not started | Phase 9 |

---

## 12. Cursor Workspace Configuration

Create `.cursor/settings.json` at the root of the build tree:

```json
{
  "files.exclude": {
    "**/obj/**": true,
    "**/.git/**": false,
    "**/ports/**": true
  },
  "search.exclude": {
    "ports/**": true,
    "kde/src/qt6/**": true
  },
  "C_Cpp.default.cppStandard": "c++23",
  "C_Cpp.default.compilerPath": "/usr/bin/clang++",
  "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
  "editor.formatOnSave": true
}
```

Recommended Cursor/VS Code extensions for this project:
- clangd (language server — generates `compile_commands.json` from CMake)
- CMake Tools
- GitLens
- Error Lens (inline static analyser output)

---

## 13. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| Full rewrite timeline underestimated | High | High | Phase 0–2 scoped conservatively; BSD base stays functional throughout |
| HardenedBSD 15-STABLE kernel crash (tracked in Feb 2026 report) | Medium | Medium | Follow hardened/15-stable/main branch; upstream fix expected |
| KDE 6.8 Wayland-only (Oct 2026) removes X11 | Certain | Low | Plan for Wayland-native from Phase 1 — already the default path |
| UDA concept doesn't translate to real hardware quirks | Medium | Medium | Prove against QEMU virtio first; bytecode interpreter handles quirks |
| GPU passthrough complexity (BIFROST workstation profile) | High | Medium | Scope separately; desktop profile works without GPU passthrough initially |
| Static analyser Layer 2 harder to build than estimated | Medium | Medium | Layer 1+3 still valuable standalone; L2 can be deferred without blocking other phases |
| Fable/Mythos access restriction continues for Australian nationals | High | Low | Opus 4.7 sufficient for architecture/contract/spec work; Mythos needed only for Phase 4+ code auditing |

---

## 14. Source Repository Map

| Component | Repository | Branch/Tag |
|---|---|---|
| HardenedBSD src | `https://rad.hardenedbsd.org/z2HLHXgL1xevBNQsf8BmQW7MpJmtm.git` | `hardened/15-stable/main` |
| HardenedBSD ports (mirror) | `https://github.com/HardenedBSD/hardenedbsd-ports.git` | `main` |
| KDE Plasma | `https://invent.kde.org/plasma/plasma-desktop.git` | `v6.6.5` |
| KDE Frameworks | `https://invent.kde.org/frameworks/` | `v6.6.0` (per-repo) |
| KWin | `https://invent.kde.org/plasma/kwin.git` | `v6.6.5` |
| Qt 6 | `https://code.qt.io/qt/qt5.git` (supermodule) | `v6.8.x` |

