# PBSD architecture and implementation plan

Execution: every task runs on DeepSeek Flash through the API, on Odin's PC with no time limit, in RAM, with a live progress runner (`tools/runner`, C++23) (cloud options in the appendices of AGENTS.md). The atomic task graph, coordination, harness, prompts and cost are in [AGENTS.md](AGENTS.md); the task list is [tasks.yaml](tasks.yaml) / [tasks.json](tasks.json). The per-package briefs below are the parent context each atomic task receives.
Status: draft, November 2026. Supersedes the stage plan in `docs/plans/cxx23-port-master-plan.md` for the kernel; that plan's per-file pipeline (stages A–H), handle set, envelopes and userland waves remain in force for userland.
## 1. Direction
- The kernel core is rewritten clean-slate in a verified memory-safe C++23 subset, proved by PRISM as it is written. Old HBSD is not proved first as an oracle.
- Everything converges on one branch and lands in one cutover. Packages run in parallel against frozen interfaces.
- Reuse beats rewrite: Linux and HBSD code is imported directly, adapted into C++, wrapped behind a verified gateway, or used as reference, per package.
- `hbsd/` stays untouched as the build and behaviour reference; HBSD on `main` stays bootable until cutover.
## 2. Existing plan docs merged here
- `docs/plans/cxx23-port-master-plan.md`: stages A–H, envelopes, handle hierarchy, waves 0–9. Kept for userland; kernel waves 4–8 replaced by phases below.
- `docs/plans/agent-port-master-plan.md`, `docs/plans/todo-passes.md`, `docs/migration/RESUME_PLAN.md`: tooling, kept.
- `docs/ROADMAP.md`: open items; sections 5 (C++ port), 6 (toolchain), 7 (CI), 14 (verified boot), 15 (hardening sysctls), 21 (performance) feed packages P0-07, P1-01, P1-09 and the extra features.
- `docs/specs/HANDLES.md`, `CAPSICUM_LINEAGE.md`, `KERNEL_CXX_ABI.md`, `UDA_DESCRIPTOR.md`, `ANALYSER.md`, `MODULE_LAYOUT.md`: inputs to the phase 0 freeze.
- Existing names in the repo (UDA, BIFROST, Yggdrasil envelopes) are referred to by path only; this plan adds no new names.
## 3. Kept, replaced, removed
| Area | Today | Decision |
|---|---|---|
| Loader, EFI, build | `hbsd/src/stand, Makefiles` | Keep loader; new CMake/ninja kernel build |
| VM | `hbsd/src/sys/vm, pbsd/kernel/vm` | Replace (algorithms adapted) |
| Scheduler | `sched_ule.c` | Replace with EEVDF |
| Security | `Capsicum, MAC, hardenedbsd/` | Capsicum/lineage become capability core; PaX features kept as invariants; MAC hooks removed internally |
| Network | `sys/net, netinet, mbuf` | Replace with sk_buff stack; pf kept |
| Storage | `geom, cam` | Replace with blk-mq; GEOM adapter for ZFS |
| Filesystems | `ZFS, UFS, tmpfs` | Keep, wrapped behind VFS gateway |
| Linux compat | `compat/linuxkpi, compat/linux` | Promote to full parity |
| Drivers | `sys/dev` | Linux drivers primary; HBSD drivers fallback |
| Hypervisor | `vmm, pbsd/bifrost` | Keep, rebase |
| Removed | `Giant shims, lockmgr, rmlocks, splay trees, softdep parallel, sched_4bsd` | Delete at cutover |

## 4. Parallel execution model
1. Phase 0 freezes every cross-package header under `pbsd/kernel/include/`. After the freeze, a header change needs a change-request package that bumps the interface version; all dependants rebuild.
2. Each package owns its directories exclusively (see `OWNERSHIP.md`). An agent never edits outside its paths; CI enforces this.
3. Packages code against headers plus mocks from P0-07, so phase 1–4 packages mostly run at once; the dependency list says when real integration is needed.
4. Every package lands on its own branch `wp/<id>` and merges into `pbsd-cutover`, never into `main`.
5. Done means: compiles under the dialect profile, PRISM obligations discharged, acceptance tests green, differential run green.
### Waves that can run simultaneously

- Wave A: P0-01, then P0-02..P0-08 in parallel.
- Wave B (after freeze, against mocks): all P1, P2, P3, P4 packages in parallel; P3-06 fans out per device class.
- Wave C: integration order driven by real dependencies: P1-02→P1-03→P1-07→P2-*→P3-*→P4-*.
- Wave D: P5 packages, then P6-01.
## 5. Verification approach

- Verified core (allocator, VM, capabilities, scheduler, rings, VFS path walk, dispatcher, gateways): PRISM proofs from contracts, required to merge.
- Adapted Linux/HBSD code inside the core: contracts at every function boundary; PRISM proves the boundary; inside bodies, sanitizers and fuzzing.
- Foreign code (Linux modules, NT drivers, translated binaries): IOMMU isolation plus generated runtime checks (P4-07).
- Differential: same workload on stock Linux, HBSD and PBSD; compare syscall traces, outputs, performance (LTP, kyua, xfstests, liburing, blktests, packetdrill, igt, Wine tests, Android CTS).
- Fuzzing: syzkaller with Linux, BSD and NT descriptions from P0-07 onward.
## 6. Phases and work packages

### Phase 0: Interface freeze and infrastructure

#### P0-01 Kernel C++ dialect and contract macros

- Owns: `pbsd/kernel/base/, docs/specs/KERNEL_CXX_ABI.md`
- Depends on: none
- Scope: Freeze the freestanding C++23 subset: no raw owning pointers, -fno-exceptions -fno-rtti, contract attributes [[expects]]/[[ensures]]/[[invariant]] lowered to PRISM annotations plus runtime checks in debug builds, typestate helper templates, affine owner/borrow/shared handle types (extend existing UniqueHandle/BorrowedHandle/SharedHandle).
- Reuse:
  - pbsd/core, pbsd/handles, docs/specs/HANDLES.md — adapt
  - Linux include/linux/compiler*.h — reference
- Acceptance: clang-tidy profile bans raw new/delete/malloc and pointer arithmetic outside pbsd/kernel/base/unsafe/; PRISM parses every contract form; 100% of existing pbsd/core compiles under the profile.
- Agent brief:

