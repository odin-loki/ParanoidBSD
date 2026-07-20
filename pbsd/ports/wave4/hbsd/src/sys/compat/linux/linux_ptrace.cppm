export module pbsd.port.wave4.hbsd.src.sys.compat.linux.linux_ptrace;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linux/linux_ptrace.c
// void linux_ptrace_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linux/linux_ptrace.c wave=wave4 loc=554
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_ptrace {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linux::linux_ptrace
