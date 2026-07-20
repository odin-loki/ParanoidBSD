export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_signal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_signal.c
// void linux_signal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_signal.c wave=wave4 loc=981
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_signal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_signal
