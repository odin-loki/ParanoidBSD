export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_idr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_idr.c
// void linux_idr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_idr.c wave=wave4 loc=820
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_idr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_idr
