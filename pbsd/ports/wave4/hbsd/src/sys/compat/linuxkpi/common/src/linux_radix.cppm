export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_radix;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_radix.c
// void linux_radix_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_radix.c wave=wave4 loc=593
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_radix {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_radix
