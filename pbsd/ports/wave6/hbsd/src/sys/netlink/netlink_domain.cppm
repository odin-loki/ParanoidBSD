export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_domain;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_domain.c
// void netlink_domain_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_domain.c wave=wave6 loc=1002
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_domain {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_domain
