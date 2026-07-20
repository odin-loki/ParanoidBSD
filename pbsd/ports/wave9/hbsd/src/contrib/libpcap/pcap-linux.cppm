export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_linux;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-linux.c
// void pcap-linux_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-linux.c wave=wave9 loc=6199
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_linux {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_linux