```text
You are implementing PBSD work package P0-01 (Kernel C++ dialect and contract macros) in github.com/odin-loki/ParanoidBSD on branch wp/P0-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/base/, docs/specs/KERNEL_CXX_ABI.md.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-01.md.
Scope: Freeze the freestanding C++23 subset: no raw owning pointers, -fno-exceptions -fno-rtti, contract attributes [[expects]]/[[ensures]]/[[invariant]] lowered to PRISM annotations plus runtime checks in debug builds, typestate helper templates, affine owner/borrow/shared handle types (extend existing UniqueHandle/BorrowedHandle/SharedHandle).
Reuse first: pbsd/core, pbsd/handles, docs/specs/HANDLES.md (adapt); Linux include/linux/compiler*.h (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: clang-tidy profile bans raw new/delete/malloc and pointer arithmetic outside pbsd/kernel/base/unsafe/; PRISM parses every contract form; 100% of existing pbsd/core compiles under the profile. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-02 Capability object model header

- Owns: `pbsd/kernel/include/cap/`
- Depends on: P0-01
- Scope: One header set: capability token type, rights mask (only narrows on derive), revocation lineage, object kinds (memory, file, socket, device, IRQ, DMA window, process, personality object). Mapping table from POSIX mode bits, NT security descriptors and Android app UIDs to capability sets.
- Reuse:
  - hbsd/src/sys/kern/sys_capability.c, sys/sys/capsicum.h, docs/specs/CAPSICUM_LINEAGE.md, pbsd/rights, pbsd/lineage — adapt
  - seL4 capability derivation tree — reference
- Acceptance: Header compiles; PRISM proves rights-monotonicity and revocation-completeness lemmas on the reference model.
- Agent brief:

```text
You are implementing PBSD work package P0-02 (Capability object model header) in github.com/odin-loki/ParanoidBSD on branch wp/P0-02.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/include/cap/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-02.md.
Scope: One header set: capability token type, rights mask (only narrows on derive), revocation lineage, object kinds (memory, file, socket, device, IRQ, DMA window, process, personality object). Mapping table from POSIX mode bits, NT security descriptors and Android app UIDs to capability sets.
Reuse first: hbsd/src/sys/kern/sys_capability.c, sys/sys/capsicum.h, docs/specs/CAPSICUM_LINEAGE.md, pbsd/rights, pbsd/lineage (adapt); seL4 capability derivation tree (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Header compiles; PRISM proves rights-monotonicity and revocation-completeness lemmas on the reference model. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-03 Memory and DMA interface headers

- Owns: `pbsd/kernel/include/mm/`
- Depends on: P0-01, P0-02
- Scope: Freeze page allocator, slab, vmalloc, kmap and DMA mapping APIs with Linux-identical C signatures on the outside and capability-typed C++ on the inside.
- Reuse:
  - Linux include/linux/gfp.h, mm.h, vmalloc.h, highmem.h, dma-mapping.h — adapt signatures
  - hbsd/src/sys/compat/linuxkpi/common/include/linux/*.h — direct import of shim signatures
- Acceptance: linuxkpi headers compile against it unchanged; signature diff vs Linux 6.x headers is empty for the listed functions.
- Agent brief:

```text
You are implementing PBSD work package P0-03 (Memory and DMA interface headers) in github.com/odin-loki/ParanoidBSD on branch wp/P0-03.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/include/mm/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-03.md.
Scope: Freeze page allocator, slab, vmalloc, kmap and DMA mapping APIs with Linux-identical C signatures on the outside and capability-typed C++ on the inside.
Reuse first: Linux include/linux/gfp.h, mm.h, vmalloc.h, highmem.h, dma-mapping.h (adapt signatures); hbsd/src/sys/compat/linuxkpi/common/include/linux/*.h (direct import of shim signatures).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: linuxkpi headers compile against it unchanged; signature diff vs Linux 6.x headers is empty for the listed functions. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-04 Device model interface headers

- Owns: `pbsd/kernel/include/device/`
- Depends on: P0-03
- Scope: struct device, struct pci_dev, struct device_driver, probe/remove, sysfs attribute ABI, IRQ and MSI-X request APIs.
- Reuse:
  - Linux include/linux/device.h, pci.h, kobject.h, sysfs.h — adapt
  - hbsd linuxkpi linux/device.h, linux/pci.h — direct import
- Acceptance: drm-kmod and three NIC drivers compile against headers with zero edits.
- Agent brief:

```text
You are implementing PBSD work package P0-04 (Device model interface headers) in github.com/odin-loki/ParanoidBSD on branch wp/P0-04.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/include/device/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-04.md.
Scope: struct device, struct pci_dev, struct device_driver, probe/remove, sysfs attribute ABI, IRQ and MSI-X request APIs.
Reuse first: Linux include/linux/device.h, pci.h, kobject.h, sysfs.h (adapt); hbsd linuxkpi linux/device.h, linux/pci.h (direct import).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: drm-kmod and three NIC drivers compile against headers with zero edits. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-05 Scheduler, I/O ring, block and network interface headers

- Owns: `pbsd/kernel/include/{sched,ring,block,net}/`
- Depends on: P0-03
- Scope: Freeze user ABI of the I/O rings (byte-compatible with Linux io_uring), scheduler entity and sched_attr, blk-mq tag set and request, sk_buff and net_device layouts.
- Reuse:
  - Linux include/uapi/linux/io_uring.h — direct import (ABI)
  - Linux include/linux/blk-mq.h, skbuff.h, netdevice.h — adapt
  - Linux include/uapi/linux/sched/types.h (sched_attr) — direct import
- Acceptance: liburing test suite compiles unchanged against the uapi header.
- Agent brief:

```text
You are implementing PBSD work package P0-05 (Scheduler, I/O ring, block and network interface headers) in github.com/odin-loki/ParanoidBSD on branch wp/P0-05.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/include/{sched,ring,block,net}/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-05.md.
Scope: Freeze user ABI of the I/O rings (byte-compatible with Linux io_uring), scheduler entity and sched_attr, blk-mq tag set and request, sk_buff and net_device layouts.
Reuse first: Linux include/uapi/linux/io_uring.h (direct import (ABI)); Linux include/linux/blk-mq.h, skbuff.h, netdevice.h (adapt); Linux include/uapi/linux/sched/types.h (sched_attr) (direct import).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: liburing test suite compiles unchanged against the uapi header. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-06 Personality dispatch and VFS object interface

- Owns: `pbsd/kernel/include/{personality,vfs}/`
- Depends on: P0-02
- Scope: Per-process personality tag, trap entry contract, syscall table format, VFS node interface carrying capability objects, path namespace mapping (drive letters, / root, Android app data).
- Reuse:
  - hbsd/src/sys/compat/linux/linux_sysvec.c tables, sys/kern/syscalls.master — adapt
  - Wine/ReactOS ntdll syscall tables — reference
  - Linux drivers/android/binder.c uapi — direct import (ABI)
- Acceptance: Generated tables for all three personalities compile; dispatcher contract verified in PRISM.
- Agent brief:

```text
You are implementing PBSD work package P0-06 (Personality dispatch and VFS object interface) in github.com/odin-loki/ParanoidBSD on branch wp/P0-06.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/include/{personality,vfs}/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-06.md.
Scope: Per-process personality tag, trap entry contract, syscall table format, VFS node interface carrying capability objects, path namespace mapping (drive letters, / root, Android app data).
Reuse first: hbsd/src/sys/compat/linux/linux_sysvec.c tables, sys/kern/syscalls.master (adapt); Wine/ReactOS ntdll syscall tables (reference); Linux drivers/android/binder.c uapi (direct import (ABI)).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Generated tables for all three personalities compile; dispatcher contract verified in PRISM. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-07 Build, CI, PRISM pipeline and differential harness

- Owns: `CMake/ninja under pbsd/, tools/prism/, .github/workflows/, tests/diff/`
- Depends on: P0-01
- Scope: One build producing the new kernel image under QEMU (amd64, arm64, riscv64); PRISM runs per-commit on changed TUs; differential runners compare syscall traces against stock Linux and HBSD.
- Reuse:
  - existing tools/, pbsd.py, scripts/, core-gates — adapt
  - Linux tools/testing/selftests, LTP, syzkaller, xfstests, liburing tests, FreeBSD kyua tests — direct import as test oracles
- Acceptance: Empty kernel boots to serial banner on all three arches in CI; PRISM job green on P0 headers.
- Agent brief:

```text
You are implementing PBSD work package P0-07 (Build, CI, PRISM pipeline and differential harness) in github.com/odin-loki/ParanoidBSD on branch wp/P0-07.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: CMake/ninja under pbsd/, tools/prism/, .github/workflows/, tests/diff/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-07.md.
Scope: One build producing the new kernel image under QEMU (amd64, arm64, riscv64); PRISM runs per-commit on changed TUs; differential runners compare syscall traces against stock Linux and HBSD.
Reuse first: existing tools/, pbsd.py, scripts/, core-gates (adapt); Linux tools/testing/selftests, LTP, syzkaller, xfstests, liburing tests, FreeBSD kyua tests (direct import as test oracles).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Empty kernel boots to serial banner on all three arches in CI; PRISM job green on P0 headers. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P0-08 Ownership map and merge rules

- Owns: `docs/plan/OWNERSHIP.md, CODEOWNERS`
- Depends on: P0-01
- Scope: Assign every directory below to exactly one package; headers under pbsd/kernel/include/ become read-only after freeze except via a change request package.
- Reuse:
- Acceptance: CI rejects a PR touching paths outside its package's ownership.
- Agent brief:

```text
You are implementing PBSD work package P0-08 (Ownership map and merge rules) in github.com/odin-loki/ParanoidBSD on branch wp/P0-08.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: docs/plan/OWNERSHIP.md, CODEOWNERS.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P0-08.md.
Scope: Assign every directory below to exactly one package; headers under pbsd/kernel/include/ become read-only after freeze except via a change request package.
Reuse first: none.
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: CI rejects a PR touching paths outside its package's ownership. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

### Phase 1: Verified core

#### P1-01 Boot, early arch and fine-grained KASLR

- Owns: `pbsd/kernel/arch/{amd64,arm64,riscv}/, pbsd/stand/`
- Depends on: P0-07
- Scope: Keep the HBSD loader and EFI path; new C++ early init; per-function section randomisation at boot via relocation table.
- Reuse:
  - hbsd/src/stand/ (loader, EFI), sys/amd64/amd64/locore.S, mpboot.S — direct import of asm; wrap C in C++
  - hbsd/src/sys/hardenedbsd/hbsd_pax_aslr.c — adapt
  - Linux arch/x86/boot/compressed/kaslr.c, FGKASLR patch series — reference
- Acceptance: Boots on QEMU/real hardware; two boots show distinct function layout; ROP gadget offsets differ.
- Agent brief:

```text
You are implementing PBSD work package P1-01 (Boot, early arch and fine-grained KASLR) in github.com/odin-loki/ParanoidBSD on branch wp/P1-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/arch/{amd64,arm64,riscv}/, pbsd/stand/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-01.md.
Scope: Keep the HBSD loader and EFI path; new C++ early init; per-function section randomisation at boot via relocation table.
Reuse first: hbsd/src/stand/ (loader, EFI), sys/amd64/amd64/locore.S, mpboot.S (direct import of asm; wrap C in C++); hbsd/src/sys/hardenedbsd/hbsd_pax_aslr.c (adapt); Linux arch/x86/boot/compressed/kaslr.c, FGKASLR patch series (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Boots on QEMU/real hardware; two boots show distinct function layout; ROP gadget offsets differ. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-02 Physical page allocator and slab

- Owns: `pbsd/kernel/mm/page/, pbsd/kernel/mm/slab/`
- Depends on: P0-03
- Scope: Buddy allocator with per-CPU lists, zero-on-free, typed slab caches; implements alloc_pages, __get_free_pages, kmalloc, kmem_cache_*.
- Reuse:
  - hbsd/src/sys/vm/vm_phys.c, uma_core.c, pbsd/kernel/vm — adapt algorithms into C++
  - Linux mm/page_alloc.c, mm/slub.c — reference for alloc_pages/GFP semantics
- Acceptance: PRISM proves no double allocation and free-list integrity; stress + KASAN-equivalent clean; linuxkpi memory selftests pass.
- Agent brief:

```text
You are implementing PBSD work package P1-02 (Physical page allocator and slab) in github.com/odin-loki/ParanoidBSD on branch wp/P1-02.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/mm/page/, pbsd/kernel/mm/slab/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-02.md.
Scope: Buddy allocator with per-CPU lists, zero-on-free, typed slab caches; implements alloc_pages, __get_free_pages, kmalloc, kmem_cache_*.
Reuse first: hbsd/src/sys/vm/vm_phys.c, uma_core.c, pbsd/kernel/vm (adapt algorithms into C++); Linux mm/page_alloc.c, mm/slub.c (reference for alloc_pages/GFP semantics).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: PRISM proves no double allocation and free-list integrity; stress + KASAN-equivalent clean; linuxkpi memory selftests pass. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-03 Virtual memory, vmalloc, kmap, W^X

- Owns: `pbsd/kernel/mm/vm/`
- Depends on: P1-02
- Scope: Page tables per arch, address-space objects, vmalloc/kmap, enforced W^X on every mapping, RO-after-init.
- Reuse:
  - hbsd/src/sys/vm/vm_map.c, vm_fault.c, vm_object.c, pmap.c (amd64/arm64/riscv) — adapt
  - hbsd_pax_noexec.c (PAGEEXEC/MPROTECT) — adapt
  - Linux mm/vmalloc.c, mm/highmem.c — reference
- Acceptance: PRISM invariant: no PTE both W and X; HBSD paxtest suite passes; mmap fuzzing clean.
- Agent brief:

```text
You are implementing PBSD work package P1-03 (Virtual memory, vmalloc, kmap, W^X) in github.com/odin-loki/ParanoidBSD on branch wp/P1-03.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/mm/vm/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-03.md.
Scope: Page tables per arch, address-space objects, vmalloc/kmap, enforced W^X on every mapping, RO-after-init.
Reuse first: hbsd/src/sys/vm/vm_map.c, vm_fault.c, vm_object.c, pmap.c (amd64/arm64/riscv) (adapt); hbsd_pax_noexec.c (PAGEEXEC/MPROTECT) (adapt); Linux mm/vmalloc.c, mm/highmem.c (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: PRISM invariant: no PTE both W and X; HBSD paxtest suite passes; mmap fuzzing clean. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-04 Capability tokens and in-kernel compartments

- Owns: `pbsd/kernel/cap/, pbsd/kernel/domain/`
- Depends on: P0-02, P1-02
- Scope: Token table, derivation and revocation tree, per-domain memory pools so the network, VFS, IPC and credential domains share no mutable globals; cross-domain calls only through typed gateways.
- Reuse:
  - hbsd/src/sys/kern/sys_capability.c, subr_capability.c, pbsd/rights, pbsd/lineage — adapt
  - hbsd/src/sys/security/mac/ — reference for hook points
- Acceptance: PRISM proves non-forgeability and revocation; fault injection in one domain cannot write another's pages (MPK/PKS or separate page tables).
- Agent brief:

```text
You are implementing PBSD work package P1-04 (Capability tokens and in-kernel compartments) in github.com/odin-loki/ParanoidBSD on branch wp/P1-04.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/cap/, pbsd/kernel/domain/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-04.md.
Scope: Token table, derivation and revocation tree, per-domain memory pools so the network, VFS, IPC and credential domains share no mutable globals; cross-domain calls only through typed gateways.
Reuse first: hbsd/src/sys/kern/sys_capability.c, subr_capability.c, pbsd/rights, pbsd/lineage (adapt); hbsd/src/sys/security/mac/ (reference for hook points).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: PRISM proves non-forgeability and revocation; fault injection in one domain cannot write another's pages (MPK/PKS or separate page tables). Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-05 Scheduler: EEVDF and loadable verified policies

- Owns: `pbsd/kernel/sched/`
- Depends on: P0-05, P1-02
- Scope: EEVDF with lag/deadline, sched_attr latency hints, P/E-core aware topology, policy-extension interface accepting PRISM-verified policies.
- Reuse:
  - Linux kernel/sched/fair.c (EEVDF), kernel/sched/ext.c, pelt.c, topology.c — adapt algorithm into C++
  - hbsd/src/sys/kern/sched_ule.c, kern_switch.c — reference for BSD thread integration
- Acceptance: Differential: hackbench, schbench, cyclictest within 5% of Linux; PRISM proves bounded lag.
- Agent brief:

```text
You are implementing PBSD work package P1-05 (Scheduler: EEVDF and loadable verified policies) in github.com/odin-loki/ParanoidBSD on branch wp/P1-05.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/sched/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-05.md.
Scope: EEVDF with lag/deadline, sched_attr latency hints, P/E-core aware topology, policy-extension interface accepting PRISM-verified policies.
Reuse first: Linux kernel/sched/fair.c (EEVDF), kernel/sched/ext.c, pelt.c, topology.c (adapt algorithm into C++); hbsd/src/sys/kern/sched_ule.c, kern_switch.c (reference for BSD thread integration).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Differential: hackbench, schbench, cyclictest within 5% of Linux; PRISM proves bounded lag. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-06 Synchronisation, epoch, per-CPU

- Owns: `pbsd/kernel/sync/`
- Depends on: P0-01
- Scope: Spinlocks, mutexes, RCU/epoch, completions, workqueues with Linux semantics for driver use.
- Reuse:
  - pbsd/kernel/sync (existing), cxx23 plan envelopes A/B/C — keep
  - Linux kernel/locking/, kernel/rcu/tree.c, include/linux/workqueue.h + kernel/workqueue.c — adapt (spinlock, mutex, RCU, workqueue, completion)
- Acceptance: TLA+/PRISM model of epoch reclamation; lockdep-equivalent clean on full test run.
- Agent brief:

```text
You are implementing PBSD work package P1-06 (Synchronisation, epoch, per-CPU) in github.com/odin-loki/ParanoidBSD on branch wp/P1-06.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/sync/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-06.md.
Scope: Spinlocks, mutexes, RCU/epoch, completions, workqueues with Linux semantics for driver use.
Reuse first: pbsd/kernel/sync (existing), cxx23 plan envelopes A/B/C (keep); Linux kernel/locking/, kernel/rcu/tree.c, include/linux/workqueue.h + kernel/workqueue.c (adapt (spinlock, mutex, RCU, workqueue, completion)).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: TLA+/PRISM model of epoch reclamation; lockdep-equivalent clean on full test run. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-07 Processes, threads, signals, ELF exec

- Owns: `pbsd/kernel/proc/`
- Depends on: P1-03, P1-04, P1-05
- Scope: Process as capability container; clone-style thread creation; ELF loader with PaX flags; signals.
- Reuse:
  - hbsd/src/sys/kern/kern_fork.c, kern_exec.c, kern_sig.c, imgact_elf.c, pbsd/kernel/process, pbsd/kernel/signal — adapt
  - Linux kernel/fork.c (clone3 flags) — reference
- Acceptance: FreeBSD kyua kern tests and LTP process tests pass via respective personalities.
- Agent brief:

```text
You are implementing PBSD work package P1-07 (Processes, threads, signals, ELF exec) in github.com/odin-loki/ParanoidBSD on branch wp/P1-07.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/proc/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-07.md.
Scope: Process as capability container; clone-style thread creation; ELF loader with PaX flags; signals.
Reuse first: hbsd/src/sys/kern/kern_fork.c, kern_exec.c, kern_sig.c, imgact_elf.c, pbsd/kernel/process, pbsd/kernel/signal (adapt); Linux kernel/fork.c (clone3 flags) (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: FreeBSD kyua kern tests and LTP process tests pass via respective personalities. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-08 PSI telemetry and resource control

- Owns: `pbsd/kernel/psi/`
- Depends on: P1-05
- Scope: Pressure stall accounting for CPU, memory, I/O; cgroup-v2-compatible control files.
- Reuse:
  - Linux kernel/sched/psi.c, cgroup v2 memory/io/cpu controllers — adapt
  - hbsd/src/sys/kern/kern_racct.c, kern_rctl.c — reference
- Acceptance: /proc/pressure output matches Linux format; stress tests show expected stall ratios.
- Agent brief:

```text
You are implementing PBSD work package P1-08 (PSI telemetry and resource control) in github.com/odin-loki/ParanoidBSD on branch wp/P1-08.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/psi/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-08.md.
Scope: Pressure stall accounting for CPU, memory, I/O; cgroup-v2-compatible control files.
Reuse first: Linux kernel/sched/psi.c, cgroup v2 memory/io/cpu controllers (adapt); hbsd/src/sys/kern/kern_racct.c, kern_rctl.c (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: /proc/pressure output matches Linux format; stress tests show expected stall ratios. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P1-09 Hardening: CFI, PAC/BTI, shadow stack, stack protector

- Owns: `pbsd/kernel/hardening/, toolchain flags`
- Depends on: P1-01
- Scope: Clang kCFI on every indirect call, PAC+BTI on arm64, IBT+shadow stack on x86, zero-init stack, saturating refcounts.
- Reuse:
  - hbsd/src/sys/hardenedbsd/*, HBSD SafeStack/CFI build glue — adapt
  - Linux arch/arm64 PAC/BTI, x86 IBT/CET shadow stack code — adapt
- Acceptance: Exploit regression suite (lkdtm-equivalent) traps every case.
- Agent brief:

```text
You are implementing PBSD work package P1-09 (Hardening: CFI, PAC/BTI, shadow stack, stack protector) in github.com/odin-loki/ParanoidBSD on branch wp/P1-09.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/hardening/, toolchain flags.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P1-09.md.
Scope: Clang kCFI on every indirect call, PAC+BTI on arm64, IBT+shadow stack on x86, zero-init stack, saturating refcounts.
Reuse first: hbsd/src/sys/hardenedbsd/*, HBSD SafeStack/CFI build glue (adapt); Linux arch/arm64 PAC/BTI, x86 IBT/CET shadow stack code (adapt).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Exploit regression suite (lkdtm-equivalent) traps every case. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

### Phase 2: I/O, storage, network, IPC

#### P2-01 Asynchronous I/O rings

- Owns: `pbsd/kernel/ring/`
- Depends on: P0-05, P1-07, P1-06
- Scope: Shared SQ/CQ rings, SQPOLL, fixed buffers/files as capabilities, zero-copy send/recv, registered rings for NVMe passthrough.
- Reuse:
  - Linux io_uring/ (io_uring.c, sqpoll.c, rsrc.c, net.c, rw.c, poll.c) — adapt into C++ behind capability checks
  - hbsd/src/sys/kern/vfs_aio.c, kern_event.c (kqueue) — reference; kqueue kept as consumer
- Acceptance: liburing test suite passes; fio io_uring engine within 5% of Linux IOPS.
- Agent brief:

```text
You are implementing PBSD work package P2-01 (Asynchronous I/O rings) in github.com/odin-loki/ParanoidBSD on branch wp/P2-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/ring/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P2-01.md.
Scope: Shared SQ/CQ rings, SQPOLL, fixed buffers/files as capabilities, zero-copy send/recv, registered rings for NVMe passthrough.
Reuse first: Linux io_uring/ (io_uring.c, sqpoll.c, rsrc.c, net.c, rw.c, poll.c) (adapt into C++ behind capability checks); hbsd/src/sys/kern/vfs_aio.c, kern_event.c (kqueue) (reference; kqueue kept as consumer).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: liburing test suite passes; fio io_uring engine within 5% of Linux IOPS. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P2-02 Block multiqueue layer

- Owns: `pbsd/kernel/block/`
- Depends on: P0-05, P1-06
- Scope: Per-CPU software queues to hardware queues, tag sets, schedulers, bio; adapter so ZFS/UFS still see a disk device.
- Reuse:
  - Linux block/blk-mq.c, blk-mq-tag.c, blk-core.c, bio.c, mq-deadline.c, kyber — adapt
  - hbsd/src/sys/geom/, sys/cam/ — reference; GEM classes for ZFS compatibility via adapter
- Acceptance: fio random 4k read on NVMe matches Linux; blktests pass.
- Agent brief:

```text
You are implementing PBSD work package P2-02 (Block multiqueue layer) in github.com/odin-loki/ParanoidBSD on branch wp/P2-02.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/block/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P2-02.md.
Scope: Per-CPU software queues to hardware queues, tag sets, schedulers, bio; adapter so ZFS/UFS still see a disk device.
Reuse first: Linux block/blk-mq.c, blk-mq-tag.c, blk-core.c, bio.c, mq-deadline.c, kyber (adapt); hbsd/src/sys/geom/, sys/cam/ (reference; GEM classes for ZFS compatibility via adapter).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: fio random 4k read on NVMe matches Linux; blktests pass. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P2-03 Unified VFS and capability object store

- Owns: `pbsd/kernel/vfs/`
- Depends on: P0-06, P1-04, P2-02
- Scope: Vnode/dentry layer where every open file is a capability; permission translation from POSIX modes, NT security descriptors, Android UIDs to one object model; ZFS, UFS, tmpfs, FUSE, ext4 (from Linux fs/ext4), NTFS (from Linux fs/ntfs3).
- Reuse:
  - hbsd/src/sys/kern/vfs_*.c, pbsd/kernel/vfs — adapt
  - Linux fs/namei.c, dcache.c, inode.c, fs/super.c — reference for dentry semantics
  - hbsd/src/sys/contrib/openzfs — direct import, wrapped
  - hbsd/src/sys/ufs, fs/tmpfs, fs/fuse — direct import, wrapped
- Acceptance: xfstests generic group passes; FreeBSD fs tests pass; PRISM proves path-walk cannot widen rights.
- Agent brief:

```text
You are implementing PBSD work package P2-03 (Unified VFS and capability object store) in github.com/odin-loki/ParanoidBSD on branch wp/P2-03.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/vfs/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P2-03.md.
Scope: Vnode/dentry layer where every open file is a capability; permission translation from POSIX modes, NT security descriptors, Android UIDs to one object model; ZFS, UFS, tmpfs, FUSE, ext4 (from Linux fs/ext4), NTFS (from Linux fs/ntfs3).
Reuse first: hbsd/src/sys/kern/vfs_*.c, pbsd/kernel/vfs (adapt); Linux fs/namei.c, dcache.c, inode.c, fs/super.c (reference for dentry semantics); hbsd/src/sys/contrib/openzfs (direct import, wrapped); hbsd/src/sys/ufs, fs/tmpfs, fs/fuse (direct import, wrapped).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: xfstests generic group passes; FreeBSD fs tests pass; PRISM proves path-walk cannot widen rights. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P2-04 Network stack on native sk_buff

- Owns: `pbsd/kernel/net/`
- Depends on: P0-05, P1-04, P2-01
- Scope: sk_buff as first-class buffer, NAPI, GRO/GSO, socket layer with BSD and Linux APIs, pf firewall kept, network domain isolated.
- Reuse:
  - Linux net/core/skbuff.c, dev.c, net/ipv4, net/ipv6, net/sched (qdisc), XDP (net/core/filter.c) — adapt
  - hbsd/src/sys/netinet, netinet6, netpfil/pf — reference; pf ported as packet filter on sk_buff
  - pbsd/kernel/net, mbuf, ifnet, socket — replace
- Acceptance: packetdrill and LTP net tests; iperf3 line-rate on 100GbE equivalent to Linux; syzkaller net fuzzing clean.
- Agent brief:

```text
You are implementing PBSD work package P2-04 (Network stack on native sk_buff) in github.com/odin-loki/ParanoidBSD on branch wp/P2-04.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/net/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P2-04.md.
Scope: sk_buff as first-class buffer, NAPI, GRO/GSO, socket layer with BSD and Linux APIs, pf firewall kept, network domain isolated.
Reuse first: Linux net/core/skbuff.c, dev.c, net/ipv4, net/ipv6, net/sched (qdisc), XDP (net/core/filter.c) (adapt); hbsd/src/sys/netinet, netinet6, netpfil/pf (reference; pf ported as packet filter on sk_buff); pbsd/kernel/net, mbuf, ifnet, socket (replace).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: packetdrill and LTP net tests; iperf3 line-rate on 100GbE equivalent to Linux; syzkaller net fuzzing clean. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P2-05 IPC: pipes, Unix sockets, Binder

- Owns: `pbsd/kernel/ipc/`
- Depends on: P1-07, P2-03
- Scope: Zero-copy pipes, Unix domain sockets with capability passing, Binder driver and binderfs.
- Reuse:
  - hbsd/src/sys/kern/sys_pipe.c, uipc_usrreq.c — adapt
  - Linux drivers/android/binder.c, binder_alloc.c, binderfs — adapt
- Acceptance: Android binder kernel selftests pass; AOSP servicemanager runs.
- Agent brief:

```text
You are implementing PBSD work package P2-05 (IPC: pipes, Unix sockets, Binder) in github.com/odin-loki/ParanoidBSD on branch wp/P2-05.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/ipc/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P2-05.md.
Scope: Zero-copy pipes, Unix domain sockets with capability passing, Binder driver and binderfs.
Reuse first: hbsd/src/sys/kern/sys_pipe.c, uipc_usrreq.c (adapt); Linux drivers/android/binder.c, binder_alloc.c, binderfs (adapt).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Android binder kernel selftests pass; AOSP servicemanager runs. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

### Phase 3: Linux device model and drivers

#### P3-01 Linux device model, PCI bus, ACPI/DT enumeration

- Owns: `pbsd/kernel/device/, pbsd/kernel/bus/pci/`
- Depends on: P0-04, P1-06
- Scope: kobject/sysfs, bus/driver matching, PCI enumeration, BARs, MSI/MSI-X, ACPI and device tree.
- Reuse:
  - Linux drivers/base/ (core.c, bus.c, driver.c, dd.c), drivers/pci/ (probe.c, pci.c, msi/) — adapt
  - hbsd/src/sys/dev/pci, sys/contrib/dev/acpica — direct import of ACPICA; PCI reference
  - hbsd linuxkpi linux_pci.c — adapt
- Acceptance: lspci/sysfs output matches Linux on same machine; hotplug works under QEMU.
- Agent brief:

```text
You are implementing PBSD work package P3-01 (Linux device model, PCI bus, ACPI/DT enumeration) in github.com/odin-loki/ParanoidBSD on branch wp/P3-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/device/, pbsd/kernel/bus/pci/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P3-01.md.
Scope: kobject/sysfs, bus/driver matching, PCI enumeration, BARs, MSI/MSI-X, ACPI and device tree.
Reuse first: Linux drivers/base/ (core.c, bus.c, driver.c, dd.c), drivers/pci/ (probe.c, pci.c, msi/) (adapt); hbsd/src/sys/dev/pci, sys/contrib/dev/acpica (direct import of ACPICA; PCI reference); hbsd linuxkpi linux_pci.c (adapt).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: lspci/sysfs output matches Linux on same machine; hotplug works under QEMU. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P3-02 IOMMU and unified DMA API

- Owns: `pbsd/kernel/iommu/`
- Depends on: P1-03, P3-01
- Scope: Per-device IOMMU domains, dma_alloc_coherent/dma_map_* backed by capability-typed DMA windows, strict mode default.
- Reuse:
  - hbsd/src/sys/x86/iommu (DMAR), sys/dev/iommu — adapt
  - Linux drivers/iommu/ (intel/, amd/, arm-smmu-v3/), kernel/dma/ — adapt
- Acceptance: PRISM proves a device can only reach its own windows; DMA attack test (PCILeech-style) fails.
- Agent brief:

```text
You are implementing PBSD work package P3-02 (IOMMU and unified DMA API) in github.com/odin-loki/ParanoidBSD on branch wp/P3-02.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/iommu/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P3-02.md.
Scope: Per-device IOMMU domains, dma_alloc_coherent/dma_map_* backed by capability-typed DMA windows, strict mode default.
Reuse first: hbsd/src/sys/x86/iommu (DMAR), sys/dev/iommu (adapt); Linux drivers/iommu/ (intel/, amd/, arm-smmu-v3/), kernel/dma/ (adapt).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: PRISM proves a device can only reach its own windows; DMA attack test (PCILeech-style) fails. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P3-03 Linux driver host, fast path

- Owns: `pbsd/kernel/linuxhost/`
- Depends on: P3-01, P3-02, P1-06
- Scope: Load unmodified Linux .ko-built-from-source modules in-kernel behind IOMMU and typed gateways; module loader with symbol versioning.
- Reuse:
  - hbsd/src/sys/compat/linuxkpi (458 files) — direct import, extended to full parity
  - Linux include/linux/* — import headers as needed
- Acceptance: nvme, ixgbe/ice/mlx5, virtio drivers built from Linux tree pass their selftests.
- Agent brief:

```text
You are implementing PBSD work package P3-03 (Linux driver host, fast path) in github.com/odin-loki/ParanoidBSD on branch wp/P3-03.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/linuxhost/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P3-03.md.
Scope: Load unmodified Linux .ko-built-from-source modules in-kernel behind IOMMU and typed gateways; module loader with symbol versioning.
Reuse first: hbsd/src/sys/compat/linuxkpi (458 files) (direct import, extended to full parity); Linux include/linux/* (import headers as needed).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: nvme, ixgbe/ice/mlx5, virtio drivers built from Linux tree pass their selftests. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P3-04 Linux driver host, sandboxed restartable path

- Owns: `pbsd/kernel/drvsandbox/, pbsd/sbin/driverhost/`
- Depends on: P3-03, P2-05
- Scope: Run USB, Wi-Fi, quirky drivers in a separate domain/process with zero-copy shared rings; supervisor restarts in milliseconds.
- Reuse:
  - Linux UIO/VFIO (drivers/vfio) — adapt
  - pbsd/uda descriptors — keep as descriptor format
  - NetBSD rump kernel — reference
- Acceptance: Kill driver mid-transfer: device recovers in under 100 ms without kernel fault.
- Agent brief:

```text
You are implementing PBSD work package P3-04 (Linux driver host, sandboxed restartable path) in github.com/odin-loki/ParanoidBSD on branch wp/P3-04.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/drvsandbox/, pbsd/sbin/driverhost/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P3-04.md.
Scope: Run USB, Wi-Fi, quirky drivers in a separate domain/process with zero-copy shared rings; supervisor restarts in milliseconds.
Reuse first: Linux UIO/VFIO (drivers/vfio) (adapt); pbsd/uda descriptors (keep as descriptor format); NetBSD rump kernel (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Kill driver mid-transfer: device recovers in under 100 ms without kernel fault. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P3-05 Graphics: DRM/KMS, GEM, TTM

- Owns: `pbsd/kernel/drm/`
- Depends on: P3-03
- Scope: Full DRM core, GEM and TTM over new mm, dma-buf, syncobj; KWin from kde/ runs on it.
- Reuse:
  - drm-kmod (freebsd/drm-kmod repo, amdgpu/i915/radeon) — direct import
  - Linux drivers/gpu/drm/ (core, ttm, scheduler, nouveau, xe) — direct import
- Acceptance: igt-gpu-tools core tests pass; Plasma Wayland session runs on AMD and Intel.
- Agent brief:

```text
You are implementing PBSD work package P3-05 (Graphics: DRM/KMS, GEM, TTM) in github.com/odin-loki/ParanoidBSD on branch wp/P3-05.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/drm/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P3-05.md.
Scope: Full DRM core, GEM and TTM over new mm, dma-buf, syncobj; KWin from kde/ runs on it.
Reuse first: drm-kmod (freebsd/drm-kmod repo, amdgpu/i915/radeon) (direct import); Linux drivers/gpu/drm/ (core, ttm, scheduler, nouveau, xe) (direct import).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: igt-gpu-tools core tests pass; Plasma Wayland session runs on AMD and Intel. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P3-06 Storage, NIC, USB, input, audio, Wi-Fi driver sets

- Owns: `pbsd/kernel/drivers/<class>/ (one package per class)`
- Depends on: P3-03, P3-04
- Scope: Split into parallel sub-packages per class; each picks fast or sandboxed path per the rule (bandwidth-critical fast, else sandboxed).
- Reuse:
  - Linux drivers/nvme, net/ethernet, usb (xhci), input, sound (ALSA), net/wireless + mac80211 — direct import via linux driver host
  - hbsd/src/sys/dev/* (e.g. dev/nvme, dev/virtio, dev/usb) — fallback where Linux driver absent
- Acceptance: Per class: vendor selftests; hardware lab matrix.
- Agent brief:

```text
You are implementing PBSD work package P3-06 (Storage, NIC, USB, input, audio, Wi-Fi driver sets) in github.com/odin-loki/ParanoidBSD on branch wp/P3-06.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/drivers/<class>/ (one package per class).
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P3-06.md.
Scope: Split into parallel sub-packages per class; each picks fast or sandboxed path per the rule (bandwidth-critical fast, else sandboxed).
Reuse first: Linux drivers/nvme, net/ethernet, usb (xhci), input, sound (ALSA), net/wireless + mac80211 (direct import via linux driver host); hbsd/src/sys/dev/* (e.g. dev/nvme, dev/virtio, dev/usb) (fallback where Linux driver absent).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Per class: vendor selftests; hardware lab matrix. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

### Phase 4: Personalities, translation, gateways

#### P4-01 Personality dispatcher and trap entry

- Owns: `pbsd/kernel/personality/`
- Depends on: P1-07, P0-06
- Scope: Per-thread personality tag chosen at exec; dispatcher routes to Linux, BSD, NT or Binder handlers; one shared capability, memory, ring, scheduler core.
- Reuse:
  - hbsd/src/sys/kern/subr_syscall.c, sys/amd64/amd64/trap.c, compat/linux sysvec — adapt
- Acceptance: PRISM proves each trap reaches exactly one handler; syscall latency within 2% of native.
- Agent brief:

```text
You are implementing PBSD work package P4-01 (Personality dispatcher and trap entry) in github.com/odin-loki/ParanoidBSD on branch wp/P4-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/personality/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-01.md.
Scope: Per-thread personality tag chosen at exec; dispatcher routes to Linux, BSD, NT or Binder handlers; one shared capability, memory, ring, scheduler core.
Reuse first: hbsd/src/sys/kern/subr_syscall.c, sys/amd64/amd64/trap.c, compat/linux sysvec (adapt).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: PRISM proves each trap reaches exactly one handler; syscall latency within 2% of native. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P4-02 Linux syscall personality

- Owns: `pbsd/kernel/personality/linux/`
- Depends on: P4-01, P2-01, P2-03, P2-04
- Scope: Full Linux syscall surface incl. epoll, eventfd, signalfd, futex2, namespaces, cgroups, seccomp-bpf, eBPF, /proc and /sys.
- Reuse:
  - hbsd/src/sys/compat/linux (linuxulator) — adapt and complete
  - Linux kernel/*.c, fs/*.c syscall entry semantics — reference
  - gVisor syscall coverage table — reference
- Acceptance: LTP syscalls group ≥98% pass; Docker/Podman containers run.
- Agent brief:

```text
You are implementing PBSD work package P4-02 (Linux syscall personality) in github.com/odin-loki/ParanoidBSD on branch wp/P4-02.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/personality/linux/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-02.md.
Scope: Full Linux syscall surface incl. epoll, eventfd, signalfd, futex2, namespaces, cgroups, seccomp-bpf, eBPF, /proc and /sys.
Reuse first: hbsd/src/sys/compat/linux (linuxulator) (adapt and complete); Linux kernel/*.c, fs/*.c syscall entry semantics (reference); gVisor syscall coverage table (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: LTP syscalls group ≥98% pass; Docker/Podman containers run. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P4-03 BSD syscall personality

- Owns: `pbsd/kernel/personality/bsd/`
- Depends on: P4-01
- Scope: Keep FreeBSD/HBSD ABI so the existing userland and ports keep working.
- Reuse:
  - hbsd/src/sys/kern/syscalls.master and handlers — adapt
  - pbsd/kernel/syscall — replace
- Acceptance: FreeBSD kyua test suite parity with HBSD baseline.
- Agent brief:

```text
You are implementing PBSD work package P4-03 (BSD syscall personality) in github.com/odin-loki/ParanoidBSD on branch wp/P4-03.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/personality/bsd/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-03.md.
Scope: Keep FreeBSD/HBSD ABI so the existing userland and ports keep working.
Reuse first: hbsd/src/sys/kern/syscalls.master and handlers (adapt); pbsd/kernel/syscall (replace).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: FreeBSD kyua test suite parity with HBSD baseline. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P4-04 Windows NT personality

- Owns: `pbsd/kernel/personality/nt/, pbsd/lib/ntdll/`
- Depends on: P4-01, P2-03, P1-04
- Scope: NT syscalls, object manager on capability objects, IRP dispatch for kernel-mode drivers inside IOMMU sandbox, security descriptors mapped to capabilities.
- Reuse:
  - Wine dlls/ntdll, server/ (wineserver objects) — adapt: move wineserver object model into kernel
  - ReactOS ntoskrnl (Io, Ob, Se, Mm) — reference for IRP and object manager
  - pbsd bifrost (VM fallback) — keep
- Acceptance: Wine conformance tests pass; selected Windows apps and a WDM sample driver run.
- Agent brief:

```text
You are implementing PBSD work package P4-04 (Windows NT personality) in github.com/odin-loki/ParanoidBSD on branch wp/P4-04.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/personality/nt/, pbsd/lib/ntdll/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-04.md.
Scope: NT syscalls, object manager on capability objects, IRP dispatch for kernel-mode drivers inside IOMMU sandbox, security descriptors mapped to capabilities.
Reuse first: Wine dlls/ntdll, server/ (wineserver objects) (adapt: move wineserver object model into kernel); ReactOS ntoskrnl (Io, Ob, Se, Mm) (reference for IRP and object manager); pbsd bifrost (VM fallback) (keep).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Wine conformance tests pass; selected Windows apps and a WDM sample driver run. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P4-05 Android personality

- Owns: `pbsd/kernel/personality/android/`
- Depends on: P4-02, P2-05
- Scope: Android app sandbox UIDs mapped to capabilities; runs AOSP system image in a container.
- Reuse:
  - AOSP bionic, ART, servicemanager — direct import userland
  - Linux binder from P2-05, ashmem/memfd — reuse
- Acceptance: CTS subset for core APIs passes.
- Agent brief:

```text
You are implementing PBSD work package P4-05 (Android personality) in github.com/odin-loki/ParanoidBSD on branch wp/P4-05.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/personality/android/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-05.md.
Scope: Android app sandbox UIDs mapped to capabilities; runs AOSP system image in a container.
Reuse first: AOSP bionic, ART, servicemanager (direct import userland); Linux binder from P2-05, ashmem/memfd (reuse).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: CTS subset for core APIs passes. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P4-06 Cross-architecture binary translation

- Owns: `pbsd/kernel/xlate/, pbsd/libexec/xlate/`
- Depends on: P4-01, P1-03
- Scope: In-dispatcher JIT for x86_64/ARM64/RISC-V user binaries; W^X-safe JIT using dual mappings.
- Reuse:
  - FEX-Emu (x86→ARM64) — direct import
  - QEMU tcg / linux-user — direct import
  - Box64 — reference
- Acceptance: Run x86_64 test suites on ARM64 host; JIT never maps W and X simultaneously.
- Agent brief:

```text
You are implementing PBSD work package P4-06 (Cross-architecture binary translation) in github.com/odin-loki/ParanoidBSD on branch wp/P4-06.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/xlate/, pbsd/libexec/xlate/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-06.md.
Scope: In-dispatcher JIT for x86_64/ARM64/RISC-V user binaries; W^X-safe JIT using dual mappings.
Reuse first: FEX-Emu (x86→ARM64) (direct import); QEMU tcg / linux-user (direct import); Box64 (reference).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Run x86_64 test suites on ARM64 host; JIT never maps W and X simultaneously. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P4-07 Runtime assertion bridge around foreign code

- Owns: `pbsd/kernel/gateway/`
- Depends on: P3-03, P4-04
- Scope: PRISM-generated checks at every gateway: bounds, ownership, capability, lock state; KFENCE-style sampled guards inside foreign modules.
- Reuse:
  - Linux KASAN/KFENCE, kernel/kcsan — adapt
  - PRISM boundary spec generator — new
- Acceptance: Injected faults in foreign drivers are caught at the gateway 100% in fuzz runs.
- Agent brief:

```text
You are implementing PBSD work package P4-07 (Runtime assertion bridge around foreign code) in github.com/odin-loki/ParanoidBSD on branch wp/P4-07.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/kernel/gateway/.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P4-07.md.
Scope: PRISM-generated checks at every gateway: bounds, ownership, capability, lock state; KFENCE-style sampled guards inside foreign modules.
Reuse first: Linux KASAN/KFENCE, kernel/kcsan (adapt); PRISM boundary spec generator (new).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Injected faults in foreign drivers are caught at the gateway 100% in fuzz runs. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

### Phase 5: Userland, desktop, hypervisor

#### P5-01 Userland convergence

- Owns: `pbsd/userland, pbsd/lib, pbsd/bin…`
- Depends on: P4-03
- Scope: Finish existing port waves 2 and 9 against the new kernel ABI; rtld-elf hardening.
- Reuse:
  - existing pbsd.py C++23 port pipeline and hbsd/src userland — keep
- Acceptance: Owned-C purge gate green.
- Agent brief:

```text
You are implementing PBSD work package P5-01 (Userland convergence) in github.com/odin-loki/ParanoidBSD on branch wp/P5-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/userland, pbsd/lib, pbsd/bin….
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P5-01.md.
Scope: Finish existing port waves 2 and 9 against the new kernel ABI; rtld-elf hardening.
Reuse first: existing pbsd.py C++23 port pipeline and hbsd/src userland (keep).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Owned-C purge gate green. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P5-02 Desktop on new graphics stack

- Owns: `kde/, pbsd/compositor, pbsd/theme`
- Depends on: P3-05
- Scope: Plasma Wayland on DRM/KMS; GPU acceleration.
- Reuse:
  - kde/ Plasma 6, KWin drm backend — direct import
- Acceptance: Plasma session daily-driver test.
- Agent brief:

```text
You are implementing PBSD work package P5-02 (Desktop on new graphics stack) in github.com/odin-loki/ParanoidBSD on branch wp/P5-02.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: kde/, pbsd/compositor, pbsd/theme.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P5-02.md.
Scope: Plasma Wayland on DRM/KMS; GPU acceleration.
Reuse first: kde/ Plasma 6, KWin drm backend (direct import).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Plasma session daily-driver test. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

#### P5-03 Hypervisor merge

- Owns: `pbsd/bifrost`
- Depends on: P1-03, P3-02
- Scope: VMs as capability objects on new core; Windows guest fallback.
- Reuse:
  - pbsd/bifrost (existing), hbsd/src/sys/amd64/vmm (bhyve) — keep and rebase onto new mm/IOMMU
- Acceptance: Windows guest boots; nested paging tests.
- Agent brief:

```text
You are implementing PBSD work package P5-03 (Hypervisor merge) in github.com/odin-loki/ParanoidBSD on branch wp/P5-03.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: pbsd/bifrost.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P5-03.md.
Scope: VMs as capability objects on new core; Windows guest fallback.
Reuse first: pbsd/bifrost (existing), hbsd/src/sys/amd64/vmm (bhyve) (keep and rebase onto new mm/IOMMU).
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: Windows guest boots; nested paging tests. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

### Phase 6: Convergence and cutover

#### P6-01 Integration and single cutover

- Owns: `branch pbsd-cutover`
- Depends on: all
- Scope: Merge all packages on the cutover branch; full differential and PRISM run; switch main build to the new kernel in one commit; old kernel kept bootable as alternate boot entry for one release.
- Reuse:
- Acceptance: All acceptance suites green on amd64/arm64/riscv64; PRISM proof obligations closed for every verified-core TU.
- Agent brief:

```text
You are implementing PBSD work package P6-01 (Integration and single cutover) in github.com/odin-loki/ParanoidBSD on branch wp/P6-01.
Read docs/plan/PBSD_PLAN.md sections 1, 4, 5 and this package. Edit only: branch pbsd-cutover.
Frozen interfaces in pbsd/kernel/include/ are read-only; if one is wrong, stop and write docs/plan/change-requests/P6-01.md.
Scope: Merge all packages on the cutover branch; full differential and PRISM run; switch main build to the new kernel in one commit; old kernel kept bootable as alternate boot entry for one release.
Reuse first: none.
Write C++23 in the PBSD kernel dialect (no raw owning pointers, contracts on every public function).
Done when: All acceptance suites green on amd64/arm64/riscv64; PRISM proof obligations closed for every verified-core TU. Run PRISM and the package tests in CI before opening the PR into pbsd-cutover. Do not name new components.
```

## 6a. Fault tolerance in PBSD itself

Every work package carries a `fault_tolerance` requirement in `tasks.yaml`, and every port, test, contract and merge task's acceptance gate includes fault-injection tests (allocation failure, I/O error, timeout, crash and restart). The common rules, frozen in P0-01 and P0-07:

- **Errors are values.** Every fallible function returns a result type. No panic on any recoverable path. A contract violation in a release build becomes a recoverable fault in its compartment.
- **Restartable parts.** Compartments, sandboxed drivers, fast-path Linux modules, the NT and Android services and the compositor can each be restarted. Capabilities carry a generation number, so stale handles fail closed after a restart.
- **Transactional state.** Process, VFS metadata and capability-table updates are transactional (journal or copy-on-write).
- **Storage integrity.** Checksums on by default (ZFS), block-layer retry and multipath failover, and read-only remount on corruption instead of panic.
- **Watchdogs.** Soft lockup and stalled-CPU detection, hung workqueue detection, DRM GPU hang reset, I/O timeouts.
- **Graceful degradation.** PSI pressure throttles before anything fails. The display falls back to a simple framebuffer, and a crashed fast-path driver falls back to the sandboxed path.
- **Crash dumps.** To disk or serial. The bootloader falls back to the previous kernel after repeated failed boots.

## 7. Additional features

- Measured and verified boot: Use HBSD's existing verified-exec plus TPM 2.0 measured boot; sign kernel and modules; dm-verity-style root (Linux drivers/md/dm-verity*).
- eBPF with verifier: Import Linux kernel/bpf/ verifier and JIT for tracing, XDP, seccomp; reuse PRISM for extra proof of loaded programs.
- DTrace retained: Keep hbsd/src/cddl DTrace providers alongside eBPF.
- Live patching: Kernel function replacement via ftrace-style trampolines, signed patches.
- Atomic A/B updates: ZFS boot environments plus existing ygg update plan item; rollback on failed boot.
- Confidential computing: AMD SEV-SNP / Intel TDX guest and host support from Linux arch/x86/coco and kvm.
- Memory tagging: ARM MTE for heap in kernel and userland; Intel LAM where available.
- Post-quantum crypto: ML-KEM/ML-DSA in kernel crypto and IPsec/WireGuard (Linux drivers/net/wireguard).
- Container and namespace support: Linux namespaces + cgroup v2 in Linux personality; jails kept for BSD personality, both mapped to capability domains.
- Real-time profile: PREEMPT_RT-style threaded IRQs and EEVDF deadline class (Linux SCHED_DEADLINE).
- Reproducible builds and SBOM: Already in roadmap; make it a cutover gate.
- Crash dump and kernel debugger: Keep HBSD kdump/ddb; add kgdb stub.

## 8. Cutover

1. Freeze `pbsd-cutover`; run full acceptance matrix on three arches and hardware lab.
2. Close all PRISM obligations; publish proof report.
3. Merge to `main` in one commit; old kernel stays as alternate boot entry for one release, then removed with dead code listed in section 3.

## 9. Risks

| Risk | Mitigation |
|---|---|
| Interface freeze wrong | Change-request packages; versioned headers; mocks catch it early |
| Linux driver semantics drift | Track one Linux LTS; import script per release |
| Proof cost on adapted code | Prove boundaries first, bodies over time; sanitizers meanwhile |
| Big-bang integration failure | Continuous integration of every wp branch into pbsd-cutover nightly; old kernel kept bootable |
| NT personality scope | Userland first via Wine object model; kernel-mode drivers last; VM fallback |
| Performance regressions from checks | Checks compiled out where PRISM proved them; benchmarks per package |
| Merge conflicts across many agents | Exclusive ownership, CODEOWNERS, CI path guard |
