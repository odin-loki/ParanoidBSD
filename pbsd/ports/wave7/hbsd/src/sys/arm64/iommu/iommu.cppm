export module pbsd.port.wave7.hbsd.src.sys.arm64.iommu.iommu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/iommu/iommu.c
// void iommu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/iommu/iommu.c wave=wave7 loc=512
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::iommu::iommu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::iommu::iommu
