export module pbsd.port.wave5.hbsd.src.sys.dev.usb.controller.ehci_mv;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/controller/ehci_mv.c
// void ehci_mv_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/controller/ehci_mv.c wave=wave5 loc=384
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::ehci_mv {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::ehci_mv
