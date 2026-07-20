export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_new;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-new.c
// void pcap-new_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-new.c wave=wave9 loc=534
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_new {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_new
