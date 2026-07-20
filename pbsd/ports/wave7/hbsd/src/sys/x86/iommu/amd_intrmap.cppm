export module pbsd.port.wave7.hbsd.src.sys.x86.iommu.amd_intrmap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/iommu/amd_intrmap.c
// void amd_intrmap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/iommu/amd_intrmap.c wave=wave7 loc=398
export namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::amd_intrmap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::amd_intrmap
