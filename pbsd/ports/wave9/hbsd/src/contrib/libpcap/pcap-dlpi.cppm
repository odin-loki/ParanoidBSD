export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_dlpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-dlpi.c
// void pcap-dlpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-dlpi.c wave=wave9 loc=2039
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_dlpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_dlpi
