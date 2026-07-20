export module pbsd.port.wave5.hbsd.src.sys.dev.usb.serial.uark;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/serial/uark.c
// void uark_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/serial/uark.c wave=wave5 loc=463
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::uark {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::uark
