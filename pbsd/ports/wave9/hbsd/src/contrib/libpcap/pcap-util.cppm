export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_util;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-util.c
// void pcap-util_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-util.c wave=wave9 loc=627
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_util {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_util
