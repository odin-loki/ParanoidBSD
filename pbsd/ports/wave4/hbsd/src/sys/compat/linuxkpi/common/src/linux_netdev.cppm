export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_netdev;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_netdev.c
// void linux_netdev_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_netdev.c wave=wave4 loc=436
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_netdev {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_netdev
