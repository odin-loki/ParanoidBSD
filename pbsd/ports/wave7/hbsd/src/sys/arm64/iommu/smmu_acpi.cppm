export module pbsd.port.wave7.hbsd.src.sys.arm64.iommu.smmu_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/iommu/smmu_acpi.c
// void smmu_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/iommu/smmu_acpi.c wave=wave7 loc=291
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::iommu::smmu_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::iommu::smmu_acpi
