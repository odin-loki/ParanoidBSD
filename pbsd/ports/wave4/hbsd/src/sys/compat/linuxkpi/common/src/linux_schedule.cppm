export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_schedule;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_schedule.c
// void linux_schedule_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_schedule.c wave=wave4 loc=475
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_schedule {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_schedule
