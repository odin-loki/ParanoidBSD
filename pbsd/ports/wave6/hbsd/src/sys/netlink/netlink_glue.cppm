export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_glue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_glue.c
// void netlink_glue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_glue.c wave=wave6 loc=292
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_glue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_glue
