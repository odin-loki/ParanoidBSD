export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_tasklet;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_tasklet.c
// void linux_tasklet_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_tasklet.c wave=wave4 loc=277
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_tasklet {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_tasklet
