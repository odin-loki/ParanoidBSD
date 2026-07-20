export module pbsd.port.wave5.hbsd.src.sys.dev.mii.mii_physubr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/mii/mii_physubr.c
// void mii_physubr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/mii/mii_physubr.c wave=wave5 loc=712
export namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::mii_physubr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::mii::mii_physubr
