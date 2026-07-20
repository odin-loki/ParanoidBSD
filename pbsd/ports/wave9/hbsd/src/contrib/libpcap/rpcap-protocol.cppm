export module pbsd.port.wave9.hbsd.src.contrib.libpcap.rpcap_protocol;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/rpcap-protocol.c
// void rpcap-protocol_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/rpcap-protocol.c wave=wave9 loc=195
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::rpcap_protocol {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::rpcap_protocol
