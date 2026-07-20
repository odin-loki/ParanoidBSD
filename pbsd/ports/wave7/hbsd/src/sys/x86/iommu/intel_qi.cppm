export module pbsd.port.wave7.hbsd.src.sys.x86.iommu.intel_qi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/iommu/intel_qi.c
// void intel_qi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/iommu/intel_qi.c wave=wave7 loc=425
export namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::intel_qi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::intel_qi
