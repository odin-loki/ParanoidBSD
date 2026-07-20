export module pbsd.port.wave9.hbsd.src.contrib.libpcap.pcap_bt_monitor_linux;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/pcap-bt-monitor-linux.c
// void pcap-bt-monitor-linux_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/pcap-bt-monitor-linux.c wave=wave9 loc=280
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_bt_monitor_linux {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::pcap_bt_monitor_linux
