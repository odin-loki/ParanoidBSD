export module pbsd.port.wave7.hbsd.src.sys.arm64.apple.apple_aic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/apple/apple_aic.c
// void apple_aic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/apple/apple_aic.c wave=wave7 loc=780
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::apple::apple_aic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::apple::apple_aic
