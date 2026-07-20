export module pbsd.port.wave5.hbsd.src.sys.dev.usb.controller.ehci_imx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/controller/ehci_imx.c
// void ehci_imx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/controller/ehci_imx.c wave=wave5 loc=506
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::ehci_imx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::ehci_imx
