export module pbsd.port.wave5.hbsd.src.sys.dev.mii.mii;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mii/mii.c
// void mii_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mii/mii.c wave=wave5 loc=670
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::mii {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::mii
