export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_common;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-common.c
// void pcap-common_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-common.c wave=wave9 loc=1663
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_common {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_common
