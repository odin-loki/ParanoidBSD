export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_common;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_common.c
// void linux_common_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_common.c wave=wave4 loc=84
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_common {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_common
