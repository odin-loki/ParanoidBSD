# Full C++23 Port — Wave Status

**Program:** ParanoidBSD full C++23 port (HardenedBSD kernel + KDE Plasma 6 + PBSD nucleus)

**Hosted build gate:** `pbsd/_build_wave.sh` → Ninja + Clang 18, full `ctest`, inventory + batch scripts → `WAVE_BUILD_OK`. **Not committed** — it exists only on the author's machine, so CI skips it with a warning and relies on the equivalent inline steps in `build-and-gate`.

## Conversion progress (2026-07-20, Burst 16 green gate)

| Metric | Value | Source |
|--------|------:|--------|
| Inventory rows (all waves) | 28 376 | `docs/migration/c_inventory.csv` |
| Hand-authored `.cppm` (nucleus) | 4 036 | `/tmp/pbsd-b16-src` find |
| Port stubs (`.cppm` under `pbsd/ports/`) | 28 380+ | batch agents |
| Hosted gate (`/tmp/pbsd-b16`) | **GREEN** | `si_harness` + `userland_harness` ALL PASS |

**Burst 16 (2026-07-20):** `/tmp/pbsd-b16` GREEN. Hand **4 036** (+1 380 vs B15). Net/fs/geom/zfs/bifrost/kernel/userland mass Composer 2.5 ports; post-rsync `_b16_autowire.py` + `_fix_cmake_paren.py` race hardening; locale wchar ODR qualifiers; msun `llround`/`rintf`/`cospif` dedupe; libc `gen` re-exports dirname/basename. SoftMmio SI-7 preserved.

**Burst 15 (2026-07-19):** `/tmp/pbsd-b15` GREEN. Hand **2 656** (+672 vs B14). Net tcp/udp_usrreq + ip_input/output; FS mqueuefs + vfs_* surge; geom dump; bifrost intercept; kernel mfence; 41 UDA descriptors + `import pbsd.core`; libthr IU import order; KDE layershell cycle avoided; msun freestanding wrappers. SoftMmio SI-7 preserved. Composer 2.5 Burst 16 running.

**Burst 10 integration (2026-07-19):** WSL `/tmp/pbsd-b10-test` (Clang 18 + Ninja). CMake dedupe via `if(NOT TARGET …)` guards; SoftMmio AHCI/MPT hooks preserved.

### Hand modules by top-level (nucleus)

| Prefix | `.cppm` |
|--------|--------:|
| userland | 876 |
| kde | 380 |
| kernel | 200 |
| uda | 131 |
| net | 92 |
| fs | 43 |
| theme | 46 |
| geom | 35 |
| zfs | 31 |
| stand | 35 |
| arch | 33 |
| compositor | 36 |
| bifrost | 31 |
| pkg | 12 |
| core + handles + rights + lineage | 4 |

### Batch stubs by wave

| Wave | Prefix / scope | Stubs | Inventory rows |
|------|----------------|------:|-----------------:|
| wave2 | `hbsd/src/lib`, `bin`, `sbin`, `usr.bin` | 4 235 | 4 234 |
| wave3 | `kde/` | 1 397 | 1 397 |
| wave4 | `hbsd/src/sys/kern` | 2 134 | 2 134 |
| wave5 | `hbsd/src/sys/dev` | 2 675 | 2 675 |
| wave6 | net/fs/geom/zfs | 1 186 | 1 186 |
| wave7 | stand/arch | 578 | 578 |
| wave8 | bifrost/vmm | 0 | — |
| wave9 | contrib leftovers | 16 172 | 16 172 |

**Partition modules (pass 3, 2026-07-19):** +18 hand ports — net `{altq,in6,gre,epair}`, fs `{tmpfs,pipefs,extattr}`, geom `{stripe,concat}`, stand `{env,devinfo,zfsboot}`, arch `{amd64.exception,arm64.cpufunc}`, bifrost `{ioapic,lapic,vpid}`, compositor `{drm}`. CMake wired (no duplicate targets); SI harness smoke in `test_wave6/7/8`.

**Partition modules (pass 2, 2026-07-19):** +32 hand ports — net `{mbuf,bpf,igmp,bridge,pf}`, fs `{attr,lockf,quota,ufs}`, geom `{part,mirror,label,taste}`, zfs `{dmu,dnode,vdev,arc,dsl}`, stand `{gpt,kmod,elf}`, arch `{amd64.pmap,amd64.apic,arm64.pmap,arm64.gic}`, bifrost `{vmcs,npt}`, compositor `{wayland.output,wayland.buffer}`, pkg `{manifest,repo,signature}`. Generators: `tools/_gen_wave689_partitions.py`, `tools/_gen_wave689_partitions_pass2.py`. SI harness expanded in `test_wave6/7/8/9`.

