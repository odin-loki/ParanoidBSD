export module pbsd.port.wave7.hbsd.src.sys.x86.iommu.iommu_utils;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/iommu/iommu_utils.c
// void iommu_utils_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/iommu/iommu_utils.c wave=wave7 loc=841
export namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::iommu_utils {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::iommu_utils
