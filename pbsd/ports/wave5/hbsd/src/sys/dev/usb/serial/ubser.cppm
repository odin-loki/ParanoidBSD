export module pbsd.port.wave5.hbsd.src.sys.dev.usb.serial.ubser;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/serial/ubser.c
// void ubser_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/serial/ubser.c wave=wave5 loc=552
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::ubser {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::ubser
