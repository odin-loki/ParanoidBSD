export module pbsd.port.wave5.hbsd.src.sys.dev.mii.nsphy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mii/nsphy.c
// void nsphy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mii/nsphy.c wave=wave5 loc=316
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::nsphy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::nsphy