**Partition modules (burst, 2026-07-19):** hand ports across net/fs/geom/zfs/stand/arch/bifrost/compositor/userland — `pbsd_net_{icmp,ifnet,route,in_pcb,arp,ether}`, TCP FSM + `pbsd_fs` vnode flags, expanded `pbsd_zfs.features`, `pbsd_stand_{efi.protocols,bootinfo,loader,reboot}`, `pbsd_arch_amd64_msr` table, `pbsd_bifrost_nested`, xdg positioner/decoration opcodes in `pbsd.compositor.wayland`; userland burst 8–9 (helpers, compression, network, sysadmin, build/auth tools); SI harness in `test_wave6_net_fs_geom_zfs`, `test_wave7_stand_arch`, `test_wave8_compositor`.

**Batch tooling:** `tools/convert_c_batch.py` (`--skip-stubbed` for incremental batches); `tools/wave_purge_c_check.py --metrics` (JSON rollup incl. `wave_partition_modules` for CI); `tools/mark_converted.py --sync` keeps `CONVERTED.md` ↔ `batch_progress.json` aligned — **not committed**, so that drift check does not currently run anywhere.

**CI:** `.github/workflows/pbsd-ci.yml` — module counts, ledger sync, full `ctest`, batch/inventory summary, L1 clang-tidy, optional L2/L4 analyser, Wave 9 purge report (`--json` artifact); `wave-build-script` job runs `pbsd/_build_wave.sh`.

## Wave 0 — Foundation (complete)

| Item | Status | Artifact |
|------|--------|----------|
| Handle / rights / lineage spec | Done | `docs/specs/HANDLES.md` |
| UDA descriptor spec | Done | `docs/specs/UDA_DESCRIPTOR.md` |
| Analyser spec | Done | `docs/specs/ANALYSER.md` |
| Kernel C++23 ABI spec | Done | `docs/specs/KERNEL_CXX_ABI.md` |
| Module layout spec | Done | `docs/specs/MODULE_LAYOUT.md` |
| Capsicum lineage spec | Done | `docs/specs/CAPSICUM_LINEAGE.md` |
| Threat model (C++23 port) | Done | `docs/security/THREAT_MODEL.md` |
| Provenance (C++23 port) | Done | `docs/PROVENANCE.md` |
| `NORMAL_CXX` / `CXXSTD=c++23` | Done | `hbsd/src/sys/conf/kern.mk`, `kmod.mk` |
| Dual-link hello kmod | Done | `pbsd/kernel/pbsd_cxx_hello.{h,cpp,c}`, `sys/modules/pbsd_cxx_hello/` |
| C inventory tooling | Done | `tools/inventory_c_sources.py`, `docs/migration/c_inventory.csv` |
| Analyser CI stub | Done | `.github/workflows/pbsd-wave0.yml`, `tools/ci/run_wave0_analyser.sh` |
| PBSD CMake + SI harness | Done | `pbsd/CMakeLists.txt`, `pbsd/tests/si_harness.cpp` |

## Wave 1 — PBSD core (complete)

| Item | Status | Artifact |
|------|--------|----------|
| `pbsd.core` / `rights` / `handles` / `lineage` | Done | `pbsd/{core,rights,handles,lineage}/` |
| UDA schema + interp + Engine concept | Done | `pbsd/uda/` |
| Analyser L2/L4 (`Pbsd*`) | Done | `pbsd/analyser/` |
| SI-1 … SI-8 harness | Done | `pbsd/tests/si_harness.cpp` |

## Wave 2 — Userland (scaffold complete)

| Item | Status | Artifact |
|------|--------|----------|
| Capsicum / Casper / Jail / Util modules | Done | `pbsd/userland/` |
| Batch port stubs (lib + bin scale) | In progress | `pbsd/ports/wave2/` (997 stubs) |
| Port order doc | Done | `docs/migration/USERLAND_PORT.md` |
| Full `bin`/`lib` TU conversion | In progress via inventory | `batch_progress.json` |

## Wave 3 — KDE (scaffold complete, pass 4 batch 3)

| Item | Status | Artifact |
|------|--------|----------|
| Aero theme package | Done | `pbsd/theme/plasma/` (v0.7.0, glow/border chrome + 124 SVG assets) |
| KF/KWin hand ports | Done | `pbsd/kde/` (312 `.cppm` in CMake targets) |
| KWin effect factory modules | Done | `pbsd/kde/kwin/effects/` (47 modules) |
| Batch port stubs | In progress | `pbsd/ports/wave3/kde/` (600 cumulative) |
| Migration approach | Done | `docs/migration/KDE_CXX23.md` |
| Full KF/Plasma/KWin TU conversion | In progress via inventory | `kde/` |

