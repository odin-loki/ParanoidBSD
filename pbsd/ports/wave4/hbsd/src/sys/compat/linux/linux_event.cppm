export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_event;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_event.c
// void linux_event_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_event.c wave=wave4 loc=716
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_event {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_event
