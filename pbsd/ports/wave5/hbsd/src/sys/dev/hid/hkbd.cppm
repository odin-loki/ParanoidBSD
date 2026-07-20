export module pbsd.port.wave5.hbsd.src.sys.dev.hid.hkbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/hid/hkbd.c
// void hkbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/hid/hkbd.c wave=wave5 loc=2112
export namespace pbsd::port::wave5::hbsd::src::sys::dev::hid::hkbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::hid::hkbd
