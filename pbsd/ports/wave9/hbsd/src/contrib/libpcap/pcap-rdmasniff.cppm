export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_rdmasniff;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-rdmasniff.c
// void pcap-rdmasniff_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-rdmasniff.c wave=wave9 loc=453
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_rdmasniff {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_rdmasniff
