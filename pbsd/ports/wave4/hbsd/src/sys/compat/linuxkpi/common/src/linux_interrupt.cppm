export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_interrupt;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_interrupt.c
// void linux_interrupt_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_interrupt.c wave=wave4 loc=251
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_interrupt {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_interrupt
