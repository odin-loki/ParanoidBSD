export module pbsd.port.wave6.hbsd.src.sys.netgraph.bluetooth.socket.ng_btsocket_hci_raw;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netgraph/bluetooth/socket/ng_btsocket_hci_raw.c
// void ng_btsocket_hci_raw_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netgraph/bluetooth/socket/ng_btsocket_hci_raw.c wave=wave6 loc=1679
export namespace pbsd::port::wave6::hbsd::src::sys::netgraph::bluetooth::socket::ng_btsocket_hci_raw {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netgraph::bluetooth::socket::ng_btsocket_hci_raw
