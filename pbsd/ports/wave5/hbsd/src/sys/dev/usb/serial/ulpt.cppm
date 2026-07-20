export module pbsd.port.wave5.hbsd.src.sys.dev.usb.serial.ulpt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/serial/ulpt.c
// void ulpt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/serial/ulpt.c wave=wave5 loc=765
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::ulpt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::serial::ulpt
