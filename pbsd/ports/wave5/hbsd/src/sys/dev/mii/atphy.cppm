export module pbsd.port.wave5.hbsd.src.sys.dev.mii.atphy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mii/atphy.c
// void atphy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mii/atphy.c wave=wave5 loc=365
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::atphy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::atphy
