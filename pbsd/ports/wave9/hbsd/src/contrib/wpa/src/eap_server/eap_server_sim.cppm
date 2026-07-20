export module pbsd.port.wave9.hbsd.src.contrib.wpa.src.eap_server.eap_server_sim;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/wpa/src/eap_server/eap_server_sim.c
// void eap_server_sim_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/wpa/src/eap_server/eap_server_sim.c wave=wave9 loc=1018
export namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::eap_server::eap_server_sim {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::wpa::src::eap_server::eap_server_sim
