export module pbsd.port.wave5.hbsd.src.sys.dev.usb.input.ukbd;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/input/ukbd.c
// void ukbd_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/input/ukbd.c wave=wave5 loc=2335
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::input::ukbd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::input::ukbd
