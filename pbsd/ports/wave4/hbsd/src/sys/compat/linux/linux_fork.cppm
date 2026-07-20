export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_fork;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_fork.c
// void linux_fork_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_fork.c wave=wave4 loc=549
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_fork {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_fork
