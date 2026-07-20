export module pbsd.port.wave4.hbsd.src.sys.kern.kern_poll;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_poll.c
// void kern_poll_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_poll.c wave=wave4 loc=582
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_poll {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_poll
