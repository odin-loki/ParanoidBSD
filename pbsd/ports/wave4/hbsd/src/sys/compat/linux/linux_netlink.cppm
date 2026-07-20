export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_netlink;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_netlink.c
// void linux_netlink_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_netlink.c wave=wave4 loc=612
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_netlink {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_netlink
