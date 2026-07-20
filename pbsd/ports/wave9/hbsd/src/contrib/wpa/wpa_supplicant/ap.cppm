export module pbsd.port.wave9.hbsd.src.contrib.wpa.wpa_supplicant.ap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wpa/wpa_supplicant/ap.c
// void ap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wpa/wpa_supplicant/ap.c wave=wave9 loc=2184
export namespace pbsd::port::wave9::hbsd::src::contrib::wpa::wpa_supplicant::ap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wpa::wpa_supplicant::ap
