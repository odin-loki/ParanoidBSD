export module pbsd.port.wave6.hbsd.src.sys.netlink.netlink_module;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netlink/netlink_module.c
// void netlink_module_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netlink/netlink_module.c wave=wave6 loc=221
export namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_module {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netlink::netlink_module
