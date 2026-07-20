export module pbsd.port.wave7.hbsd.src.sys.arm64.arm64.uio_machdep;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/arm64/uio_machdep.c
// void uio_machdep_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/arm64/uio_machdep.c wave=wave7 loc=143
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::uio_machdep {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::arm64::uio_machdep
