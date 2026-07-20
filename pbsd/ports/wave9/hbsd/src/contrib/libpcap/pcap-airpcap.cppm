export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_airpcap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-airpcap.c
// void pcap-airpcap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-airpcap.c wave=wave9 loc=1055
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_airpcap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_airpcap
