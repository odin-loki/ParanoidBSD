export module pbsd.port.wave5.hbsd.src.sys.dev.usb.usb_error;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/usb_error.c
// void usb_error_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/usb_error.c wave=wave5 loc=95
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::usb_error {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::usb_error
