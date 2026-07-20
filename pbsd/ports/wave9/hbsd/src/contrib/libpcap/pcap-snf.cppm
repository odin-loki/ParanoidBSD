export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_snf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-snf.c
// void pcap-snf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-snf.c wave=wave9 loc=614
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_snf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_snf
