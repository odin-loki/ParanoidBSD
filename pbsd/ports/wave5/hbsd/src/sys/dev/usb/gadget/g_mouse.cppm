export module pbsd.port.wave5.hbsd.src.sys.dev.usb.gadget.g_mouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/gadget/g_mouse.c
// void g_mouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/gadget/g_mouse.c wave=wave5 loc=454
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::gadget::g_mouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::gadget::g_mouse
