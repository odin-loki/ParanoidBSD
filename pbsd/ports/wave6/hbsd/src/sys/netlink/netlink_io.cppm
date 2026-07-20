export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_io;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_io.c
// void netlink_io_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_io.c wave=wave6 loc=372
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_io {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_io