## Wave 4 — Kernel core (scaffold complete)

| Item | Status | Artifact |
|------|--------|----------|
| Process / VM / alloc / sched / PaX-MAC | Done | `pbsd/kernel/` |
| Capsicum bridge + dual-link shims | Done | `pbsd/kernel/capsicum_bridge.cppm`, `shims/` |
| `sys/kern` batch stubs | In progress | `pbsd/ports/wave4/` (276 stubs) |

## Wave 5 — UDA + drivers (scaffold complete)

| Item | Status | Artifact |
|------|--------|----------|
| Engines (block/net/display/input/sensor) | Done | `pbsd/uda/engines/` |
| Descriptors (virtio_blk/net, nvme, usbhid) | Done | `pbsd/uda/descriptors/` |
| Typed newbus | Done | `pbsd/uda/pbsd.uda.newbus.cppm` |
| `sys/dev` batch stubs | In progress | `pbsd/ports/wave5/` (487 stubs) |

## Wave 6 — Net / FS / ZFS (scaffold complete)

| Item | Status | Artifact |
|------|--------|----------|
| Cap-checked net/fs façades | Done | `pbsd/net` (+route, in_pcb, arp, ether, mbuf, bpf, igmp, bridge, pf), `pbsd/fs` (+namei, stat, fcntl, dirent, attr, lockf, quota, ufs) |
| GEOM + ZFS stubs | Done | `pbsd/geom` (+disk, slice, flashmap, part, mirror, label, taste), `pbsd/zfs` (+props, zio, spa, dmu, dnode, vdev, arc, dsl) |
| Batch stubs | In progress | `pbsd/ports/wave6/` (297 stubs) |

## Wave 7 — Arch / stand (scaffold complete)

| Item | Status | Artifact |
|------|--------|----------|
| EFI stand façade | Done | `pbsd/stand/` (+loader, reboot, gpt, kmod, elf) |
| amd64 / arm64 C++ stubs (asm stays in hbsd) | Done | `pbsd/arch/` (+cpufunc, exception, amd64.pmap/apic, arm64.pmap/gic) |
| Batch stubs | In progress | `pbsd/ports/wave7/` (114 stubs) |

## Wave 8 — BIFROST + compositor (scaffold complete)

| Item | Status | Artifact |
|------|--------|----------|
| BIFROST + virtio PV + EPT + VMCB | Done | `pbsd/bifrost/` (+nested, ept, vmcb, vmcs, npt) |
| Native compositor + layer/seat/xdg | Done | `pbsd/compositor/` (+wayland.layer, seat, xdg, output, buffer) |
| Linuxulator strategy | Done | `docs/migration/LINUXULATOR.md` |

## Wave 9 — Purge C / 0.1 gate (tooling complete)

| Item | Status | Artifact |
|------|--------|----------|
| Purge gate + exceptions list | Done | `tools/wave_purge_c_check.py` (`--gate`, `--metrics`), `docs/migration/c_purge_exceptions.txt` |
| Contrib batch stubs | In progress | `pbsd/ports/wave9/` (200 stubs) |
| Exit criteria | Done | `docs/migration/PURGE_C.md` |
| Atomic pkg skeleton | Done | `pbsd/pkg/` (+manifest, repo, signature), `test_wave9_pkg()` |
| Analyser L1–L4 CI | Done | `.github/workflows/pbsd-ci.yml` |
| Owned `.c` count → 0 | Gate open | Inventory still reports residual C in `hbsd/`/`kde/` |

## Test status

| Suite | Command | Expected |
|-------|---------|----------|
| SI harness (SI-1 … SI-8) | `ctest -R si_harness` | Pass |
| Userland harness | `ctest -R userland_harness` | Pass |
| Full hosted gate | `bash pbsd/_build_wave.sh` or `/tmp/pbsd-b4` ctest | `WAVE_BUILD_OK` + all ctests pass |

**Last verified (2026-07-19):** `/tmp/pbsd-b4` — 3680/3680 Ninja targets; `ctest` **2/2 pass** (`si_harness`, `userland_harness`; Clang 18, Debug). **WAVE_BUILD_OK**

## Blockers

- **In-tree kmod load** needs a HardenedBSD host (not Windows-only CI).
- **Full-tree C→C++23 conversion** continues file-by-file from `c_inventory.csv`; scaffolds + nucleus are the Wave exit for this sprint; Wave 9 `--gate` stays non-blocking until owned `.c` hits zero.
