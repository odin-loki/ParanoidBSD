export module pbsd.port.wave9.hbsd.src.contrib.wpa.src.eap_peer.eap_methods;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wpa/src/eap_peer/eap_methods.c
// void eap_methods_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wpa/src/eap_peer/eap_methods.c wave=wave9 loc=378
export namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::eap_peer::eap_methods {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::eap_peer::eap_methods
