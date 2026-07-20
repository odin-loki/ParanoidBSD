export module pbsd.port.wave9.hbsd.src.contrib.libpcap.sf_pcap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/sf-pcap.c
// void sf-pcap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/sf-pcap.c wave=wave9 loc=1277
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::sf_pcap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::sf_pcap
