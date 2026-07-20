export module pbsd.port.wave5.hbsd.src.sys.dev.usb.serial.ubsa;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/serial/ubsa.c
// void ubsa_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/serial/ubsa.c wave=wave5 loc=692
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::ubsa {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::ubsa
