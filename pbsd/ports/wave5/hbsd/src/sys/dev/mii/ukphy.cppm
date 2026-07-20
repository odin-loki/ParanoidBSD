export module pbsd.port.wave5.hbsd.src.sys.dev.mii.ukphy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mii/ukphy.c
// void ukphy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mii/ukphy.c wave=wave5 loc=156
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::ukphy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::ukphy
