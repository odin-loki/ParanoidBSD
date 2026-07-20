export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.vfp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/vfp.c
// void vfp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/vfp.c wave=wave7 loc=1219
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::vfp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::vfp
