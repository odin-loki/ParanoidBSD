export module pbsd.port.wave5.hbsd.src.sys.dev.usb.input.wsp;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/input/wsp.c
// void wsp_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/input/wsp.c wave=wave5 loc=1673
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::input::wsp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::input::wsp
