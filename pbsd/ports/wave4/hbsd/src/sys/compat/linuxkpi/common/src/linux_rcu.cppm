export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_rcu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_rcu.c
// void linux_rcu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_rcu.c wave=wave4 loc=461
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_rcu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_rcu
