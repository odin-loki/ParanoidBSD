export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_fpu;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_fpu.c
// void linux_fpu_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_fpu.c wave=wave4 loc=99
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_fpu {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_fpu
