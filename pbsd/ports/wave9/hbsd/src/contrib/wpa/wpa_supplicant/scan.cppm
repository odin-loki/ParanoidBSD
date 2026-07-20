export module pbsd.port.wave9.hbsd.src.contrib.wpa.wpa_supplicant.scan;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wpa/wpa_supplicant/scan.c
// void scan_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wpa/wpa_supplicant/scan.c wave=wave9 loc=3848
export namespace pbsd::port::wave9::hbsd::src::contrib::wpa::wpa_supplicant::scan {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wpa::wpa_supplicant::scan
