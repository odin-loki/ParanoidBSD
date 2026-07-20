export module pbsd.port.wave7.hbsd.src.sys.x86.iommu.intel_utils;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/iommu/intel_utils.c
// void intel_utils_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/iommu/intel_utils.c wave=wave7 loc=551
export namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::intel_utils {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::intel_utils
