export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.io.iommu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/io/iommu.c
// void iommu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/io/iommu.c wave=wave7 loc=363
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::io::iommu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::io::iommu
