# Kernel Dual-Link Shims (Wave 4)

C++23 modules under `pbsd/kernel/` export typed policy. hbsd C KPI calls into
them via `extern "C"` entry points declared in `pbsd/kernel/shims/pbsd_kernel_dual_link.h`.

## Header (C side)

```c
#include "pbsd_kernel_dual_link.h"

pbsd_cap_rights_t bits = pbsd_kernel_encode_rights(1, 0, 1, 0);
if (pbsd_kernel_validate_wx(prot) != PBSD_STATUS_OK) { /* deny W+X */ }
if (pbsd_kernel_cap_check(have0, have1, need0, need1) != PBSD_STATUS_OK) { /* ENOTCAPABLE */ }
unsigned vmprot = pbsd_kernel_cap_rights_to_vmprot(have0, have1);
if (pbsd_kernel_pax_mprotect_enforce(old_prot, new_prot, 1) != PBSD_STATUS_OK) { /* KERN_PROTECTION_FAILURE */ }
```

## Module (C++ side)

`pbsd.kernel.shim` implements the C exports by delegating to:

- `pbsd.kernel.capsicum` — rights algebra (`subr_capability.c`)
- `pbsd.kernel.capsicum_bridge` — rights encode/decode, lineage revoke, cap_enter glue
- `pbsd.kernel.vm` — SI-3 W^X validation, PROT_MAX helpers
- `pbsd.kernel.pax_mac` — PaX MPROTECT / PAGEEXEC enforcement
- `pbsd.kernel.prot` — `p_cansee` / `cr_bsd_visible`

## Wiring in hbsd

Add to kmod Makefile:

```
SRCS+=  pbsd_kernel_dual_link_stub.c
NORMAL_CXX+= pbsd.kernel.shim
```

The stub `.c` file includes only the header; the C++ BMI provides symbols at link time.

See also `docs/specs/KERNEL_CXX_ABI.md`.
