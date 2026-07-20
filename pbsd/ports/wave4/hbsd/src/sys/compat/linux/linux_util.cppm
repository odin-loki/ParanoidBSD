export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_util;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_util.c
// void linux_util_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_util.c wave=wave4 loc=341
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_util {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_util
