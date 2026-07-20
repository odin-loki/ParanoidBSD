export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap.c
// void pcap_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap.c wave=wave9 loc=4640
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap
