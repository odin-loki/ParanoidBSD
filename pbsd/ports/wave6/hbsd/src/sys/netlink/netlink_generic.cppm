export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_generic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_generic.c
// void netlink_generic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_generic.c wave=wave6 loc=533
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_generic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_generic
