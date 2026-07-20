export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_dos;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-dos.c
// void pcap-dos_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-dos.c wave=wave9 loc=1550
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_dos {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_dos
