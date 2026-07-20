export module pbsd.port.wave7.hbsd.src.stand.usb.usb_busdma_loader;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/usb/usb_busdma_loader.c
// void usb_busdma_loader_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/usb/usb_busdma_loader.c wave=wave7 loc=618
export namespace pbsd::port::wave7::hbsd::src::stand::usb::usb_busdma_loader {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::usb::usb_busdma_loader
