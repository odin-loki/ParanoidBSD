export module pbsd.port.wave7.hbsd.src.sys.x86.iommu.amd_ctx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/x86/iommu/amd_ctx.c
// void amd_ctx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/x86/iommu/amd_ctx.c wave=wave7 loc=620
export namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::amd_ctx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::x86::iommu::amd_ctx
