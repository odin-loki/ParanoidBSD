export module pbsd.port.wave9.hbsd.src.contrib.wpa.src.eap_server.ikev2;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wpa/src/eap_server/ikev2.c
// void ikev2_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wpa/src/eap_server/ikev2.c wave=wave9 loc=1198
export namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::eap_server::ikev2 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::eap_server::ikev2
