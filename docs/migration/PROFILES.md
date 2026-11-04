# Kernel / Stand Profile Configuration Notes

**Status:** Wave 7 scaffold  
**Scope:** Freestanding C++23 module profiles for kernel, stand, and arch layers.

## Compiler profile (freestanding)

All kernel-facing modules (`pbsd.kernel.*`, `pbsd.arch.*`, `pbsd.stand`, modules
importing `pbsd.handles`) use:

```
-std=c++23
-fno-exceptions
-fno-rtti
-ffreestanding
-fno-stack-protector
```

Arch-specific flags remain in hbsd Makefiles; C++ modules consume them via
`NORMAL_CXX` once wired in `kmod.mk`.

## Module profiles

| Profile | Modules | Hosted CI | Target |
|---------|---------|-----------|--------|
| `kernel-core` | `pbsd.kernel.*` | Yes (CMake) | `sys/modules/` kmods |
| `uda-freestanding` | `pbsd.uda.*` | Yes | `sys/dev/` via newbus |
| `net-fs` | `pbsd.net`, `pbsd.fs`, `pbsd.geom`, `pbsd.zfs` | Yes | `sys/net*`, `sys/fs/` |
| `stand-efi` | `pbsd.stand` | Yes | `stand/efi/` |
| `arch-stub` | `pbsd.arch.amd64`, `pbsd.arch.arm64` | Yes | `sys/<arch>/` (C++ only; asm in hbsd) |

## amd64 notes

- Page size: 4 KiB (`pbsd.arch.amd64::kPageSize`).
- Stack alignment: 16 bytes at `%rsp` for EFI handoff.
- Assembly entry (`locore`, `swtch`, IST) stays in `hbsd/src/sys/amd64/`.

## arm64 notes

- Page size: 4 KiB (`pbsd.arch.arm64::kPageSize`).
- PAN/UAO feature flags exposed as typed `CpuFeature` enum for later EL1 policy glue.
- Trap vectors and context switch asm stays in `hbsd/src/sys/arm64/`.

## Dual-link

Kernel modules may mix C KPI and C++23 BMIs. C callers use headers under
`pbsd/kernel/shims/` (see `pbsd_kernel_dual_link.h`). No exceptions cross the
boundary — errors are `pbsd::Status` / integer codes per `KERNEL_CXX_ABI.md`.

## CMake vs hbsd build

- **CMake** (`pbsd/CMakeLists.txt`): hosted proof build for CI, SI harness, analyser.
- **hbsd** (`sys/modules/`, `stand/`): authoritative freestanding build once Wave 4–7
  modules are registered in Makefiles.

## Next steps

1. Register `pbsd.kernel.shim` in `sys/modules/pbsd_kernel_shim/Makefile`.
2. Add arch-specific `-mcmodel=kernel` / `-mgeneral-regs-only` via profile fragments.
3. Extend SI harness with VM W^X (SI-3) and zero-alloc (SI-4) checks.
