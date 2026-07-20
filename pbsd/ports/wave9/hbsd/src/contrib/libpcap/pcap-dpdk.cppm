export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_dpdk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-dpdk.c
// void pcap-dpdk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-dpdk.c wave=wave9 loc=1083
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_dpdk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_dpdk
