export module pbsd.port.wave5.hbsd.src.sys.dev.usb.wlan.if_urtw;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/usb/wlan/if_urtw.c
// void if_urtw_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/usb/wlan/if_urtw.c wave=wave5 loc=4430
export namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::wlan::if_urtw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::usb::wlan::if_urtw
