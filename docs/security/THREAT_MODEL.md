# PBSD Threat Model

**Status:** Living — expand each wave  
**Port context:** ParanoidBSD full C++23 port of HardenedBSD + KDE Plasma 6

This document covers the PBSD security nucleus and the migration from legacy C to C++23 freestanding modules. Kernel C++ TUs inherit HardenedBSD exploit-mitigation assumptions (W^X, KASLR where enabled) plus PBSD capability discipline.

## Assets

- Capability handles and lineage trees (SI-1, SI-2, SI-6)
- UDA MMIO / IRQ grants and descriptor bytecode (SI-7)
- Compositor session FDs and Wayland globals
- BIFROST `VmHandle` guest slots (SI-8)
- Legacy C surfaces during migration (`docs/migration/c_inventory.csv`)

## Priority surfaces

1. Driver / UDA boundary — interpreter must not escape granted MMIO windows (SI-7)
2. Capsicum lineage revoke races — subtree invalidation vs concurrent `grant` (SI-6)
3. Dual-link C/C++23 kmod boundary — no exception or RTTI crossing (Wave 0 `pbsd_cxx_hello`)
4. Screencast / Wayland globals — compositor wave
5. BIFROST guest escape — VM handle lifecycle (SI-8)
6. Residual C code until Wave 9 purge — memory-unsafe patterns in unmigrated TUs

## STRIDE

| Threat | Mitigation |
|--------|------------|
| Spoofing handles | Closed type set + lineage; no raw pointer re-wrap |
| Tampering descriptors | PROVENANCE + immutable `span<const RegInsn>` |
| Repudiation | Audit hooks (Wave 4 kernel bridge) |
| Info disclosure | No raw pointer escape; W^X; `-fno-rtti` in kernel |
| DoS | ISR L4; bounded `CheckEq` / `WaitUs` in UDA bytecode |
| Elevation | SI-1 rights subset; SI-2 no ambient authority |

## C++23 port-specific risks

| Risk | Control |
|------|---------|
| `new`/`delete` in kernel | L1 clang-tidy + `-fno-exceptions` |
| Static init order in kmods | Minimal globals; dual-link hello validates ctor path |
| Hosted analyser false negatives | L2 plugin on handle closed set before Wave 4 kernel merge |
| BMI / module ODR across kmod/kernel | Wave 4 BMI install path; until then header-only C ABI |

## References

- `docs/specs/HANDLES.md`, `docs/specs/UDA_DESCRIPTOR.md`, `docs/specs/CAPSICUM_LINEAGE.md`
- `docs/specs/KERNEL_CXX_ABI.md` — freestanding flags and dual-link rules
- `docs/PROVENANCE.md` — module source policy
