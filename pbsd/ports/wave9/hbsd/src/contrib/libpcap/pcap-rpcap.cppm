export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_rpcap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-rpcap.c
// void pcap-rpcap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-rpcap.c wave=wave9 loc=3631
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_rpcap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_rpcap
