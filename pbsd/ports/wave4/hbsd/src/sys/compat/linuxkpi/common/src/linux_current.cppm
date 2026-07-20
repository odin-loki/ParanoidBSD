export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_current;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_current.c
// void linux_current_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_current.c wave=wave4 loc=337
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_current {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_current
