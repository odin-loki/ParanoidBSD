export module pbsd.port.wave5.hbsd.src.sys.dev.iommu.busdma_iommu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/iommu/busdma_iommu.c
// void busdma_iommu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/iommu/busdma_iommu.c wave=wave5 loc=1119
export namespace pbsd::port::wave5::hbsd::src::sys::dev::iommu::busdma_iommu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::iommu::busdma_iommu
