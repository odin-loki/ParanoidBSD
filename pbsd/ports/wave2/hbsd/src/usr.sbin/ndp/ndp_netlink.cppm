export module pbsd.port.wave2.hbsd.src.usr_sbin.ndp.ndp_netlink;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/ndp/ndp_netlink.c
// void ndp_netlink_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/ndp/ndp_netlink.c wave=wave2 loc=524
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::ndp::ndp_netlink {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::ndp::ndp_netlink
