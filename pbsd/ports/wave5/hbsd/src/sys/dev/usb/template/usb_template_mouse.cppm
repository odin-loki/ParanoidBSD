export module pbsd.port.wave5.hbsd.src.sys.dev.usb.template.usb_template_mouse;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/template/usb_template_mouse.c
// void usb_template_mouse_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/template/usb_template_mouse.c wave=wave5 loc=290
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::template::usb_template_mouse {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::template::usb_template_mouse
