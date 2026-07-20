export module pbsd.port.wave5.hbsd.src.sys.dev.usb.controller.ohci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/controller/ohci.c
// void ohci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/controller/ohci.c wave=wave5 loc=2686
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::ohci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::ohci
