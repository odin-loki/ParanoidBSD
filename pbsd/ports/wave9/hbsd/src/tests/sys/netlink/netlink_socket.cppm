export module pbsd.port.wave9.hbsd.src.tests.sys.netlink.netlink_socket;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tests/sys/netlink/netlink_socket.c
// void netlink_socket_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tests/sys/netlink/netlink_socket.c wave=wave9 loc=341
export namespace pbsd::port::wave9::hbsd::src::tests::sys::netlink::netlink_socket {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tests::sys::netlink::netlink_socket
