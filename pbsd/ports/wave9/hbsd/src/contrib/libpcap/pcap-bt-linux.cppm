export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_bt_linux;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-bt-linux.c
// void pcap-bt-linux_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-bt-linux.c wave=wave9 loc=453
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_bt_linux {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_bt_linux
