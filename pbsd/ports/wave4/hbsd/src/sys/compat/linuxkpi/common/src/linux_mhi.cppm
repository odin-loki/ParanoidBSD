export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_mhi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_mhi.c
// void linux_mhi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_mhi.c wave=wave4 loc=89
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_mhi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_mhi
