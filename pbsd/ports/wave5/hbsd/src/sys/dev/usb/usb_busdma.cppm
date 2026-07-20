export module pbsd.port.wave5.hbsd.src.sys.dev.usb.usb_busdma;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/usb_busdma.c
// void usb_busdma_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/usb_busdma.c wave=wave5 loc=1110
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::usb_busdma {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::usb_busdma
