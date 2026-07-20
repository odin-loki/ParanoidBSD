export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.usb_mouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/usb_mouse.c
// void usb_mouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/usb_mouse.c wave=wave2 loc=841
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::usb_mouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::usb_mouse
