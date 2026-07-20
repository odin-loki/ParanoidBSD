export module pbsd.port.wave5.hbsd.src.sys.dev.usb.controller.generic_ohci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/controller/generic_ohci.c
// void generic_ohci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/controller/generic_ohci.c wave=wave5 loc=330
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::generic_ohci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::controller::generic_ohci
